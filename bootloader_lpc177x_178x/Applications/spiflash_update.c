#include "lpc407x_8x_177x_8x.h"
#include "spiflash_update.h"
#include "drv_spiflash.h"
#include "drv_iap.h"
#include "config.h"
#include "board.h"

/** The origin buffer on RAM */
#define BUFF_SIZE           4096
#define RBIGU4(p)       ((p)[0]<<24|(p)[1]<<16|(p)[2]<<8|(p)[3])

#define UPDATE_INFO_ADDR 897*4096
#define UPDATE_DATA_ADDR 898*4096
struct Update_Info
{
 int32_t magic;
 uint32_t size;
};
static uint8_t info_buf[256];
struct Update_Info info;
int spiflash_check_update(void)
{

	SPI_FLASH_Init();
	SPI_FLASH_BufferRead(info_buf,UPDATE_INFO_ADDR,256);
  info.size= RBIGU4(info_buf);
	info.magic= RBIGU4(info_buf+4);
//		 printf("size:%d\r\n",info.size);
//	printf("magic:%x\r\n",info.magic);
	if(info.magic==NOCRP)
{
	//printf("magic:%x\r\n",info.magic);
 return 1;
}else{
 return 0;
}
}
void spiflash_update(void)
{
	uint16_t index=0;
	uint32_t faddr=USER_START_SECTOR_ADDRESS,data_addr=UPDATE_DATA_ADDR;
	int32_t len=info.size;
	uint8_t *buffer=(uint8_t*)RAM_START;
	IAP_STATUS_CODE ret;
  __disable_irq();
/* Prepare sector(s) for write operation */
	ret=PrepareSector(USER_START_SECTOR,USER_END_SECTOR);
	if(ret!=CMD_SUCCESS)
	{
  // printf(" Prepare sectors error!\r\n");
	}
/* ²Á³ýAPPÇøÓò */
		ret=EraseSector(USER_START_SECTOR,USER_END_SECTOR);
	if(ret!=CMD_SUCCESS)
	{
  // printf(" Erase sectors error!\r\n");
	}
	
	while(len>0){
		SPI_FLASH_BufferRead(buffer,data_addr,BUFF_SIZE);
	ret=CopyRAM2Flash(faddr,(uint32_t)RAM_START,BUFF_SIZE);
	if(ret!=CMD_SUCCESS)
	{
 //printf("copy to flash error:%d,index:%d!\r\n",faddr,index);
	}
	index++;
	faddr+=BUFF_SIZE;
	data_addr+=BUFF_SIZE;
	len-=BUFF_SIZE;
}
if(ret==CMD_SUCCESS)
	{
	//	printf("write to flash success\r\n");
  SPI_FLASH_SectorErase(UPDATE_INFO_ADDR);
	//	SPI_Flash_PowerDown();
  }
	__enable_irq();
}
