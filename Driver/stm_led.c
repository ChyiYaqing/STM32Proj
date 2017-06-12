#include "stm32f10x.h"

void LED_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	/***** GPIO∂Àø⁄…Ë÷√ ¿∂µ∆ *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_0;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC,&gpio_init_struct);
	
	/***** GPIO∂Àø⁄…Ë÷√ ¬Ãµ∆ *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_1;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC,&gpio_init_struct);
	
	/***** GPIO∂Àø⁄…Ë÷√ ∫Ïµ∆ *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_2;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC,&gpio_init_struct);
	
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	GPIO_SetBits(GPIOC,GPIO_Pin_1);
	GPIO_SetBits(GPIOC,GPIO_Pin_2);
}

/***** π ’œµ∆ *****/
void LEDRed(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	GPIO_SetBits(GPIOC,GPIO_Pin_1);
	GPIO_ResetBits(GPIOC,GPIO_Pin_2);			//∫Ïµ∆¡¡
}

/***** ‘À––µ∆ *****/
void LEDGreen(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_0);
	GPIO_SetBits(GPIOC,GPIO_Pin_2);
	GPIO_ResetBits(GPIOC,GPIO_Pin_1);			//¬Ãµ∆¡¡
}

/***** ø’œ–µ∆ *****/
void LEDBlue(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_1);
	GPIO_SetBits(GPIOC,GPIO_Pin_2);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);			//¿∂µ∆¡¡
}
