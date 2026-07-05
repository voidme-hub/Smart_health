#include "Core-Y100P.h"
#include "string.h"

void UART5_Init (uint32_t baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 银尔达 Core-Y100P WiFi: USART5 部分重映射(UART5_TX_2/RX_2) -> PE9(TX)/PE12(RX)
     * 原板走线为 PE8/PE9, 但 PE8 无任何 UART 功能(仅 FSMC_D5/TIM1_CH1N_3/TIM10_BKIN_3/FSMC_NBL1),
     * 故 RX 线需从 PE8 改接到 PE12; TX 保留在 PE9(UART5_TX_2)。
     * 选用 UART5 而非 UART6: UART6 各重映射均与本工程已占用外设冲突
     * (USART3=PB10/PB11, I2C1=PB6/PB7, LED=PC0), 而 UART5 的 _2 重映射(PE9/PE12)与 LCD(PE0~PE4) 不冲突。 */
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_UART5, ENABLE);

    GPIO_PinRemapConfig (GPIO_PartialRemap_USART5, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;         /* PE9 = UART5_TX */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;        /* PE12 = UART5_RX */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOE, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init (UART5, &USART_InitStructure);

    USART_ITConfig (UART5, USART_IT_RXNE, ENABLE);
    USART_ITConfig (UART5, USART_IT_IDLE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    USART_Cmd (UART5, ENABLE);
}

UART5_RxBuffer uart5_rx = {0};
void UART5_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void UART5_IRQHandler (void) {
    if (USART_GetITStatus (UART5, USART_IT_RXNE) != RESET) {
        u8 data = (u8)USART_ReceiveData (UART5);
        // 最多接收254个字节，最后一个字节为'\0'
        if (uart5_rx.len < sizeof (uart5_rx.data) - 1) {
            uart5_rx.data[uart5_rx.len++] = data;
        } else {
            // 接收缓冲区已满，丢弃当前字符串
            memset (uart5_rx.data, 0, sizeof (uart5_rx.data));
            uart5_rx.len = 0;
        }
        USART_ClearITPendingBit (UART5, USART_IT_RXNE);
    }

    if (USART_GetFlagStatus (UART5, USART_FLAG_IDLE) == SET) {
        UART5->STATR;
        UART5->DATAR;
        uart5_rx.data[uart5_rx.len] = '\0';
        uart5_rx.flag = 1;
        uart5_rx.len = 0;
    }
}

void UART5_SendDate(const uint8_t *data){
    while(*data){
        while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
        USART_SendData(UART5, *data++);
    }
}
