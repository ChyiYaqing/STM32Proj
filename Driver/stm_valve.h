#ifndef __STM_VALVE_H__
#define __STM_VALVE_H__

#include "common.h"

/***** 定义阀类型 *****/ 
#define VT_VALVE3			0			//三通阀
#define VT_VALVE2			1			//二通阀
#define VT_VALVE10		2			//十通阀
#define VT_VALVE16		3			//十六通阀
#define VT_VALVE4			4			//四通阀

/***** 定义阀命令类型 *****/
#define VCT_NO  0					//无命令
#define VCT_CP  1					//查询当前阀位置命令
#define VCT_CW	3					//顺时针切换阀到指定位置命令
#define VCT_CC	4					//逆时针切换阀到指定位置命令

extern u8 current_valve_type;

u8 ValveGOn(u8 valve_type, u8 n, u8 delay, BOOL is_show_error);
u8 ValveCXn(u8 valve_type, u8 cmd_type, u8 n, u8 delay);
u8 V3V16Go1Together(void);

#endif
