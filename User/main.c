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

void Initial(void);		//最初的函数
u8 EquipSelfCheck(void);//设备自我检查
u8 PipelineWash(void);	//管道清理
u8 SampleInletWash(void);//进样口清理
u8 End(void);  //结束
void Emergency(void);//突发情况

int main(void)
{
	short set_ring_temp, set_pipeline_temp;  //定义变量：设置温度，设置管道温度
	u16 inlet_wash_delay_count = 0, sample_delay_count = 0, set_flow, set_flow_data;
	//入口清理延迟计数            进样口延迟计数            设置流量    设置流量数据
	//（全部数据都是无符号16位二进种的数据类型）
	u8 i, vp, delay, sample_step = 0, sample_count = 0, get_ring_temp_delay_count = 0;
	// 无符号8位二进制数据：延时，样品的步骤，获取温度的延迟计数
	RCC_Config();									//配置系统时钟
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//设置中断优先级分组
	PC_USART_Init(9600);					//设置与上位机通讯的串口波特率为9600
	RS485_USART_Init(9600);					//设置与485总线通讯的串口波特率为9600
	RS485_GPIO_Init();						//设置485总线与通用输入输出口的初始化
	GC_GPIO_Init();							//气相色谱中输入输出口的初始化
	LED_GPIO_Init();						//ＬＥＤ端口初始化，该函数在ｌｅｄ．ｃ的文件中
	MFC_ADC_Init();
	MFC_DAC_Init();
	Timer_Init();							//定时器初始化
	DS18B20_GPIO_Init();					//ds_18b20端口初始化
	TC_GPIO_Init();								
	Pump_GPIO_Init();						//真空泵的端口初始化
	Initial();
	
	f_status.fs_data = 0xFF;				//0xFF对于状态报告帧来说是一个无效的数据，故以此来作为设备启动的状态值
	for(i=1;i<4;i++)
	{
		SendStatusFrame();  //发送状态框，该函数在pc.c中
		Delay_ms(100);
	}
	f_status.fs_data = 0x00;
	i = 0;	
	while(1)
	{
		if(PC_USART_RX_STA&0x8000)							//判断上位机是否有数据过来
		{
			if(BCC(&pc_usart_rx_buf[1],pc_usart_rx_buf[0]-1) == pc_usart_rx_buf[pc_usart_rx_buf[0]])			//判断数据是否有效
			{
				switch(pc_usart_rx_buf[1])
				{
					case CMD_ACK:									//应答帧，应答指令设为0，宏定义在stm_pc.h中
						switch(pc_usart_rx_buf[2])
						{
							case CMD_STATUS:      						//状态报告指令 6
								FLAGS |= 0x0001;						//置状态帧发送成功标志
								break;
							case CMD_ERROR:								//错误报告指令 7
								FLAGS |= 0x0010;						//置错误帧发送成功标志
								break;
						}
						break;
					case CMD_PARAM_SET:						//参数设置帧   1
						f_ack.fa_data = CMD_PARAM_SET;
						SendAckFrame();
						time_parameter = *((P_TIME_PARAMETER)&pc_usart_rx_buf[2]);			//获取时间参数
						break;
					case CMD_SAMPLE_INLET_SET:		//进样口设置帧
						f_ack.fa_data = CMD_SAMPLE_INLET_SET;
						SendAckFrame();
						sample_inlet = *((P_SAMPLE_INLET)&pc_usart_rx_buf[2]);					//获取进样口数据						
						break;
					case CMD_SAMPLE_INLET_WASH:		//进样口冲洗帧
						f_ack.fa_data = CMD_SAMPLE_INLET_WASH;
						SendAckFrame();
						LEDGreen();
						if(ValveGOn(VT_VALVE3,2,14,TRUE)==SUCCESS)			//三通阀执行GO指令从位置1转到2
						{
							i = 0;
							FLAGS |= 0x0100;							//置进样口冲洗标志
						}
						else
							Emergency();	//突发状况
						break;
					case CMD_START_SAMPLE:				//开始进样帧
						f_ack.fa_data = CMD_START_SAMPLE;
						SendAckFrame();
						i = 0;
						FLAGS |= 0x0002;								//置进样开始标志
						LEDGreen();
						PUMP_START;
						break;
					case CMD_FLOW_SET:						//流量设置帧
						f_ack.fa_data = CMD_FLOW_SET;
						SendAckFrame();
						set_flow = *((u16*)&pc_usart_rx_buf[2]);													//获取要设置的流量值
						set_flow_data = (u16)((5.0*4095/(0.1*3.3))*(set_flow/1000.0));
						FLAGS |= 0x0020;								//置MFC有无标志
						if(sample_step==3)
						{
							DAC_SetChannel1Data(DAC_Align_12b_R,set_flow_data);							//设置MFC到指定的流量
							send_ff_frq = 5;							//5s上传一次流量帧
						}
						else
							send_ff_frq = 5;							//5s上传一次流量帧
						break;
					case CMD_RING_TEMP_SET:				//定量环温度设置帧
						f_ack.fa_data = CMD_RING_TEMP_SET;
						SendAckFrame();
						set_ring_temp = *((short*)&pc_usart_rx_buf[2]);											//获取要设置的温度值
						FLAGS |= 0x0040;								//置TCRing有无标志
						get_ring_temp_delay_count = 3*TIME_BASE;
						break;					
					case CMD_PIPELINE_TEMP_SET:		//管路温度设置帧
						f_ack.fa_data = CMD_PIPELINE_TEMP_SET;
						SendAckFrame();
						set_pipeline_temp = *((short*)&pc_usart_rx_buf[2]);									//获取要设置的温度值
						FLAGS |= 0x0400;								//置TCPipeline有无标志
						break;
					case CMD_SELF_CHECK:					//设备自检帧
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
					case CMD_PIPELINE_WASH:				//管路冲洗帧
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
					case CMD_PAUSE_SAMPLE:				//暂停进样帧
						f_ack.fa_data = CMD_PAUSE_SAMPLE;
						SendAckFrame();
						if(End()==SUCCESS)
						{
							f_status.fs_data &= 0x10;			//置三通阀和十六通阀状态位为大气，置十通阀状态位为B
							SendStatusFrame();
							resend_sf_times = 2;
							sample_step = 0;
							sample_delay_count = 0;
							LEDBlue();
							PUMP_STOP;
						}
						DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC
						FLAGS &= 0xFFFD;								//清进样开始标志
						break;
					case CMD_RESUME_SAMPLE:				//继续进样帧
						f_ack.fa_data = CMD_RESUME_SAMPLE;
						SendAckFrame();
						FLAGS |= 0x0002;								//置进样开始标志
						LEDGreen();
						PUMP_START;
						break;
					case CMD_STOP_SAMPLE:					//终止进样帧
						f_ack.fa_data = CMD_STOP_SAMPLE;
						SendAckFrame();
						if(End()==SUCCESS)
						{
							f_status.fs_data &= 0x10;			//置三通阀和十六通阀状态位为大气，置十通阀状态位为B
							SendStatusFrame();
							resend_sf_times = 2;
							i = 0;
							sample_step = 0;
							sample_count = 0;
							sample_delay_count = 0;
							LEDBlue();
							PUMP_STOP;
						}
						DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC
						FLAGS &= 0xFFFD;								//清进样开始标志
						break;
					case CMD_STOP_INLET_WASH:			//终止进样口冲洗帧
						f_ack.fa_data = CMD_STOP_INLET_WASH;
						SendAckFrame();
					
						/***** 阀回到初始位置 *****/
						if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置2转到1
						{
							FLAGS &= 0xFEFF;								//清进样口冲洗标志
							break;
						}
						if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//十六通阀执行GO指令转到位置1
						{
							FLAGS &= 0xFEFF;								//清进样口冲洗标志
							break;
						}
						if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置A
						{
							FLAGS &= 0xFEFF;								//清进样口冲洗标志
							break;
						}
						f_status.fs_data = 0x00;
						SendStatusFrame();
						resend_sf_times = 2;
						i = 0;
						inlet_wash_delay_count = 0;
						FLAGS &= 0xFEFF;									//清进样口冲洗标志
						LEDBlue();
						break;
					default:
						break;
				}
			}
			PC_USART_RX_STA = 0;
		}
		if(FLAGS&0x0100)												//判断是否开始进样口冲洗
		{
			if(inlet_wash_delay_count==0)								//判断时间是否到
			{
				for(;i<SAMPLE_INLET_NUM;i++)
				{
					if(sample_inlet.si_inlet&(1<<i))				//检查需要冲洗的进样口
					{
						vp = i/2+2;					//计算进样口所在的阀位置
						if(ValveGOn(VT_VALVE16,vp,24,TRUE)==FAILURE)
						{
							Emergency();
							FLAGS &= 0xFEFF;							//清进样口冲洗标志
							break;
						}

						if((i+1)%2)					//判断进样口的位置：如果是奇数，则位于下圈；如果是偶数，则位于上圈
						{														
							if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置A
							{
								Emergency();
								FLAGS &= 0xFEFF;						//清进样口冲洗标志
								break;
							}						
						}
						else
						{														
							if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置B
							{
								Emergency();
								FLAGS &= 0xFEFF;						//清进样口冲洗标志
								break;
							}						
						}
						inlet_wash_delay_count = time_parameter.tp_sample_inlet_wash_time*TIME_BASE;
						break;
					}
				}
				if(i==SAMPLE_INLET_NUM)						//判断所有进样口是否冲洗完成
				{
					/***** 阀回到初始位置 *****/					
					if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置2转到1
					{
						FLAGS &= 0xFEFF;								//清进样口冲洗标志
						continue;
					}
					if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//十六通阀执行GO指令转到位置1
					{
						FLAGS &= 0xFEFF;								//清进样口冲洗标志
						continue;
					}			
					if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置A
					{
						FLAGS &= 0xFEFF;								//清进样口冲洗标志
						continue;
					}
					f_status.fs_data = 0x00;
					SendStatusFrame();
					resend_sf_times = 2;
					i = 0;
					inlet_wash_delay_count = 0;
					FLAGS &= 0xFEFF;							//清进样口冲洗标志
					LEDBlue();
				}
			}
		}
		if(FLAGS&0x0002)											//判断是否开始进样
		{
			if(sample_delay_count==0)						//判断时间是否到
			{
				if(sample_count==0)								//判断一个进样口的进样是否结束
				{
					for(;i<SAMPLE_INLET_NUM;i++)
					{
						if(sample_inlet.si_inlet&(1<<i))				//检查下一个需要进样的进样口
						{
							sample_count = (sample_inlet.si_sample_times[i/2]>>(i%2)*4)&0x0F;					//获取进样口的进样次数
							FLAGS |= 0x0004;						//置选择进样口标志
							break;
						}
					}
				}
				if(i==SAMPLE_INLET_NUM)					//判断所有进样口是否进样完成
				{
					if(!(FLAGS&0x0008))							//判断是否有进行过GC分析后期时间补偿
					{
						sample_delay_count = (time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//设置GC分析后期时间补偿
						FLAGS |= 0x0008;							//置GC分析后期时间补偿标志
					}
					else
					{
						if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)						//四通阀执行GO指令切换到位置A
							Emergency();
						else if(ValveGOn(VT_VALVE10,'B',5,TRUE)==SUCCESS)			//十通阀执行GO指令切换到位置B
						{
							f_status.fs_data &= 0xDF;		//置十通阀状态位为B				
							SendStatusFrame();
							resend_sf_times = 2;
							i = 0;
							LEDBlue();
							PUMP_STOP;				
						}
						FLAGS &= 0xFFF5;							//清进样开始标志，清GC分析后期时间补偿标志						
					}
				}
				else
				{
					sample_step++;
					switch(sample_step)
					{
						case 1:			//开始管路冲洗，此时GC可能还在继续分析中
							if(ValveGOn(VT_VALVE3,2,14,TRUE)==SUCCESS)			//三通阀执行GO指令从位置1转到2
							{
								f_status.fs_data &= 0x3F;			//置三通阀状态位为氮气
								f_status.fs_data |= 0x40;			
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = (time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//设置管路冲洗时间
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;							//清进样开始标志
							}
							break;
						case 2:			//开始管路后冲洗，GC分析结束
							if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)						//四通阀执行GO指令切换到位置A
							{
								Emergency();
								FLAGS &= 0xFFFD;							//清进样开始标志
							}
							else if(ValveGOn(VT_VALVE10,'B',5,TRUE)==SUCCESS)			//十通阀执行GO指令切换到位置B
							{
								f_status.fs_data &= 0xDF;			//置十通阀状态位为B
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = (time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//设置管路后冲洗时间
								if(FLAGS&0x0200)							//判断MFC是否OK
									DAC_SetChannel1Data(DAC_Align_12b_R,set_flow_data);													//开启MFC
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;							//清进样开始标志
							}
							break;
						case 3:			//开始定量环进样
							if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置2转到1
							{
								DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC																
								FLAGS &= 0xFFFD;							//清进样开始标志
								break;
							}
														
							vp = i/2+2;			//计算进样口所在的阀位置
							if(vp>9)																
								delay = (412+(vp-(vp%10+1)*2-1-1)*251+240)/100;			//vp在后半圈
							else								
								delay = (412+(vp-1-1)*251+240)/100;									//vp在前半圈
							if(ValveGOn(VT_VALVE16,vp,delay,TRUE)==FAILURE)
							{
								DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC																								
								FLAGS &= 0xFFFD;								//清进样开始标志
								break;
							}							
							f_status.fs_data |= i/2+1;				//置十六通阀状态位为相应的进样口组
							
							if(FLAGS&0x0004)			//判断是否要选择一个进样口
							{
								if((i+1)%2)					//判断进样口的位置：如果是奇数，则位于下圈；如果是偶数，则位于上圈
								{									
									if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置A
									{
										DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC																														
										FLAGS &= 0xFFFD;						//清进样开始标志
										break;
									}	
									f_status.fs_data &= 0xEF;			//置二通阀状态位为下圈
								}
								else
								{									
									if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置B
									{
										DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC																														
										FLAGS &= 0xFFFD;						//清进样开始标志
										break;
									}
									f_status.fs_data |= 0x10;			//置二通阀状态位为上圈
								}
								FLAGS &= 0xFFFB;		//清选择进样口标志
							}
														
							if(ValveGOn(VT_VALVE3,4,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置1转到4
							{
								Emergency();
								DAC_SetChannel1Data(DAC_Align_12b_R,0);				//关闭MFC																								
								FLAGS &= 0xFFFD;								//清进样开始标志
								break;
							}
							f_status.fs_data &= 0x3F;					//置三通阀状态位为真空泵
							f_status.fs_data |= 0x80;
							SendStatusFrame();
							resend_sf_times = 2;
							sample_delay_count = time_parameter.tp_ring_sample_time*TIME_BASE;						//设置定量环进样时间
							if(FLAGS&0x0200)									//判断MFC是否OK
								send_ff_frq = 5;								//5s上传一次流量帧
							break;
						case 4:			//开始压力平衡
							if(FLAGS&0x0020)									//判断有无配备MFC
							{
								if(cfd_avrg==0)						//判断管路上是否有流量
								{
									f_error.fe_data = E_PIPELINE_EQUIP_FAULT;
									SendErrorFrame();
									resend_ef_times = 2;
									LEDRed();
									Emergency();
									FLAGS &= 0xFFFD;							//清进样开始标志
									break;
								}
							}
							if(V3V16Go1Together()==SUCCESS)		//三通阀和十六通阀同时转向位置1
							{
								f_status.fs_data &= 0x30;				//置三通阀和十六通阀状态位为大气
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = time_parameter.tp_pressure_balance_time*TIME_BASE;			//设置压力平衡时间
							}
							else																							
								FLAGS &= 0xFFFD;								//清进样开始标志
							if(FLAGS&0x0200)									//判断MFC是否OK
							{
								DAC_SetChannel1Data(DAC_Align_12b_R,0);
								send_ff_frq = 5;								//5s上传一次流量帧
							}
							break;
						case 5:			//开始GC分析，先分析一个定量环中的样品气
							if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)==0)			//判断GC是否准备就绪
							{
								f_error.fe_data = E_GC_NOT_READY;
								SendErrorFrame();
								resend_ef_times = 2;
								LEDRed();
								FLAGS &= 0xFFFD;								//清进样开始标志
								break;
							}						
							if(ValveGOn(VT_VALVE10,'A',5,TRUE)==SUCCESS)					//十通阀执行GO指令切换到位置A
							{
								f_status.fs_data |= 0x20;				//置十通阀状态位为A
								SendStatusFrame();
								resend_sf_times = 2;
								sample_delay_count = time_parameter.tp_gc_analysis_time*TIME_BASE;			//设置GC分析时间
								GCStart();											//向GC发开始进样分析信号
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;								//清进样开始标志
							}
							break;
						case 6:			//继续分析下一个定量环中的样品气
							if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_12)==0)			//判断GC是否准备就绪
							{
								f_error.fe_data = E_GC_NOT_READY;
								SendErrorFrame();
								resend_ef_times = 2;
								LEDRed();
								FLAGS &= 0xFFFD;								//清进样开始标志
								break;
							}
							if(ValveGOn(VT_VALVE4,'B',5,TRUE)==SUCCESS)					//四通阀执行GO指令切换到位置B
							{
								sample_delay_count = (time_parameter.tp_gc_analysis_time-time_parameter.tp_pipeline_wash_time/2)*TIME_BASE;			//设置GC分析前期时间
								GCStart();											//向GC发开始进样分析信号
							}
							else
							{
								Emergency();
								FLAGS &= 0xFFFD;								//清进样开始标志
							}	
							break;							
						default:		
							break;
					}
				}
			}
		}
		if(!(FLAGS&0x0001) && resend_sf_delay_count==0 && resend_sf_times!=0)			//判断状态帧发送是否成功
		{
			SendStatusFrame();
			resend_sf_times--;
		}
		if(!(FLAGS&0x0010) && resend_ef_delay_count==0 && resend_ef_times!=0)			//判断错误帧发送是否成功
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
				sample_step = 0;							//准备新一轮进样
				sample_count--;
				if(sample_count==0)						//判断一个进样口的进样是否结束
					i++;
			}
		}
		if(FLAGS&0x0080)									//判断DS18B20是否OK
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
					get_ring_temp_delay_count = 1*TIME_BASE;		//每秒检测一次温度
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
	
	/***** 应答帧初始化 *****/
	f_ack.fa_start = STX;
	f_ack.fa_len = 3;
	f_ack.fa_cmd = CMD_ACK;
	f_ack.fa_end = ETX;
	
	/***** 状态报告帧初始化 *****/
	f_status.fs_start = STX;
	f_status.fs_len = 3;
	f_status.fs_cmd = CMD_STATUS;
	f_status.fs_end = ETX;
	
	/***** 错误报告帧初始化 *****/
	f_error.fe_start = STX;
	f_error.fe_len = 3;
	f_error.fe_cmd = CMD_ERROR;
	f_error.fe_end = ETX;
	
	/***** 流量报告帧初始化 *****/
	f_flow.ff_start = STX;
	f_flow.ff_len = 4;
	f_flow.ff_cmd = CMD_FLOW;
	f_flow.ff_end = ETX;
	
	/***** 定量环温度报告帧初始化 *****/
	f_ring_temp.frt_start = STX;
	f_ring_temp.frt_len = 4;
	f_ring_temp.frt_cmd = CMD_RING_TEMP;
	f_ring_temp.frt_end = ETX;
	
	/***** 管路温度报告帧初始化 *****/
	f_pipeline_temp.fpt_start = STX;
	f_pipeline_temp.fpt_len = 4;
	f_pipeline_temp.fpt_cmd = CMD_PIPELINE_TEMP;
	f_pipeline_temp.fpt_end = ETX;
}

u8 EquipSelfCheck(void)
{	
	FLAGS &= 0xFD7F;						//清MFC OK标志，清DS18B20 OK标志
	
	/***** 阀先回到初始位置 *****/
	if(ValveGOn(VT_VALVE3,1,24,TRUE)==FAILURE)			//三通阀执行GO指令转到位置1
		return FAILURE;
	if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//十六通阀执行GO指令转到位置1
		return FAILURE;	
	if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置A
		return FAILURE;
	if(ValveGOn(VT_VALVE10,'B',5,TRUE)==FAILURE)		//十通阀执行GO指令切换到位置B
		return FAILURE;
	if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)			//四通阀执行GO指令切换到位置A
		return FAILURE;
	
	/***** 开始阀的检查 *****/
	if(ValveCXn(VT_VALVE16,VCT_CW,16,42)==FAILURE)		//十六通阀执行CW指令从位置1转到16
		return FAILURE;
	if(ValveCXn(VT_VALVE16,VCT_CW,1,7)==FAILURE)		//十六通阀执行CW指令从位置16转到1
		return FAILURE;
	if(ValveCXn(VT_VALVE16,VCT_CC,2,42)==FAILURE)		//十六通阀执行CC指令从位置1转到2
		return FAILURE;
	if(ValveCXn(VT_VALVE16,VCT_CC,1,7)==FAILURE)		//十六通阀执行CC指令从位置2转到1
		return FAILURE;
	
	if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)				//二通阀执行GO指令切换到位置B
		return FAILURE;
	if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)				//二通阀执行GO指令切换到位置A
		return FAILURE;
	
	if(ValveGOn(VT_VALVE10,'A',5,TRUE)==FAILURE)			//十通阀执行GO指令切换到位置A
	{
		Emergency();
		return FAILURE;
	}
	if(ValveGOn(VT_VALVE10,'B',5,TRUE)==FAILURE)			//十通阀执行GO指令切换到位置B
		return FAILURE;
	
	if(ValveGOn(VT_VALVE4,'B',5,TRUE)==FAILURE)				//四通阀执行GO指令切换到位置B
		return FAILURE;
	Delay_s(5);
	if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)				//四通阀执行GO指令切换到位置A
		return FAILURE;
	
	if(ValveCXn(VT_VALVE3,VCT_CW,4,34)==FAILURE)			//三通阀执行CW指令从位置1转到4
		return FAILURE;
	if(ValveCXn(VT_VALVE3,VCT_CW,1,14)==FAILURE)			//三通阀执行CW指令从位置4转到1
		return FAILURE;
	if(ValveCXn(VT_VALVE3,VCT_CC,2,34)==FAILURE)			//三通阀执行CC指令从位置1转到2
	{
		Emergency();
		return FAILURE;
	}
	Delay_s(5);
	if(ValveCXn(VT_VALVE3,VCT_CC,1,14)==FAILURE)			//三通阀执行CC指令从位置2转到1
		return FAILURE;
	
	if(FLAGS&0x0020)							//判断有无配备MFC
	{	
		/***** 开始管路设备的检查 *****/
		DAC_SetChannel1Data(DAC_Align_12b_R,4095);			//开启MFC
		if(ValveGOn(VT_VALVE3,4,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置1转到4
			return FAILURE;
		Delay_s(5);
		DAC_SetChannel1Data(DAC_Align_12b_R,0);					//关闭MFC
		if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置4转到1
			return FAILURE;
		if(cfd_avrg==0)							//判断管路上是否有流量
		{
			f_error.fe_data = E_PIPELINE_EQUIP_FAULT;
			SendErrorFrame();
			resend_ef_times = 2;
			LEDRed();
			return FAILURE;
		}
		FLAGS |= 0x0200;						//置MFC OK标志
	}
	
	if(FLAGS&0x0040)							//判断有无配备TCRing
	{	
		/***** 开始DS18B20的检查 *****/
		if(DS18B20_GetTemperature() == -128)
		{
			f_error.fe_data = E_DS18B20_ERROR;
			SendErrorFrame();
			resend_ef_times = 2;
			LEDRed();
			return FAILURE;
		}
		FLAGS |= 0x0080;						//置DS18B20 OK标志
	}
	
	return SUCCESS;
}

u8 PipelineWash(void)
{
	/***** 管路冲洗 *****/
	if(ValveGOn(VT_VALVE2,'B',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置B
		return FAILURE;
	if(ValveGOn(VT_VALVE3,2,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置1转到2
	{
		Emergency();
		return FAILURE;
	}
	Delay_s(time_parameter.tp_pipeline_wash_time);
	if(ValveGOn(VT_VALVE2,'A',5,TRUE)==FAILURE)			//二通阀执行GO指令切换到位置A
	{
		Emergency();
		return FAILURE;
	}
	Delay_s(time_parameter.tp_pipeline_wash_time);
	
	/***** 阀回到初始位置 *****/
	if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//三通阀执行GO指令从位置2转到1
		return FAILURE;
	
	return SUCCESS;
}

u8 End(void)
{
	if(ValveGOn(VT_VALVE3,1,14,TRUE)==FAILURE)			//三通阀执行GO指令转到位置1
	{
		Emergency();
		return FAILURE;
	}
	if(ValveGOn(VT_VALVE16,1,24,TRUE)==FAILURE)			//十六通阀执行GO指令转到位置1
	{
		Emergency();
		return FAILURE;
	}
	if(ValveGOn(VT_VALVE10,'B',5,TRUE)==FAILURE)		//十通阀执行GO指令切换到位置B
		return FAILURE;
	if(ValveGOn(VT_VALVE4,'A',5,TRUE)==FAILURE)			//四通阀执行GO指令切换到位置A
		return FAILURE;
	
	return SUCCESS;
}

void Emergency(void)
{
	ValveGOn(VT_VALVE3,1,14,FALSE);				//三通阀执行GO指令转到位置1
	ValveGOn(VT_VALVE10,'B',5,FALSE);			//十通阀执行GO指令切换到位置B
}
