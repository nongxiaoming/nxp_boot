#ifndef __DRV_LED_H
#define __DRV_LED_H
#include "lpc407x_8x_177x_8x.h"


#define led_on()   (LPC_GPIO0->CLR=0x01<<22)
#define led_off()   (LPC_GPIO0->SET=0x01<<22)

void led_hw_init(void);

#endif
