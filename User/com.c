#include "FreeRTOS.h"
#include "task.h"

#include "ch32v30x.h"

#include "com.h"


#define GPIO_PIN_MAX               (15U)
#define GPIO_PIN_SPLIT             (8U)
#define GPIO_CFG_SHIFT             (2U)
#define GPIO_CFG_MASK              ((uint32_t)0x0FU)
#define GPIO_MODE_CFG_MASK         ((uint32_t)0x0FU)
#define GPIO_MODE_EXT_MASK         ((uint32_t)0x10U)

static int gpio_write_verify32(__IO uint32_t *reg, uint32_t value)
{
    uint32_t first = 0U;
    uint32_t second = 0U;

    *reg = value;
    first = *reg;
    second = *reg;
    if(first != second)
    {
        return GPIO_EIO;
    }

    return GPIO_OK;
}

static int gpio_get_port(uint8_t port, GPIO_TypeDef **gpio, uint32_t *rcc_mask)
{
    if((gpio == 0) || (rcc_mask == 0))
    {
        return GPIO_EINVAL;
    }

    switch(port)
    {
        case GPIO_PORT_A:
            *gpio = GPIOA;
            *rcc_mask = RCC_IOPAEN;
            break;
        case GPIO_PORT_B:
            *gpio = GPIOB;
            *rcc_mask = RCC_IOPBEN;
            break;
        case GPIO_PORT_C:
            *gpio = GPIOC;
            *rcc_mask = RCC_IOPCEN;
            break;
        case GPIO_PORT_D:
            *gpio = GPIOD;
            *rcc_mask = RCC_IOPDEN;
            break;
        case GPIO_PORT_E:
            *gpio = GPIOE;
            *rcc_mask = RCC_IOPEEN;
            break;
        default:
            return GPIO_ENODEV;
    }

    return GPIO_OK;
}

static int gpio_build_cfg(gpio_mode_t mode, gpio_pull_t pull, gpio_speed_t speed, uint32_t *cfg_value)
{
    GPIOMode_TypeDef hw_mode = GPIO_Mode_IN_FLOATING;
    GPIOSpeed_TypeDef hw_speed = GPIO_Speed_2MHz;

    if(cfg_value == 0)
    {
        return GPIO_EINVAL;
    }

    switch(mode)
    {
        case GPIO_MODE_INPUT:
            if(pull == GPIO_PULL_NONE)
            {
                hw_mode = GPIO_Mode_IN_FLOATING;
            }
            else if(pull == GPIO_PULL_UP)
            {
                hw_mode = GPIO_Mode_IPU;
            }
            else if(pull == GPIO_PULL_DOWN)
            {
                hw_mode = GPIO_Mode_IPD;
            }
            else
            {
                return GPIO_EINVAL;
            }
            break;
        case GPIO_MODE_OUTPUT_PP:
            if(pull != GPIO_PULL_NONE)
            {
                return GPIO_EINVAL;
            }
            hw_mode = GPIO_Mode_Out_PP;
            break;
        case GPIO_MODE_OUTPUT_OD:
            if(pull != GPIO_PULL_NONE)
            {
                return GPIO_EINVAL;
            }
            hw_mode = GPIO_Mode_Out_OD;
            break;
        case GPIO_MODE_AF_PP:
            if(pull != GPIO_PULL_NONE)
            {
                return GPIO_EINVAL;
            }
            hw_mode = GPIO_Mode_AF_PP;
            break;
        case GPIO_MODE_AF_OD:
            if(pull != GPIO_PULL_NONE)
            {
                return GPIO_EINVAL;
            }
            hw_mode = GPIO_Mode_AF_OD;
            break;
        case GPIO_MODE_ANALOG:
            if(pull != GPIO_PULL_NONE)
            {
                return GPIO_EINVAL;
            }
            hw_mode = GPIO_Mode_AIN;
            break;
        default:
            return GPIO_EINVAL;
    }

    switch(speed)
    {
        case GPIO_SPEED_2MHZ:
            hw_speed = GPIO_Speed_2MHz;
            break;
        case GPIO_SPEED_10MHZ:
            hw_speed = GPIO_Speed_10MHz;
            break;
        case GPIO_SPEED_50MHZ:
            hw_speed = GPIO_Speed_50MHz;
            break;
        default:
            return GPIO_EINVAL;
    }

    *cfg_value = ((uint32_t)hw_mode) & GPIO_MODE_CFG_MASK;
    if((((uint32_t)hw_mode) & GPIO_MODE_EXT_MASK) != 0U)
    {
        *cfg_value |= (uint32_t)hw_speed;
    }

    return GPIO_OK;
}

static int gpio_config_pin(GPIO_TypeDef *gpio, uint8_t pin, uint32_t cfg_value)
{
    uint32_t reg_value = 0U;
    uint32_t shift = 0U;
    uint32_t reg_mask = 0U;
    int ret = GPIO_OK;

    if(pin < GPIO_PIN_SPLIT)
    {
        reg_value = gpio->CFGLR;
        shift = ((uint32_t)pin) << GPIO_CFG_SHIFT;
        reg_mask = GPIO_CFG_MASK << shift;
        reg_value = (reg_value & ~reg_mask) | (cfg_value << shift);
        ret = gpio_write_verify32(&gpio->CFGLR, reg_value);
    }
    else
    {
        reg_value = gpio->CFGHR;
        shift = ((uint32_t)(pin - GPIO_PIN_SPLIT)) << GPIO_CFG_SHIFT;
        reg_mask = GPIO_CFG_MASK << shift;
        reg_value = (reg_value & ~reg_mask) | (cfg_value << shift);
        ret = gpio_write_verify32(&gpio->CFGHR, reg_value);
    }

    return ret;
}

int gpio_init(uint8_t port, uint8_t pin, gpio_mode_t mode, gpio_pull_t pull, gpio_speed_t speed)
{
    GPIO_TypeDef *gpio = 0;                 /* GPIO 基址 */
    uint32_t rcc_mask = 0U;                 /* 时钟掩码 */
    uint32_t cfg_value = 0U;                /* 配置编码 */
    uint16_t pin_mask = 0U;                 /* 引脚掩码 */
    int ret = GPIO_OK;                      /* 返回码 */

    if(pin > GPIO_PIN_MAX) { return GPIO_ERANGE; } /* 参数检查 */
    ret = gpio_get_port(port, &gpio, &rcc_mask);   /* 端口映射 */
    if(ret != GPIO_OK) { return ret; }
    ret = gpio_build_cfg(mode, pull, speed, &cfg_value); /* 模式构造 */
    if(ret != GPIO_OK) { return ret; }

    pin_mask = (uint16_t)((uint16_t)GPIO_Pin_0 << pin); /* 生成掩码 */

    taskENTER_CRITICAL(); /* 临界区保护 */
    do
    {
        uint32_t rcc_value = RCC->APB2PCENR | rcc_mask; /* 使能时钟 */
        ret = gpio_write_verify32(&RCC->APB2PCENR, rcc_value); /* 写后读校验 */
        if(ret != GPIO_OK) { break; }

        ret = gpio_config_pin(gpio, pin, cfg_value); /* 配置寄存器 */
        if(ret != GPIO_OK) { break; }

        if(mode == GPIO_MODE_INPUT) /* 上下拉设置 */
        {
            if(pull == GPIO_PULL_UP) { ret = gpio_write_verify32(&gpio->BSHR, pin_mask); }
            else if(pull == GPIO_PULL_DOWN) { ret = gpio_write_verify32(&gpio->BCR, pin_mask); }
        }
    } while(0);
    taskEXIT_CRITICAL(); /* 退出临界区 */

    return ret; /* 返回状态 */
}



