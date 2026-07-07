#include "iwdg.h"
#include "ch32v30x_iwdg.h"
#include "ch32v30x_rcc.h"

/* IWDG 硬件超时约 26 秒（预分频 256 × 重载 4096 / LSI 40kHz，已是最大值）。
   实际复位周期由 led_task 的喂狗持续时间控制（当前为 8 分钟）。 */
void IWDG_Init(void)
{
    RCC_LSICmd(ENABLE);
    while((RCC->RSTSCKR & (1 << 1)) == 0) { }
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    while(IWDG_GetFlagStatus(IWDG_FLAG_PVU) == SET) { }
    IWDG_SetReload(0x0FFF);
    while(IWDG_GetFlagStatus(IWDG_FLAG_RVU) == SET) { }
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
}
