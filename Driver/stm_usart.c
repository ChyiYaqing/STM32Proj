#include "common.h"
#include "stm_usart.h"
#include "stm_valve.h"

u16 PC_USART_RX_STA;			//其每一位的定义为：bit15 - 接收完成标志；bit14 - 开始接收标志；bit13~0 - 接收数据的个数
u8 pc_usart_rx_buf[PC_USART_REC_LEN];
u8 *pc_usart_tx_buf;
u8 pc_usart_tx_count;
u8 rs485_usart_rx_buf;
u8 *rs485_usart_tx_buf;
u8 rs485_usart_tx_count;

void PC_USART_Init(u32 baud)
{
	GPIO_InitTypeDef gpio_init_struct;
	USART_InitTypeDef usart_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	
	/***** GPIO端口设置 USART1_TX *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_9;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&gpio_init_struct);
	
	/***** GPIO端口设置 USART1_RX *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_10;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&gpio_init_struct);
	
	/***** USART初始化设置 *****/
	usart_init_struct.USART_BaudRate = baud;
	usart_init_struct.USART_WordLength = USART_WordLength_8b;
	usart_init_struct.USART_StopBits = USART_StopBits_1;
	usart_init_struct.USART_Parity = USART_Parity_No;
	usart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init_struct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART1,&usart_init_struct);
	
	/***** NVIC中断设置 *****/
	nvic_init_struct.NVIC_IRQChannel = USART1_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);
	
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART1,USART_IT_TC,ENABLE);
	USART_Cmd(USART1,ENABLE);
}

void RS485_USART_Init(u32 baud)
{
	GPIO_InitTypeDef gpio_init_struct;
	USART_InitTypeDef usart_init_struct;
	NVIC_InitTypeDef nvic_init_struct;
	
	/***** GPIO端口设置 USART2_TX *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_2;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA,&gpio_init_struct);
	
	/***** GPIO端口设置 USART2_RX *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_3;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&gpio_init_struct);
	
	/***** USART初始化设置 *****/
	usart_init_struct.USART_BaudRate = baud;
	usart_init_struct.USART_WordLength = USART_WordLength_8b;
	usart_init_struct.USART_StopBits = USART_StopBits_1;
	usart_init_struct.USART_Parity = USART_Parity_No;
	usart_init_struct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_init_struct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART2,&usart_init_struct);
	
	/***** NVIC中断设置 *****/
	nvic_init_struct.NVIC_IRQChannel = USART2_IRQn;
	nvic_init_struct.NVIC_IRQChannelPreemptionPriority = 1;
	nvic_init_struct.NVIC_IRQChannelSubPriority = 0;
	nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init_struct);
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	USART_ITConfig(USART2,USART_IT_TC,ENABLE);
	USART_Cmd(USART2,ENABLE);
}

void RS485_GPIO_Init(void)
{
	GPIO_InitTypeDef gpio_init_struct;
	
	/***** GPIO端口设置 485 RDE *****/
	gpio_init_struct.GPIO_Pin = GPIO_Pin_3;
	gpio_init_struct.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init_struct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC,&gpio_init_struct);	
	
	GPIO_SetBits(GPIOC,GPIO_Pin_3);
}

void SendDataToPC(u8 *p_data)
{
	while(pc_usart_tx_buf!=NULL);
	pc_usart_tx_buf = &p_data[1];
	pc_usart_tx_count = p_data[1]+2;
	USART_ClearFlag(USART1,USART_FLAG_TC);
	USART_SendData(USART1,p_data[0]);
}

void SendCmdTo485(u8 *p_cmd, u8 len)
{
	while(rs485_usart_tx_buf!=NULL);
	GPIO_ResetBits(GPIOC,GPIO_Pin_3);			//485总线状态改为发送
	Delay_ms(2);
	rs485_usart_tx_buf = &p_cmd[1];
	rs485_usart_tx_count = len-1;
	USART_ClearFlag(USART2,USART_FLAG_TC);
	USART_SendData(USART2,p_cmd[0]);
	rs485_usart_rx_buf = 0;
}
