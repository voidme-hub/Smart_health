#include "Core-Y100P.h"
#include "string.h"

/* DMA 接收和发送缓冲区 */
static uint8_t uart6_rx_dma_buf[UART6_RX_DMA_BUF_SIZE];
static uint8_t uart6_tx_dma_buf[UART6_TX_DMA_BUF_SIZE];
static volatile uint8_t uart6_tx_busy = 0;

/* DMA 初始化函数 */
static void UART6_DMA_Init(void) {
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 使能 DMA2 时钟 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    /* ===== 配置 DMA2_Channel7 用于 UART6 RX (循环模式) ===== */
    DMA_DeInit(DMA2_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&UART6->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)uart6_rx_dma_buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  // 外设到内存
    DMA_InitStructure.DMA_BufferSize = UART6_RX_DMA_BUF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // 循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel7, &DMA_InitStructure);
    DMA_Cmd(DMA2_Channel7, ENABLE);

    /* ===== 配置 DMA2_Channel6 用于 UART6 TX (正常模式) ===== */
    DMA_DeInit(DMA2_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&UART6->DATAR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)uart6_tx_dma_buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  // 内存到外设
    DMA_InitStructure.DMA_BufferSize = 0;  // 初始为0，发送时设置
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // 正常模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA2_Channel6, &DMA_InitStructure);

    /* 使能 DMA2_Channel6 传输完成中断 */
    DMA_ITConfig(DMA2_Channel6, DMA_IT_TC, ENABLE);

    /* 配置 DMA2_Channel6 中断 */
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void UART6_Init (uint32_t baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 银尔达 Core-Y100P WiFi: UART6 部分重映射 -> PB8(TX)/PB9(RX) */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_UART6, ENABLE);

    GPIO_PinRemapConfig (GPIO_PartialRemap_USART6, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;         /* PB8 = UART6_TX */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;         /* PB9 = UART6_RX */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init (UART6, &USART_InitStructure);

    /* 初始化 DMA */
    UART6_DMA_Init();

    /* 使能 UART6 的 DMA 接收和发送请求 */
    USART_DMACmd(UART6, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(UART6, USART_DMAReq_Tx, ENABLE);

    /* 只使能 IDLE 中断，RXNE 由 DMA 处理 */
    USART_ITConfig (UART6, USART_IT_IDLE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    USART_Cmd (UART6, ENABLE);
}

UART6_RxBuffer uart6_rx = {0};

/* UART6 中断处理函数：处理 IDLE 中断 */
void UART6_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void UART6_IRQHandler (void) {
    if (USART_GetFlagStatus (UART6, USART_FLAG_IDLE) == SET) {
        /* 清除 IDLE 标志 */
        volatile uint32_t tmp;
        tmp = UART6->STATR;
        tmp = UART6->DATAR;
        (void)tmp;

        /* 计算接收到的数据长度 */
        uint16_t recv_len = UART6_RX_DMA_BUF_SIZE - DMA_GetCurrDataCounter(DMA2_Channel7);

        if (recv_len > 0 && recv_len < UART6_RX_DMA_BUF_SIZE) {
            /* 复制数据到用户缓冲区 */
            memcpy(uart6_rx.data, uart6_rx_dma_buf, recv_len);
            uart6_rx.data[recv_len] = '\0';
            uart6_rx.len = recv_len;
            uart6_rx.flag = 1;
        }

        /* 重置 DMA 接收 */
        DMA_Cmd(DMA2_Channel7, DISABLE);
        DMA_SetCurrDataCounter(DMA2_Channel7, UART6_RX_DMA_BUF_SIZE);
        DMA_Cmd(DMA2_Channel7, ENABLE);
    }
}

/* DMA2_Channel6 中断处理函数：处理 TX 完成 */
void DMA2_Channel6_IRQHandler(void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void DMA2_Channel6_IRQHandler(void) {
    if (DMA_GetITStatus(DMA2_IT_TC6) != RESET) {
        DMA_ClearITPendingBit(DMA2_IT_TC6);

        /* 等待 UART 发送完成 */
        while (USART_GetFlagStatus(UART6, USART_FLAG_TC) == RESET);

        /* 关闭 DMA 发送 */
        DMA_Cmd(DMA2_Channel6, DISABLE);
        uart6_tx_busy = 0;
    }
}

/* DMA 发送数据函数 */
void UART6_SendData_DMA(const uint8_t *data, uint16_t len) {
    if (len == 0 || len > UART6_TX_DMA_BUF_SIZE) {
        return;
    }

    /* 等待上一次发送完成 */
    while (uart6_tx_busy);

    uart6_tx_busy = 1;

    /* 复制数据到 DMA 缓冲区 */
    memcpy(uart6_tx_dma_buf, data, len);

    /* 配置 DMA 传输长度并启动 */
    DMA_Cmd(DMA2_Channel6, DISABLE);
    DMA_SetCurrDataCounter(DMA2_Channel6, len);
    DMA_Cmd(DMA2_Channel6, ENABLE);
}

/* DMA 发送字符串函数 */
void UART6_SendString_DMA(const uint8_t *str) {
    uint16_t len = 0;
    while (str[len] != '\0' && len < UART6_TX_DMA_BUF_SIZE) {
        len++;
    }
    UART6_SendData_DMA(str, len);
}
