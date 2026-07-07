#include "max30102.h"
#include "ch32v30x_i2c.h"
#include "debug.h"

/* ---------------- I2C 事件等待 (带超时, 避免从机缺失时挂死任务) ---------------- */

static uint8_t MAX30102_I2C_WaitEvent(uint32_t event)
{
    uint32_t timeout = 0x10000;
    while (!I2C_CheckEvent(MAX30102_I2C, event))
    {
        if ((timeout--) == 0)
            return 0;
    }
    return 1;
}

static uint8_t MAX30102_I2C_WaitNotBusy(void)
{
    uint32_t timeout = 0x10000;
    while (I2C_GetFlagStatus(MAX30102_I2C, I2C_FLAG_BUSY))
    {
        if ((timeout--) == 0)
            return 0;
    }
    return 1;
}

/* ---------------- 硬件 I2C1 初始化: PB6(SCL)/PB7(SDA) ---------------- */

static void MAX30102_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef  I2C_InitStructure = {0};

    RCC_APB2PeriphClockCmd(MAX30102_I2C_CLK_GPIO | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(MAX30102_I2C_CLK_PERI, ENABLE);

    I2C_DeInit(MAX30102_I2C);

    GPIO_InitStructure.GPIO_Pin = MAX30102_I2C_SCL_PIN | MAX30102_I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MAX30102_I2C_PORT, &GPIO_InitStructure);

    I2C_InitStructure.I2C_ClockSpeed = MAX30102_I2C_SPEED;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(MAX30102_I2C, &I2C_InitStructure);

    I2C_Cmd(MAX30102_I2C, ENABLE);

    for (volatile uint32_t i = 0; i < 10000; i++);
}

/* ---------------- High Level Functions ---------------- */

uint8_t max30102_Bus_Write(uint8_t Register_Address, uint8_t Word_Data)
{
    if (!MAX30102_I2C_WaitNotBusy()) goto cmd_fail;

    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Transmitter);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto cmd_fail;

    I2C_SendData(MAX30102_I2C, Register_Address);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto cmd_fail;

    I2C_SendData(MAX30102_I2C, Word_Data);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto cmd_fail;

    I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
    return 1;

cmd_fail:
    I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
    return 0;
}

uint8_t max30102_Bus_Read(uint8_t Register_Address)
{
    uint8_t data = 0;

    if (!MAX30102_I2C_WaitNotBusy()) goto cmd_fail;

    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Transmitter);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto cmd_fail;

    I2C_SendData(MAX30102_I2C, Register_Address);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto cmd_fail;

    /* 重复起始, 切换为接收方向 */
    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Receiver);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) goto cmd_fail;

    /* 单字节读取: NACK + STOP 后再读数据 */
    I2C_AcknowledgeConfig(MAX30102_I2C, DISABLE);
    I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) goto cmd_fail;

    data = I2C_ReceiveData(MAX30102_I2C);
    I2C_AcknowledgeConfig(MAX30102_I2C, ENABLE);
    return data;

cmd_fail:
    I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
    I2C_AcknowledgeConfig(MAX30102_I2C, ENABLE);
    return 0;
}

void max30102_FIFO_ReadWords(uint8_t Register_Address, uint16_t Word_Data[][2], uint8_t count)
{
    uint8_t total = (uint8_t)(count * 4);
    uint8_t buf[4];
    uint8_t bi = 0, sample = 0;

    if (count == 0) return;

    if (!MAX30102_I2C_WaitNotBusy()) goto cmd_fail;

    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Transmitter);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto cmd_fail;

    I2C_SendData(MAX30102_I2C, Register_Address);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto cmd_fail;

    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Receiver);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) goto cmd_fail;

    for (uint8_t idx = 0; idx < total; idx++)
    {
        if (idx == (uint8_t)(total - 1))
        {
            I2C_AcknowledgeConfig(MAX30102_I2C, DISABLE);
            I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
        }
        if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) goto cmd_fail;

        buf[bi++] = I2C_ReceiveData(MAX30102_I2C);
        if (bi == 4)
        {
            Word_Data[sample][0] = (((uint16_t)buf[0] << 8) | buf[1]);
            Word_Data[sample][1] = (((uint16_t)buf[2] << 8) | buf[3]);
            bi = 0;
            sample++;
        }
    }
    I2C_AcknowledgeConfig(MAX30102_I2C, ENABLE);
    return;

cmd_fail:
    I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
    I2C_AcknowledgeConfig(MAX30102_I2C, ENABLE);
}

void max30102_FIFO_ReadBytes(uint8_t Register_Address, uint8_t *Data)
{
    if (!MAX30102_I2C_WaitNotBusy()) goto cmd_fail;

    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Transmitter);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) goto cmd_fail;

    I2C_SendData(MAX30102_I2C, Register_Address);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) goto cmd_fail;

    I2C_GenerateSTART(MAX30102_I2C, ENABLE);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) goto cmd_fail;

    I2C_Send7bitAddress(MAX30102_I2C, MAX30102_I2C_ADDR, I2C_Direction_Receiver);
    if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) goto cmd_fail;

    for (uint8_t i = 0; i < 6; i++)
    {
        if (i == 5)
        {
            I2C_AcknowledgeConfig(MAX30102_I2C, DISABLE);
            I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
        }
        if (!MAX30102_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) goto cmd_fail;

        Data[i] = I2C_ReceiveData(MAX30102_I2C);
    }
    I2C_AcknowledgeConfig(MAX30102_I2C, ENABLE);
    return;

cmd_fail:
    I2C_GenerateSTOP(MAX30102_I2C, ENABLE);
    I2C_AcknowledgeConfig(MAX30102_I2C, ENABLE);
}

void MAX30102_Reset(void)
{
    max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
    max30102_Bus_Write(REG_MODE_CONFIG, 0x40);
}

/* 掉电模式: MODE_CONFIG.SHDN=1，关闭内部 LED/ADC，寄存器配置保留 */
void MAX30102_Shutdown(void)
{
    max30102_Bus_Write(REG_MODE_CONFIG, 0x80);
}

/* 唤醒: 清 SHDN 位恢复 SpO2 模式，并清空 FIFO 丢弃掉电期间陈旧数据 */
void MAX30102_Wakeup(void)
{
    max30102_Bus_Write(REG_MODE_CONFIG, 0x03);
    max30102_Bus_Write(REG_FIFO_WR_PTR, 0x00);
    max30102_Bus_Write(REG_OVF_COUNTER, 0x00);
    max30102_Bus_Write(REG_FIFO_RD_PTR, 0x00);
}

void MAX30102_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    uint8_t partID;

    /* Initialize INT Pin */
    RCC_APB2PeriphClockCmd(MAX30102_INT_CLK, ENABLE);
    GPIO_InitStructure.GPIO_Pin = MAX30102_INT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(MAX30102_INT_PORT, &GPIO_InitStructure);

    /* Initialize Hardware I2C1 */
    MAX30102_I2C_Init();

    /* Verify I2C Communication */
    partID = max30102_Bus_Read(REG_PART_ID);
    if (partID != 0x15)
    {
        printf("ERROR: MAX30102 NOT detected! PartID=0x%02X\r\n", partID);
        return;
    }

    /* Config MAX30102 */
    MAX30102_Reset();
    for (volatile uint32_t i = 0; i < 100000; i++);

    max30102_Bus_Write(REG_INTR_ENABLE_1, 0xc0);
    max30102_Bus_Write(REG_INTR_ENABLE_2, 0x00);
    max30102_Bus_Write(REG_FIFO_CONFIG, 0x4f);
    max30102_Bus_Write(REG_SPO2_CONFIG, 0x27);
    max30102_Bus_Write(REG_LED1_PA, 0x24);
    max30102_Bus_Write(REG_LED2_PA, 0x24);
    max30102_Bus_Write(REG_PILOT_PA, 0x7f);
    max30102_Bus_Write(REG_MODE_CONFIG, 0x03);

    for (volatile uint32_t i = 0; i < 10000; i++);

    max30102_Bus_Write(REG_FIFO_WR_PTR, 0x00);
    max30102_Bus_Write(REG_OVF_COUNTER, 0x00);
    max30102_Bus_Write(REG_FIFO_RD_PTR, 0x00);

    printf("MAX30102 Init OK (I2C@400kHz)\r\n");
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
    uint8_t ach_i2c_data[6];

    /* 读取FIFO数据（6字节：RED[3] + IR[3]） */
    max30102_FIFO_ReadBytes(REG_FIFO_DATA, ach_i2c_data);

    /* 解析RED LED数据 (18位) */
    *pun_red_led = ((uint32_t)(ach_i2c_data[0] & 0x03) << 16) |
                   ((uint32_t)ach_i2c_data[1] << 8) |
                   ach_i2c_data[2];

    /* 解析IR LED数据 (18位) */
    *pun_ir_led = ((uint32_t)(ach_i2c_data[3] & 0x03) << 16) |
                  ((uint32_t)ach_i2c_data[4] << 8) |
                  ach_i2c_data[5];
}
