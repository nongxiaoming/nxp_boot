#include "lpc407x_8x_177x_8x.h"
#include "spiflash_update.h"
#include "drv_spiflash.h"
#include "drv_iap.h"
#include "config.h"
#include "board.h"

/** The origin buffer on RAM */
#define BUFF_SIZE           4096
//#ifdef __IAR_SYSTEMS_ICC__
//#pragma data_alignment=4
//uint8_t buffer[BUFF_SIZE];
//#else
//uint8_t __attribute__ ((aligned (4))) buffer[BUFF_SIZE];
//#endif
#define UPDATE_INFO_ADDR 897*4096
#define UPDATE_DATA_ADDR 898*4096
struct Update_Info
{
 uint32_t magic;
 uint32_t size;
};
static uint8_t info_buf[256];
struct Update_Info *info;
int spiflash_check_update(void)
{
	SPI_FLASH_Init();
	SPI_FLASH_BufferRead(info_buf,UPDATE_INFO_ADDR,BUFF_SIZE);
  info=(struct Update_Info *)info_buf;
	if(info->magic==NOCRP)
{
 return 1;
}else{
 return 0;
}
}
void spiflash_update(void)
{
	uint32_t faddr=USER_END_SECTOR_ADDRESS,data_addr=UPDATE_DATA_ADDR;
	uint32_t len=info->size;
	uint8_t *buffer=(uint8_t*)RAM_START;
	IAP_STATUS_CODE ret;
  __disable_irq();
/* Prepare sector(s) for write operation */
	ret=PrepareSector(USER_START_SECTOR,USER_END_SECTOR);
	if(ret!=CMD_SUCCESS)
	{

	}
/* ²Á³ýAPPÇøÓò */
		ret=EraseSector(USER_START_SECTOR,USER_END_SECTOR);
	if(ret!=CMD_SUCCESS)
	{

	}
	while(len>0){
		SPI_FLASH_BufferRead(buffer,data_addr,BUFF_SIZE);
	ret=CopyRAM2Flash(faddr,(uint32_t)buffer,BUFF_SIZE);
	if(ret!=CMD_SUCCESS)
	{

	}
	faddr+=BUFF_SIZE;
	data_addr+=BUFF_SIZE;
	len-=BUFF_SIZE;
}
	__enable_irq();
}
