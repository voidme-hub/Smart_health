#include "max30102.h"
#include "myiic.h"
#include "debug.h"

// ---------------- High Level Functions ----------------

uint8_t max30102_Bus_Write(uint8_t Register_Address, uint8_t Word_Data)
{
    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_WR);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;
    
    MAX30102_IIC_Send_Byte(Register_Address);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;
    
    MAX30102_IIC_Send_Byte(Word_Data);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;
    
    MAX30102_IIC_Stop();
    return 1;

cmd_fail:
    MAX30102_IIC_Stop();
    return 0;
}

uint8_t max30102_Bus_Read(uint8_t Register_Address)
{
    uint8_t data;
    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_WR);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;
    
    MAX30102_IIC_Send_Byte((uint8_t)Register_Address);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;
    
    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_RD);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;
    
    // Read with NACK (0)
    data = MAX30102_IIC_Read_Byte(0);
    
    MAX30102_IIC_Stop();
    return data;

cmd_fail:
    MAX30102_IIC_Stop();
    return 0;
}

void max30102_FIFO_ReadWords(uint8_t Register_Address, uint16_t Word_Data[][2], uint8_t count)
{
    uint8_t i = 0;
    uint8_t no = count;
    uint8_t data1, data2;

    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_WR);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;

    MAX30102_IIC_Send_Byte((uint8_t)Register_Address);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;

    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_RD);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;

    while (no)
    {
        data1 = MAX30102_IIC_Read_Byte(1); // ACK
        data2 = MAX30102_IIC_Read_Byte(1); // ACK
        Word_Data[i][0] = (((uint16_t)data1 << 8) | data2);

        data1 = MAX30102_IIC_Read_Byte(1); // ACK
        if (no == 1)
            data2 = MAX30102_IIC_Read_Byte(0); // Last byte NACK
        else
            data2 = MAX30102_IIC_Read_Byte(1); // ACK
            
        Word_Data[i][1] = (((uint16_t)data1 << 8) | data2);

        no--;
        i++;
    }
    MAX30102_IIC_Stop();
    return;

cmd_fail:
    MAX30102_IIC_Stop();
}

void max30102_FIFO_ReadBytes(uint8_t Register_Address, uint8_t *Data)
{
    max30102_Bus_Read(REG_INTR_STATUS_1);
    max30102_Bus_Read(REG_INTR_STATUS_2);

    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_WR);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;

    MAX30102_IIC_Send_Byte((uint8_t)Register_Address);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;

    MAX30102_IIC_Start();
    MAX30102_IIC_Send_Byte(MAX30102_I2C_ADDR | I2C_RD);
    if (MAX30102_IIC_Wait_Ack() != 0) goto cmd_fail;

    Data[0] = MAX30102_IIC_Read_Byte(1);
    Data[1] = MAX30102_IIC_Read_Byte(1);
    Data[2] = MAX30102_IIC_Read_Byte(1);
    Data[3] = MAX30102_IIC_Read_Byte(1);
    Data[4] = MAX30102_IIC_Read_Byte(1);
    Data[5] = MAX30102_IIC_Read_Byte(0); // NACK

    MAX30102_IIC_Stop();
    return;

cmd_fail:
    MAX30102_IIC_Stop();
}

void MAX30102_Reset(void)
{
    max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
    max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
}

void MAX30102_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Initialize INT Pin
    RCC_APB2PeriphClockCmd(MAX30102_INT_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = MAX30102_INT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // Input Pull Up
    GPIO_Init(MAX30102_INT_PORT, &GPIO_InitStructure);

    // Initialize I2C
    MAX30102_IIC_Init();
    
    // Config MAX30102
    MAX30102_Reset();
    
    // Add delays between writes if needed, but bus write has wait_ack
    max30102_Bus_Write(REG_INTR_ENABLE_1, 0xc0); // INTR setting
    max30102_Bus_Write(REG_INTR_ENABLE_2, 0x00);
    max30102_Bus_Write(REG_FIFO_WR_PTR, 0x00); 
    max30102_Bus_Write(REG_OVF_COUNTER, 0x00); 
    max30102_Bus_Write(REG_FIFO_RD_PTR, 0x00); 
    max30102_Bus_Write(REG_FIFO_CONFIG, 0x2f); 
    max30102_Bus_Write(REG_MODE_CONFIG, 0x03); 
    max30102_Bus_Write(REG_SPO2_CONFIG, 0x2d); 
    max30102_Bus_Write(REG_LED1_PA, 0x2a);	   
    max30102_Bus_Write(REG_LED2_PA, 0x2a);	   
    max30102_Bus_Write(REG_PILOT_PA, 0x24);	   
    
    printf("MAX30102 Init Complete\r\n");
}

void maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data)
{
    max30102_Bus_Write(uch_addr, uch_data);
}

void maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data)
{
    *puch_data = max30102_Bus_Read(uch_addr);
}

void maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led)
{
    uint32_t un_temp;
    unsigned char uch_temp;
    char ach_i2c_data[6];
    *pun_red_led = 0;
    *pun_ir_led = 0;

    maxim_max30102_read_reg(REG_INTR_STATUS_1, &uch_temp);
    maxim_max30102_read_reg(REG_INTR_STATUS_2, &uch_temp);

    max30102_FIFO_ReadBytes(REG_FIFO_DATA, (uint8_t *)ach_i2c_data);

    un_temp = (unsigned char)ach_i2c_data[0];
    un_temp <<= 16;
    *pun_red_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[1];
    un_temp <<= 8;
    *pun_red_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[2];
    *pun_red_led += un_temp;

    un_temp = (unsigned char)ach_i2c_data[3];
    un_temp <<= 16;
    *pun_ir_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[4];
    un_temp <<= 8;
    *pun_ir_led += un_temp;
    un_temp = (unsigned char)ach_i2c_data[5];
    *pun_ir_led += un_temp;
    *pun_red_led &= 0x03FFFF; 
    *pun_ir_led &= 0x03FFFF;  
}
