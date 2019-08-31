#ifndef User_main_H_
#define User_main_H_

#include "main.h"

void User_main(void);


/*Flash第三扇区用来存掉电不丢失的系统数据 例如系统配置和IAP信息等 */
/*需要注意的是 STM32F4的第三扇区只能按扇区擦除 */
/*意思是想要更改该扇区内的某个内容 需要完全读出该扇区的数据(16kByte) 然后擦除扇区 然后再编程 */

//系统信息基地址
#define __SystemInfoAddr 0x0800C000

#define __SystemInfo_Header *(uint16_t*)(__SystemInfoAddr + 0)
#define __SystemInfo_FlashVersion *(uint16_t*)(__SystemInfoAddr + 2)


#endif 