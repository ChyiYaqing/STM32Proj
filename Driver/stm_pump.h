#ifndef __STM_PUMP_H__
#define __STM_PUMP_H__

#define PUMP_START 		GPIO_SetBits(GPIOC,GPIO_Pin_4)			//������
#define PUMP_STOP			GPIO_ResetBits(GPIOC,GPIO_Pin_4)		//��ֹͣ

void Pump_GPIO_Init(void);

#endif
