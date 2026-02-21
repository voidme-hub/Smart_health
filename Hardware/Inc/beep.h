#ifndef __BEEP_H__
#define __BEEP_H__

#include "ch32v30x.h"   

#define BEEP_OFF (GPIO_ResetBits(GPIOB,GPIO_Pin_12))
#define BEEP_ON  (GPIO_SetBits(GPIOB,GPIO_Pin_12))
void Beep_Init(void);


#endif
