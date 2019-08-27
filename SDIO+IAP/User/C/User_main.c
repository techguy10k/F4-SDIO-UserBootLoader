#include "User_main.h"
#include "stdio.h"
#include "sdio.h"
#include "stm32f4xx_hal.h"

//SDIO操作头文件
#include "sdio.h"

//FatFs测试头文件
#include "fatfs.h"
#include "ff.h"
uint8_t String[] = {"SD.txt"};
uint8_t ReadBuffer[7] = {0};
uint8_t ReadBufferS[10] = {0};
uint8_t ReadBufferSS[10] = {0};
uint8_t ReadBufferDir[20] = {0};
uint32_t ReadOutCount = 0;

//FLASH测试头文件


//printf串口需要的头文件
#include "usart.h"

void User_main(void)
{
	
	while(1)
	{
		/* 根目录单文件二进制读写操作测试 */
		HAL_Delay(500);
		printf("OpCodeMount %d \r\n",f_mount(&SDFatFS,(const TCHAR*)SDPath,1));
		HAL_Delay(50);//东西太多HC12发不完 下面的50ms延迟同理
		/* 打开文件 读取文件 获取当前文件指针 移动文件指针到0处 写入字符串 */
		printf("OpCodeOpen  %d \r\n",f_open(&SDFile,(const TCHAR*)String,FA_READ | FA_WRITE));
		HAL_Delay(50);
		printf("OpCodeRead  %d \r\n",f_read(&SDFile,ReadBuffer,6,(unsigned int*)ReadOutCount));
		HAL_Delay(50);
		printf("ReadOut     %s \r\n",ReadBuffer);
		HAL_Delay(50);
		printf("OpCodeTell  %d \r\n",(int)f_tell(&SDFile));
		HAL_Delay(50);
		printf("OpCodelseek %d \r\n",f_lseek(&SDFile,0));
		HAL_Delay(50);
		printf("OpCodeTell  %d \r\n",(int)f_tell(&SDFile));
		HAL_Delay(50);
		printf("OpCodeWrite %d \r\n",f_write(&SDFile,String,7,(unsigned int*)ReadOutCount));
		HAL_Delay(50);
		/* 同步文件 */
		printf("OpCodeSync  %d \r\n",f_sync(&SDFile));
		/* 根目录单文件字符读写操作测试 如果不是二进制文件 建议使用此等操纵方式 */
		printf("OpCodeReadS    \r\n");
		HAL_Delay(50);
		f_lseek(&SDFile,0);
		ReadOutCount = 7;
		f_gets((TCHAR*)ReadBufferS,ReadOutCount,&SDFile);
		printf("ReadOut     %s \r\n",ReadBufferS);
		HAL_Delay(50);
		printf("OpCodeWriteS    \r\n");
		HAL_Delay(50);
		f_lseek(&SDFile,0);
		f_printf(&SDFile,"SD.txt");
		f_sync(&SDFile);
		f_truncate(&SDFile);
		f_close(&SDFile);
		/* 目录层面操作测试 文件夹相关 */
		f_getcwd((TCHAR*)ReadBufferDir,20);
		printf("Current Dir %s\r\n",ReadBufferDir);
		HAL_Delay(50);
		printf("Make a dir at /\r\n");
		HAL_Delay(50);
		f_mkdir("DIR");
		f_chdir("DIR");
		printf("OpCodeOpen  %d \r\n",f_open(&SDFile,"SD1.txt",FA_READ | FA_WRITE | FA_OPEN_ALWAYS));
		HAL_Delay(50);
		ReadOutCount = 7;
		f_gets((TCHAR*)ReadBufferSS,ReadOutCount,&SDFile);
		printf("ReadOut     %s \r\n",ReadBufferSS);
		HAL_Delay(50);
		printf("OpCodeWriteS    \r\n");
		HAL_Delay(50);
		f_lseek(&SDFile,0);
		f_printf(&SDFile,"SD.txt");
		f_sync(&SDFile);
		f_truncate(&SDFile);
		f_close(&SDFile);
		f_getcwd((TCHAR*)ReadBufferDir,20);
		printf("Current Dir %s\r\n",ReadBufferDir);
		
		
		/* FLASH访问测试 */
		uint8_t Data = 0;
		
		for(uint32_t counter = 0x8000000;counter < (0x8000000 + 10);counter ++)
		{
			Data = *((uint8_t*)(counter));
			printf("%02X ",(unsigned int)Data);
			HAL_Delay(50);
		}
		printf("\r\n");
		/* FLASH写读测试 */
		Data = *((uint8_t*)(0x800C000));
		printf("FlashRead %d\r\n",Data);
		
		uint32_t tick = HAL_GetTick();
		HAL_FLASH_Unlock();
		//先擦除才能写
		FLASH_Erase_Sector(3,FLASH_VOLTAGE_RANGE_3);
		//先清除错误标志位再进行flash写操作 否则报错
		__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,0x800C000,0xA5);
		HAL_FLASH_GetError();
		tick = HAL_GetTick() - tick;
		printf("time %d\r\n",tick);
		HAL_FLASH_Lock();
		
		Data = *((uint8_t*)(0x800C000));
		printf("FlashRead %d\r\n",Data);
		/* 测试结束 */
		
		
		
		
		f_close(&SDFile);
		HAL_Delay(50);
		printf("System halted.\r\n");
		while(1);		
	}
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


