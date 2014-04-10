
#ifndef __DRV_CRC_H
#define __DRV_CRC_H

/* Includes ------------------------------------------------------------------- */
#include "LPC407x_8x_177x_8x.h"



#ifdef __cplusplus
extern "C"
{
#endif

    /* Private macros ------------------------------------------------------------- */
    /** @defgroup CRC_Private_Macros CRC Private Macros
     * @{
     */

    /* -------------------------- BIT DEFINITIONS ----------------------------------- */
    /*********************************************************************//**
     * Macro defines for CRC mode register
     **********************************************************************/
#define CRC_BIT_RVS_WR          (1<<2)
#define CRC_CMPL_WR             (1<<3)
#define CRC_BIT_RVS_SUM         (1<<4)
#define CRC_CMPL_SUM            (1<<5)


    /**
     * @}
     */
    /* Private types ------------------------------------------------------------- */
    typedef enum
    {
        CRC_POLY_CRCCCITT = 0,          /** CRC CCITT polynomial */
        CRC_POLY_CRC16,                 /** CRC-16 polynomial */
        CRC_POLY_CRC32                  /** CRC-32 polynomial */
    }
    CRC_Type;

    typedef enum
    {
        CRC_WR_8BIT = 1,                /** 8-bit write: 1-cycle operation */
        CRC_WR_16BIT = 2,                   /** 16-bit write: 2-cycle operation */
        CRC_WR_32BIT = 4,                   /** 32-bit write: 4-cycle operation */
    } CRC_WR_SIZE;

    /* Public Functions ----------------------------------------------------------- */
    /** @defgroup CRC_Public_Functions CRC Public Functions
     * @{
     */
    void CRC_Init(CRC_Type CRCType);
    void CRC_Reset(void);
    uint32_t CRC_CalcDataChecksum(uint32_t data, CRC_WR_SIZE SizeType);
    uint32_t CRC_CalcBlockChecksum(void *blockdata, uint32_t blocksize, CRC_WR_SIZE SizeType);

    /**
     * @}
     */


#ifdef __cplusplus
}
#endif


#endif /* __DRV_CRC_H */

