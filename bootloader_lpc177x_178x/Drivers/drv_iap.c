#include "drv_iap.h"


//  IAP Command
typedef void (*IAP)(uint32_t *cmd, uint32_t *result);
IAP iap_entry = (IAP) IAP_LOCATION;
#define IAP_Call    iap_entry

/** @addtogroup IAP_Public_Functions LCD Public Function
 * @ingroup IAP
 * @{
 */


/*********************************************************************//**
 * @brief     Get Sector Number
 *
 * @param[in] adr      Sector Address
 *
 * @return    Sector Number.
 *
 **********************************************************************/
uint32_t GetSecNum(uint32_t adr)
{
    uint32_t n;

    n = adr >> 12;                               //  4kB Sector
    if (n >= 0x10)
    {
        n = 0x0E + (n >> 3);                       // 32kB Sector
    }

    return (n);                                  // Sector Number
}

/*********************************************************************//**
 * @brief     Prepare sector(s) for write operation
 *
 * @param[in] start_sec           The number of start sector
 * @param[in] end_sec             The number of end sector
 *
 * @return    CMD_SUCCESS/BUSY/INVALID_SECTOR.
 *
 **********************************************************************/
IAP_STATUS_CODE PrepareSector(uint32_t start_sec, uint32_t end_sec)
{
    IAP_COMMAND_Type command;
    command.cmd    = IAP_PREPARE;                    // Prepare Sector for Write
    command.param[0] = start_sec;                    // Start Sector
    command.param[1] = end_sec;                      // End Sector
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command
    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief     Copy RAM to Flash
 *
 * @param[in] dest            destination buffer (in Flash memory).
 * @param[in] source       source buffer (in RAM).
 * @param[in] size            the write size.
 *
 * @return    CMD_SUCCESS.
 *                  SRC_ADDR_ERROR/DST_ADDR_ERROR
 *                  SRC_ADDR_NOT_MAPPED/DST_ADDR_NOT_MAPPED
 *                  COUNT_ERROR/SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION
 *                  BUSY
 *
 **********************************************************************/
IAP_STATUS_CODE CopyRAM2Flash(uint32_t dest, uint32_t source, uint32_t size)
{
    uint32_t sec;
    IAP_STATUS_CODE status;
    IAP_COMMAND_Type command;

    // Prepare sectors
    sec = GetSecNum(dest);
    status = PrepareSector(sec, sec);
    if (status != CMD_SUCCESS)
        return status;

    // write
    command.cmd    = IAP_COPY_RAM2FLASH;             // Copy RAM to Flash
    command.param[0] = dest;                         // Destination Flash Address
    command.param[1] = source;                       // Source RAM Address
    command.param[2] =  size;                          // Number of bytes
    command.param[3] =  SystemCoreClock / 1000;         // CCLK in kHz
    IAP_Call(&command.cmd, &command.status);               // Call IAP Command

    return (IAP_STATUS_CODE)command.status;             // Finished without Errors
}

/*********************************************************************//**
 * @brief     Erase sector(s)
 *
 * @param[in] start_sec    The number of start sector
 * @param[in] end_sec      The number of end sector
 *
 * @return    CMD_SUCCESS.
 *                  INVALID_SECTOR
 *                  SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION
 *                  BUSY
 *
 **********************************************************************/
IAP_STATUS_CODE EraseSector(uint32_t start_sec, uint32_t end_sec)
{
    IAP_COMMAND_Type command;
    IAP_STATUS_CODE status;

    // Prepare sectors
    status = PrepareSector(start_sec, end_sec);
    if (status != CMD_SUCCESS)
        return status;

    // Erase sectors
    command.cmd    = IAP_ERASE;                    // Prepare Sector for Write
    command.param[0] = start_sec;                  // Start Sector
    command.param[1] = end_sec;                    // End Sector
    command.param[2] =  SystemCoreClock / 1000;         // CCLK in kHz
    IAP_Call(&command.cmd, &command.status);       // Call IAP Command
    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief         Blank check sector(s)
 *
 * @param[in]     start_sec    The number of start sector
 * @param[in]     end_sec      The number of end sector
 * @param[out]    first_nblank_loc  The offset of the first non-blank word
  * @param[out]   first_nblank_val  The value of the first non-blank word
 *
 * @return        CMD_SUCCESS.
 *                  INVALID_SECTOR
 *                  SECTOR_NOT_BLANK
 *                  BUSY
 *
 **********************************************************************/
IAP_STATUS_CODE BlankCheckSector(uint32_t start_sec, uint32_t end_sec,
                                 uint32_t *first_nblank_loc,
                                 uint32_t *first_nblank_val)
{
    IAP_COMMAND_Type command;

    command.cmd    = IAP_BLANK_CHECK;                // Prepare Sector for Write
    command.param[0] = start_sec;                    // Start Sector
    command.param[1] = end_sec;                      // End Sector
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command

    if (command.status == SECTOR_NOT_BLANK)
    {
        // Update out value
        if (first_nblank_loc != NULL)
            *first_nblank_loc =  command.result[0];
        if (first_nblank_val != NULL)
            *first_nblank_val =  command.result[1];
    }

    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief      Read part identification number
 *
 * @param[out] partID  Part ID
 *
 * @return     CMD_SUCCESS
 *
 **********************************************************************/
IAP_STATUS_CODE ReadPartID(uint32_t *partID)
{
    IAP_COMMAND_Type command;
    command.cmd = IAP_READ_PART_ID;
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command

    if (command.status == CMD_SUCCESS)
    {
        if (partID != NULL)
            *partID = command.result[0];
    }

    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief      Read boot code version. The version is interpreted as <major>.<minor>.
 *
 * @param[out] major  The major
 * @param[out] minor  The minor
 *
 * @return     CMD_SUCCESS
 *
 **********************************************************************/
IAP_STATUS_CODE ReadBootCodeVer(uint32_t *bootCodeVer)
{
    IAP_COMMAND_Type command;
    command.cmd = IAP_READ_BOOT_VER;
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command

    if (command.status == CMD_SUCCESS)
    {
        if (bootCodeVer != NULL)
            *bootCodeVer = command.result[0];
    }

    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief      Read Device serial number.
 *
 * @param[out] uid   Serial number.
 *
 * @return     CMD_SUCCESS
 *
 **********************************************************************/
IAP_STATUS_CODE ReadDeviceSerialNum(uint32_t *uid)
{
    IAP_COMMAND_Type command;
    command.cmd = IAP_READ_SERIAL_NUMBER;
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command

    if (command.status == CMD_SUCCESS)
    {
        if (uid != NULL)
        {
            uint32_t i = 0;
            for (i = 0; i < 4; i++)
                uid[i] =  command.result[i];
        }
    }

    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief     compare the memory contents at two locations.
 *
 * @param[in] addr1   The address of the 1st buffer (in RAM/Flash).
 * @param[in] addr2   The address of the 2nd buffer (in RAM/Flash).
 * @param[in] size    Number of bytes to be compared; should be a multiple of 4.
 *
 * @return    CMD_SUCCESS
 *                  COMPARE_ERROR
 *                  COUNT_ERROR (Byte count is not a multiple of 4)
 *                  ADDR_ERROR
 *                  ADDR_NOT_MAPPED
 *
 **********************************************************************/
IAP_STATUS_CODE Compare(uint32_t addr1, uint32_t addr2, uint32_t size)
{
    IAP_COMMAND_Type command;
    command.cmd = IAP_COMPARE;
    command.param[0] = addr1;
    command.param[1] = addr2;
    command.param[2] = size;
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command

    return (IAP_STATUS_CODE)command.status;
}

/*********************************************************************//**
 * @brief     Re-invoke ISP.
 *
 * @param[in] None.
 *
 * @return    None.
 *
 **********************************************************************/
void InvokeISP(void)
{
    IAP_COMMAND_Type command;
    command.cmd = IAP_REINVOKE_ISP;
    IAP_Call(&command.cmd, &command.status);         // Call IAP Command
}

