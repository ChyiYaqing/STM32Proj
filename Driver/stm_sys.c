#include "stm32f10x.h"

void RCC_Config(void)
{
	SystemInit();			//��ʼ��ϵͳʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);			//ʹ��PA,PB,PC,PDʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);			//ʹ��USART1ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);			//ʹ��USART2ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);				//ʹ��ADC1ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);													//����ADC��Ƶ����Ϊ6
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);				//ʹ��DACʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); 			//ʹ��TIM3ʱ��
}
