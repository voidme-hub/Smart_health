#ifndef __SHT31_H__
#define __SHT31_H__

#include "ch32v30x.h"

extern float temp_val;
extern float hum_val;

#define IIC_SCL_Low  (GPIO_ResetBits(GPIOA,GPIO_Pin_2))
#define IIC_SCL_High  (GPIO_SetBits(GPIOA,GPIO_Pin_2))

#define IIC_SDA_Low  (GPIO_ResetBits(GPIOA,GPIO_Pin_1))
#define IIC_SDA_High  (GPIO_SetBits(GPIOA,GPIO_Pin_1))

#define IIC_SDA_IN  (GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))




void IIC_Software_Init(void); //��ʼ��IICӲ��
// void IIC_Start(void);   //��ʼ�ź�
// void IIC_Stop(void);    //ֹͣ�ź�
// void IIC_SendAck(u8 ack); //��������1bitӦ��
// u8 IIC_RecAck(void);    //��������1bitӦ��
// void IIC_SendData(u8 data); //IIC����8bit����
// u8 IIC_RecData(void);    //IIC����8bit����

u8 SHT31_ReadData(void);


#endif


