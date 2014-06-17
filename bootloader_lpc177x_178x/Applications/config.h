#ifndef  __CONFIG_H
#define  __CONFIG_H

//*** <<< Use Configuration Wizard in Context Menu >>> ***

/*
// <h> Flash Configuration
//   <o0> User Start Sector <2=> Sector 2 <3=> Sector 3
//   <o1> User Start Sector Address <0x2000=> Sector 2 <0x3000=> Sector 3
//   <o2> User End Sector <10=> Sector 10 <14=> Sector 14 <29=> Sector 29
//   <o3> User End Sector Address <0x1FFFF=> Sector 10 <0x3FFFF=> Sector 14 <0x7FFFF=> Sector 29
//   <o4> Code Read Protection
//        <0x11223344=> NO CRP <0x12345678=> CRP1 <0x87654321=> CRP2 <0x43218765=> CRP3
// </h>
*/

#define USER_START_SECTOR 2
#define USER_START_SECTOR_ADDRESS 8192
#define USER_END_SECTOR 29
#define USER_END_SECTOR_ADDRESS 524287
#define CRP 0x11223344
#define USER_START_SECTOR_SIZE 0x1000
#define RAM_START 0x10000000
#define RAM_END	  0x10002FFF

extern const unsigned fcclk_KHz;
extern const unsigned crp;

#define CRP1  0x12345678
#define CRP2  0x87654321
#define CRP3  0x43218765
#define NOCRP 0x11223344

/*
// <h> Update Entry Pin
//   <o0> Port
//        <0x2009C014=> Port 0
//        <0x2009C034=> Port 1
//        <0x2009C054=> Port 2
//        <0x2009C074=> Port 3
//        <0x2009C094=> Port 4
//   <o1> Pin <0-31>
// </h>
*/
#define ISP_ENTRY_GPIO_REG LPC_GPIO3->PIN  /* Port */
#define ISP_ENTRY_PIN 	   0          /* Pin  */

/*
// <h> Interface Selection
//   <o0> Interface
//        <1=> Ethernet <0=> Other
// </h>
*/
#define USE_ETHERNET 	1			   /* Use Interface */

/*
// <h> Ethernet Options
//   <o0> Use Debug
//        <1=> Yes <0=> No 
//   <o1> PHY chip
//        <0=> National PHY <1=> Other PHY
//   <o2> PHY address  <00-31>
//   <o3> MAC_Address_6 <0x00-0xFF>   
//   <o4> MAC_Address_5 <0x00-0xFF>  
//   <o5> MAC_Address_4 <0x00-0xFF>  
//   <o6> MAC_Address_3 <0x00-0xFF>  
//   <o7> MAC_Address_2 <0x00-0xFF>  
//   <o8> MAC_Address_1 <0x00-0xFF>  
// </h>
*/
#define ETHERNET_DEBUG 	0			     /* Use Debug */

#define MYMAC_6         0x0C		   /* MAC_6 */
#define MYMAC_5         0x1D		   /* MAC_5 */
#define MYMAC_4         0x12		   /* MAC_4 */
#define MYMAC_3         0xE0		   /* MAC_3 */
#define MYMAC_2         0x1F		   /* MAC_2 */
#define MYMAC_1         0x10       /* MAC_1 */ 
            
/*
// <h> Activity Indicator LED
//   <o0> Enabled
//        <1=> Yes  <0=> No
//   <o1> Select LED (Port 2 - pin X)
//        LED <0-7>
// </h>
*/
#define  LED_ENABLED	1			   /* LED enabled */
#define  PORT0_PIN		22			   /* Port0 22 pin */

/*
// <h> Protocol Options
//   <o0> Timeout (milliseconds) <0-1000000>   
// </h>
*/
#define ETHERNET_TIMEOUT 60000         /* Ethernet timeout */

#endif  /* __SBL_CONFIG_H__ */
