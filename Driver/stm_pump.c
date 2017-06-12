#include "stm32f10x.h"

void Pump_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	/***** GPIO∂Àø⁄…Ë÷√ pump *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_4;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC,&gpio_init_struct);	
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);
}
