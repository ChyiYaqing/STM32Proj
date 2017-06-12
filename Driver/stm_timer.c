#include "stm32f10x.h"

void Timer_Init(void)
{
	TIM_TimeBaseInitTypeDef tim_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	
	/***** TIM初始化设置 *****/
	tim_init_struct.TIM_Prescaler = 7199;
	tim_init_struct.TIM_Period = 499;
	tim_init_struct.TIM_ClockDivision = TIM_CKD_DIV1;
	tim_init_struct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3,&tim_init_struct);
	
	/***** NVIC中断设置 *****/
	nvic_init_struct.NVIC_IRQChannel = TIM3_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 3;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM3,ENABLE);
}
