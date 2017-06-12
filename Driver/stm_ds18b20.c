#include "common.h"
#include "stm_ds18b20.h"

void DS18B20_GPIO_Init(void)
{
 	GPIO_InitTypeDef gpio_init_struct;
 	
 	gpio_init_struct.GPIO_Pin = GPIO_Pin_14;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
 	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_Init(GPIOD,&gpio_init_struct);

 	GPIO_SetBits(GPIOD,GPIO_Pin_14);
}

u8 DS18B20_InitSeq(void)
{
	u8 retry = 0;
	
	DS18B20_GPIO_OUT();
	GPIO_ResetBits(GPIOD,GPIO_Pin_14);			//����DQ����
	Delay_us(750);
	GPIO_SetBits(GPIOD,GPIO_Pin_14);				//�ͷ�����
	Delay_us(15);
	DS18B20_GPIO_IN(); 
  while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14)==1 && retry<200)
	{
		retry++;
		Delay_us(1);
	}	 
	if(retry>=200)
		return FAILURE;
	retry = 0;
  while(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14)==0 && retry<240)
	{
		retry++;
		Delay_us(1);
	}
	if(retry>=240)
		return FAILURE;
	
	return SUCCESS;
}

u8 DS18B20_ReadBit(void)
{
	DS18B20_GPIO_OUT();
  GPIO_ResetBits(GPIOD,GPIO_Pin_14);			//����DQ���� 
	Delay_us(2);
  GPIO_SetBits(GPIOD,GPIO_Pin_14);				//�ͷ�����
	DS18B20_GPIO_IN();
	Delay_us(12);
	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_14)==1)
	{
		Delay_us(50);
		return 1;
	}
  else
	{
		Delay_us(50);
		return 0;
	}
}

u8 DS18B20_ReadByte(void)
{
	u8 i, t, data = 0;
	
	for(i=0;i<8;i++)
	{
		t = DS18B20_ReadBit();
		data |= t<<i;
	}
	
	return data;
}

void DS18B20_WriteByte(u8 data)     
{             
  u8 i;
	
	DS18B20_GPIO_OUT();
  for(i=0;i<8;i++) 
	{
		if(data&(1<<i))
		{
			GPIO_ResetBits(GPIOD,GPIO_Pin_14);		//����DQ���� 
			Delay_us(2);                            
			GPIO_SetBits(GPIOD,GPIO_Pin_14);			//д1
			Delay_us(60);             
		}
		else 
		{
			GPIO_ResetBits(GPIOD,GPIO_Pin_14);		//д0
			Delay_us(60);             
			GPIO_SetBits(GPIOD,GPIO_Pin_14);			//�ͷ�����
			Delay_us(2);                          
		}
  }
}

u8 DS18B20_StartConvert(void)
{
	if(DS18B20_InitSeq()==FAILURE)			//��ʼ������
		return FAILURE;
	DS18B20_WriteByte(0xCC);						//Skip Rom
	DS18B20_WriteByte(0x44);						//��ʼ�¶�ת��
	
	return SUCCESS;
}

short DS18B20_GetTemperature(void)
{
	short temp;
	u16 data = 0;

	if(DS18B20_InitSeq()==FAILURE)					//��ʼ������
		return -128;													//�˴���-128����ʾ��ȡ�¶ȴ�����
  DS18B20_WriteByte(0xCC);								//Skip Rom
  DS18B20_WriteByte(0xBE);								//���洢��    
  data |= DS18B20_ReadByte();
  data |= DS18B20_ReadByte()<<8;
	if(data&0xF800)													//�ж�������
		temp = -(u16)((~data+1)*0.0625);
	else
		temp = (u16)(data*0.0625);
	
  if(DS18B20_StartConvert()==FAILURE)			//��ʼ�¶�ת��
		return -128;													//��ȡ�¶ȴ���
	
	return temp;    
}
