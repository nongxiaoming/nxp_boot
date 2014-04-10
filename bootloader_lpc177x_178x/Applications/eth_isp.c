
#include "lpc407x_8x_177x_8x.h"

#include "drv_iap.h"
#include "config.h"
#include "comms.h"
#include "drv_timer.h"
#include "board.h"
#include "eth_isp.h"

char cmd_buf[CMD_SIZE];
char param0[PARAM_SIZE];
char param1[PARAM_SIZE];
char param2[PARAM_SIZE];
char param3[PARAM_SIZE];
char param4[PARAM_SIZE];
char * param_buf[NO_OF_ISP_PARAMS];
int lock;
unsigned crp_after_reset;

const int unlock_code=UNLOCK_CODE;

int is_div(unsigned value,unsigned div)
{
    if(value & (div -1))
    {
        return(1);
    }
    else
    {
        return(0);
    }
}/* is_div */

void __rt_div0(void)
{
} /* __rt_div0 */

int str_cmp(char * src, char * dst)
{
    int i = 0;
    
    for(i=0;src[i] == dst[i];i++)
    {
        if(src[i] == NUL)
        {
            return(NUL);
        }
    }
    return(src[i]-dst[i]);
} /* str_cmp */

int a_to_i(char * inbuf,unsigned * outint)
{
    unsigned int value=0,i;
    
    for(i=0;inbuf[i] != 0;i++)
    {
        if(inbuf[i] >= '0' && inbuf[i] <= '9')
        {
            value = 10 * value +(inbuf[i] - '0');
        }
        else
        {
            return(1);
        }
    }
    *outint = value;
    return(0);
} /* a_to_i */


char * i_to_a(unsigned i_num, char * str, unsigned str_len)
{
    char remainder;

    str[--str_len] = NUL;

    if(i_num == 0)
    {
        str[--str_len] = '0';
        return(str + str_len);
    }

    while (((str_len>0) && (i_num!=0)))
    {
        str_len--;
        remainder = (char) (i_num % 10);
        if ( remainder <= 9 )
        {
            str[str_len] = remainder + '0';
        }
        else  
        {
            str[str_len] = remainder - 10 + 'A';
        }
        i_num = i_num/10;
    }
    return (str + str_len);
} /* i_to_a */

const char sync_str[] = "Synchronized";
void run_isp(void)
{
    int count;

    /* wait for host to initiate communication */
	getline(cmd_buf,CMD_SIZE,&count);

	/* send "Synchronized" string response */
	sendline_crlf((char *)sync_str);

    /* wait for host to respond */
	getline(cmd_buf,CMD_SIZE,&count);

	/* Compare HOST response with the original string */
	if(str_cmp(cmd_buf,(char *)sync_str) == 0)
	{
		/* Autobaud is successful. Get out of while loop. */
	}
	else
	{
	}
}

unsigned param_check(char * param_str,unsigned * param_ptr,int param_type,int count)
{
    int rc,return_code,in_flash,in_ram;
    return_code = 0;
    /* check if conversion from str to integer is required */
    if(param_str != NUL)
    {
        rc = a_to_i(param_str,param_ptr);
        if(rc != 0)
        {
            return(PARAM_ERROR);
        }
    }

    if(param_type == NO_PARAM_CHECK)
    {
        /* further testing is not required. Verification is done in  
           related commands */
        return(0);
    }
    
    rc = is_div(*param_ptr,BY_4);
    if( rc != 0)
    {   
        if(param_type == COUNT)
        {
            return_code = COUNT_ERROR;
        }
        else
        {
            return_code = ADDR_ERROR;
            /* Now distinguish between SRC & DST if required */
            if(param_type == RAM_ADDRESS)
            {
                return_code = SRC_ADDR_ERROR;
            }
            if(param_type == FLASH_ADDRESS)
            {
                return_code = DST_ADDR_ERROR;
            }
        } /* Address error */
    } /* is_div by 4 error */

    /* Check for address mapping if required. Each type of address requires a 
    different type of test. Do this testing only if previous test is ok */
    if(return_code == 0)
    {
        if( ((*param_ptr >= RAM_START) && ((*param_ptr+count) <= (RAM_END+1))) )
        {
            in_ram = TRUE;
        }
        else
        {
            in_ram = FALSE;
        }
        if( ((*param_ptr >= USER_START_SECTOR_ADDRESS) && ((*param_ptr+count) <= (USER_END_SECTOR_ADDRESS+1))) )
        {
            in_flash = TRUE;
        }
        else
        {
            in_flash = FALSE;
        }
        if(param_type == RAM_OR_FLASH_ADDRESS)
        {
            if( !(in_ram || in_flash) )
            {
                return_code = ADDR_NOT_MAPPED;
            }
        }
        if(param_type == RAM_ADDRESS)
        {
            if( !(in_ram) )
            {
                return_code = SRC_ADDR_NOT_MAPPED;
            }
        }
    }
    return(return_code);
}/* param_check */


char decode(char c)
{
    if(c == 0x60)
    {
        return(0x00);
    }
    else
    {
        return(c - SP);
    }
}

unsigned uudecode(char * inbuf, char * outbuf, int * count)
{
    unsigned sum;
    int in_byte_count,out_byte_count;
    
    sum=0;
    out_byte_count=0;
    in_byte_count = decode(*inbuf);
    /* Do not decode if byte count is > 45 */
    if( (in_byte_count > 0) && (in_byte_count <= UU_LINE) )
    {
        for (++inbuf; in_byte_count > 0; inbuf += 4, in_byte_count -= 3)
        {
            /* !!! Test for valid printable character is not done !!! */
            if(in_byte_count >= 3)
            {
                outbuf[0] = (decode(inbuf[0]) << 2) | (decode(inbuf[1]) >> 4);
                outbuf[1] = (decode(inbuf[1]) << 4) | (decode(inbuf[2]) >> 2);
                outbuf[2] = (decode(inbuf[2]) << 6) | (decode(inbuf[3]));
                out_byte_count += 3;
                sum = sum + outbuf[0] + outbuf[1] + outbuf[2];
                outbuf+=3;
            }
            else
            {
                 if(in_byte_count >= 1)
                {
                    outbuf[0] = (decode(inbuf[0]) << 2) | (decode(inbuf[1]) >> 4);
                    out_byte_count++;
                    sum = sum + outbuf[0];
                  /* No need to increment the outbuf as this is the last for loop iteration */
                }
                if(in_byte_count >= 2)
                {
                    outbuf[1] = (decode(inbuf[1]) << 4) | (decode(inbuf[2]) >> 2);
                    out_byte_count++;
                    sum = sum + outbuf[1];
                  /* No need to increment the outbuf as this is the last for loop iteration */
                }
            }
        } /* for loop */
    }
    *count = out_byte_count;
    return(sum);
} /* uudecode */

char encode(char c)
{
    if( c == 0x00)
    {
        return(0x60);
    }
    else
    {
        return(c+SP);
    }
}

unsigned uuencode(char * inbuf, char * outbuf, int count)
{
    unsigned sum;
    char byte1,byte2;
    sum = 0;
    *outbuf = encode(count);
    outbuf++;
    while(count>0)
    {
        if(count >= 3)
        {
            byte1 = inbuf[1];
            byte2 = inbuf[2];
            sum = sum + inbuf[0] + inbuf[1] + inbuf[2];
        }
        else
        {
            if(count == 2)
            {
                byte1 = inbuf[1];
                byte2 = inbuf[1];
                sum = sum + inbuf[0] + inbuf[1];
            }
            else
            {
                byte1 = inbuf[0];
                byte2 = inbuf[0];
                sum = sum + inbuf[0];
            }
        }
        outbuf[0] = encode(((inbuf[0] >> 2) & 0x3F));
        outbuf[1] = encode(((inbuf[0] & 0x03) << 4) + ((byte1 & 0xF0) >> 4));
        outbuf[2] = encode(((byte1 & 0x0F) << 2) + ((byte2 & 0xC0) >> 6));
        outbuf[3] = encode((byte2 & 0x3F));
        outbuf+=4;
        inbuf+=3;
        count-=3;
    } /* while loop */
    outbuf[0] = NUL;
    return(sum);
} /* uuencode */

/* strings used for checksum handshake by read and write commands */
const char ok[] = "OK";
const char resend[] = "RESEND";

void write_to_ram(void)
{
    unsigned dst,checksum,recvd_checksum;
    unsigned line_ctr,last_dst;
    int count,decode_count,last_cnt,char_cnt;
    unsigned rc;
    
    /* Check count validity first as mapping check requires count */
    rc = param_check(param_buf[ISP_PARAM_2],(unsigned *)&count,COUNT,count);
    if(rc == 0)
    {
        rc = param_check(param_buf[ISP_PARAM_1],&dst,RAM_ADDRESS,count);
        if (rc == SRC_ADDR_NOT_MAPPED)
        {
            rc = ADDR_NOT_MAPPED;
        }
        if (rc == SRC_ADDR_ERROR)
        {
            rc = ADDR_ERROR;
        }
    }
    /* Do not allow write to ISP RAM variable area */
    if( (crp_after_reset == CRP1) )
    {
         if( ( dst < (RAM_START) ) )
         {
             rc = CODE_READ_PROTECTION_ENABLED;
         }
     }
 
    /* Print command result on serial channel */
    sendline_crlf(i_to_a(rc,param_buf[ISP_PARAM_0],PARAM_SIZE));

    /* execute command only if parameters are ok */
    if(rc != CMD_SUCCESS)
    {
        return;
    }
    
    /* Initialize checksum related variables */
    checksum = 0;
    line_ctr = 0;
    last_dst = dst;
    last_cnt = count;
    while(count>0)
    {
        if(getline(cmd_buf,CMD_SIZE,&char_cnt) != CMD_SUCCESS)
        {
            /* Abort command */
            break;
        }
        /* Keep track of number of lines decoded. Need to verify checksum
           once the LINE_CHECKSUM lines are receieved */
        if(char_cnt != 0) /* Ignore empty lines */
        {
            line_ctr++;
            checksum = checksum + uudecode(cmd_buf,(char *)dst,&decode_count);
            dst = dst + decode_count;
            count = count - decode_count;
        }
        /* Need checksum handshake ? */
        if( ((line_ctr == LINE_CHECKSUM) || (count == 0)) )
        {
            char_cnt = 0;
            /* Get checksum string (ASCII decimal) from host */
            while(char_cnt == 0)
            {
                rc = getline(cmd_buf,CMD_SIZE,&char_cnt);
            }
            if( rc != CMD_SUCCESS)
            {
                break;
            }
            /* No error check while conversion to integer. If there is an error
            it will be trapped by wrong checksum */
            a_to_i(cmd_buf,&recvd_checksum);
            if( checksum == recvd_checksum)
            {
                last_cnt = count;
                last_dst = dst;
                sendline_crlf((char *)ok);
            }
            else
            {
                count = last_cnt;
                dst = last_dst;
                sendline_crlf((char *)resend);
            }
            /* line counter and checksum has to be reset anyway */
            line_ctr = 0;
            checksum = 0;
        }
    }/* While loop */
} /* write_to_ram */

void read(void)
{
    unsigned src,checksum,last_src,line_ctr;
    int count,encode_count,last_cnt;
    unsigned rc;
	int char_cnt;

    /* Check count validity first as address mapping check requires count */
    rc = param_check(param_buf[ISP_PARAM_2],(unsigned *)&count,COUNT,NUL);
    if(rc == 0)
    {
         rc = param_check(param_buf[ISP_PARAM_1],&src,RAM_OR_FLASH_ADDRESS,count);
    }
    /* Print command result on serial channel */
    sendline_crlf(i_to_a(rc,param_buf[ISP_PARAM_0],PARAM_SIZE));
    /* execute command only if parameters are ok */
    if(rc != CMD_SUCCESS)
    {
        return;
    }
    
    /* Initialize checksum related variables */
    checksum = 0;
    line_ctr = 0;
    last_src = src;
    last_cnt = count;

    while(count > 0)
    {
        if(count > UU_LINE)
        {
            encode_count = UU_LINE;
        }
        else
        {
            encode_count = count;
        }
        checksum = checksum + uuencode((char *)src,cmd_buf,encode_count);
        if(sendline_crlf(cmd_buf) != 1)
        {
            /* Abort command */
            break;
        }
        /* Keep track of number of lines encoded. Need to send checksum
           once the LINE_CHECKSUM lines are transmitted */
        line_ctr++;        
        src = src + encode_count;
        count = count - encode_count;
        if( ((line_ctr == LINE_CHECKSUM) || (count == 0)) )
        {
            /* Tranmitt checksum string (ASCII decimal) to host */
            sendline_crlf(i_to_a(checksum,cmd_buf,PARAM_SIZE));
            /* Read host response */
            rc = getline(cmd_buf,CMD_SIZE,&char_cnt);
            if( rc != CMD_SUCCESS)
            {
                break;
            }
            if(str_cmp(cmd_buf,(char *)ok) == 0)
            {
                last_cnt = count;
                last_src = src;
            }
            else
            {
                count = last_cnt;
                src = last_src;
            }
            /* line counter and checksum has to be reset anyway */
            line_ctr = 0;
            checksum = 0;
        }
    } /* while count > 0 */
} /* read */


void go_cmd(void)
{
    unsigned rc;
    void (*go)(void);
    
    if(lock == FALSE)
    {
        rc = CMD_SUCCESS;
    }
    else
    {
        rc = CMD_LOCKED;
    }
        
    if(rc == CMD_SUCCESS)
    {
        /* Convert and check Go addres */
        rc = param_check(param_buf[ISP_PARAM_1],(unsigned *)&go, \
             RAM_OR_FLASH_ADDRESS,NUL);
    }
    
    if(rc == CMD_SUCCESS)
    {
        /* Check for processor mode switch */
        if(*(param_buf[ISP_PARAM_2]) == 'T')
        {
            /* ARM Thumb compiler generates BX instruction when pointer 
            to a function is used. Set last bit of the address to 1 
            to prevent mode switch to ARM */
            go = (void(*)(void))( ((unsigned)(go) | 0x1) );
        }
        else
        {
            /* Mode parameter is not specified or incorrect */
            rc = PARAM_ERROR;
        }
    }
    /* Print command result on serial channel */
    sendline_crlf(i_to_a(rc,param_buf[ISP_PARAM_1],PARAM_SIZE));
    /* execute command only if all the parameters are ok */
    if(rc == CMD_SUCCESS)
    {
	    /* revert any peripheral changes */
        Timer_DeInit();
        go();
    }
} /* go */


void unlock(void)
{
    int u_code;
    unsigned rc;
    rc = param_check(param_buf[ISP_PARAM_1],(unsigned *)&u_code,NO_PARAM_CHECK,NUL);
    /* execute command only if parameters are ok */
    if(rc == 0)
    {
        if(u_code == unlock_code)
        {
            lock = FALSE;    
        }
        else
        {
            rc = INVALID_CODE;
        }
    }
    /* Print command result on serial channel */
    sendline_crlf(i_to_a(rc,param_buf[ISP_PARAM_1],PARAM_SIZE));
} /* unlock */
//iap相关函数
void prepare_sector(void)
{
    /* Convert ascii string to integer. Do not validate sector numbers as it is
       done in IAP prepare sector for write. Parameter type is deliberately 
       set to NO_PARAM_CHECK so that param_check function just converts ascii 
       to integer 
    */
	  uint32_t start_sec=0,end_sec=0;
	  uint32_t status=0;
    status = param_check(param_buf[ISP_PARAM_1], \
    &start_sec,NO_PARAM_CHECK,NUL);

    if( status == CMD_SUCCESS )
    {
        status = param_check(param_buf[ISP_PARAM_2], \
        &end_sec,NO_PARAM_CHECK,NUL);
    }
        
    if( status == CMD_SUCCESS )
    {
        status=PrepareSector(start_sec,end_sec);
    }
    sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
    PARAM_SIZE));
} /* prepare_sector */

void copy(void)
{
    if(lock == TRUE)
    {
        sendline_crlf(i_to_a(CMD_LOCKED,param_buf[ISP_PARAM_1],PARAM_SIZE));
    }
    else
    {
    /* Convert ascii string to integer. Do not validate sector numbers as it is
    done in IAP copy RAM to FLASH.Parameter type is deliberately 
    set to NO_PARAM_CHECK so that param_check function just converts ascii to integer
    */
			 uint32_t dest=0,source=0,size=0;
	     uint32_t status=0;
        status = param_check(param_buf[ISP_PARAM_1], \
        &dest,NO_PARAM_CHECK,NUL);

        if( status == CMD_SUCCESS )
        {
            status = param_check(param_buf[ISP_PARAM_2], \
            &source,NO_PARAM_CHECK,NUL);
        }

        if( status == CMD_SUCCESS )
        {
            status = param_check(param_buf[ISP_PARAM_3], \
            &size,NO_PARAM_CHECK,NUL);
        }

        /*如果为CRP1模式 */
        if( (crp_after_reset == CRP1) )
        {   
					  //不允许擦除用户app区域起始扇区
            if( (dest < (USER_START_SECTOR_ADDRESS+USER_START_SECTOR_SIZE)) )
            {
                status = CODE_READ_PROTECTION_ENABLED;
            }
        }

        if( status == CMD_SUCCESS )
        {
            status=CopyRAM2Flash(dest,source,size);
        }
        sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
        PARAM_SIZE));
    }
} /* copy */

void erase(void)
{
    if(lock == TRUE)
    {
        sendline_crlf(i_to_a(CMD_LOCKED,param_buf[ISP_PARAM_1],PARAM_SIZE));
    }
    else
    {
    /* Convert ascii string to integer. Do not validate sector numbers as it is
       done in IAP erase sector. Parameter type is deliberately 
       set to NO_PARAM_CHECK so that param_check function just converts ascii to integer 
    */
				  uint32_t start_sec=0,end_sec=0;
	       uint32_t status=0;
        status = param_check(param_buf[ISP_PARAM_1], \
        &start_sec,NO_PARAM_CHECK,NUL);

        if( status == CMD_SUCCESS )
        {
            status = param_check(param_buf[ISP_PARAM_2], \
            &end_sec,NO_PARAM_CHECK,NUL);
        }

        /* If code read protection level 2 or 3 is enabled allow erase command only when
           all sectors are selected for erase */
        if( (crp_after_reset == CRP2) ||                     \
            (crp_after_reset == CRP3) 
           )
        {
            if( (start_sec != USER_START_SECTOR) ||                         \
                (end_sec != USER_END_SECTOR)
              )
            {
                status = CODE_READ_PROTECTION_ENABLED;
            }
        }

        /* If code read protection level 1 is enabled allow erase command only when
           user start sector is NOT selected for erase */
        else if( (crp_after_reset == CRP1) )
        {
           if( (start_sec == USER_START_SECTOR) &&                         \
               (end_sec != USER_END_SECTOR)
             )
            {
                status = CODE_READ_PROTECTION_ENABLED;
            }
        }

        if( status == CMD_SUCCESS )
        {
          status= EraseSector(start_sec, end_sec);
        }
        /* Update code protection status if all sectors are erased */
        if( (start_sec == USER_START_SECTOR) &&                         \
            (end_sec == USER_END_SECTOR)
          )
        {
            crp_after_reset = NOCRP;       
        }
        sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
        PARAM_SIZE));
    }
} /* erase */

void blank_check(void)
{
    /* Convert ascii string to integer. Do not validate sector numbers as it is
       done in IAP blank check sector. Parameter type is deliberately 
       set to NO_PARAM_CHECK so that param_check function just converts ascii 
       to integer
    */
	  	  uint32_t start_sec=0,end_sec=0;
	      uint32_t result[2];
	       uint32_t status=0;
    status = param_check(param_buf[ISP_PARAM_1], \
    &start_sec,NO_PARAM_CHECK,NUL);
    
    if( status == CMD_SUCCESS )
    {
        status = param_check(param_buf[ISP_PARAM_2], \
        &end_sec,NO_PARAM_CHECK,NUL);
    }    
    
    if( status == CMD_SUCCESS )
    {
        status=BlankCheckSector(start_sec,end_sec,&result[0],&result[1]);
    }
    sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
    PARAM_SIZE));
    if(status == SECTOR_NOT_BLANK)
    {
        if( ( (crp_after_reset == CRP1)       ||       \
              (crp_after_reset == CRP2)       ||       \
              (crp_after_reset == CRP3) )              \
          )
        {
            result[0]=0;
            result[1]=0;
        }
        sendline_crlf(i_to_a(result[0],param_buf[ISP_PARAM_1], \
        PARAM_SIZE));
        sendline_crlf(i_to_a(result[1],param_buf[ISP_PARAM_1], \
        PARAM_SIZE));
    }
} /* blank_check */

void compare_mem(void)
{
    /* Convert ascii string to integer. Do not validate address & count as it is
    done in IAP COMPARE command.Parameter type is deliberately 
    set to NO_PARAM_CHECK so that param_check function just converts ascii to integer
    */
	   uint32_t dest=0,source=0,size=0;
	     uint32_t status=0;
    status = param_check(param_buf[ISP_PARAM_1], \
    &dest,NO_PARAM_CHECK,NUL);

    if( status == CMD_SUCCESS )
    {
        status = param_check(param_buf[ISP_PARAM_2], \
        &source,NO_PARAM_CHECK,NUL);
    }

    if( status == CMD_SUCCESS )
    {
        status = param_check(param_buf[ISP_PARAM_3], \
        &size,NO_PARAM_CHECK,NUL);
    }

    if( status == CMD_SUCCESS )
    {
      status=Compare(dest,source,size);
    }
    sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
    PARAM_SIZE));
    if(status == COMPARE_ERROR)
    {
        sendline_crlf(i_to_a(0,param_buf[ISP_PARAM_1], \
        PARAM_SIZE));
    }
}

int tokenize(char * in_buf,int no_of_chars_in_inbuf,char * out_buf[],   \
             int max_tokens,int max_chars_per_token )
{
    int i,token_count=0,pos=0, chars_in_token=0;

    if(in_buf[0] == NUL)
    {
        return(0);
    }
    for(i=0;i<=no_of_chars_in_inbuf;i++)
    {
        if( token_count+1 > max_tokens)
        {
            return(token_count+1);
        }
        
        if( (in_buf[i] != SP)  &&                                      \
            (chars_in_token < (max_chars_per_token -2)) )
        {
            *(out_buf[token_count]+pos) = in_buf[i];
            pos++;
            chars_in_token++;
        }
        else if( (in_buf[i+1] != SP) ||                                \
                 (chars_in_token == (max_chars_per_token -2)) )
        {
            *(out_buf[token_count]+pos) = NUL;
            token_count++;
            pos=0;
            chars_in_token=0;
        }            
    }
    return(token_count+1);
} /* tokenize */

void enable_interrupts(unsigned interrupts)
{
  __enable_irq();
}

void disable_interrupts(unsigned interrupts)
{
  __disable_irq();
}

void execute_user_code(void)
{
    void (*user_code_entry)(void);
    user_code_entry = (void (*)(void))(USER_START_SECTOR_ADDRESS | 0x00000001);
    user_code_entry();
}

unsigned user_code_present(void)
{
	   uint32_t start_sec=0,end_sec=0;
	   uint32_t status=0;
    start_sec = USER_START_SECTOR;
    end_sec = USER_START_SECTOR;
    status=BlankCheckSector(start_sec,end_sec,NULL,NULL);
	if( status == CMD_SUCCESS )
	{
	    return (FALSE);
	}
	else
	{
	    return (TRUE);
	}
}

unsigned check_isp_entry_pin(void)
{
  if( ISP_ENTRY_GPIO_REG & (0x1<<ISP_ENTRY_PIN) )
	{
	    return(TRUE);
	}
	else
	{
	    return(FALSE);
	}
}


void exec_isp_cmd(int no_of_param)
{
    int c;
    uint32_t result[4];
	  uint32_t status=0;
    c = *(param_buf[ISP_CMD_CODE]);
    if(c != NUL);
    {
        switch(c)
        {
        /* Write to RAM command */
        case 'W':
            write_to_ram();
            break;
        /* Read command */
        case 'R':
            read();
            break;
        /* Go command */
        case 'G':
            go_cmd();
            break;
        /* Unlock command */
        case 'U':
            unlock();
            break;
        /* Read Part Identification number command */
        case 'J':
             status=ReadPartID(&result[0]);
            sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a(result[0],param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
        break;
        /* read serial number command */
        case 'N':
            status=ReadDeviceSerialNum(&result[0]);
            sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a(result[0],param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a(result[1],param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a(result[2],param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a(result[3],param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
        break;
        /* Read Boot Code Version number command */
        case 'K':
            status=ReadBootCodeVer(&result[0]);
            sendline_crlf(i_to_a(status,param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a((char)result[0],param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            sendline_crlf(i_to_a((char)(result[0]>>8),param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            break;
        /* Prepare sector for write operation command */
        case 'P':
            prepare_sector();
            break;
        /* Copy RAM to FLASH command */
        case 'C':
            copy();
            break;
        /* Erase sector command */
        case 'E':
            erase();
            break;
        /* Blank check sector command */
        case 'I':
            blank_check();
            break;
        /* Compare Memory */
        case 'M':
            compare_mem();
            break;
        /* Invalid Command */
        default:
            sendline_crlf(i_to_a(INVALID_COMMAND,param_buf[ISP_PARAM_1], \
            PARAM_SIZE));
            break;
        } /* switch(c) */
    } /* if c != NUL */
} /* exec_isp_cmd */

/* only returns if timeout occurred */
void isp_cmd_loop(void)
{
    int cmd_len,no_of_param,crp_block_common_cmd,crp_block_crp2_crp3_cmd;
	int result;
    
	/* initialize to avoid compiler warning  */
    /* Lock Flash Write/Erase/Go Commands */
    lock = TRUE;
    
    /* intialize parameter buffer */
    param_buf[ISP_PARAM_0] = param0;
    param_buf[ISP_PARAM_1] = param1;
    param_buf[ISP_PARAM_2] = param2;
    param_buf[ISP_PARAM_3] = param3;
    param_buf[ISP_PARAM_4] = param4;

    /* Initialize cp_after_reset variable to allow ISP session to program 
       code protection enabled user application */
	crp_after_reset = crp;

	crp_block_crp2_crp3_cmd = TRUE;

	while(1)
    {
	    result = getline(cmd_buf,CMD_SIZE,&cmd_len);
        if(result == 0)
        {
            /* To safegaurd against insufficient parameters fill parameter buffers with
               Non numeric ascii charcaters */
            for(no_of_param=0;no_of_param<NO_OF_ISP_PARAMS;no_of_param++)
            {
                *(param_buf[no_of_param]) = 'z';
            }
            no_of_param = tokenize(&cmd_buf[0],cmd_len,&param_buf[0],NO_OF_ISP_PARAMS,PARAM_SIZE);  
            if(no_of_param != 0)
            {
               /* Common ISP commands to be blocked for code read protection Level 1, 2 or 3 */
               if( ( (*(param_buf[ISP_CMD_CODE]) == 'R') ||              \
                     (*(param_buf[ISP_CMD_CODE]) == 'G') ||              \
                     (*(param_buf[ISP_CMD_CODE]) == 'M') ) )
               {
                  crp_block_common_cmd = TRUE;
               }
               else
               {
                  crp_block_common_cmd = FALSE;
                  /* ISP commands to be blocked for code read protection Level 2 and 3 */
                  if( ( (*(param_buf[ISP_CMD_CODE]) == 'W') ||              \
                        (*(param_buf[ISP_CMD_CODE]) == 'C') ) )
                  {
                     crp_block_crp2_crp3_cmd = TRUE;
                  }
                  else
                  {
                     crp_block_crp2_crp3_cmd = FALSE;
                  }
               }

               if( ( (crp_after_reset == CRP1) ||            \
                     (crp_after_reset == CRP2) ||     \
                     (crp_after_reset == CRP3) )      \
                                        &&                                 \
                    ( crp_block_common_cmd == TRUE )
                  )
                {
                    /* Code read protection Level 1, 2 or 3 enabled. Block Read,
                       Go and Compare ISP commands */
                    sendline_crlf(i_to_a(CODE_READ_PROTECTION_ENABLED, \
                    param_buf[ISP_PARAM_1],PARAM_SIZE));
                }  
                else
                {
                   if( ( (crp_after_reset == CRP2) ||            \
                         (crp_after_reset == CRP3) )  
                                         &&                                   \
                        ( crp_block_crp2_crp3_cmd == TRUE ) )
                   {
                    /* Code read protection Level 2 or 3 is enabled. Block Write
                       and Copy ISP commands */
                       sendline_crlf(i_to_a(CODE_READ_PROTECTION_ENABLED, \
                       param_buf[ISP_PARAM_1],PARAM_SIZE));
                   }
                   else
                   {
                       exec_isp_cmd(no_of_param);
                   }
                }
            } /* if(no_of_param != 0) */
        } /* if(serial_getline(cmd_buf,CMD_SIZE,&cmd_len) == 0) */
		else
		{
		  /* timeout */
		  return;
		}
	} /* While(1) loop */
} /* isp_cmd_loop */
