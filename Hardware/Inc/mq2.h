#ifndef MQ_2_H
#define MQ_2_H

#include "ch32v30x.h"


extern float Smoke_PPM;
u16 MQ2_ReadData (void);
void mq2_adc (void);

#endif
