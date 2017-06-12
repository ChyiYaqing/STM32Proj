#include "stm_led.h"
#include "stm_usart.h"
#include "stm_pc.h"
#include "stm_valve.h"

u8 vc_cp[5] = {'/',0,'C','P','\r'};			//阀命令：当前位置查询
u8 vc_gon[7] = {'/',0,'G','O'};					//阀命令：切换到指定位置
u8 current_valve_type;

u8 ValveGOn(u8 valve_type, u8 n, u8 delay, BOOL is_show_error)
{
	u8 i, len;

	vc_gon[1] = vc_cp[1] = valve_type+0x30;
	if(valve_type==VT_VALVE10 || valve_type==VT_VALVE2 || valve_type==VT_VALVE4)
	{
		vc_gon[4] = n;
		vc_gon[5] = '\r';
		len = 6;
	}
	else
	{
		if(n>9)			
		{
			vc_gon[4] = n/10+0x30;
			vc_gon[5] = n%10+0x30;
			vc_gon[6] = '\r';
			len = 7;
		}
		else
		{
			vc_gon[4] = n+0x30;
			vc_gon[5] = '\r';
			len = 6;
		}
	}
	current_valve_type = valve_type;
	for(i=1;i<4;i++)
	{
		SendCmdTo485(vc_gon,len);
		for(;delay>0;delay--)
			Delay_ms(100);
		SendCmdTo485(vc_cp,5);
		Delay_ms(100);					
		if(rs485_usart_rx_buf==n)
			return SUCCESS;				
	}
	if(is_show_error)
	{
		switch(valve_type)
		{			
			case VT_VALVE3:
				f_error.fe_data = E_VALVE3_FAULT;
				break;			
			case VT_VALVE2:
				f_error.fe_data = E_VALVE2_FAULT;
				break;
			case VT_VALVE10:
				f_error.fe_data = E_VALVE10_FAULT;
				break;
			case VT_VALVE16:
				f_error.fe_data = E_VALVE16_FAULT;
				break;
			case VT_VALVE4:
				f_error.fe_data = E_VALVE4_FAULT;
				break;
		}
		SendErrorFrame();
		resend_ef_times = 2;
		LEDRed();
	}
	
	return FAILURE;
}

u8 ValveCXn(u8 valve_type, u8 cmd_type, u8 n, u8 delay)
{
	u8 i, len, cmd[7];
	
	cmd[0] = '/';
	cmd[1] = vc_cp[1] = valve_type+0x30;
	cmd[2] = 'C';
	if(cmd_type==VCT_CW)
		cmd[3] = 'W';
	else if(cmd_type==VCT_CC)
		cmd[3] = 'C';
	if(n>9)			
	{
		cmd[4] = n/10+0x30;
		cmd[5] = n%10+0x30;
		cmd[6] = '\r';
		len = 7;
	}
	else
	{
		cmd[4] = n+0x30;
		cmd[5] = '\r';
		len = 6;
	}
	current_valve_type = valve_type;
	for(i=1;i<4;i++)
	{
		SendCmdTo485(cmd,len);
		for(;delay>0;delay--)
			Delay_ms(100);
		SendCmdTo485(vc_cp,5);
		Delay_ms(100);					
		if(rs485_usart_rx_buf==n)
			return SUCCESS;								
	}
	switch(valve_type)
	{
		case VT_VALVE3:
			f_error.fe_data = E_VALVE3_FAULT;
			break;
		case VT_VALVE16:
			f_error.fe_data = E_VALVE16_FAULT;
			break;		
	}
	SendErrorFrame();
	resend_ef_times = 2;
	LEDRed();
	
	return FAILURE;
}

u8 V3V16Go1Together(void)
{
	u8 i;
	
	vc_gon[1] = VT_VALVE3+0x30;
	vc_gon[4] = '1';
	vc_gon[5] = '\r';
	SendCmdTo485(vc_gon,6);						//三通阀执行GO指令从位置4转到1
	Delay_ms(500);
	vc_gon[1] = VT_VALVE16+0x30;
	SendCmdTo485(vc_gon,6);						//十六通阀执行GO指令转到位置1
	Delay_ms(1000);
	Delay_ms(1400);
	vc_gon[1] = vc_cp[1] = VT_VALVE3+0x30;
	current_valve_type = VT_VALVE3;
	for(i=1;i<4;i++)
	{		
		SendCmdTo485(vc_cp,5);
		Delay_ms(100);
		if(rs485_usart_rx_buf==1)
			break;
		else if(i==3)
			continue;
		
		SendCmdTo485(vc_gon,6);					//三通阀执行GO指令从位置4转到1
		Delay_ms(1400);
	}
	if(i==4)
		f_error.fe_data = E_VALVE3_FAULT;
	else
	{
		vc_gon[1] = vc_cp[1] = VT_VALVE16+0x30;
		current_valve_type = VT_VALVE16;
		for(i=1;i<4;i++)
		{
			SendCmdTo485(vc_cp,5);
			Delay_ms(100);
			if(rs485_usart_rx_buf==1)
				return SUCCESS;
			else if(i==3)
				continue;
										
			SendCmdTo485(vc_gon,6);				//十六通阀执行GO指令转到位置1
			Delay_ms(1000);
			Delay_ms(1400);			
		}
		if(i==4)
			f_error.fe_data = E_VALVE16_FAULT;
	}
	SendErrorFrame();
	resend_ef_times = 2;
	LEDRed();
	
	return FAILURE;
}
