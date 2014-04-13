#include "lpc407x_8x_177x_8x.h"
#include "drv_led.h"
#include "config.h"
#include "comms.h"
#include "eth_isp.h"
#include "board.h"
#include "drv_uart.h"

const unsigned crp __attribute__((section(".ARM.__at_0x2FC"))) = CRP;

void enter_isp(void)
{
  board_init();
  init_comms();
  
  /* loops when timeout occurs */
  while (1) {
    comm_handshake();
    isp_cmd_loop();
  }
}
int main(void)
{
  if( user_code_present() )
  { 
		LPC_IOCON->P3_0&=~0x07;
		LPC_GPIO3->DIR&=~(0x01<<0);
    if ( crp == CRP3 )
	  {
      // CRP3 is enabled and user flash start sector is not blank, 
		  //execute the user code
		  execute_user_code();
    }
	  else
	  {
			 if(spiflash_check_update()==1)
      {
			 spiflash_update();
			}
	    if ( check_isp_entry_pin() )
		  {
		    // isp entry not requested and CRP3 not enabled
		    execute_user_code();
		  }
		  else
		  {
		    // isp entry requested and CRP3 not enabled
			  enter_isp();
		  }
	  }
  }
  // User code not present, enter isp
  enter_isp();

  return 1;
}
