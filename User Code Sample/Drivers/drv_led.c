#include "drv_led.h"


/**
*Ó²¼þÁ¬½Ó
*LED1 <-->P0_22
*/
void led_hw_init(void)
{
 LPC_IOCON->P0_22=0x00;
 LPC_GPIO0->DIR|=0x01<<22;
 LPC_GPIO0->CLR=0x01<<22;
}
