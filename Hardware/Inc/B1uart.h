#ifndef B1UART_H
#define B1UART_H


#include "ch32v30x.h"
#define Usart3_Baud 	38400
 
typedef struct{

	u8 data[256];
	u16 len;
	u8 flag;

}USART3STRUCT;

extern USART3STRUCT u3;
extern float Body_Temp;


void USART3_Init();	//USART3?????
 
void Take_Temperature(void);			//?????????
 
#endif
