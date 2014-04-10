#include "drv_uart.h"
#include "lpc407x_8x_177x_8x.h"
#include "config.h"
#include "board.h"

#if ETHERNET_DEBUG
#define CR     0x0D

/*******************************************************************************************
* 函数名：UART0_Init()
* 参数  ：void
* 返回值：void
* 描述  ：UART0初始化函数，在使用UART0前先调用
*********************************************************************************************/
void UART0_Init(uint32_t baudrate)
{

    uint32_t Fdiv = 0;
    /*
     * 初始化UART0管脚连接
     * P0.2连接到UART0的TXD端
     * P0.3连接到UART0的RXD端
     */
    LPC_IOCON->P0_2 |= 0x01;
    LPC_IOCON->P0_3 |= 0x01;
    /*开启UART0的电源*/
    LPC_SC->PCONP |= 0x01 << 3;

    /*使能访问除数锁寄存器(DLAB=1),
    *只有DLAB为1时写入DLM和DLL的
    *波特率分频值才会有效
    */
    LPC_UART0->LCR |= 0x80;
    /*配置UART0的波特率*/
    Fdiv = (PeripheralClock / 16) / baudrate ;
    LPC_UART0->DLM = Fdiv / 256;
    LPC_UART0->DLL = Fdiv % 256;
    /*禁止访问除数锁寄存器(DLAB=1)，以防错误修改*/
    LPC_UART0->LCR &= ~0x80;
    /*定义数据位为8bit，停止位为1，校验位为NONE*/
    LPC_UART0->LCR |= 0x03;
    /*使能并复位FIFO*/
    LPC_UART0->FCR = 0x07;
}

/*******************************************************************************************
* 函数名：UART0_SendByte()
* 参数  ：uint8_t c  要发送字符
* 返回值：void
* 描述  ：UART0发送一个字符函数
*********************************************************************************************/
void UART0_SendByte(uint8_t c)
{
    while (0 == (LPC_UART0->LSR & (0x01 << 6))); //等待串口空闲
    LPC_UART0->THR = c;                          //将发送字符写入发送保持寄存器
}
/********************************************************************************************************************************
*函数名：fputc()
* 参数：int ch，FILE *f
* 返回值：int
* 功能：重新定义stdio.h中的fputc()函数，使printf()输出到USART1
*********************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
    UART0_SendByte((uint8_t)ch);                             //发送1个字节
    return ch;                                               //返回 ch
}

#endif
