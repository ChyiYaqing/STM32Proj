#ifndef __STM_DS18B20_H__
#define __STM_DS18B20_H__

#include "stm32f10x.h"

/***** GPIO端口设置 *****/
#define DS18B20_GPIO_IN()  {GPIOD->CRH &= 0xF0FFFFFF; GPIOD->CRH |= 8<<24;}			//将PD14设置为上拉/下拉输入模式
#define DS18B20_GPIO_OUT() {GPIOD->CRH &= 0xF0FFFFFF; GPIOD->CRH |= 3<<24;}			//将PD14设置为通用推挽输出模式

void DS18B20_GPIO_Init(void);
u8 DS18B20_InitSeq(void);
short DS18B20_GetTemperature(void);

#endif
