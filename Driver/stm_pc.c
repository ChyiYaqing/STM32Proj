#include "common.h"
#include "stm_usart.h"
#include "stm_pc.h"
#include "stm_pump.h"

FRAME_ACK f_ack;
FRAME_STATUS f_status;
FRAME_ERROR f_error;
FRAME_FLOW f_flow;
FRAME_RING_TEMP f_ring_temp;
FRAME_PIPELINE_TEMP f_pipeline_temp;
u8 resend_sf_times;
u8 resend_sf_delay_count;
u8 resend_ef_times;
u8 resend_ef_delay_count;
u8 send_ff_frq;

void SendAckFrame(void)
{
	f_ack.fa_bcc = BCC(&f_ack.fa_cmd,f_ack.fa_len-1);			//����У��ֵ
	SendDataToPC((u8*)&f_ack);							//����Ӧ��֡
}

void SendStatusFrame(void)
{
	f_status.fs_bcc = BCC(&f_status.fs_cmd,f_status.fs_len-1);			//����У��ֵ
	SendDataToPC((u8*)&f_status);						//����״̬֡
	resend_sf_delay_count = 3*TIME_BASE;
	FLAGS &= 0xFFFE;												//��״̬֡���ͳɹ���־
}

void SendErrorFrame(void)
{
	f_error.fe_bcc = BCC(&f_error.fe_cmd,f_error.fe_len-1);			//����У��ֵ
	SendDataToPC((u8*)&f_error);						//���ʹ���֡
	resend_ef_delay_count = 3*TIME_BASE;
	FLAGS &= 0xFFEF;												//�����֡���ͳɹ���־
	PUMP_STOP;
}

void SendFlowFrame(void)
{
	f_flow.ff_bcc = BCC(&f_flow.ff_cmd,f_flow.ff_len-1);				//����У��ֵ
	SendDataToPC((u8*)&f_flow);							//��������֡
}

void SendRingTempFrame(void)
{
	f_ring_temp.frt_bcc = BCC(&f_ring_temp.frt_cmd,f_ring_temp.frt_len-1);				//����У��ֵ
	SendDataToPC((u8*)&f_ring_temp);				//���Ͷ������¶�֡
}

void SendPipelineTempFrame(void)
{
	f_pipeline_temp.fpt_bcc = BCC(&f_pipeline_temp.fpt_cmd,f_pipeline_temp.fpt_len-1);				//����У��ֵ
	SendDataToPC((u8*)&f_pipeline_temp);		//���͹�·�¶�֡
}
