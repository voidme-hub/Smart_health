#include "iwdg.h"
#include "ch32v30x_iwdg.h"
#include "ch32v30x_rcc.h"

void IWDG_Init3min(void)
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
