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
uint32_t ReadOutCount = 0;
unsigned long Free = 0;

//printf串口需要的头文件
#include "usart.h"

void User_main(void)
{
	
	while(1)
	{
		HAL_Delay(500);
		printf("OpCodeMount %d \r\n",f_mount(&SDFatFS,(const TCHAR*)SDPath,1));
		HAL_Delay(50);
		printf("OpCodeOpen  %d \r\n",f_open(&SDFile,(const TCHAR*)String,FA_READ | FA_WRITE));
		HAL_Delay(50);
		printf("OpCodeRead  %d \r\n",f_read(&SDFile,ReadBuffer,7,(unsigned int*)ReadOutCount));
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
		printf("OpCodeSync  %d \r\n",f_sync(&SDFile));
		
		
		
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


