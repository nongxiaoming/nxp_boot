#include "drv_spiflash.h"
#include "drv_spi.h"

/******************************************************************************************
*函数名：SPI_FLASH_Init()
* 参数：void
* 返回值：void
* 功能：SPIFLASH初始化函数，外部调用
*********************************************************************************************/
void SPI_FLASH_Init(void)
{
	      LPC_IOCON->P0_16 = 0;
        LPC_GPIO0->DIR|=(0x01<<16);
			  LPC_GPIO0->SET|=(0x01<<16);
        SSP0_Init(5000000);
}
/******************************************************************************************
*函数名：SPI_FLASH_SectorErase()
* 参数：uint32_t SectorAddr   块地址
* 返回值：void
* 功能：SPIFLASH扇区擦除函数，外部调用
*********************************************************************************************/
void SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
  /*发送写数据使能指令*/
  SPI_FLASH_WriteEnable();
	/*等待数据写完，保证写操作是空闲的*/
  SPI_FLASH_WaitForWriteEnd();
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送扇区擦除指令*/
  SSP0_SendByte(W25X_SectorErase);
  /*发送块地址高8位*/
  SSP0_SendByte((SectorAddr & 0xFF0000) >> 16);
  /*发送块地址中8位*/
  SSP0_SendByte((SectorAddr & 0xFF00) >> 8);
  /*发送块地址低8位*/
  SSP0_SendByte(SectorAddr & 0xFF);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 等待写完毕*/
  SPI_FLASH_WaitForWriteEnd();
}

/******************************************************************************************
*函数名：SPI_FLASH_BulkErase()
* 参数：void
* 返回值：void
* 功能：SPIFLASH整片擦除函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BulkErase(void)
{
  /*使能写入*/
  SPI_FLASH_WriteEnable();
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送整片擦除指令*/
  SSP0_SendByte(W25X_ChipErase);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 等待写完成*/
  SPI_FLASH_WaitForWriteEnd();
}

/******************************************************************************************
*函数名：SPI_FLASH_PageWrite()
* 参数：uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite 数据指针，写入地址，写入的个数
* 返回值：void
* 功能：SPIFLASH页写入数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
   /*使能写入*/
  SPI_FLASH_WriteEnable();
  /*使能片选*/
  SPI_FLASH_CS_LOW();
  /* 发送页写入指令*/
  SSP0_SendByte(W25X_PageProgram);
  /*发送高8位数据地址*/
  SSP0_SendByte((WriteAddr & 0xFF0000) >> 16);
  /*发送中8位数据地址*/
  SSP0_SendByte((WriteAddr & 0xFF00) >> 8);
  /*发送低8位数据地址*/
  SSP0_SendByte(WriteAddr & 0xFF);
  /*检测写入的数据是否超出页的容量大小*/
  if(NumByteToWrite > SPI_FLASH_PerWritePageSize)
  {
     NumByteToWrite = SPI_FLASH_PerWritePageSize;
  }
  /*循环写入数据*/
  while (NumByteToWrite--)
  {
    /*发送数据*/
    SSP0_SendByte(*pBuffer);
    /* 指针移到下一个写入数据 */
    pBuffer++;
  }
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  /* 等待写完成*/
  SPI_FLASH_WaitForWriteEnd();
}

/******************************************************************************************
*函数名：SPI_FLASH_BufferWrite()
* 参数：uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite 数据指针，写入地址，写入的个数
* 返回值：void
* 功能：SPIFLASH多个数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
  Addr = WriteAddr % SPI_FLASH_PageSize;                           //计算写入的页的对应初始地址
  count = SPI_FLASH_PageSize - Addr;
  NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;                //计算总共要写的页数
  NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;               //计算剩余单个页写的数据个数
  if (Addr == 0) /* 如果要写入的页地址为0，说明正好整页写数据，没有偏移*/
  {
    if (NumOfPage == 0) /* 如果计算的写入页数为0，说明数据量在一个页的范围内，可直接进行页的写*/
    {
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);      //进行页写数据
    }
    else /* 如果要写的页数大于0*/
    { 
			/*先将开头数据进行整页写入*/
      while (NumOfPage--)
      { 
				//整页写入
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				//地址偏移
        WriteAddr +=  SPI_FLASH_PageSize;
				//数据指针偏移
        pBuffer += SPI_FLASH_PageSize;
      }
       //将剩余数据个数写入
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
    }
  }
  else /*如果写入的地址不在页的开头位置*/
  {
    if (NumOfPage == 0) /*如果写入数据页的个数为0，即数据小于一页容量*/
    {
      if (NumOfSingle > count) /*如果剩余数据大于当前页的剩余容量*/
      {
        temp = NumOfSingle - count;     //计算超出的数据个数
				/*写满当前页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
				/*设置地址偏移*/
        WriteAddr +=  count;
				/*设置数据指针偏移*/
        pBuffer += count;
        /*将剩余量写入新的页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
      }
      else  /*如果剩余数据小于当前页的剩余容量*/
      {
				/*直接写入当前页*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumByteToWrite);
      }
    }
    else /*如果写入数据页的个数大于0，即数据大于一页容量*/
    {
      NumByteToWrite -= count;         //总数据减去当前页剩余的容量
      NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;  //计算要写的整页个数
      NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize; //计算剩余数据个数
      /*将开头数据写入当前页剩余字节个数*/
      SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
			/*设置地址偏移*/
      WriteAddr +=  count;
			/*设置数据指针偏移*/
      pBuffer += count;
       /*开始剩下数据的整页写入*/
      while (NumOfPage--)
      {
				/*写入一个页的字节数*/
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, SPI_FLASH_PageSize);
				/*设置地址偏移*/
        WriteAddr +=  SPI_FLASH_PageSize;
				/*设置指针偏移*/
        pBuffer += SPI_FLASH_PageSize;
      }
      /*如果剩余数据大于0，将剩余的个数写入下一个页*/
      if (NumOfSingle != 0)
      {
        SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
      }
    }
  }
}

/******************************************************************************************
*函数名：SPI_FLASH_BufferRead()
* 参数：uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead 数据指针，读出的地址，读出的个数
* 返回值：void
* 功能：SPIFLASH多个数据函数，外部调用
*********************************************************************************************/
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送读数据指令*/
  SSP0_SendByte(W25X_ReadData);
  /*发送24位数据地址*/
  /* 发送高8位数据地址*/
  SSP0_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*发送中8位数据地址*/
  SSP0_SendByte((ReadAddr& 0xFF00) >> 8);
  /*发送低8位数据地址*/
  SSP0_SendByte(ReadAddr & 0xFF);
  while (NumByteToRead--) /* 循环读取数据*/
  {
    /*读取一个字节数据*/
    *pBuffer = SSP0_SendByte(Dummy_Byte);
    /*数据指针加1*/
    pBuffer++;
  }
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
}

/******************************************************************************************
*函数名：SPI_FLASH_ReadID()
* 参数：void
* 返回值：uint32_t 器件ID
* 功能：SPIFLASH读取ID函数，外部调用
*********************************************************************************************/
uint32_t SPI_FLASH_ReadID(void)
{
  uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

  /* 使能片选 */
  SPI_FLASH_CS_LOW();

  /*发送识别器件ID号*/
  SSP0_SendByte(W25X_JedecDeviceID);
  /* 读取一个字节*/
  Temp0 = SSP0_SendByte(Dummy_Byte);
  /* 读取一个字节*/
  Temp1 = SSP0_SendByte(Dummy_Byte);
   /* 读取一个字节*/
  Temp2 = SSP0_SendByte(Dummy_Byte);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;
  return Temp;
}
/******************************************************************************************
*函数名：SPI_FLASH_ReadDeviceID()
* 参数：void
* 返回值：uint32_t 设备ID
* 功能：SPIFLASH读取设备ID函数，外部调用
*********************************************************************************************/
uint32_t SPI_FLASH_ReadDeviceID(void)
{
  uint32_t Temp = 0;
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送读取ID指令*/
  SSP0_SendByte(W25X_DeviceID);
  SSP0_SendByte(Dummy_Byte);
  SSP0_SendByte(Dummy_Byte);
  SSP0_SendByte(Dummy_Byte);
  /*读取8位数据*/
  Temp = SSP0_SendByte(Dummy_Byte);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
  return Temp;
}
/******************************************************************************************
*函数名：SPI_FLASH_StartReadSequence()
* 参数：uint32_t ReadAddr 24位读地址
* 返回值：void
* 功能：SPIFLASH读开始函数，外部调用
*********************************************************************************************/
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr)
{
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /* 发送读数据指令*/
  SSP0_SendByte(W25X_ReadData);
  /*发送24位数据地址*/
  /* 发送高8位数据地址 */
  SSP0_SendByte((ReadAddr & 0xFF0000) >> 16);
  /*发送中8位数据地址*/
  SSP0_SendByte((ReadAddr& 0xFF00) >> 8);
  /*发送低8位数据地址*/
  SSP0_SendByte(ReadAddr & 0xFF);
}

/******************************************************************************************
*函数名：SPI_FLASH_WriteEnable()
* 参数：void
* 返回值：void
* 功能：SPIFLASH写使能函数，外部调用
*********************************************************************************************/
void SPI_FLASH_WriteEnable(void)
{
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送写使能指令*/
  SSP0_SendByte(W25X_WriteEnable);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
}

/******************************************************************************************
*函数名：SPI_FLASH_WaitForWriteEnd()
* 参数：void
* 返回值：void
* 功能：SPIFLASH等待写完毕函数，外部调用
*********************************************************************************************/
void SPI_FLASH_WaitForWriteEnd(void)
{
  uint8_t FLASH_Status = 0;
   /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送读状态指令 */
  SSP0_SendByte(W25X_ReadStatusReg);
  /*循环发送空数据直到FLASH芯片空闲*/
  do
  {
    /* 发送空字节 */
    FLASH_Status = SSP0_SendByte(Dummy_Byte);
  }
  while ((FLASH_Status & WIP_Flag) != 0); /* 检测是否空闲*/
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
}

/******************************************************************************************
*函数名：SPI_Flash_PowerDown()
* 参数：void
* 返回值：void
* 功能：SPIFLASH进入掉电模式函数，外部调用
*********************************************************************************************/
void SPI_Flash_PowerDown(void)   
{ 
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /*发送掉电指令 */
  SSP0_SendByte(W25X_PowerDown);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();
}   

/******************************************************************************************
*函数名：SPI_Flash_WAKEUP()
* 参数：void
* 返回值：void
* 功能：SPIFLASH唤醒掉电模式函数，外部调用
*********************************************************************************************/
void SPI_Flash_WAKEUP(void)   
{
  /* 使能片选 */
  SPI_FLASH_CS_LOW();
  /* 发送退出掉电模式指令 */
  SSP0_SendByte(W25X_ReleasePowerDown);
  /*失能片选*/
  SPI_FLASH_CS_HIGH();              
}  
