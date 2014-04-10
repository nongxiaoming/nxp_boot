#ifndef  _ISP_IAP_H
#define  _ISP_IAP_H

#define     CR                  0x0D
#define     LF                  0x0A
#define     NUL                 0x00
#define     SP                  0x20
#define     NO_OF_IAP_PARAMS    5
#define     NO_OF_ISP_PARAMS    5
#define     PARAM_SIZE          15
#define     ISP_CMD_CODE        0
#define     ISP_PARAM_0         0
#define     ISP_PARAM_1         1
#define     ISP_PARAM_2         2
#define     ISP_PARAM_3         3
#define     ISP_PARAM_4         4

#define     CMD_SIZE            70
#define     UU_LINE             45
#define     LINE_CHECKSUM       20
#define     TRUE                1
#define     FALSE               (!TRUE)
#define     UNLOCK_CODE         0x5A5A
#define     BY_4                4

typedef enum
{
PARAM_ERROR=12, /* Insufficient number of parameters */
ADDR_ERROR=13, /* Address not on word boundary */
ADDR_NOT_MAPPED=14,
CMD_LOCKED=15, /* Command is locked */
INVALID_CODE=16, /* Unlock code is invalid */
INVALID_BAUD_RATE=17,
INVALID_STOP_BIT=18,
CODE_READ_PROTECTION_ENABLED=19
}Command_ErrorCode;
typedef enum
{
COUNT=100,
FLASH_PROG_COUNT,
RAM_OR_FLASH_ADDRESS,
RAM_ADDRESS,
FLASH_ADDRESS,
NO_PARAM_CHECK
}Parameter_Type;

#define CMD_SUCCESS 0
#define IAP_ADDRESS 0x1FFF1FF1

void execute_user_code(void);
unsigned user_code_present(void);
unsigned check_isp_entry_pin(void);
void isp_cmd_loop(void);
int str_cmp(char *, char *);

#endif /* _ISP_IAP_H */


