#include "common.h"

void GC_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	/***** GPIO�˿����� GC_READY *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_12;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	/***** GPIO�˿����� GC_START *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	/***** GPIO�˿����� GC_STOP *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_11;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD,&gpio_init_struct);
	
	GPIO_ResetBits(GPIOD,GPIO_Pin_10);
	GPIO_ResetBits(GPIOD,GPIO_Pin_11);
}

void GCStart(void)
{
	GPIO_SetBits(GPIOD,GPIO_Pin_10);						//��GC����ʼ���������ź�
	Delay_ms(20);
	GPIO_ResetBits(GPIOD,GPIO_Pin_10);
}

void GCStop(void)
{
	GPIO_SetBits(GPIOD,GPIO_Pin_11);						//��GC��ֹͣ�����ź�
	Delay_ms(20);
	GPIO_ResetBits(GPIOD,GPIO_Pin_11);
}
