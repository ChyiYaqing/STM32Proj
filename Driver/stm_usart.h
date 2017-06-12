#ifndef __STM_USART_H__
#define __STM_USART_H__

#include "stm32f10x.h"

#define PC_USART_REC_LEN				80			//与PC端上位机通讯的串口接收最大长度

extern u16 PC_USART_RX_STA;
extern u8 pc_usart_rx_buf[PC_USART_REC_LEN];			//PC从串口上拿到的数据存放在此变量中
extern u8 *pc_usart_tx_buf;
extern u8 pc_usart_tx_count;
extern u8 rs485_usart_rx_buf;
extern u8 *rs485_usart_tx_buf;
extern u8 rs485_usart_tx_count;

void PC_USART_Init(u32 baud);
void RS485_USART_Init(u32 baud);
void RS485_GPIO_Init(void);
void SendDataToPC(u8 *p_data);
void SendCmdTo485(u8 *p_cmd, u8 len);

#endif
