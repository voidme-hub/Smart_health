#include "myiic.h"

void IIC_Delay(void)
{
    volatile uint32_t count = 0;
    while (count < 160) {// 96个__NOP消耗1us
        __NOP();
        count++;
    }
}

static void MAX30102_IIC_SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = MAX30102_IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(MAX30102_IIC_PORT, &GPIO_InitStructure);
}

static void MAX30102_IIC_SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = MAX30102_IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(MAX30102_IIC_PORT, &GPIO_InitStructure);
}

void MAX30102_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(MAX30102_IIC_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MAX30102_IIC_SCL_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MAX30102_IIC_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MAX30102_IIC_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(MAX30102_IIC_PORT, &GPIO_InitStructure);

    MAX30102_IIC_SCL_HIGH;
    MAX30102_IIC_SDA_HIGH;
}

void MAX30102_IIC_Start(void)
{
    MAX30102_IIC_SDA_OUT();
    MAX30102_IIC_SDA_HIGH;
    MAX30102_IIC_SCL_HIGH;
    IIC_Delay();
    MAX30102_IIC_SDA_LOW;
    IIC_Delay();
    MAX30102_IIC_SCL_LOW;
}

void MAX30102_IIC_Stop(void)
{
    MAX30102_IIC_SDA_OUT();
    MAX30102_IIC_SCL_LOW;
    MAX30102_IIC_SDA_LOW;
    IIC_Delay();
    MAX30102_IIC_SCL_HIGH;
    IIC_Delay();
    MAX30102_IIC_SDA_HIGH;
    IIC_Delay();
}

uint8_t MAX30102_IIC_Wait_Ack(void)
{
    uint8_t ucErrTime = 0;
    MAX30102_IIC_SDA_IN();
    MAX30102_IIC_SDA_HIGH;
    IIC_Delay();
    MAX30102_IIC_SCL_HIGH;
    IIC_Delay();
    while (MAX30102_READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            MAX30102_IIC_Stop();
            return 1;
        }
    }
    MAX30102_IIC_SCL_LOW;
    return 0;
}

void MAX30102_IIC_Ack(void)
{
    MAX30102_IIC_SCL_LOW;
    MAX30102_IIC_SDA_OUT();
    MAX30102_IIC_SDA_LOW;
    IIC_Delay();
    MAX30102_IIC_SCL_HIGH;
    IIC_Delay();
    MAX30102_IIC_SCL_LOW;
}

void MAX30102_IIC_NAck(void)
{
    MAX30102_IIC_SCL_LOW;
    MAX30102_IIC_SDA_OUT();
    MAX30102_IIC_SDA_HIGH;
    IIC_Delay();
    MAX30102_IIC_SCL_HIGH;
    IIC_Delay();
    MAX30102_IIC_SCL_LOW;
}

void MAX30102_IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
    MAX30102_IIC_SDA_OUT();
    MAX30102_IIC_SCL_LOW;
    for (t = 0; t < 8; t++)
    {
        if ((txd & 0x80) >> 7)
            MAX30102_IIC_SDA_HIGH;
        else
            MAX30102_IIC_SDA_LOW;
        txd <<= 1;
        IIC_Delay();
        MAX30102_IIC_SCL_HIGH;
        IIC_Delay();
        MAX30102_IIC_SCL_LOW;
        IIC_Delay();
    }
}

uint8_t MAX30102_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    MAX30102_IIC_SDA_IN();
    for (i = 0; i < 8; i++)
    {
        MAX30102_IIC_SCL_LOW;
        IIC_Delay();
        MAX30102_IIC_SCL_HIGH;
        receive <<= 1;
        if (MAX30102_READ_SDA)
            receive++;
        IIC_Delay();
    }
    if (!ack)
        MAX30102_IIC_NAck();
    else
        MAX30102_IIC_Ack();
    return receive;
}
