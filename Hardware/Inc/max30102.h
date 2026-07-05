#ifndef __MAX30102_H
#define __MAX30102_H

#include "ch32v30x.h"

#define MAX30102_INT_PORT GPIOB
#define MAX30102_INT_PIN GPIO_Pin_4
#define MAX30102_INT_CLK RCC_APB2Periph_GPIOB
#define MAX30102_INT GPIO_ReadInputDataBit(MAX30102_INT_PORT, MAX30102_INT_PIN)

/* 硬件 I2C1 接口: PB6(SCL) / PB7(SDA)
 * CH32V307 上 I2C1 的默认引脚即为 PB6/PB7 (无需重映射)
 * (与原软件 I2C myiic.h 的引脚一致, 与板上走线匹配) */
#define MAX30102_I2C            I2C1
#define MAX30102_I2C_PORT       GPIOB
#define MAX30102_I2C_SCL_PIN    GPIO_Pin_6
#define MAX30102_I2C_SDA_PIN    GPIO_Pin_7
#define MAX30102_I2C_CLK_GPIO   RCC_APB2Periph_GPIOB
#define MAX30102_I2C_CLK_PERI   RCC_APB1Periph_I2C1
#define MAX30102_I2C_SPEED      300000  // 400kHz Fast Mode

// 寄存器地址
#define REG_INTR_STATUS_1    0x00
#define REG_INTR_STATUS_2    0x01
#define REG_INTR_ENABLE_1    0x02
#define REG_INTR_ENABLE_2    0x03
#define REG_FIFO_WR_PTR      0x04
#define REG_OVF_COUNTER      0x05
#define REG_FIFO_RD_PTR      0x06
#define REG_FIFO_DATA        0x07
#define REG_FIFO_CONFIG      0x08
#define REG_MODE_CONFIG      0x09
#define REG_SPO2_CONFIG      0x0A
#define REG_LED1_PA          0x0C
#define REG_LED2_PA          0x0D
#define REG_PILOT_PA         0x10
#define REG_MULTI_LED_CTRL1  0x11
#define REG_MULTI_LED_CTRL2  0x12
#define REG_REV_ID           0xFE
#define REG_PART_ID          0xFF

// 器件地址
#define MAX30102_I2C_ADDR 0xAE
#define I2C_WR 0x00
#define I2C_RD 0x01

extern uint8_t dis_hr;   
extern uint8_t dis_spo2; 

void MAX30102_Init(void);
void MAX30102_Reset(void);
uint8_t max30102_Bus_Write(uint8_t Register_Address, uint8_t Word_Data);
uint8_t max30102_Bus_Read(uint8_t Register_Address);
void max30102_FIFO_ReadWords(uint8_t Register_Address, uint16_t Word_Data[][2], uint8_t count);
void max30102_FIFO_ReadBytes(uint8_t Register_Address, uint8_t *Data);
void maxim_max30102_write_reg(uint8_t uch_addr, uint8_t uch_data);
void maxim_max30102_read_reg(uint8_t uch_addr, uint8_t *puch_data);
void maxim_max30102_read_fifo(uint32_t *pun_red_led, uint32_t *pun_ir_led);

#endif
