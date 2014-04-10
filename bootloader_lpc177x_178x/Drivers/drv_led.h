#ifndef __DRV_LED_H
#define __DRV_LED_H
#include "lpc407x_8x_177x_8x.h"

#define LED1 0x00
#define LED2 0x01
#define LED3 0x02
#define LED4 0x03

#define led_on(x)   (LPC_GPIO4->CLR=0x01<<(14+x))
#define led_off(x)   (LPC_GPIO4->SET=0x01<<(14+x))

void led_hw_init(void);

#endif
