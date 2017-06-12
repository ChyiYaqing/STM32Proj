#ifndef __STM_DS18B20_H__
#define __STM_DS18B20_H__

#include "stm32f10x.h"

/***** GPIO�˿����� *****/
#define DS18B20_GPIO_IN()  {GPIOD->CRH &= 0xF0FFFFFF; GPIOD->CRH |= 8<<24;}			//��PD14����Ϊ����/��������ģʽ
#define DS18B20_GPIO_OUT() {GPIOD->CRH &= 0xF0FFFFFF; GPIOD->CRH |= 3<<24;}			//��PD14����Ϊͨ���������ģʽ

void DS18B20_GPIO_Init(void);
u8 DS18B20_InitSeq(void);
short DS18B20_GetTemperature(void);

#endif
