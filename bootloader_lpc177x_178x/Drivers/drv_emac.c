#include "lpc407x_8x_177x_8x.h"
#include "drv_emac.h"
#include "config.h"
#include "drv_timer.h"
#include "eth_isp.h"
#include <string.h>

#if ETHERNET_DEBUG
#include "drv_uart.h"
#endif


#define EtherHdrLen     14
#define IpHdrLen        20
#define HdrLen          42          /* Ethernet hdr + IP hdr + UDP hdr  */

#define FrameSize       data_size+HdrLen
#define PACKET_BUF_SIZE 120
#define CR              0x0D
#define LF              0x0A

#define true            1
#define false           0

#define HTONS(n) ((((unsigned short)((n) & 0xff)) << 8) | (((n) & 0xff00) >> 8))

#define FRAMEr   ((struct ethernet_hdr *)&rxbuffer[0])
#define PACKETr  ((struct packet_hdr *)&rxbuffer[14])
#define PAYLOADr    ((struct payload *)&rxbuffer[42])

#define FRAMEt   ((struct ethernet_hdr *)&txbuffer[0])
#define PACKETt  ((struct packet_hdr *)&txbuffer[14])
#define PAYLOADt    ((struct payload *)&txbuffer[42])

const char sync[]   =   "Synchronized";
const char Ok[]     =   "OK";

char synchro;
char rxbuffer[PACKET_BUF_SIZE];
char txbuffer[PACKET_BUF_SIZE];
char MyMAC[6];
char HostMAC[6];
unsigned short DestPort;
unsigned short SrcPort;
unsigned short data_size = 0;

static unsigned short *rxptr;
static unsigned short *txptr;

struct ethernet_hdr
{
    char destination[6];
    char source[6];
    unsigned short protocol;
};

struct packet_hdr
{
    char fill1[2];
    unsigned short iplen;
    char fill2[6];
    unsigned short iphdrchksum;
    char srcipaddr[4];
    char destipaddr[4];
    unsigned short srcport;
    unsigned short destport;
    unsigned short udplen;
    unsigned short udpchksum;
};

struct payload
{
    char data[1472];
};

unsigned long ethernet_timeout;

//*****************************************************//
//  Private functions                                  //
//*****************************************************//
void write_PHY(int PhyReg, int Value)
{
    unsigned int tout;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
    LPC_EMAC->MWTD = Value;

    /* Wait utill operation completed */
    tout = 0;
    for (tout = 0; tout < MII_WR_TOUT; tout++)
    {
        if ((LPC_EMAC->MIND & MIND_BUSY) == 0)
        {
            break;
        }
    }
}

unsigned short read_PHY(unsigned char PhyReg)
{
    unsigned int tout;

    LPC_EMAC->MADR = DP83848C_DEF_ADR | PhyReg;
    LPC_EMAC->MCMD = MCMD_READ;

    /* Wait until operation completed */
    tout = 0;
    for (tout = 0; tout < MII_RD_TOUT; tout++)
    {
        if ((LPC_EMAC->MIND & MIND_BUSY) == 0)
        {
            break;
        }
    }
    LPC_EMAC->MCMD = 0;
    return (LPC_EMAC->MRDD);
}

unsigned short ReadFrame_EMAC(void)
{
    return (*rxptr++);
}

void CopyFromFrame_EMAC(void *Dest, unsigned short Size)
{
    unsigned short *piDest;

    piDest = Dest;
    while (Size > 1)
    {
        *piDest++ = ReadFrame_EMAC();
        Size -= 2;
    }
    if (Size)
    {
        *(unsigned char *)piDest = (char)ReadFrame_EMAC();
    }
}

int same_mac(char *mac1, char *mac2)
{
    int i;

    for (i = 0; i < 6; i++)
    {
        if (mac1[i] != mac2[i])
            return false;
    }
    return true;
}

int filter_pass(void)
{

    if (!synchro)
        return true;

    if (! same_mac(FRAMEr->source, HostMAC))
    {
#if ETHERNET_DEBUG
        printf("Frame from different MAC: Filtered out!\r\n");
#endif
        return false;
    }
    if (PACKETr->destport != DestPort)
    {
#if ETHERNET_DEBUG
        printf("Packet from different Destination Port: Filtered out!\r\n");
#endif
        return false;
    }
    if (PACKETr->srcport != SrcPort)
    {
#if ETHERNET_DEBUG
        printf("Packet to different Source Port: Filtered out!\r\n");
#endif
        return false;
    }
    return true;
}

void WriteFrame_EMAC(unsigned short Data)
{
    *txptr++ = Data;
}

void CopyToFrame_EMAC(void *Source, unsigned int Size)
{
    unsigned short *piSource;

    piSource = Source;
    Size = (Size + 1) & 0xFFFE;
    while (Size > 0)
    {
        WriteFrame_EMAC(*piSource++);
        Size -= 2;
    }
}

int chksum16(void *buf1, short len)
{
    unsigned short *buf = buf1;
    int chksum16, chksum = 0;

    while (len > 0)
    {
        if (len == 1)
            chksum16 = ((*buf) & 0x00FF);
        else
            chksum16 = (*buf);
        chksum = chksum + HTONS(chksum16);
        *buf++;
        len -= 2;
    }
    return (~(chksum + ((chksum & 0xFFFF0000) >> 16)) & 0xFFFF);
}
/* init rx descriptor */
void rx_descr_init(void)
{
    unsigned int i;

    for (i = 0; i < NUM_RX_FRAG; i++)
    {
        RX_DESC_PACKET(i)  = RX_BUF(i);
        RX_DESC_CTRL(i)    = RCTRL_INT | (ETH_FRAG_SIZE - 1);
        RX_STAT_INFO(i)    = 0;
        RX_STAT_HASHCRC(i) = 0;
    }

    /* Set EMAC Receive Descriptor Registers. */
    LPC_EMAC->RxDescriptor    = RX_DESC_BASE;
    LPC_EMAC->RxStatus        = RX_STAT_BASE;
    LPC_EMAC->RxDescriptorNumber = NUM_RX_FRAG - 1;

    /* Rx Descriptors Point to 0 */
    LPC_EMAC->RxConsumeIndex  = 0;
}

/* init tx descriptor */
void tx_descr_init(void)
{
    unsigned int i;

    for (i = 0; i < NUM_TX_FRAG; i++)
    {
        TX_DESC_PACKET(i) = TX_BUF(i);
        TX_DESC_CTRL(i)   = (1ul << 31) | (1ul << 30) | (1ul << 29) | (1ul << 28) | (1ul << 26) | (ETH_FRAG_SIZE - 1);
        TX_STAT_INFO(i)   = 0;
    }

    /* Set EMAC Transmit Descriptor Registers. */
    LPC_EMAC->TxDescriptor    = TX_DESC_BASE;
    LPC_EMAC->TxStatus        = TX_STAT_BASE;
    LPC_EMAC->TxDescriptorNumber = NUM_TX_FRAG - 1;

    /* Tx Descriptors Point to 0 */
    LPC_EMAC->TxProduceIndex  = 0;
}
//*****************************************************//
//  Public functions                                   //
//*****************************************************//
void init_emac(void)
{
    unsigned int regv, tout, id1, id2, i;

    synchro = false;

#if LED_ENABLED
    /* configure activity LED pin as output */
    LPC_IOCON->P0_22 = 0;
    LPC_GPIO0->DIR |= (1UL << PORT0_PIN);
    LPC_GPIO0->SET = (1UL << PORT0_PIN);
#endif

#if ETHERNET_DEBUG
    UART0_Init(115200);
    printf("Ethernet Secondary Bootloader: Target in debug mode\r\n");
#endif

    /* Power Up the EMAC controller. */
    LPC_SC->PCONP |= (1UL << 30);

    /* Enable P1 Ethernet Pins. */
    LPC_IOCON->P1_0  &= ~0x07;    /*  ENET I/O config */
    LPC_IOCON->P1_0  |= 0x01;     /* ENET_TXD0 */
    LPC_IOCON->P1_1  &= ~0x07;
    LPC_IOCON->P1_1  |= 0x01;     /* ENET_TXD1 */
    LPC_IOCON->P1_4  &= ~0x07;
    LPC_IOCON->P1_4  |= 0x01;     /* ENET_TXEN */
    LPC_IOCON->P1_8  &= ~0x07;
    LPC_IOCON->P1_8  |= 0x01;     /* ENET_CRS */
    LPC_IOCON->P1_9  &= ~0x07;
    LPC_IOCON->P1_9  |= 0x01;     /* ENET_RXD0 */
    LPC_IOCON->P1_10 &= ~0x07;
    LPC_IOCON->P1_10 |= 0x01;     /* ENET_RXD1 */
    LPC_IOCON->P1_14 &= ~0x07;
    LPC_IOCON->P1_14 |= 0x01;     /* ENET_RX_ER */
    LPC_IOCON->P1_15 &= ~0x07;
    LPC_IOCON->P1_15 |= 0x01;     /* ENET_REF_CLK */

    LPC_IOCON->P1_16 &= ~0x07;    /* ENET/PHY I/O config */
    LPC_IOCON->P1_16 |= 0x01;     /* ENET_MDC */
    LPC_IOCON->P1_17 &= ~0x07;
    LPC_IOCON->P1_17 |= 0x01;     /* ENET_MDIO */

    /* Reset all EMAC internal modules. */
    LPC_EMAC->MAC1    = MAC1_RES_TX | MAC1_RES_MCS_TX | MAC1_RES_RX | MAC1_RES_MCS_RX |
                        MAC1_SIM_RES | MAC1_SOFT_RES;
    LPC_EMAC->Command = CR_REG_RES | CR_TX_RES | CR_RX_RES;

    /* A short delay after reset. */
    for (tout = 100; tout; tout--);

    /* Initialize MAC control registers. */
    LPC_EMAC->MAC1 = MAC1_PASS_ALL;
    LPC_EMAC->MAC2 = MAC2_CRC_EN | MAC2_PAD_EN;
    LPC_EMAC->MAXF = ETH_MAX_FLEN;
    LPC_EMAC->CLRT = CLRT_DEF;
    LPC_EMAC->IPGR = IPGR_DEF;

    /* Enable Reduced MII interface. */
    LPC_EMAC->Command = CR_RMII | CR_PASS_RUNT_FRM;

    /* Put the PHY chip in reset mode */
    write_PHY(PHY_REG_BMCR, 0x8000);

    /* Wait for hardware reset to end. */
    for (tout = 0; tout < 0x100000; tout++)
    {
        regv = read_PHY(PHY_REG_BMCR);
        if (!(regv & 0x8000))
        {
            /* Reset complete */
            break;
        }
    }

    /* MII Mgmt Configuration register and MII Mgnt hardware Reset       */
    /* host clock divided by 20, no suppress preamble, no scan increment */
    LPC_EMAC->MCFG = MCFG_CLK_DIV20  | MCFG_RES_MII;
    for (i = 0; i < 0x40; i++);
    LPC_EMAC->MCFG &= (~MCFG_RES_MII);    /* Clear the reset */
    LPC_EMAC->MCMD = 0;

    /* Check if this is a DP83848C PHY. */
    id1 = read_PHY(PHY_REG_IDR1);
    id2 = read_PHY(PHY_REG_IDR2);
    if (((id1 << 16) | (id2 & 0xFFF0)) == DP83848C_ID)
    {
        /* Configure the PHY device */

        /* Use autonegotiation about the link speed. */
        write_PHY(PHY_REG_BMCR, PHY_AUTO_NEG);
        /* Wait to complete Auto_Negotiation. */
        for (tout = 0; tout < 0x100000; tout++)
        {
            regv = read_PHY(PHY_REG_BMSR);
            if (regv & 0x0020)
            {
                /* Autonegotiation Complete. */
                break;
            }
        }
    }

    /* Check the link status. */
    for (tout = 0; tout < 0x10000; tout++)
    {
        regv = read_PHY(PHY_REG_STS);
        if (regv & 0x0001)
        {
            /* Link is on. */
            break;
        }
    }

    /* Configure Full/Half Duplex mode. */
    if (regv & 0x0004)
    {
        /* Full duplex is enabled. */
        LPC_EMAC->MAC2    |= MAC2_FULL_DUP;
        LPC_EMAC->Command |= CR_FULL_DUP;
        LPC_EMAC->IPGT     = IPGT_FULL_DUP;
    }
    else
    {
        /* Half duplex mode. */
        LPC_EMAC->IPGT = IPGT_HALF_DUP;
    }

    /* Configure 100MBit/10MBit mode. */
    if (regv & 0x0002)
    {
        /* 10MBit mode. */
        LPC_EMAC->SUPP = 0;
    }
    else
    {
        /* 100MBit mode. */
        LPC_EMAC->SUPP = SUPP_SPEED;
    }


    /* Set the Ethernet MAC Address registers */
    LPC_EMAC->SA0 = (MYMAC_1 << 8) | MYMAC_2;
    LPC_EMAC->SA1 = (MYMAC_3 << 8) | MYMAC_4;
    LPC_EMAC->SA2 = (MYMAC_5 << 8) | MYMAC_6;

    /* save the Ethernet MAC Address to MyMAC[] */
    MyMAC[0] = MYMAC_6;
    MyMAC[1] = MYMAC_5;
    MyMAC[2] = MYMAC_4;
    MyMAC[3] = MYMAC_3;
    MyMAC[4] = MYMAC_2;
    MyMAC[5] = MYMAC_1;

    /* Initialize Tx and Rx DMA Descriptors */
    rx_descr_init();
    tx_descr_init();

    /* Receive Broadcast and Perfect Match Packets */
    LPC_EMAC->RxFilterCtrl = RFC_PERFECT_EN;


    /* Enable receive and transmit mode of MAC Ethernet core */
    LPC_EMAC->Command  |= (CR_RX_EN | CR_TX_EN);
    LPC_EMAC->MAC1     |= MAC1_REC_EN;

    /* Complete some IP & UDP header values */
    txbuffer[12] = 0x08;
    txbuffer[13] = 0x00;
    txbuffer[14] = 0x45;
    txbuffer[15] = 0x00;
    txbuffer[18] = 0xD8;
    txbuffer[19] = 0xF4;
    txbuffer[20] = 0x00;
    txbuffer[21] = 0x00;
    txbuffer[22] = 0x80;
    txbuffer[23] = 0x11;
}

// gets a command from the next packet received
// returns 0 if packet received, 1 if timeout
unsigned emac_getline(char *buf, int max_len, int *count)
{
    unsigned short RxLen;
    unsigned int idx, i;

#if LED_ENABLED
    int k = 0;
#endif

    /* get timestamp for end of timeout period */
    ethernet_timeout = Timer_GetCurrentTick() + ETHERNET_TIMEOUT;

    /* Check for Received frames */
    while (1)
    {
        while (LPC_EMAC->RxConsumeIndex == LPC_EMAC->RxProduceIndex)
        {
#if LED_ENABLED
            // FIXME
            /* increment the counter and check timeout  */
            if (k++ > 200000)
            {
                k = 0;
                /* toogle the LED           */
                if (LPC_GPIO0->PIN & (1 << PORT0_PIN))
                    LPC_GPIO0->CLR |= (1 << PORT0_PIN);
                else
                    LPC_GPIO0->SET |= (1 << PORT0_PIN);
            }
#endif

            /* no filtered packets received during timeout period */
            if (Timer_CheckTimeout(ethernet_timeout))
            {
#if ETHERNET_DEBUG
                printf("Timed out - going back to looking for handshake\r\n");
#endif
                return 1;
            }
        }

        idx = LPC_EMAC->RxConsumeIndex;

        /* check for crc error */
        if (RX_STAT_INFO(idx) & RINFO_CRC_ERR)
        {
#if ETHERNET_DEBUG
            printf("Rx CRC error\r\n");
#endif
            /* ignore frame */
            if (++idx == NUM_RX_FRAG)
                idx = 0;
            LPC_EMAC->RxConsumeIndex = idx;
            continue;
        }

        /* Get the Length and a pointer to the data  */
        RxLen = (RX_STAT_INFO(idx) & RINFO_SIZE) - 3;
        rxptr = (unsigned short *)RX_DESC_PACKET(idx);
        CopyFromFrame_EMAC(rxbuffer, RxLen);
        if (++idx == NUM_RX_FRAG)
            idx = 0;
        LPC_EMAC->RxConsumeIndex = idx;
        if (filter_pass())
            break;
    }
    /* limit incoming data for protection  */
    if ((RxLen - 42) > max_len)
        RxLen = max_len + 42;
    memcpy(buf, PAYLOADr->data, RxLen - 42);

    /* detect '0x0A' (string terminator) and replace it by CR+LF  */
    for (i = 0; i < max_len; i++)
    {
        if (buf[i] == CR)
        {
            buf[i] = '\0';
            break;
        }
    }
    (*count) = i;

#if ETHERNET_DEBUG
    /* print received command   */
    if (RxLen > 60)
    {
        printf("Rx:  ...Data...\r\n");
    }
    else
    {
        printf("Rx: %s\r\n", buf);
    }
    printf("\r\n-------------------\r\n");
#endif

    // reset timestamp for end of timeout period
    ethernet_timeout = Timer_GetCurrentTick() + ETHERNET_TIMEOUT;

    return (0);
}

unsigned emac_sendline_crlf(char *buf)
{
    int i;
    unsigned int idx;

    /* detect '\0' (string terminator) and replace it by CR+LF  */
    for (i = 0; i < CMD_SIZE; i++)
    {
        PAYLOADt->data[i] = buf[i] ;
        if (buf[i] == '\0')
        {
            PAYLOADt->data[i] = CR;
            PAYLOADt->data[i + 1] = LF;
            break;
        }
    }
    data_size = i + 2;

#if ETHERNET_DEBUG
    printf("Tx:  %s\r\n", buf);
    printf("\r\n-------------------\r\n");
#endif

    idx  = LPC_EMAC->TxProduceIndex;
    txptr = (unsigned short *)TX_DESC_PACKET(idx);
    TX_DESC_CTRL(idx) = FrameSize | TCTRL_LAST;
    memcpy(FRAMEt->destination, FRAMEr->source, 6);
    memcpy(FRAMEt->source, MyMAC, 6);
    PACKETt->iplen = HTONS(FrameSize - EtherHdrLen);
    memcpy(PACKETt->destipaddr, PACKETr->srcipaddr, 4);
    memcpy(PACKETt->srcipaddr, PACKETr->destipaddr, 4);
    PACKETt->iphdrchksum = 0;
    PACKETt->iphdrchksum = HTONS(chksum16(PACKETt->fill1, IpHdrLen));
    PACKETt->destport = PACKETr->srcport;
    PACKETt->srcport = PACKETr->destport;
    PACKETt->udplen = HTONS(FrameSize - EtherHdrLen - IpHdrLen);
    PACKETt->udpchksum = 0;
    CopyToFrame_EMAC(&txbuffer[0], FrameSize);
    if (++idx == NUM_TX_FRAG) idx = 0;
    LPC_EMAC->TxProduceIndex = idx;
    return (1);

}

void emac_handshake(void)
{

    int count;
    char buf[15];
    char buf1[15];

    synchro = false;

    while (1)
    {
        if (emac_getline(buf, 1, &count) == 0)
        {
            if (buf[0] == '?')
            {
                /* save Dest & Source Ports   */
                DestPort = PACKETr->destport;
                SrcPort  = PACKETr->srcport;

#if ETHERNET_DEBUG
                /* print Dest & Source Ports   */
                printf("Dest: %d\r\n", DestPort);
                printf("Source: %d\r\n", SrcPort);
#endif
                /* save Host MAC address   */
                memcpy(HostMAC, FRAMEr->source, 6);

#if ETHERNET_DEBUG
                /* print Host MAC address  */
                printf("\r\nSource MAC: %2X-%2X-%2X-%2X-%2X-%2X\r\n", HostMAC[5], HostMAC[4], HostMAC[3], HostMAC[2], HostMAC[1], HostMAC[0]);
#endif

                emac_sendline_crlf((char *)&sync[0]);
                emac_getline(buf, 15, &count);
                memcpy(&buf1[0], &sync[0], sizeof(sync));
                if (strcmp(&buf[0], &buf1[0]) == 0)
                {
                    emac_sendline_crlf((char *)&Ok[0]);
                    synchro = true;
                    break;
                }
            }
        }
    }
}

