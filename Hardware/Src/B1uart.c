#include "B1uart.h"
#include "string.h"

// 全局 USART3 接收结构体
USART3STRUCT u3 = {0};

// 测温命令（原样保留）
uint8_t command[7] = {0xAA, 0xA5, 0x03, 0x01, 0x04, 0x55}; // 测温命令
float Body_Temp = 0;

void USART3_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能 GPIOB 和 USART3 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // 配置 PB10 为 USART3_TX（复用推挽输出）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 PB11 为 USART3_RX（浮空输入）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 USART3
    USART_InitStructure.USART_BaudRate = Usart3_Baud;                               // 波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 8 位数据
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 1 位停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;                 // 使能接收和发送
    USART_Init(USART3, &USART_InitStructure);

    // 使能 USART3 RXNE 和 IDLE 中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);

    // 配置 NVIC (USART3)
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能 USART3
    USART_Cmd(USART3, ENABLE);
}

// 单片机向传感器发送命令函数（发送仍然使用阻塞发送，因为命令短且可靠）
static void USART3_SendCommand(uint8_t *command_buf, uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        // 等待发送数据寄存器为空
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
            ;
        // 发送数据
        USART_SendData(USART3, command_buf[i]);
    }
    // 等待发送完成
    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
        ;
}


void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));//串口1快速中断
// USART3 中断服务程序：处理 RXNE（接收字节）和 IDLE（帧结束）
void USART3_IRQHandler(void)
{
    // RXNE: 接收到一个字节
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        USART_ClearITPendingBit(USART3,USART_IT_RXNE);
        uint8_t data = (uint8_t)USART_ReceiveData(USART3);
        if (u3.len < sizeof(u3.data))
        {
            u3.data[u3.len++] = data;
        }
        else
        {
            // 缓冲区溢出，丢弃或处理（这里选择丢弃最新字节）
            memset(u3.data, 0, sizeof(u3.data));
            u3.len = 0;
        }
    }

    // IDLE: 总线空闲，表示帧结束（读取 SR 然后 DR 来清除 IDLE 标志）
    if (USART_GetFlagStatus(USART3, USART_FLAG_IDLE) == SET)
    {
        // 读取 SR 和 DR 清除 IDLE
        volatile uint32_t tmp;
        tmp = USART3->STATR;
        tmp = USART3->DATAR;
        (void)tmp;
        u3.len = 0;
        // 标记接收完成
        u3.flag = 1;
    }
}

void Take_Temperature(void)
{
   
    // 发送测温命令（阻塞快速发送）
    USART3_SendCommand(command, sizeof(command));
    // for (uint16_t i = 0; i < 7; i++)
    // {
    //     // 可打开下面注释查看每字节值
    //      printf("response[%d]=0x%02X ", i, u3.data[i]);
    // }

    Body_Temp = ((uint16_t)u3.data[5] << 8) | u3.data[6];
    Body_Temp = Body_Temp / 10.0f;

}
