#include "drv_spi.h"

void SSP_SetClock(LPC_SSP_TypeDef *SSPx, uint32_t clk)
{
    uint32_t prescale, div, cmp_clk;

    /* Find closest divider to get at or under the target frequency.
       Use smallest prescale possible and rely on the divider to get
       the closest target frequency */
    div = 0;
    cmp_clk = 0xFFFFFFFF;
    prescale = 2;
    while (cmp_clk > clk)
    {
        cmp_clk = PeripheralClock / ((div + 1) * prescale);
        if (cmp_clk > clk)
        {
            div++;
            if (div > 0xFF)
            {
                div = 0;
                prescale += 2;
            }
        }
    }

    /* Write computed prescaler and divider back to register */
    SSPx->CR0 &= (~(0xff << 8));
    SSPx->CR0 |= (div << 8);
    SSPx->CPSR = prescale;
}

void SSP0_Init(uint32_t clk)
{
    /*
     * Initialize SPI pin connect
     * P2.22 - SCK
     * P2.26 - MISO
     * P2.27 - MOSI
     */
    uint8_t i, temp;
    LPC_IOCON->P0_15 = 0x02;
    LPC_IOCON->P0_17 = 0x02;
    LPC_IOCON->P0_18 = 0x02;
    /*使能SSP0的时钟*/
    LPC_SC->PCONP |= (0x01 << 21);
    /* 设置数据长度为8位，帧格式SPI*/
    LPC_SSP0->CR0 = 07;
    /*设置时钟输出极性*/
    LPC_SSP0->CR0 |= (SSP0_CPOL << 6);
    /*设置时钟输出相位*/
    LPC_SSP0->CR0 |= (SSP0_CPHA << 7);
    /*设置SPI的速率*/
    SSP_SetClock(LPC_SSP0, clk);
#if SSP0_INT
    /*使能SSP0 中断*/
    NVIC_EnableIRQ(SSP0_IRQn);
#endif
    for (i = 0; i < 8; i++)
    {
        temp = LPC_SSP0->DR;         // Clear the RxFIFO
    }
    /*使能SSP0*/
    LPC_SSP0->CR1 = (0x01 << 1);
}
uint8_t SSP0_SendByte(uint8_t byte)
{
    uint8_t rec = 0;
    /*等待空闲，检测FIFO是否已满*/
    while ((LPC_SSP0->SR & ((0x01 << 1) | (0x01 << 4))) != 0x02);
    /*发送数据*/
    LPC_SSP0->DR = byte;
    /*等待空闲及接收数据*/
    while ((LPC_SSP0->SR & ((0x01 << 2) | (0x01 << 4))) != 0x04);
    rec = LPC_SSP0->DR;
    return rec;
}
