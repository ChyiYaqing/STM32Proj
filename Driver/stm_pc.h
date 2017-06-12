#ifndef __STM_PC_H__
#define __STM_PC_H__

#include "common.h"

/***** 定义数据帧标志 *****/
#define STX			0x02			//数据帧开始标志
#define ETX			0x03			//数据帧结束标志

/***** 定义指令宏 *****/
#define CMD_ACK									0	 	//应答指令
#define CMD_PARAM_SET						1	 	//参数设置指令
#define CMD_SAMPLE_INLET_SET		2	 	//进样口设置指令
#define CMD_PAUSE_SAMPLE				3	 	//暂停进样指令
#define CMD_RESUME_SAMPLE				4	 	//继续进样指令
#define CMD_STOP_SAMPLE					5	 	//终止进样指令
#define CMD_STATUS							6	 	//状态报告指令
#define CMD_ERROR								7	 	//错误报告指令
#define CMD_SELF_CHECK					8		//设备自检指令
#define CMD_PIPELINE_WASH				9		//管路冲洗指令
#define CMD_FLOW_SET						10	//流量设置指令
#define CMD_FLOW								11	//流量报告指令
#define CMD_RING_TEMP_SET				12	//定量环温度设置指令
#define CMD_RING_TEMP						13	//定量环温度报告指令
#define CMD_SAMPLE_INLET_WASH		14	//进样口冲洗指令
#define CMD_START_SAMPLE				15	//开始进样指令
#define CMD_STOP_INLET_WASH			16	//终止进样口冲洗指令
#define CMD_PIPELINE_TEMP_SET		17	//管路温度设置指令
#define CMD_PIPELINE_TEMP				18	//管路温度报告指令

/***** 定义错误宏 *****/
#define E_OK										0			//无错误
#define E_VALVE10_FAULT					1			//十通阀故障
#define E_VALVE16_FAULT					2			//十六通阀故障
#define E_VALVE3_FAULT					3			//三通阀故障
#define E_VALVE2_FAULT					4			//二通阀故障
#define E_GC_NOT_READY					5			//GC未准备就绪
#define E_PIPELINE_EQUIP_FAULT	6			//管路设备故障，包括：MFC故障、真空泵故障或管路堵塞
#define E_DS18B20_ERROR					7			//DS18B20不存在、故障或未准备就绪
#define E_VALVE4_FAULT					8			//四通阀故障

/***** 定义结构体 *****/
#pragma pack(push,1)

typedef struct
{
	u8 fa_start;			//帧头
	u8 fa_len;				//数据长度
	u8 fa_cmd;				//指令
	u8 fa_data;				//数据
	u8 fa_bcc;				//校验
	u8 fa_end;				//帧尾
}FRAME_ACK, *P_FRAME_ACK;				//应答帧结构体

typedef struct
{
	u8 fs_start;			//帧头
	u8 fs_len;				//数据长度
	u8 fs_cmd;				//指令
	u8 fs_data;				//数据
	u8 fs_bcc;				//校验
	u8 fs_end;				//帧尾
}FRAME_STATUS, *P_FRAME_STATUS;			//状态报告帧结构体

typedef struct
{
	u8 fe_start;			//帧头
	u8 fe_len;				//数据长度
	u8 fe_cmd;				//指令
	u8 fe_data;				//数据
	u8 fe_bcc;				//校验
	u8 fe_end;				//帧尾
}FRAME_ERROR, *P_FRAME_ERROR;			//错误报告帧结构体

typedef struct
{
	u8 	ff_start;			//帧头
	u8 	ff_len;				//数据长度
	u8 	ff_cmd;				//指令
	u16 ff_data;			//数据
	u8 	ff_bcc;				//校验
	u8 	ff_end;				//帧尾
}FRAME_FLOW, *P_FRAME_FLOW;				//流量报告帧结构体

typedef struct
{
	u8 		frt_start;			//帧头
	u8 		frt_len;				//数据长度
	u8 		frt_cmd;				//指令
	short frt_data;				//数据
	u8 		frt_bcc;				//校验
	u8 		frt_end;				//帧尾
}FRAME_RING_TEMP, *P_FRAME_RING_TEMP;				//定量环温度报告帧结构体

typedef struct
{
	u8 		fpt_start;			//帧头
	u8 		fpt_len;				//数据长度
	u8 		fpt_cmd;				//指令
	short fpt_data;				//数据
	u8 		fpt_bcc;				//校验
	u8 		fpt_end;				//帧尾
}FRAME_PIPELINE_TEMP, *P_FRAME_PIPELINE_TEMP;				//管路温度报告帧结构体

typedef struct
{
	u16 tp_sample_inlet_wash_time;			//进样口冲洗时间
	u16 tp_pipeline_wash_time;					//管路冲洗时间
	u16 tp_ring_sample_time;						//定量环进样时间
	u16 tp_pressure_balance_time;				//压力平衡时间
	u16 tp_gc_analysis_time;						//GC分析时间
}TIME_PARAMETER, *P_TIME_PARAMETER;											//时间参数结构体

typedef struct
{
	u32 si_inlet;												//进样口
	u8 si_sample_times[16];							//进样次数
}SAMPLE_INLET, *P_SAMPLE_INLET;													//进样口结构体

#pragma pack(pop)

extern FRAME_ACK f_ack;
extern FRAME_STATUS f_status;
extern FRAME_ERROR f_error;
extern FRAME_FLOW f_flow;
extern FRAME_RING_TEMP f_ring_temp;
extern FRAME_PIPELINE_TEMP f_pipeline_temp;
extern u8 resend_sf_times;
extern u8 resend_sf_delay_count;
extern u8 resend_ef_times;
extern u8 resend_ef_delay_count;
extern u8 send_ff_frq;

void SendAckFrame(void);
void SendStatusFrame(void);
void SendErrorFrame(void);
void SendFlowFrame(void);
void SendRingTempFrame(void);
void SendPipelineTempFrame(void);

#endif
