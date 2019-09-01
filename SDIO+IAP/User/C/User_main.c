#include "User_main.h"
#include "stdio.h"
#include "sdio.h"
#include "stm32f4xx_hal.h"

//SDIO操作头文件
#include "sdio.h"

//FatFs测试头文件
#include "fatfs.h"
#include "ff.h"
#include "stdlib.h"
uint8_t String[20] = {0};
uint16_t SdVersion = 0;
//FLASH测试头文件


//printf串口需要的头文件
#include "usart.h"

//还没有封装的函数
uint8_t IAP_FlashErase_APP(uint32_t ImageSize);
uint8_t IAP_FlashProgram(FIL* file);
void IAP_Jump2App(uint32_t AppFlashBase);

void User_main(void)
{
	
	while(1)
	{
		HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
		HAL_Delay(50); //等待系统稳定 不一定需要
		
		if(__SystemInfo_Header != 0x5AA5)
		{
			printf("Incorrect SystemInfo Header.\r\nSector 3 Will be format.\r\n");
			HAL_Delay(50);
			printf("Will lost all SystemInfo.\r\n");
			HAL_Delay(50);
			
			HAL_FLASH_Unlock();
			FLASH_Erase_Sector(3,FLASH_VOLTAGE_RANGE_3);			
			//写Header 供下次识别
		  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
		  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,__SystemInfoAddr + 0,0x5AA5);
			//重置默认版本
		  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
		  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,__SystemInfoAddr + 2,0x0000);
			HAL_FLASH_Lock();
		}
		
		
		//如果不能mountSD卡 则报错
		if(f_mount(&SDFatFS,(const TCHAR*)SDPath,1) != FR_OK)
		{
			printf("SdCard Mount Error : %d\r\n",f_mount(&SDFatFS,(const TCHAR*)SDPath,1));
		}
		
		//到根目录查询是否有一个叫dload的文件夹 不存在就创建dload文件夹
		f_chdir("/");
		if(f_stat("/dload",NULL) != FR_OK)
		{
			printf("/dload Not Found,Thus /dload Will Be Create.\r\n");
			f_mkdir("/dload");
		}
		
		//检查是否有一个叫 update.bin 的文件在/dload中 如果有就进一步检查是否有 version.txt(ANSI编码) 然后读取第一个字符串进行版本判断
		//如果没有版本信息 除非触发强制刷写(外部强制更新按钮) 否则不刷写
		f_chdir("/dload");
		if(f_stat("update.bin",NULL) != FR_OK)
		{
			printf("No update.bin Found,Jump to User Application\r\n");
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
			IAP_Jump2App(0x08010000);
		}
		else if(f_stat("update.bin",NULL) == FR_OK)
		{
			if(f_stat("version.txt",NULL) != FR_OK)
			{
				SdVersion = 0;
			}
			else if(f_stat("version.txt",NULL) == FR_OK)
			{
				f_open(&SDFile,"version.txt",FA_READ);
				SdVersion = atoi(f_gets((TCHAR*)String,20,&SDFile));
				f_close(&SDFile);
				printf("SdCard Version %d\r\n",SdVersion);
				HAL_Delay(50);
				printf("Flash  Version %d\r\n",__SystemInfo_FlashVersion);
				HAL_Delay(50);
				
				//版本判断 此if语句前一个参数为flash版本 稍后会替换为结构体
				if(__SystemInfo_FlashVersion >= SdVersion)
				{
					printf("Update Abort.\r\n");
					HAL_Delay(50);
					printf("Jump to User APP.\r\n");
					HAL_Delay(50);
					HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
					IAP_Jump2App(0x08010000);
				}
				else
				{
					printf("Updating...\r\n");
					
					//重写版本信息
					HAL_FLASH_Unlock();
					FLASH_Erase_Sector(3,FLASH_VOLTAGE_RANGE_3);			
					//写Header 供下次识别
					__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,__SystemInfoAddr + 0,0x5AA5);
					//重置默认版本
					__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,__SystemInfoAddr + 2,SdVersion);
					HAL_FLASH_Lock();
					HAL_Delay(50);
					
					f_open(&SDFile,"update.bin",FA_READ);
					//获取文件大小 根据文件大小擦除flash 如果文件过大则发出警告 在IAP_FlashErase()中实现
				  printf("IAP_Erase ERR Code %d\r\n",IAP_FlashErase_APP(f_size(&SDFile)));
					printf("IAP_Program ERR Code %d\r\n",IAP_FlashProgram(&SDFile));
					f_close(&SDFile);
					printf("Jump to User App.\r\n");
					HAL_Delay(50);
					HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
					IAP_Jump2App(0x08010000);
				}
				
			}				
		}
		
		f_close(&SDFile);
		HAL_Delay(50);
		printf("System halted.\r\n");
		while(1);		
	}
}




uint8_t IAP_FlashErase_APP(uint32_t ImageSize)
{
	//stm32f407vetx app区内存大小 iap程序+配置信息 预计会占用前4个16KB扇区
	//分别是sec0~sec3 
	//app从sec4 0x08010000 开始 到 0x0807FFFF 结束
	//想适配其他芯片 本flash擦除函数需要根据其他芯片的情况具体修改
	
	FLASH_EraseInitTypeDef flasherase;
	uint32_t Flash_Erase_ErrorSector = 0;
	
	flasherase.Banks = FLASH_BANK_1;
	flasherase.NbSectors = 0;
	flasherase.Sector = FLASH_SECTOR_4;
	flasherase.TypeErase = FLASH_TYPEERASE_SECTORS;
	flasherase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	
	printf("Sector start from 4\r\n");
	
	//image过大
	if(ImageSize > 458752)
	{
		printf("ImageSize too Large.\r\n");
		return 1;
	}
	else if(ImageSize > 327680)
	{
		printf("All sector erase.\r\n");
		printf("Sector Erase to Sector 6\r\n");
		//扇区擦除部分
		HAL_FLASH_Unlock();		
		flasherase.NbSectors = 4;
		HAL_FLASHEx_Erase(&flasherase,&Flash_Erase_ErrorSector);
		HAL_FLASH_Lock();
		return 0;
	}
	else if(ImageSize > 196608)
	{
		printf("Sector Erase to Sector 6\r\n");
		//扇区擦除部分
		HAL_FLASH_Unlock();
		flasherase.NbSectors = 3;
		HAL_FLASHEx_Erase(&flasherase,&Flash_Erase_ErrorSector);
		HAL_FLASH_Lock();
		return 0;		
	}
	else if(ImageSize > 65536)
	{
		printf("Sector Erase to Sector 5\r\n");
		//扇区擦除部分
		HAL_FLASH_Unlock();
		flasherase.NbSectors = 2;
		HAL_FLASHEx_Erase(&flasherase,&Flash_Erase_ErrorSector);
		HAL_FLASH_Lock();
		return 0;				
	}
	else if(ImageSize <= 65536)
	{
		printf("Sector Erase to Sector 4\r\n");
		//扇区擦除部分
		HAL_FLASH_Unlock();
		flasherase.NbSectors = 1;
		HAL_FLASHEx_Erase(&flasherase,&Flash_Erase_ErrorSector);
		HAL_FLASH_Lock();
		return 0;
	}
	
	return 0;
}


uint8_t IAP_FlashProgram(FIL* file)
{
	uint32_t ImageSize = 0;
	uint32_t ReadCount = 0;
	uint8_t  ReadBuffer = 0;
	
	//已经提前f_open()过了
	printf("Flash program Start.\r\n");
	HAL_Delay(50);
	ImageSize = f_size(&SDFile);
	printf("ImageSize %d.\r\n",ImageSize);
	HAL_Delay(50);
	printf("Flash Program Start AT 0x0801 0000.\r\n");
	HAL_Delay(50);
	
	//移动文件指针到文件开头
	f_lseek(&SDFile,0);
	
	HAL_FLASH_Unlock();
	
	for(uint32_t counter = 0;counter < ImageSize;counter ++)
	{
		//读文件一个字节 文件指针自动增加
		f_read(&SDFile,&ReadBuffer,1,&ReadCount);
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
		//字节模式编程flash基地址+counter 编程内容为读出来的内容
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,0x08010000 + counter,ReadBuffer);	
	}
	
	HAL_FLASH_Lock();
	
	
	return 0;
}


void IAP_Jump2App(uint32_t AppFlashBase)
{
	uint32_t JumpAddr = 0;
	void (*pFun)(void); //定义一个函数指针.用于指向APP程序入口
	
	
	//APP程序要跳转到复位向量 也就是App基地址+4的地址
	JumpAddr = *(uint32_t *)( AppFlashBase + 4);
	
	//生成跳转函数.将复位中断向量地址做为函数指针
	pFun = (void (*)(void))JumpAddr;
	
	//关闭所有外设
	HAL_DeInit();
		
	//执行用户App
	(*pFun)();
	
}



#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif 


/* printf重定向到串口1 */
int fputc(int ch,FILE *f)
{
    uint8_t temp[1]={ch};
    HAL_UART_Transmit(&huart1,temp,1,10);        //UartHandle是串口的句柄
		return ch;
}


PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,10);
	return ch;
}
/* ********************************************* */


