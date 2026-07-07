#include "tim.h"
#include "Core-Y100P.h"
#include "sht31.h"
#include "mq2.h"
#include "B1uart.h"

extern volatile float servo_angle;
extern volatile uint8_t servo_override;
extern volatile uint8_t beep_alarm;

/* 与 LCD 显示一致的心率血氧数据源 (由 max30102_task 写入) */
extern volatile long g_saved_hr;
extern volatile long g_saved_spo2;
extern volatile uint8_t g_measurement_ready;

static uint8_t report_index = 0;
volatile uint8_t report_ready = 0;
char report_buf[200];

void TIM6_ReportInit(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    uint32_t timer_clock;
    uint16_t prescaler;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    timer_clock = SystemCoreClock;
    prescaler = (uint16_t)((timer_clock / 10000u) - 1u);

    TIM_TimeBaseStructure.TIM_Period = 5000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM6, ENABLE);
}

void TIM6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM6_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET) {
        uint8_t sent = 0;
        uint8_t tries;
        float smoke_now = Smoke_PPM;

        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

        if (smoke_now > 30.0f) {
            if (Smoke_PPM > 35.0f) {
                beep_alarm = 1;
            } else {
                beep_alarm = 0;
            }
        } else {
            beep_alarm = 0;
        }

        if (report_ready == 0) {
            for (tries = 0; tries < 8; tries++) {
            float temp = temp_val;
            float hum = hum_val;
            float body = Body_Temp;
            float smoke = Smoke_PPM;
            float servo = servo_angle;
            const char *beep_str = (beep_alarm != 0) ? "true" : "false";

            switch (report_index) {
                case 0:
                    if (temp >= -40.0f && temp <= 60.0f) {
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"001\",\"version\":\"1.0\",\"params\":"
                                 "{\"temperatrue\":{\"value\":%.1f}}}\r\n", temp);
                        sent = 1;
                    }
                    break;
                case 1:
                    snprintf(report_buf, sizeof(report_buf),
                             "{\"id\":\"002\",\"version\":\"1.0\",\"params\":"
                             "{\"Beep\":{\"value\":%s}}}\r\n", beep_str);
                    sent = 1;
                    break;
                case 2:
                    if (hum >= 0.0f && hum <= 100.0f) {
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"003\",\"version\":\"1.0\",\"params\":"
                                 "{\"Hum\":{\"value\":%.1f}}}\r\n", hum);
                        sent = 1;
                    }
                    break;
                case 3:
                    if (body >= 0.0f && body <= 100.0f) {
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"004\",\"version\":\"1.0\",\"params\":"
                                 "{\"Body_Tem\":{\"value\":%.1f}}}\r\n", body);
                        sent = 1;
                    }
                    break;
                case 4:
                    if (smoke >= 0.0f && smoke <= 100.0f) {
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"005\",\"version\":\"1.0\",\"params\":"
                                 "{\"smoke_ppm\":{\"value\":%.2f}}}\r\n", smoke);
                        sent = 1;
                    }
                    break;
                case 5:
                    /* 与 LCD 显示一致: 使用 g_saved_hr, HR>90 减 20 */
                    if (g_measurement_ready && g_saved_hr > 0) {
                        long display_hr = g_saved_hr;
                        if (display_hr > 90) {
                            display_hr -= 20;
                        }
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"006\",\"version\":\"1.0\",\"params\":"
                                 "{\"Heart_Rate\":{\"value\":%ld}}}\r\n", (long)display_hr);
                        sent = 1;
                    }
                    break;
                case 6:
                    /* 与 LCD 显示一致: 使用 g_saved_spo2 */
                    if (g_measurement_ready && g_saved_spo2 > 0) {
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"007\",\"version\":\"1.0\",\"params\":"
                                 "{\"SQO2\":{\"value\":%ld}}}\r\n", (long)g_saved_spo2);
                        sent = 1;
                    }
                    break;
                default:
                    if (servo >= 0.0f && servo <= 180.0f) {
                        snprintf(report_buf, sizeof(report_buf),
                                 "{\"id\":\"008\",\"version\":\"1.0\",\"params\":"
                                 "{\"servo\":{\"value\":%d}}}\r\n", (int)(servo + 0.5f));
                        sent = 1;
                    }
                    break;
            }

            report_index = (report_index + 1) % 8;
            if (sent) {
                report_ready = 1;
                break;
            }
            }
        }
    }
}
