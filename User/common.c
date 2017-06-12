#include "common.h"

u16 current_flow_data[CURRENT_FLOW_DATA_LEN];
u16 cfd_avrg;
short current_ring_temp;
short current_pipeline_temp;
u16 FLAGS;			/* ��ÿһλ�Ķ���Ϊ��bit0-״̬֡���͵�PC�ɹ���־��bit1-������ʼ��־��bit2-ѡ������ڱ�־��bit3-GC��������ʱ�䲹����־��
					 bit4-����֡���͵�PC�ɹ���־��bit5-��������������(MFC)���ޱ�־��bit6-�������¿�ģ��(TCRing)���ޱ�־��
					bit7-DS18B20 OK��־��bit8-�����ڳ�ϴ��־��bit9-MFC OK��־��bit10-��·�¿�ģ��(TCPipeline)���ޱ�־�� */

u8 BCC(u8 *p_data, u8 len)
{
	u8 i, bcc = 0;

	for(i=0;i<len;i++)
		bcc ^= p_data[i];   //��򴫽����Ĳ��� 

	return bcc;
}

void BubbleSort_u16(u16 *p_data, u8 len)			//ð����������С�����˳������
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
	SysTick->VAL = 0x00;					//��ռ�����
	SysTick->CTRL = 0x01;					//ʹ�ܣ���ʼ����
	do
	{
		temp = SysTick->CTRL;				//��ȡ��ǰ������ֵ
	}while(temp&0x01 && !(temp&(1<<16)));					//�ȴ�ʱ�䵽
	
	SysTick->CTRL = 0x00;					//�رռ�����
	SysTick->VAL = 0x00;					//��ռ�����	 
}

void Delay_ms(u16 n)
{	 		  	  
	u32 temp;
	
	SysTick->LOAD = 9000*n;
	SysTick->VAL = 0x00;					//��ռ�����
	SysTick->CTRL = 0x01;					//ʹ�ܣ���ʼ����
	do
	{
		temp = SysTick->CTRL;					//��ȡ��ǰ������ֵ
	}while(temp&0x01 && !(temp&(1<<16)));					//�ȴ�ʱ�䵽
	
	SysTick->CTRL = 0x00;					//�رռ�����
	SysTick->VAL = 0x00;					//��ռ�����	  	    
}

void Delay_s(u16 n)
{
	for(;n>0;n--)
		Delay_ms(1000);
}
