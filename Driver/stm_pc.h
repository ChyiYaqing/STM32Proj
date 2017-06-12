#ifndef __STM_PC_H__
#define __STM_PC_H__

#include "common.h"

/***** ��������֡��־ *****/
#define STX			0x02			//����֡��ʼ��־
#define ETX			0x03			//����֡������־

/***** ����ָ��� *****/
#define CMD_ACK									0	 	//Ӧ��ָ��
#define CMD_PARAM_SET						1	 	//��������ָ��
#define CMD_SAMPLE_INLET_SET		2	 	//����������ָ��
#define CMD_PAUSE_SAMPLE				3	 	//��ͣ����ָ��
#define CMD_RESUME_SAMPLE				4	 	//��������ָ��
#define CMD_STOP_SAMPLE					5	 	//��ֹ����ָ��
#define CMD_STATUS							6	 	//״̬����ָ��
#define CMD_ERROR								7	 	//���󱨸�ָ��
#define CMD_SELF_CHECK					8		//�豸�Լ�ָ��
#define CMD_PIPELINE_WASH				9		//��·��ϴָ��
#define CMD_FLOW_SET						10	//��������ָ��
#define CMD_FLOW								11	//��������ָ��
#define CMD_RING_TEMP_SET				12	//�������¶�����ָ��
#define CMD_RING_TEMP						13	//�������¶ȱ���ָ��
#define CMD_SAMPLE_INLET_WASH		14	//�����ڳ�ϴָ��
#define CMD_START_SAMPLE				15	//��ʼ����ָ��
#define CMD_STOP_INLET_WASH			16	//��ֹ�����ڳ�ϴָ��
#define CMD_PIPELINE_TEMP_SET		17	//��·�¶�����ָ��
#define CMD_PIPELINE_TEMP				18	//��·�¶ȱ���ָ��

/***** �������� *****/
#define E_OK										0			//�޴���
#define E_VALVE10_FAULT					1			//ʮͨ������
#define E_VALVE16_FAULT					2			//ʮ��ͨ������
#define E_VALVE3_FAULT					3			//��ͨ������
#define E_VALVE2_FAULT					4			//��ͨ������
#define E_GC_NOT_READY					5			//GCδ׼������
#define E_PIPELINE_EQUIP_FAULT	6			//��·�豸���ϣ�������MFC���ϡ���ձù��ϻ��·����
#define E_DS18B20_ERROR					7			//DS18B20�����ڡ����ϻ�δ׼������
#define E_VALVE4_FAULT					8			//��ͨ������

/***** ����ṹ�� *****/
#pragma pack(push,1)

typedef struct
{
	u8 fa_start;			//֡ͷ
	u8 fa_len;				//���ݳ���
	u8 fa_cmd;				//ָ��
	u8 fa_data;				//����
	u8 fa_bcc;				//У��
	u8 fa_end;				//֡β
}FRAME_ACK, *P_FRAME_ACK;				//Ӧ��֡�ṹ��

typedef struct
{
	u8 fs_start;			//֡ͷ
	u8 fs_len;				//���ݳ���
	u8 fs_cmd;				//ָ��
	u8 fs_data;				//����
	u8 fs_bcc;				//У��
	u8 fs_end;				//֡β
}FRAME_STATUS, *P_FRAME_STATUS;			//״̬����֡�ṹ��

typedef struct
{
	u8 fe_start;			//֡ͷ
	u8 fe_len;				//���ݳ���
	u8 fe_cmd;				//ָ��
	u8 fe_data;				//����
	u8 fe_bcc;				//У��
	u8 fe_end;				//֡β
}FRAME_ERROR, *P_FRAME_ERROR;			//���󱨸�֡�ṹ��

typedef struct
{
	u8 	ff_start;			//֡ͷ
	u8 	ff_len;				//���ݳ���
	u8 	ff_cmd;				//ָ��
	u16 ff_data;			//����
	u8 	ff_bcc;				//У��
	u8 	ff_end;				//֡β
}FRAME_FLOW, *P_FRAME_FLOW;				//��������֡�ṹ��

typedef struct
{
	u8 		frt_start;			//֡ͷ
	u8 		frt_len;				//���ݳ���
	u8 		frt_cmd;				//ָ��
	short frt_data;				//����
	u8 		frt_bcc;				//У��
	u8 		frt_end;				//֡β
}FRAME_RING_TEMP, *P_FRAME_RING_TEMP;				//�������¶ȱ���֡�ṹ��

typedef struct
{
	u8 		fpt_start;			//֡ͷ
	u8 		fpt_len;				//���ݳ���
	u8 		fpt_cmd;				//ָ��
	short fpt_data;				//����
	u8 		fpt_bcc;				//У��
	u8 		fpt_end;				//֡β
}FRAME_PIPELINE_TEMP, *P_FRAME_PIPELINE_TEMP;				//��·�¶ȱ���֡�ṹ��

typedef struct
{
	u16 tp_sample_inlet_wash_time;			//�����ڳ�ϴʱ��
	u16 tp_pipeline_wash_time;					//��·��ϴʱ��
	u16 tp_ring_sample_time;						//����������ʱ��
	u16 tp_pressure_balance_time;				//ѹ��ƽ��ʱ��
	u16 tp_gc_analysis_time;						//GC����ʱ��
}TIME_PARAMETER, *P_TIME_PARAMETER;											//ʱ������ṹ��

typedef struct
{
	u32 si_inlet;												//������
	u8 si_sample_times[16];							//��������
}SAMPLE_INLET, *P_SAMPLE_INLET;													//�����ڽṹ��

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
