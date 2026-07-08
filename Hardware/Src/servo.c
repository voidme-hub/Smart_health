#include "servo.h"
#include "string.h"

/*
函数功能：风扇初始化
形参：u16 ccr
返回值：void
函数说明：
motor -- PB5  --  TIM3_CH2  -- 复用模式
*/
void Servo_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    uint32_t timer_clock;
    uint16_t prescaler;

    // 1. 开启时钟
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    // 2. 先配置 AFIO 重映射（TIM3_CH2 -> PB5），再配置 GPIO
    //    顺序反了会导致 PB5 关联到默认复用功能而非 TIM3_CH2，PWM 出不到引脚
    GPIO_PinRemapConfig (GPIO_PartialRemap_TIM3, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    timer_clock = SystemCoreClock;
    prescaler = (uint16_t)((timer_clock / 10000u) - 1u);

    TIM_TimeBaseStructure.TIM_Period = 200 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 向上计数模式
    TIM_TimeBaseInit (TIM3, &TIM_TimeBaseStructure);

    // 5. 初始化PWM模式（通道2）
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;              // PWM模式1
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 使能输出
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;      // 输出极性为高
    TIM_OCInitStructure.TIM_Pulse = 5;
    TIM_OC2Init (TIM3, &TIM_OCInitStructure);

    // 6. 使能预装载寄存器
    TIM_OC2PreloadConfig (TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig (TIM3, ENABLE);  // 使能ARR预装载

    // 7. 启动定时器
    TIM_Cmd (TIM3, ENABLE);
}

void Servo_SetAngle (float angle) {
    u16 pulse_width;

    // 将角度转换为脉冲计数值
    // 角度范围0~180度，对应脉冲宽度0.5ms~2.5ms (5~25个计数单位)
    // 公式：pulse_width = (angle / 180) * (25 - 5) + 5
    pulse_width = (u16)((angle / 180.0) * 20 + 5);

    // 限制脉冲宽度在安全范围内
    if (pulse_width < 5)
        pulse_width = 5;
    if (pulse_width > 25)
        pulse_width = 25;

    // 设置比较寄存器的值，改变PWM脉宽
    TIM_SetCompare2 (TIM3, pulse_width);
}

u8 parse_servo_angle (const char *s, float *out) {
    const char *p = strstr (s, "\"servo\"");
    if (p == 0) {
        p = strstr (s, "servo");
    }
    if (p == 0) {
        return 0;
    }
    p = strchr (p, ':');
    if (p == 0) {
        return 0;
    }
    p++;
    while (*p == ' ' || *p == '\"' || *p == '\t' || *p == '\r' || *p == '\n') {
        p++;
    }
    /* 手动解析整数：newlib-nano 默认未链接 _scanf_float，sscanf %f 会静默失败 */
    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }
    if (*p < '0' || *p > '9') {
        return 0;
    }
    int value = 0;
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (*p - '0');
        p++;
    }
    *out = (float)(sign * value);
    return 1;
}
