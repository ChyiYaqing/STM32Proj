#ifndef __STM_VALVE_H__
#define __STM_VALVE_H__

#include "common.h"

/***** ���巧���� *****/ 
#define VT_VALVE3			0			//��ͨ��
#define VT_VALVE2			1			//��ͨ��
#define VT_VALVE10		2			//ʮͨ��
#define VT_VALVE16		3			//ʮ��ͨ��
#define VT_VALVE4			4			//��ͨ��

/***** ���巧�������� *****/
#define VCT_NO  0					//������
#define VCT_CP  1					//��ѯ��ǰ��λ������
#define VCT_CW	3					//˳ʱ���л�����ָ��λ������
#define VCT_CC	4					//��ʱ���л�����ָ��λ������

extern u8 current_valve_type;

u8 ValveGOn(u8 valve_type, u8 n, u8 delay, BOOL is_show_error);
u8 ValveCXn(u8 valve_type, u8 cmd_type, u8 n, u8 delay);
u8 V3V16Go1Together(void);

#endif
