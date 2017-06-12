#include "common.h"
#include "stm_sys.h"
#include "stm_usart.h"
#include "stm_pc.h"
#include "stm_valve.h"
#include "stm_gc.h"
#include "stm_led.h"
#include "stm_mfc.h"
#include "stm_timer.h"
#include "stm_ds18b20.h"
#include "stm_tc.h"
#include "stm_pump.h"

TIME_PARAMETER time_parameter;
SAMPLE_INLET sample_inlet;

void Initial(void);		//����ĺ���
u8 EquipSelfCheck(void);//�豸���Ҽ��
u8 PipelineWash(void);	//�ܵ�����
u8 SampleInletWash(void);//����������
u8 End(void);  //����
void Emergency(void);//ͻ�����

int main(void)
{
	short set_ring_temp, set_pipeline_temp;  //��������������¶ȣ����ùܵ��¶�
	u16 inlet_wash_delay_count = 0, sample_delay_count = 0, set_flow, set_flow_data;
	//��������ӳټ���            �������ӳټ���            ��������    ������������
	//��ȫ�����ݶ����޷���16λ�����ֵ��������ͣ�
	u8 i, vp, delay, sample_step = 0, sample_count = 0, get_ring_temp_delay_count = 0;
	// �޷���8λ���������ݣ���ʱ����Ʒ�Ĳ��裬��ȡ�¶ȵ��ӳټ���
	RCC_Config();									//����ϵͳʱ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//�����ж����ȼ�����
	PC_USART_Init(9600);					//��������λ��ͨѶ�Ĵ��ڲ�����Ϊ9600
	RS485_USART_Init(9600);					//������485����ͨѶ�Ĵ��ڲ�����Ϊ9600
	RS485_GPIO_Init();						//����485������ͨ����������ڵĳ�ʼ��
	GC_GPIO_Init();							//����ɫ������������ڵĳ�ʼ��
	LED_GPIO_Init();						//�̣ţĶ˿ڳ�ʼ�����ú����ڣ��䣮����ļ���
	MFC_ADC_Init();
	MFC_DAC_Init();
	Timer_Init();							//��ʱ����ʼ��
	DS18B20_GPIO_Init();					//ds_18b20�˿ڳ�ʼ��
	TC_GPIO_Init();								
	Pump_GPIO_Init();						//��ձõĶ˿ڳ�ʼ��
	Initial();
	
	f_status.fs_data = 0xFF;				//0xFF����״̬����֡��˵��һ����Ч�����ݣ����Դ�����Ϊ�豸������״ֵ̬
	for(i=1;i<4;i++)
	{
		SendStatusFrame();  //����״̬�򣬸ú�����pc.c��
		Delay_ms(100);
	}
	f_status.fs_data = 0x00;
	i = 0;	
	while(1)
	{
		if(PC_USART_RX_STA&0x8000)							//�ж���λ���Ƿ������ݹ���
		{
			if(BCC(&pc_usart_rx_buf[1],pc_usart_rx_buf[0]-1) == pc_usart_rx_buf[pc_usart_rx_buf[0]])			//�ж������Ƿ���Ч
			{
				switch(pc_usart_rx_buf[1])
				{
					case CMD_ACK:									//Ӧ��֡��Ӧ��ָ����Ϊ0���궨����stm_pc.h��
						switch(pc_usart_rx_buf[2])
						{
							case CMD_STATUS:      						//״̬����ָ�� 6
								FLAGS |= 0x0001;						//��״̬֡���ͳɹ���־
								break;
							case CMD_ERROR:								//���󱨸�ָ�� 7
								FLAGS |= 0x0010;						//�ô���֡���ͳɹ���־
								break;
						}
						break;
					case CMD_PARAM_SET:						//��������֡   1
						f_ack.fa_data = CMD_PARAM_SET;
						SendAckFrame();
						time_parameter = *((P_TIME_PARAMETER)&pc_usart_rx_buf[2]);			//��ȡʱ�����
						break;
					case CMD_SAMPLE_INLET_SET:		//����������֡
						f_ack.fa_data = CMD_SAMPLE_INLET_SET;
						SendAckFrame();
						sample_inlet = *((P_SAMPLE_INLET)&pc_usart_rx_buf[2]);					//��ȡ����������						
						break;
					case CMD_SAMPLE_INLET_WASH:		//�����ڳ�ϴ֡
						f_ack.fa_data = CMD_SAMPLE_INLET_WASH;
						SendAckFrame();
						LEDGreen();
						if(ValveGOn(VT_VALVE3,2,14,TRUE)==SUCCESS)			//��ͨ��ִ��GOָ���λ��1ת��2
						{
							i = 0;
							FLAGS |= 0x0100;							//�ý����ڳ�ϴ��־
						}
						else
							Emergency();	//ͻ��״��
						break;
					case CMD_START_SAMPLE:				//��ʼ����֡
						f_ack.fa_data = CMD_START_SAMPLE;
						SendAckFrame();
						i = 0;
						FLAGS |= 0x0002;								//�ý�����ʼ��־
						LEDGreen();
						PUMP_START;
						break;
					case CMD_FLOW_SET:						//��������֡
						f_ack.fa_data = CMD_FLOW_SET;
						SendAckFrame();
						set_flow = *((u16*)&pc_usart_rx_buf[2]);													//��ȡҪ���õ�����ֵ
						set_flow_data = (u16)((5.0*4095/(0.1*3.3))*(set_flow/1000.0));
						FLAGS |= 0x0020;								//��MFC���ޱ�־
						if(sample_step==3)
						{
							DAC_SetChannel1Data(DAC_Align_12b_R,set_flow_data);							//����MFC��ָ��������
							send_ff_frq = 5;							//5s�ϴ�һ������֡
						}
						else
							send_ff_frq = 5;							//5s�ϴ�һ������֡
						break;
					case CMD_RING_TEMP_SET:				//�������¶�����֡
						f_ack.fa_data = CMD_RING_TEMP_SET;
						SendAckFrame();
						set_ring_temp = *((short*)&pc_usart_rx_buf[2]);											//��ȡҪ���õ��¶�ֵ
						FLAGS |= 0x0040;								//��TCRing���ޱ�־
						get_ring_temp_delay_count = 3*TIME_BASE;
						break;					
					case CMD_PIPELINE_TEMP_SET:		//��·�¶�����֡
						f_ack.fa_data = CMD_PIPELINE_TEMP_SET;
						SendAckFrame();
						set_pipeline_temp = *((short*)&pc_usart_rx_buf[2]);									//��ȡҪ���õ��¶�ֵ
						FLAGS |= 0x0400;								//��TCPipeline���ޱ�־
						break;
					case CMD_SELF_CHECK:					//�豸�Լ�֡
						f_ack.fa_data = CMD_SELF_CHECK;
						SendAckFrame();
						LEDGreen();
						PUMP_START;
						if(EquipSelfCheck()==SUCCESS)
						{
							f_error.fe_data = E_OK;
							SendErrorFrame();
							resend_ef_times = 2;
							i = 0;
							inlet_wash_delay_count = 0;
							sample_step = 0;
							sample_count = 0;
							sample_delay_count = 0;
							resend_sf_times = 0;
							resend_sf_delay_count = 0;
							resend_ef_times = 0;
							resend_ef_delay_count = 0;
							FLAGS &= 0x06E0;
							f_status.fs_data = 0x00;				
							LEDBlue();
							PUMP_STOP;
						}
						break;
					case CMD_PIPELINE_WASH:				//��·��ϴ֡
						f_ack.fa_data = CMD_PIPELINE_WASH;
						SendAckFrame();
						LEDGreen();
						if(PipelineWash()==SUCCESS)
						{
							f_status.fs_data = 0x00;
							SendStatusFrame();
							resend_sf_times = 2;
							LEDBlue();
						}
						break;
					case CMD_PAUSE_SAMPLE:				//��ͣ����֡
						f_ack.fa_data = CMD_PAUSE_SAMPLE;
						SendAckFrame();
						if(End()==SUCCESS)
						{
							f_status.fs_data &= 0x10;			//����ͨ����ʮ��ͨ��״̬λΪ��������ʮͨ��״̬λΪB
							SendStatusFrame();
							resend_sf_times = 2;
							sample_step = 0;
							sample_delay_count = 0;
							LEDBlue();
							PUMP_STOP;
						}
						DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC
						FLAGS &= 0xFFFD;								//�������ʼ��־
						break;
					case CMD_RESUME_SAMPLE:				//��������֡
						f_ack.fa_data = CMD_RESUME_SAMPLE;
						SendAckFrame();
						FLAGS |= 0x0002;								//�ý�����ʼ��־
						LEDGreen();
						PUMP_START;
						break;
					case CMD_STOP_SAMPLE:					//��ֹ����֡
						f_ack.fa_data = CMD_STOP_SAMPLE;
						SendAckFrame();
						if(End()==SUCCESS)
						{
							f_status.fs_data &= 0x10;			//����ͨ����ʮ��ͨ��״̬λΪ��������ʮͨ��״̬λΪB
							SendStatusFrame();
							resend_sf_times = 2;
							i = 0;
							sample_step = 0;
							sample_count = 0;
							sample_delay_count = 0;
							LEDBlue();
							PUMP_STOP;
						}
						DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC
						FLAGS &= 0xFFFD;								//�������ʼ��־
						break;
					case CMD_STOP_INLET_WASH:			//��ֹ�����ڳ�ϴ֡
						f_ack.fa_data = CMD_STOP_INLET_WASH;
						SendAckFrame();
					
						/***** ���ص���ʼλ�� *****/
						if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��2ת��1
						{
							FLAGS &= 0xFEFF;								//������ڳ�ϴ��־
							break;
						}
						if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//ʮ��ͨ��ִ��GOָ��ת��λ��1
						{
							FLAGS &= 0xFEFF;								//������ڳ�ϴ��־
							break;
						}
						if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
						{
							FLAGS &= 0xFEFF;								//������ڳ�ϴ��־
							break;
						}
						f_status.fs_data = 0x00;
						SendStatusFrame();
						resend_sf_times = 2;
						i = 0;
						inlet_wash_delay_count = 0;
						FLAGS &= 0xFEFF;									//������ڳ�ϴ��־
						LEDBlue();
						break;
					default:
						break;
				}
			}
			PC_USART_RX_STA = 0;
		}
		if(FLAGS&0x0100)												//�ж��Ƿ�ʼ�����ڳ�ϴ
		{
			if(inlet_wash_delay_count==0)								//�ж�ʱ���Ƿ�
			{
				for(;i<SAMPLE_INLET_NUM;i++)
				{
					if(sample_inlet.si_inlet&(1<<i))				//�����Ҫ��ϴ�Ľ�����
					{
						vp = i/2+2;					//������������ڵķ�λ��
						if(ValveGOn(VT_VALVE16,vp,24,TRUE)==FAILURE)
						{
							Emergency();
							FLAGS &= 0xFEFF;							//������ڳ�ϴ��־
							break;
						}

						if((i+1)%2)					//�жϽ����ڵ�λ�ã��������������λ����Ȧ�������ż������λ����Ȧ
						{														
							if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
							{
								Emergency();
								FLAGS &= 0xFEFF;						//������ڳ�ϴ��־
								break;
							}						
						}
						else
						{														
							if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��B
							{
								Emergency();
								FLAGS &= 0xFEFF;						//������ڳ�ϴ��־
								break;
							}						
						}
						inlet_wash_delay_count = time_parameter.tp_sample_inlet_wash_time*TIME_BASE;
						break;
					}
				}
				if(i==SAMPLE_INLET_NUM)						//�ж����н������Ƿ��ϴ���
				{
					/***** ���ص���ʼλ�� *****/					
					if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��2ת��1
					{
						FLAGS &= 0xFEFF;								//������ڳ�ϴ��־
						continue;
					}
					if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//ʮ��ͨ��ִ��GOָ��ת��λ��1
					{
						FLAGS &= 0xFEFF;								//������ڳ�ϴ��־
						continue;
					}			
					if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
					{
						FLAGS &= 0xFEFF;								//������ڳ�ϴ��־
						continue;
					}
					f_status.fs_data = 0x00;
					SendStatusFrame();
					resend_sf_times = 2;
					i = 0;
					inlet_wash_delay_count = 0;
					FLAGS &= 0xFEFF;							//������ڳ�ϴ��־
					LEDBlue();
				}
			}
		}
		if(FLAGS&0x0002)											//�ж��Ƿ�ʼ����
		{
			if(sample_delay_count==0)						//�ж�ʱ���Ƿ�
			{
				if(sample_count==0)								//�ж�һ�������ڵĽ����Ƿ����
				{
					for(;i<SAMPLE_INLET_NUM;i++)
					{
						if(sample_inlet.si_inlet&(1<<i))				//�����һ����Ҫ�����Ľ�����
						{
							sample_count = (sample_inlet.si_sample_times[i/2]>>(i%2)*4)&0x0F;					//��ȡ�����ڵĽ�������
							FLAGS |= 0x0004;						//��ѡ������ڱ�־
							break;
						}
					}
				}
				if(i==SAMPLE_INLET_NUM)					//�ж����н������Ƿ�������
				{
					if(!(FLAGS&0x0008))							//�ж��Ƿ��н��й�GC��������ʱ�䲹��
					{
						sample_delay_count = (time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//����GC��������ʱ�䲹��
						FLAGS |= 0x0008;							//��GC��������ʱ�䲹����־
					}
					else
					{
						if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)						//��ͨ��ִ��GOָ���л���λ��A
							Emergency();
						else if(ValveGOn(VT_VALVE10,'B',5,TRUE)==SUCCESS)			//ʮͨ��ִ��GOָ���л���λ��B
						{
							f_status.fs_data &= 0xDF;		//��ʮͨ��״̬λΪB				
							SendStatusFrame();
							resend_sf_times = 2;
							i = 0;
							LEDBlue();
							PUMP_STOP;				
						}
						FLAGS &= 0xFFF5;							//�������ʼ��־����GC��������ʱ�䲹����־						
					}
				}
				else
				{
					sample_step++;
					switch(sample_step)
					{
						case 1:			//��ʼ��·��ϴ����ʱGC���ܻ��ڼ���������
							if(ValveGOn(VT_VALVE3,2,14,TRUE)==SUCCESS)			//��ͨ��ִ��GOָ���λ��1ת��2
							{
								f_status.fs_data &= 0x3F;			//����ͨ��״̬λΪ����
								f_status.fs_data |= 0x40;			
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = (time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//���ù�·��ϴʱ��
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;							//�������ʼ��־
							}
							break;
						case 2:			//��ʼ��·���ϴ��GC��������
							if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)						//��ͨ��ִ��GOָ���л���λ��A
							{
								Emergency();
								FLAGS &= 0xFFFD;							//�������ʼ��־
							}
							else if(ValveGOn(VT_VALVE10,'B',5,TRUE)==SUCCESS)			//ʮͨ��ִ��GOָ���л���λ��B
							{
								f_status.fs_data &= 0xDF;			//��ʮͨ��״̬λΪB
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = (time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//���ù�·���ϴʱ��
								if(FLAGS&0x0200)							//�ж�MFC�Ƿ�OK
									DAC_SetChannel1Data(DAC_Align_12b_R,set_flow_data);													//����MFC
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;							//�������ʼ��־
							}
							break;
						case 3:			//��ʼ����������
							if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��2ת��1
							{
								DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC																
								FLAGS &= 0xFFFD;							//�������ʼ��־
								break;
							}
														
							vp = i/2+2;			//������������ڵķ�λ��
							if(vp>9)																
								delay = (412+(vp-(vp%10+1)*2-1-1)*251+240)/100;			//vp�ں��Ȧ
							else								
								delay = (412+(vp-1-1)*251+240)/100;									//vp��ǰ��Ȧ
							if(ValveGOn(VT_VALVE16,vp,delay,TRUE)==FAILURE)
							{
								DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC																								
								FLAGS &= 0xFFFD;								//�������ʼ��־
								break;
							}							
							f_status.fs_data |= i/2+1;				//��ʮ��ͨ��״̬λΪ��Ӧ�Ľ�������
							
							if(FLAGS&0x0004)			//�ж��Ƿ�Ҫѡ��һ��������
							{
								if((i+1)%2)					//�жϽ����ڵ�λ�ã��������������λ����Ȧ�������ż������λ����Ȧ
								{									
									if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
									{
										DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC																														
										FLAGS &= 0xFFFD;						//�������ʼ��־
										break;
									}	
									f_status.fs_data &= 0xEF;			//�ö�ͨ��״̬λΪ��Ȧ
								}
								else
								{									
									if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��B
									{
										DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC																														
										FLAGS &= 0xFFFD;						//�������ʼ��־
										break;
									}
									f_status.fs_data |= 0x10;			//�ö�ͨ��״̬λΪ��Ȧ
								}
								FLAGS &= 0xFFFB;		//��ѡ������ڱ�־
							}
														
							if(ValveGOn(VT_VALVE3,4,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��1ת��4
							{
								Emergency();
								DAC_SetChannel1Data(DAC_Align_12b_R,0);				//�ر�MFC																								
								FLAGS &= 0xFFFD;								//�������ʼ��־
								break;
							}
							f_status.fs_data &= 0x3F;					//����ͨ��״̬λΪ��ձ�
							f_status.fs_data |= 0x80;
							SendStatusFrame();
							resend_sf_times = 2;
							sample_delay_count = time_parameter.tp_ring_sample_time*TIME_BASE;						//���ö���������ʱ��
							if(FLAGS&0x0200)									//�ж�MFC�Ƿ�OK
								send_ff_frq = 5;								//5s�ϴ�һ������֡
							break;
						case 4:			//��ʼѹ��ƽ��
							if(FLAGS&0x0020)									//�ж������䱸MFC
							{
								if(cfd_avrg==0)						//�жϹ�·���Ƿ�������
								{
									f_error.fe_data = E_PIPELINE_EQUIP_FAULT;
									SendErrorFrame();
									resend_ef_times = 2;
									LEDRed();
									Emergency();
									FLAGS &= 0xFFFD;							//�������ʼ��־
									break;
								}
							}
							if(V3V16Go1Together()==SUCCESS)		//��ͨ����ʮ��ͨ��ͬʱת��λ��1
							{
								f_status.fs_data &= 0x30;				//����ͨ����ʮ��ͨ��״̬λΪ����
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = time_parameter.tp_pressure_balance_time*TIME_BASE;			//����ѹ��ƽ��ʱ��
							}
							else																							
								FLAGS &= 0xFFFD;								//�������ʼ��־
							if(FLAGS&0x0200)									//�ж�MFC�Ƿ�OK
							{
								DAC_SetChannel1Data(DAC_Align_12b_R,0);
								send_ff_frq = 5;								//5s�ϴ�һ������֡
							}
							break;
						case 5:			//��ʼGC�������ȷ���һ���������е���Ʒ��
							if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)==0)			//�ж�GC�Ƿ�׼������
							{
								f_error.fe_data = E_GC_NOT_READY;
								SendErrorFrame();
								resend_ef_times = 2;
								LEDRed();
								FLAGS &= 0xFFFD;								//�������ʼ��־
								break;
							}						
							if(ValveGOn(VT_VALVE10,'A',5,TRUE)==SUCCESS)					//ʮͨ��ִ��GOָ���л���λ��A
							{
								f_status.fs_data |= 0x20;				//��ʮͨ��״̬λΪA
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = time_parameter.tp_gc_analysis_time*TIME_BASE;			//����GC����ʱ��
								GCStart();											//��GC����ʼ���������ź�
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;								//�������ʼ��־
							}
							break;
						case 6:			//����������һ���������е���Ʒ��
							if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)==0)			//�ж�GC�Ƿ�׼������
							{
								f_error.fe_data = E_GC_NOT_READY;
								SendErrorFrame();
								resend_ef_times = 2;
								LEDRed();
								FLAGS &= 0xFFFD;								//�������ʼ��־
								break;
							}
							if(ValveGOn(VT_VALVE4,'B',5,TRUE)==SUCCESS)					//��ͨ��ִ��GOָ���л���λ��B
							{
								sample_delay_count = (time_parameter.tp_gc_analysis_time-time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//����GC����ǰ��ʱ��
								GCStart();											//��GC����ʼ���������ź�
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;								//�������ʼ��־
							}	
							break;							
						default:		
							break;
					}
				}
			}
		}
		if(!(FLAGS&0x0001) && resend_sf_delay_count==0 && resend_sf_times!=0)			//�ж�״̬֡�����Ƿ�ɹ�
		{
			SendStatusFrame();
			resend_sf_times--;
		}
		if(!(FLAGS&0x0010) && resend_ef_delay_count==0 && resend_ef_times!=0)			//�жϴ���֡�����Ƿ�ɹ�
		{
			SendErrorFrame();
			resend_ef_times--;
		}
		Delay_ms(500);	
		if(resend_sf_delay_count>0)
			resend_sf_delay_count--;
		if(resend_ef_delay_count>0)
			resend_ef_delay_count--;
		if(inlet_wash_delay_count>0)
		{
			inlet_wash_delay_count--;
			if(inlet_wash_delay_count==0)
				i++;
		}
		if(sample_delay_count>0)
		{
			sample_delay_count--;
			if(sample_delay_count==0 && sample_step==6)
			{				
				sample_step = 0;							//׼����һ�ֽ���
				sample_count--;
				if(sample_count==0)						//�ж�һ�������ڵĽ����Ƿ����
					i++;
			}
		}
		if(FLAGS&0x0080)									//�ж�DS18B20�Ƿ�OK
		{
			if(get_ring_temp_delay_count>0)
			{
				get_ring_temp_delay_count--;
				if(get_ring_temp_delay_count==0)
				{
					current_ring_temp = DS18B20_GetTemperature();
					if(current_ring_temp<set_ring_temp)
						TCRingHeat();
					else if(current_ring_temp>set_ring_temp)
						TCRingStop();					
					get_ring_temp_delay_count = 1*TIME_BASE;		//ÿ����һ���¶�
				}
			}
		}
	}
}

void Initial(void)
{
	u8 i;
	
	PC_USART_RX_STA = 0;
	for(i=0;i<PC_USART_REC_LEN;i++)
		pc_usart_rx_buf[i] = 0;
	pc_usart_tx_buf = NULL;
	pc_usart_tx_count = 0;
	rs485_usart_rx_buf = 0;
	rs485_usart_tx_buf = NULL;
	rs485_usart_tx_count = 0;
	resend_sf_times = 0;
	resend_sf_delay_count = 0;
	resend_ef_times = 0;
	resend_ef_delay_count = 0;
	for(i=0;i<CURRENT_FLOW_DATA_LEN;i++)
		current_flow_data[i] = 0;	
	cfd_avrg = 0;
	current_ring_temp = 0;
	current_pipeline_temp = 0;
	FLAGS = 0;
	
	/***** Ӧ��֡��ʼ�� *****/
	f_ack.fa_start = STX;
	f_ack.fa_len = 3;
	f_ack.fa_cmd = CMD_ACK;
	f_ack.fa_end = ETX;
	
	/***** ״̬����֡��ʼ�� *****/
	f_status.fs_start = STX;
	f_status.fs_len = 3;
	f_status.fs_cmd = CMD_STATUS;
	f_status.fs_end = ETX;
	
	/***** ���󱨸�֡��ʼ�� *****/
	f_error.fe_start = STX;
	f_error.fe_len = 3;
	f_error.fe_cmd = CMD_ERROR;
	f_error.fe_end = ETX;
	
	/***** ��������֡��ʼ�� *****/
	f_flow.ff_start = STX;
	f_flow.ff_len = 4;
	f_flow.ff_cmd = CMD_FLOW;
	f_flow.ff_end = ETX;
	
	/***** �������¶ȱ���֡��ʼ�� *****/
	f_ring_temp.frt_start = STX;
	f_ring_temp.frt_len = 4;
	f_ring_temp.frt_cmd = CMD_RING_TEMP;
	f_ring_temp.frt_end = ETX;
	
	/***** ��·�¶ȱ���֡��ʼ�� *****/
	f_pipeline_temp.fpt_start = STX;
	f_pipeline_temp.fpt_len = 4;
	f_pipeline_temp.fpt_cmd = CMD_PIPELINE_TEMP;
	f_pipeline_temp.fpt_end = ETX;
}

u8 EquipSelfCheck(void)
{	
	FLAGS &= 0xFD7F;						//��MFC OK��־����DS18B20 OK��־
	
	/***** ���Ȼص���ʼλ�� *****/
	if(ValveGOn(VT_VALVE3,1,24,TRUE)==FAILURE)			//��ͨ��ִ��GOָ��ת��λ��1
		return FAILURE;
	if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//ʮ��ͨ��ִ��GOָ��ת��λ��1
		return FAILURE;	
	if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
		return FAILURE;
	if(ValveGOn(VT_VALVE10,'B',5,TRUE)==FAILURE)		//ʮͨ��ִ��GOָ���л���λ��B
		return FAILURE;
	if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
		return FAILURE;
	
	/***** ��ʼ���ļ�� *****/
	if(ValveCXn(VT_VALVE16,VCT_CW,16,42)==FAILURE)		//ʮ��ͨ��ִ��CWָ���λ��1ת��16
		return FAILURE;
	if(ValveCXn(VT_VALVE16,VCT_CW,1,7)==FAILURE)		//ʮ��ͨ��ִ��CWָ���λ��16ת��1
		return FAILURE;
	if(ValveCXn(VT_VALVE16,VCT_CC,2,42)==FAILURE)		//ʮ��ͨ��ִ��CCָ���λ��1ת��2
		return FAILURE;
	if(ValveCXn(VT_VALVE16,VCT_CC,1,7)==FAILURE)		//ʮ��ͨ��ִ��CCָ���λ��2ת��1
		return FAILURE;
	
	if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)				//��ͨ��ִ��GOָ���л���λ��B
		return FAILURE;
	if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)				//��ͨ��ִ��GOָ���л���λ��A
		return FAILURE;
	
	if(ValveGOn(VT_VALVE10,'A',5,TRUE)==FAILURE)			//ʮͨ��ִ��GOָ���л���λ��A
	{
		Emergency();
		return FAILURE;
	}
	if(ValveGOn(VT_VALVE10,'B',5,TRUE)==FAILURE)			//ʮͨ��ִ��GOָ���л���λ��B
		return FAILURE;
	
	if(ValveGOn(VT_VALVE4,'B',5,TRUE)==FAILURE)				//��ͨ��ִ��GOָ���л���λ��B
		return FAILURE;
	Delay_s(5);
	if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)				//��ͨ��ִ��GOָ���л���λ��A
		return FAILURE;
	
	if(ValveCXn(VT_VALVE3,VCT_CW,4,34)==FAILURE)			//��ͨ��ִ��CWָ���λ��1ת��4
		return FAILURE;
	if(ValveCXn(VT_VALVE3,VCT_CW,1,14)==FAILURE)			//��ͨ��ִ��CWָ���λ��4ת��1
		return FAILURE;
	if(ValveCXn(VT_VALVE3,VCT_CC,2,34)==FAILURE)			//��ͨ��ִ��CCָ���λ��1ת��2
	{
		Emergency();
		return FAILURE;
	}
	Delay_s(5);
	if(ValveCXn(VT_VALVE3,VCT_CC,1,14)==FAILURE)			//��ͨ��ִ��CCָ���λ��2ת��1
		return FAILURE;
	
	if(FLAGS&0x0020)							//�ж������䱸MFC
	{	
		/***** ��ʼ��·�豸�ļ�� *****/
		DAC_SetChannel1Data(DAC_Align_12b_R,4095);			//����MFC
		if(ValveGOn(VT_VALVE3,4,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��1ת��4
			return FAILURE;
		Delay_s(5);
		DAC_SetChannel1Data(DAC_Align_12b_R,0);					//�ر�MFC
		if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��4ת��1
			return FAILURE;
		if(cfd_avrg==0)							//�жϹ�·���Ƿ�������
		{
			f_error.fe_data = E_PIPELINE_EQUIP_FAULT;
			SendErrorFrame();
			resend_ef_times = 2;
			LEDRed();
			return FAILURE;
		}
		FLAGS |= 0x0200;						//��MFC OK��־
	}
	
	if(FLAGS&0x0040)							//�ж������䱸TCRing
	{	
		/***** ��ʼDS18B20�ļ�� *****/
		if(DS18B20_GetTemperature() == -128)
		{
			f_error.fe_data = E_DS18B20_ERROR;
			SendErrorFrame();
			resend_ef_times = 2;
			LEDRed();
			return FAILURE;
		}
		FLAGS |= 0x0080;						//��DS18B20 OK��־
	}
	
	return SUCCESS;
}

u8 PipelineWash(void)
{
	/***** ��·��ϴ *****/
	if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��B
		return FAILURE;
	if(ValveGOn(VT_VALVE3,2,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��1ת��2
	{
		Emergency();
		return FAILURE;
	}
	Delay_s(time_parameter.tp_pipeline_wash_time);
	if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
	{
		Emergency();
		return FAILURE;
	}
	Delay_s(time_parameter.tp_pipeline_wash_time);
	
	/***** ���ص���ʼλ�� *****/
	if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���λ��2ת��1
		return FAILURE;
	
	return SUCCESS;
}

u8 End(void)
{
	if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//��ͨ��ִ��GOָ��ת��λ��1
	{
		Emergency();
		return FAILURE;
	}
	if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//ʮ��ͨ��ִ��GOָ��ת��λ��1
	{
		Emergency();
		return FAILURE;
	}
	if(ValveGOn(VT_VALVE10,'B',5,TRUE)==FAILURE)		//ʮͨ��ִ��GOָ���л���λ��B
		return FAILURE;
	if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)			//��ͨ��ִ��GOָ���л���λ��A
		return FAILURE;
	
	return SUCCESS;
}

void Emergency(void)
{
	ValveGOn(VT_VALVE3,1,14,FALSE);				//��ͨ��ִ��GOָ��ת��λ��1
	ValveGOn(VT_VALVE10,'B',5,FALSE);			//ʮͨ��ִ��GOָ���л���λ��B
}
