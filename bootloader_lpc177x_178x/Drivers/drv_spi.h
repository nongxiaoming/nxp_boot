#ifndef __SSPI_H
#define __SSPI_H
#include "lpc407x_8x_177x_8x.h"

#define SSP0_CPOL    0
#define SSP0_CPHA    0
#define SSP0_INT     0

#define SSP1_CPOL    1
#define SSP1_CPHA    1
#define SSP1_INT     0

void SSP_SetClock(LPC_SSP_TypeDef *SSPx, uint32_t clk);
void SSP0_Init(uint32_t clk);
uint8_t SSP0_SendByte(uint8_t byte);
void SSP1_Init(uint32_t clk);
uint8_t SSP1_SendByte(uint8_t byte);
#endif
