#include "common.h"

u16 current_flow_data[CURRENT_FLOW_DATA_LEN];
u16 cfd_avrg;
short current_ring_temp;
short current_pipeline_temp;
u16 FLAGS;			/* 其每一位的定义为：bit0-状态帧发送到PC成功标志；bit1-进样开始标志；bit2-选择进样口标志；bit3-GC分析后期时间补偿标志；
					 bit4-错误帧发送到PC成功标志；bit5-质量流量控制器(MFC)有无标志；bit6-定量环温控模块(TCRing)有无标志；
					bit7-DS18B20 OK标志；bit8-进样口冲洗标志；bit9-MFC OK标志；bit10-管路温控模块(TCPipeline)有无标志； */

u8 BCC(u8 *p_data, u8 len)
{
	u8 i, bcc = 0;

	for(i=0;i<len;i++)
		bcc ^= p_data[i];   //异或传进来的参数 

	return bcc;
}

void BubbleSort_u16(u16 *p_data, u8 len)			//冒泡排序：以由小到大的顺序排列
{
	u8 i, j;
	u16 temp;

	for(i=0;i<len-1;i++)			            
	{                
		for(j=0;j<len-1-i;j++)                
		{                    
			if(p_data[j]>p_data[j+1])                    
			{                            
				temp = p_data[j];
				p_data[j] = p_data[j+1];
				p_data[j+1] = temp;               
			}    
		}      
	}
}

u16 GetFlowData(void)
{
	u8 i;
	u32	sum = 0;
	
	BubbleSort_u16(current_flow_data,CURRENT_FLOW_DATA_LEN);
	for(i=5;i<15;i++)
		sum += current_flow_data[i];
	
	return sum/10;
}

void Delay_us(u32 n)
{
	u32 temp;
	
	SysTick->LOAD = 9*n;
	SysTick->VAL = 0x00;					//清空计数器
	SysTick->CTRL = 0x01;					//使能，开始计数
	do
	{
		temp = SysTick->CTRL;				//读取当前倒计数值
	}while(temp&0x01 && !(temp&(1<<16)));					//等待时间到
	
	SysTick->CTRL = 0x00;					//关闭计数器
	SysTick->VAL = 0x00;					//清空计数器	 
}

void Delay_ms(u16 n)
{	 		  	  
	u32 temp;
	
	SysTick->LOAD = 9000*n;
	SysTick->VAL = 0x00;					//清空计数器
	SysTick->CTRL = 0x01;					//使能，开始计数
	do
	{
		temp = SysTick->CTRL;					//读取当前倒计数值
	}while(temp&0x01 && !(temp&(1<<16)));					//等待时间到
	
	SysTick->CTRL = 0x00;					//关闭计数器
	SysTick->VAL = 0x00;					//清空计数器	  	    
}

void Delay_s(u16 n)
{
	for(;n>0;n--)
		Delay_ms(1000);
}
