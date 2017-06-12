#include "common.h"

void MFC_ADC_Init(void)   //质量流量控制器(MFC)3660
{
	GPIO_InitTypeDef gpio_init_struct;
	ADC_InitTypeDef adc_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	
	/***** GPIO端口设置 ADC1通道1 *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_1;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&gpio_init_struct);
	
	ADC_DeInit(ADC1);
	
	/***** ADC初始化设置 *****/
	adc_init_struct.ADC_Mode = ADC_Mode_Independent;
	adc_init_struct.ADC_ScanConvMode = DISABLE;
	adc_init_struct.ADC_ContinuousConvMode = DISABLE;
	adc_init_struct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	adc_init_struct.ADC_DataAlign = ADC_DataAlign_Right;
	adc_init_struct.ADC_NbrOfChannel = 1;																	
	ADC_Init(ADC1,&adc_init_struct);
	
	/***** NVIC中断设置 *****/
	nvic_init_struct.NVIC_IRQChannel = ADC1_2_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 2;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);
	
	ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
	Delay_us(1);
	ADC_Cmd(ADC1,ENABLE);
	ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
}

void ADCStart(void)
{
	ADC_RegularChannelConfig(ADC1,ADC_Channel_1,1,ADC_SampleTime_239Cycles5);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
}

void MFC_DAC_Init(void)   //为了控制流量，将数字信号转换为模拟信号
{
	GPIO_InitTypeDef gpio_init_struct;
	DAC_InitTypeDef dac_init_struct;
	
	/***** GPIO端口设置 DAC通道1 *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_4;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&gpio_init_struct);
	
	/***** DAC初始化设置 *****/
	dac_init_struct.DAC_Trigger = DAC_Trigger_None;
	dac_init_struct.DAC_WaveGeneration = DAC_WaveGeneration_None;
	dac_init_struct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	dac_init_struct.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
	DAC_Init(DAC_Channel_1,&dac_init_struct);
	
	DAC_Cmd(DAC_Channel_1,ENABLE);
	DAC_SetChannel1Data(DAC_Align_12b_R,0);
}
