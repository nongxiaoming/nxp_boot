//-----------------------------------------------------------------------------
// Software that is described herein is for illustrative purposes only
// which provides customers with programming information regarding the
// products. This software is supplied "AS IS" without any warranties.
// NXP Semiconductors assumes no responsibility or liability for the
// use of the software, conveys no license or title under any patent,
// copyright, or mask work right to the product. NXP Semiconductors
// reserves the right to make changes in the software without
// notification. NXP Semiconductors also make no representation or
// warranty that such application will be suitable for the specified
// use without further testing or modification.
//-----------------------------------------------------------------------------

#include "lpc407x_8x_177x_8x.h"
#include "board.h"
#include "drv_timer.h"


void init_MAM(void)
{
}

void init_PCB(void)
{
}

void VectorsRemap(void)
{
}

void board_init(void)
{
    //init_PLL();
    Timer_Init();
//UART0_Init(115200);
    // wait for PLL to complete initialization after POR
    delay_ms(100);
    init_MAM();
    init_PCB();
    VectorsRemap();
}
