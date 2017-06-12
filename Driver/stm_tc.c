#include "stm32f10x.h"

void TC_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	/***** GPIO端口设置 ring fan *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_13;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	/***** GPIO端口设置 ring heater *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_15;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);
	GPIO_ResetBits(GPIOD,GPIO_Pin_15);
}

void TCRingHeat(void)
{
	GPIO_SetBits(GPIOD,GPIO_Pin_15);				//ring heater上电
	GPIO_SetBits(GPIOD,GPIO_Pin_13);				//ring fan上电
}

void TCRingStop(void)
{
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);			//ring fan断电
	GPIO_ResetBits(GPIOD,GPIO_Pin_15);			//ring heater断电
}
