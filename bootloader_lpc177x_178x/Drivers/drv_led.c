#include "drv_led.h"


/**
*Ó²¼þÁ¬½Ó
*LED1 <-->P4_14
*LED2 <-->P4_15
*LED3 <-->P4_16
*LED4 <-->P4_17
*/
void led_hw_init(void)
{
    LPC_IOCON->P4_14 = 0x00;
    LPC_IOCON->P4_15 = 0x00;
    LPC_IOCON->P4_16 = 0x00;
    LPC_IOCON->P4_17 = 0x00;
    LPC_GPIO4->DIR |= 0x0f << 14;
    LPC_GPIO4->SET = 0x0f << 14;
}
