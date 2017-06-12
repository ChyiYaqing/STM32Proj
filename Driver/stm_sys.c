#include "stm32f10x.h"

void RCC_Config(void)
{
	SystemInit();			//初始化系统时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);			//使能PA,PB,PC,PD时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);			//使能USART1时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);			//使能USART2时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);				//使能ADC1时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);													//设置ADC分频因子为6
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);				//使能DAC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); 			//使能TIM3时钟
}
