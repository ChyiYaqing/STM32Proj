  /**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "common.h"
#include "stm_usart.h"
#include "stm_pc.h"
#include "stm_valve.h"
#include "stm_mfc.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/***** PC_USART中断服务程序 *****/
void USART1_IRQHandler(void)
{
	u8 res;
	static u8	i = 0;

	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)			//接收中断
	{
		res = USART_ReceiveData(USART1);			//读取接收到的数据
		if(res==STX && !(PC_USART_RX_STA&0x4000))				//检测数据帧的帧头
		{
			PC_USART_RX_STA |= 0x4000;					//开始数据帧的接收
			return;
		}
		if(PC_USART_RX_STA&0x4000 && !(PC_USART_RX_STA&0x8000))			//判断是否开始接收，接收是否完成
		{
			if((PC_USART_RX_STA&0x3FFF)==pc_usart_rx_buf[0]+1)			//判断是否到达帧尾
			{
				if(res==ETX)											//检测数据帧的帧尾
					PC_USART_RX_STA |= 0x8000; 			//一帧数据接收完成
				else
					PC_USART_RX_STA = 0;
			}
			else
			{			
				pc_usart_rx_buf[PC_USART_RX_STA&0x3FFF] = res;
				PC_USART_RX_STA++;
				if((PC_USART_RX_STA&0x3FFF)>=PC_USART_REC_LEN)
					PC_USART_RX_STA = 0;
			}
		}
	}
	if(USART_GetITStatus(USART1,USART_IT_TC)!=RESET)			//发送中断
	{
		if(pc_usart_tx_count>0)			//判断一帧数据是否发送完成
		{
			USART_SendData(USART1,pc_usart_tx_buf[i]);			//发送数据
			i++;
			pc_usart_tx_count--;
		}
		else
		{
			USART_ClearITPendingBit(USART1,USART_IT_TC);
			pc_usart_tx_buf = NULL;
			i = 0;
		}
	}
}

/***** 485_USART中断服务程序 *****/
void USART2_IRQHandler(void)
{
	u8 res;
	static u8	temp,	i = 0, idx = 0;

	if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)			//接收中断
	{
		res = USART_ReceiveData(USART2);			//读取接收到的数据
		if(current_valve_type==VT_VALVE10 || current_valve_type==VT_VALVE2 || current_valve_type==VT_VALVE4)
		{
			switch(idx)
			{
				case 0:
					if(res=='/')
						idx++;
					break;
				case 1:
					if(res==current_valve_type+0x30)
						idx++;
					else
						idx = 0;
					break;
				case 2:
					idx++;
					break;
				case 3:
					if(res=='P')
						idx++;
					else
						idx = 0;
					break;
				case 4:
					if(res=='o')
						idx++;
					else
						idx = 0;
					break;
				case 5:
					if(res=='s')
						idx++;
					else
						idx = 0;
					break;
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
					idx++;
					break;
				case 12:
					if(res=='i')
						idx++;
					else
						idx = 0;
					break;
				case 13:
					if(res=='s')
						idx++;
					else
						idx = 0;
					break;
				case 14:
				case 15:
					idx++;
					break;
				case 16:
					if(res=='A' || res=='B')
					{
						rs485_usart_rx_buf = res;			//获取当前阀位置的值
						idx = 0;
					}
					else
						idx = 0;
					break;
				default:
					idx = 0;
			}
		}
		else
		{
			switch(idx)
			{
				case 0:
					if(res=='/')
						idx++;
					break;
				case 1:
					if(res==current_valve_type+0x30)
						idx++;
					else
						idx = 0;
					break;
				case 2:
					if(res=='C')
						idx++;
					else
						idx = 0;
					break;
				case 3:
					if(res=='P')
						idx++;
					else
						idx = 0;
					break;
				case 4:
					if(res>='0' && res<='9')
					{
						temp = res;
						idx++;
					}
					else
						idx = 0;
					break;
				case 5:
					if(res>='0' && res<='9')
					{
						rs485_usart_rx_buf = (temp-0x30)*10+(res-0x30);			//获取当前阀位置的值
						idx = 0;
					}
					else
						idx = 0;
					break;
				default:
					idx = 0;
			}
		}	
	}
	if(USART_GetITStatus(USART2,USART_IT_TC)!=RESET)			//发送中断
	{
		if(rs485_usart_tx_count>0)			//判断数据是否发送完成
		{
			USART_SendData(USART2,rs485_usart_tx_buf[i]);			//发送数据
			i++;
			rs485_usart_tx_count--;
		}
		else
		{
			USART_ClearITPendingBit(USART2,USART_IT_TC);
			rs485_usart_tx_buf = NULL;
			i = 0;
			GPIO_SetBits(GPIOC,GPIO_Pin_3);			//485总线状态改为接收
		}
	}
}

/***** MFC_ADC中断服务程序 *****/
void ADC1_2_IRQHandler(void)
{
	static u8 i = 0;
	
	current_flow_data[i] = ADC_GetConversionValue(ADC1);
	if(++i==CURRENT_FLOW_DATA_LEN)
		i = 0;
}

/***** Timer3中断服务程序 *****/
void TIM3_IRQHandler(void)
{
	static u8 cnt_sec = 0, cnt_frt = 0, cnt_fpt = 0;
	static u16 cnt_ff = 0;
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)			//判断是否发生了更新中断
	{
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		
		cnt_sec++;
		if(cnt_sec>=20)									//1s计时到
		{
			if(FLAGS&0x0200)							//判断MFC是否OK
			{
				cnt_ff++;
				if(cnt_ff>=send_ff_frq)			//设定的流量帧上传时间到
				{
					f_flow.ff_data = (u16)(((0.1*3.3/(5.0*4095))*cfd_avrg)*1000);
					SendFlowFrame();
					cnt_ff = 0;
				}			
			}
			if(FLAGS&0x0080)							//判断DS18B20是否OK
			{
				cnt_frt++;
				if(cnt_frt>=5)							//5s计时到，上传定量环温度帧
				{
					f_ring_temp.frt_data = current_ring_temp;
					SendRingTempFrame();
					cnt_frt = 0;
				}
			}
			if(FLAGS&0x0400)							//判断有无配备TCPipeline
			{
				cnt_fpt++;
				if(cnt_fpt>=6)							//6s计时到，上传管路温度帧
				{
					f_pipeline_temp.fpt_data = current_pipeline_temp;
					SendPipelineTempFrame();
					cnt_fpt = 0;
				}
			}
			if(FLAGS&0x0020)							//判断有无配备MFC
				cfd_avrg = GetFlowData();
			cnt_sec = 0;
		}	
		if(FLAGS&0x0020)								//判断有无配备MFC
			ADCStart();
	}
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
