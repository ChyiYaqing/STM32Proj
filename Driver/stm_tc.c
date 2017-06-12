#include "stm32f10x.h"

void TC_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	/***** GPIO�˿����� ring fan *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_13;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	/***** GPIO�˿����� ring heater *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_15;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);
	GPIO_ResetBits(GPIOD,GPIO_Pin_15);
}

void TCRingHeat(void)
{
	GPIO_SetBits(GPIOD,GPIO_Pin_15);				//ring heater�ϵ�
	GPIO_SetBits(GPIOD,GPIO_Pin_13);				//ring fan�ϵ�
}

void TCRingStop(void)
{
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);			//ring fan�ϵ�
	GPIO_ResetBits(GPIOD,GPIO_Pin_15);			//ring heater�ϵ�
}
