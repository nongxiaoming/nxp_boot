#include "lpc407x_8x_177x_8x.h"
#include "drv_led.h"
#include "delay.h"

#define NVIC_VTOR_MASK              0x3FFFFF80
int main(void)
{
	   /* NVIC Configuration */
    /* Set the Vector Table base location at 0x00002000 */
 SCB->VTOR  = 0x00002000;
 systick_hw_init();
 led_hw_init();
	
 while(1)
	 {
    led_on();
	  //delay(5000000);
		 delay_ms(500);
    led_off();
	  //delay(5000000);
		  delay_ms(500);
   }
}
