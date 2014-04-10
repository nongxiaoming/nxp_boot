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

#ifndef  _BOARD_INIT_H
#define  _BOARD_INIT_H

// PLL configuration, M = 64, N = 6
// input = 12MHz, Fcco = (2 x M x 12MHz) / N = 256MHz
// divded by 4 for CCLK of 64MHz
#define PLL_MSEL (64 - 1)
#define PLL_NSEL (6 - 1)
#define CCLK_DIV (4 - 1)

// PLL configuration, M = 14, N = 1
// input = 10MHz, Fcco = (2 x M x 10MHz) / N = 280MHz
// divided by 4 for CCLK of 70MHz 
//#define PLL_MSEL (14 - 1)
//#define PLL_NSEL (1 - 1)
//#define CCLK_DIV (4 - 1)

// frequency of CCLK in kHz
#define fcclk_KHz 64000

void board_init(void);

#endif  /* _BOARD_INIT_H */
