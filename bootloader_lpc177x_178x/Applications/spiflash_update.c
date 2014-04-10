#include "lpc407x_8x_177x_8x.h"
#include "spiflash_update.h"
#include "drv_spiflash.h"
#include "config.h"
#include "board.h"

/** The origin buffer on RAM */
#define BUFF_SIZE           1024
#ifdef __IAR_SYSTEMS_ICC__
#pragma data_alignment=4
uint8_t buffer[BUFF_SIZE];
#else
uint8_t __attribute__ ((aligned (4))) buffer[BUFF_SIZE];
#endif

int spiflash_check_update(void)
{
 return 0;
}
void spiflash_update(void)
{

}
