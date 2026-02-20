#ifndef __COM_H__
#define __COM_H__

#include "FreeRTOS.h"
#include "task.h"

#include "ch32v30x.h"


typedef enum
{
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT_PP,
    GPIO_MODE_OUTPUT_OD,
    GPIO_MODE_AF_PP,
    GPIO_MODE_AF_OD,
    GPIO_MODE_ANALOG
} gpio_mode_t;

typedef enum
{
    GPIO_PULL_NONE = 0,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN
} gpio_pull_t;

typedef enum
{
    GPIO_SPEED_2MHZ = 0,
    GPIO_SPEED_10MHZ,
    GPIO_SPEED_50MHZ
} gpio_speed_t;

#define GPIO_PORT_A                 GPIO_PortSourceGPIOA
#define GPIO_PORT_B                 GPIO_PortSourceGPIOB
#define GPIO_PORT_C                 GPIO_PortSourceGPIOC
#define GPIO_PORT_D                 GPIO_PortSourceGPIOD
#define GPIO_PORT_E                 GPIO_PortSourceGPIOE

#define GPIO_OK                     (0)
#define GPIO_EINVAL                 (-22)
#define GPIO_ERANGE                 (-34)
#define GPIO_ENODEV                 (-19)
#define GPIO_EIO                    (-5)

int gpio_init(uint8_t port, uint8_t pin, gpio_mode_t mode, gpio_pull_t pull, gpio_speed_t speed);

#define GPIO_OUT_PP(port, pin)      gpio_init((port), (pin), GPIO_MODE_OUTPUT_PP, GPIO_PULL_NONE, GPIO_SPEED_50MHZ)
#define GPIO_OUT_OD(port, pin)      gpio_init((port), (pin), GPIO_MODE_OUTPUT_OD, GPIO_PULL_NONE, GPIO_SPEED_50MHZ)
#define GPIO_IN_FLOAT(port, pin)    gpio_init((port), (pin), GPIO_MODE_INPUT, GPIO_PULL_NONE, GPIO_SPEED_2MHZ)
#define GPIO_IN_PU(port, pin)       gpio_init((port), (pin), GPIO_MODE_INPUT, GPIO_PULL_UP, GPIO_SPEED_2MHZ)
#define GPIO_IN_PD(port, pin)       gpio_init((port), (pin), GPIO_MODE_INPUT, GPIO_PULL_DOWN, GPIO_SPEED_2MHZ)



#endif
