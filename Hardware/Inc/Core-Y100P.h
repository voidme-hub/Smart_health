#ifndef __CORE_Y100P_H__
#define __CORE_Y100P_H__

#include "ch32v30x.h"



typedef struct {
    u8 data[256];
	u16 len;
	u8 flag;
} UART5_RxBuffer;

extern UART5_RxBuffer uart5_rx;

void UART5_Init(uint32_t baudrate);
void UART5_SendDate(const uint8_t *data);

#endif
