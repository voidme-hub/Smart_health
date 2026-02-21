#include "key.h"
#include "FreeRTOS.h"
#include "task.h"

void key_init(void)
{
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    gpio_init.GPIO_Pin = KEY_PIN;
    gpio_init.GPIO_Mode = GPIO_Mode_IPD;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_GPIO, &gpio_init);

    GPIO_ResetBits(KEY_GPIO, KEY_PIN);
}

u8 key_is_pressed(void)
{   
    return (GPIO_ReadInputDataBit(KEY_GPIO, KEY_PIN) == Bit_RESET) ? 1u : 0u;
}
