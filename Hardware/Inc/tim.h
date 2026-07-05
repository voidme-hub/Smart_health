#ifndef __TIM_REPORT_H__
#define __TIM_REPORT_H__

#include "ch32v30x.h"

void TIM6_ReportInit(void);
extern volatile uint8_t report_ready;
extern char report_buf[200];

#endif
