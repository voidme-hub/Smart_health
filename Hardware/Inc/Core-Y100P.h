#ifndef __CORE_Y100P_H__
#define __CORE_Y100P_H__

#include "ch32v30x.h"

#define UART6_RX_DMA_BUF_SIZE  256
#define UART6_TX_DMA_BUF_SIZE  256

typedef struct {
    u8 data[UART6_RX_DMA_BUF_SIZE];
	u16 len;
	u8 flag;
} UART6_RxBuffer;

extern UART6_RxBuffer uart6_rx;

void UART6_Init(uint32_t baudrate);
void UART6_SendData_DMA(const uint8_t *data, uint16_t len);
void UART6_SendString_DMA(const uint8_t *str);

#endif
