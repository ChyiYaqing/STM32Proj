#ifndef __COMMON_H__
#define __COMMON_H__

#include "stm32f10x.h"

#undef 	NULL
#define NULL 		0

#define TIME_BASE			2

#define SUCCESS			1
#define FAILURE			0

#define CURRENT_FLOW_DATA_LEN		20			//存放当前流量数据的数组长度
#define SAMPLE_INLET_NUM        30      //进样口数

typedef enum {FALSE,TRUE}BOOL;

extern u16 current_flow_data[CURRENT_FLOW_DATA_LEN];
extern u16 cfd_avrg;
extern short current_ring_temp;
extern short current_pipeline_temp;
extern u16 FLAGS;

u8 BCC(u8 *p_data, u8 len);
void BubbleSort_u16(u16 *p_data, u8 len);
u16 GetFlowData(void);
void Delay_us(u32 n);
void Delay_ms(u16 n);
void Delay_s(u16 n);

#endif
