#include "beep.h"

void Beep_Init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);

	GPIO_InitTypeDef  GPIO_InitStruct={0};
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Speed =  GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	BEEP_OFF;
}
