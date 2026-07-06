/********************************** (C) COPYRIGHT
 * ******************************* File Name          : main.c Author   : WCH
 * Version            : V1.0.0
 * Date               : 2021/06/06
 * Description        : Main program body.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/

/*
 *@Note
 *task1 and task2 alternate printing
 */

#include "FreeRTOS.h"
#include "debug.h"
#include "task.h"
#include "queue.h"
#include "stdarg.h"

#include "com.h"
#include "max30102_app.h"
#include "sht31.h"
#include "B1uart.h"
#include "Core-Y100P.h"
#include "lcd.h"
#include "key.h"
#include "servo.h"
#include "mq2.h"
#include "iwdg.h"
#include "beep.h"
#include "pic.h"
#include "tim.h"


/* Global define */
#define TASK1_TASK_PRIO 5
#define TASK1_STK_SIZE 256

#define TASK2_TASK_PRIO 5
#define TASK2_STK_SIZE 1024

#define TASK3_TASK_PRIO 5
#define TASK3_STK_SIZE 512

#define TASK4_TASK_PRIO 5
#define TASK4_STK_SIZE 512

#define TASK5_TASK_PRIO 5
#define TASK5_STK_SIZE 512

#define TASK6_TASK_PRIO 5
#define TASK6_STK_SIZE 512

#define TASK7_TASK_PRIO 5
#define TASK7_STK_SIZE 512

#define TASK8_TASK_PRIO 5
#define TASK8_STK_SIZE 512

#define TASK9_TASK_PRIO 5
#define TASK9_STK_SIZE 512

#define LOG_TASK_PRIO 6
#define LOG_TASK_STK_SIZE 512
#define LOG_QUEUE_LENGTH 16
#define LOG_MSG_MAX_LEN 96

/* Global Variable */
TaskHandle_t led_task_Handler;
TaskHandle_t create_task_Handler;
TaskHandle_t max30102_task_Handler;
TaskHandle_t sht31_task_Handler;
TaskHandle_t B1uart_task_Handler;
TaskHandle_t Core_Y100P_task_Handler;
TaskHandle_t lcd_task_Handler;
TaskHandle_t key_task_Handler;
TaskHandle_t mq2_task_Handler;
TaskHandle_t beep_task_Handler;

static volatile uint8_t lcd_page = 0;
volatile float servo_angle = 0.0f;
volatile uint8_t servo_override = 0;
volatile uint8_t beep_alarm = 0;

/* MAX30102测量结果（用于LCD显示）*/
volatile long g_saved_hr = 0;
volatile long g_saved_spo2 = 0;
volatile uint8_t g_measurement_ready = 0;


TaskHandle_t log_task_Handler;
QueueHandle_t log_queue;

typedef struct {
    char msg[LOG_MSG_MAX_LEN];
} log_item_t;

static void log_printf (const char *fmt, ...) {
    log_item_t item;
    va_list args;

    va_start (args, fmt);
    vsnprintf (item.msg, LOG_MSG_MAX_LEN, fmt, args);
    va_end (args);

    xQueueSend (log_queue, &item, 0);
}

static void log_task (void *pvParameters) {
    log_item_t item;
    while (1) {
        if (xQueueReceive (log_queue, &item, portMAX_DELAY) == pdPASS) {
            printf ("%s\r\n", item.msg);
        }
    }
}

static int log_init (void) {
    log_queue = xQueueCreate (LOG_QUEUE_LENGTH, sizeof (log_item_t));
    if (log_queue == 0)
        return 0;

    if (xTaskCreate (log_task, "log_task", LOG_TASK_STK_SIZE, NULL,
                     LOG_TASK_PRIO, &log_task_Handler) != pdPASS)
        return 0;
    return 1;
}

#define LOG(...) log_printf (__VA_ARGS__)

static void led_task (void *pvParameters) {
    GPIO_OUT_PP (GPIO_PORT_C, 0);
    while (1) {
        static TickType_t start_tick = 0;
        if (start_tick == 0) {
            start_tick = xTaskGetTickCount();
        }
        if ((xTaskGetTickCount() - start_tick) < pdMS_TO_TICKS (180000)) {
            IWDG_Feed();
        }
        GPIO_SetBits (GPIOC, GPIO_Pin_0);
        vTaskDelay (500);
        GPIO_ResetBits (GPIOC, GPIO_Pin_0);
        vTaskDelay (500);
    }
}

static void beep_task (void *pvParameters) {

    while (1) {
        if (beep_alarm) {
            BEEP_ON;
        } else {
            BEEP_OFF;
        }
        vTaskDelay (pdMS_TO_TICKS (50));
    }
}

static void B1uart_task (void *pvParameters) {
    USART3_Init();
    while (1) {
        Take_Temperature();
        LOG ("Body_Temp:%.2f\r\n", Body_Temp);
        vTaskDelay (500);
    }
}

static void Core_Y100P_task (void *pvParameters) {
    while (1) {
        if (uart5_rx.flag == 1) {
            uart5_rx.flag = 0;
            LOG ("%s", uart5_rx.data);
            {
                float angle = 0.0f;
                if (parse_servo_angle ((const char *)uart5_rx.data, &angle)) {
                    if (angle >= 0.0f && angle <= 180.0f) {
                        servo_angle = angle;
                        servo_override = 1;
                        Servo_SetAngle (servo_angle);
                    }
                }
            }
        }
        if (report_ready) {
            UART5_SendDate ((const uint8_t *)report_buf);
            report_ready = 0;
        }
        vTaskDelay (5);
    }
}

static void sht31_task (void *pvParameters) {
    int ret = 0;
    IIC_Software_Init();
    vTaskDelay (20);
    LOG ("sht31_task start");
    while (1) {
        ret = SHT31_ReadData();
        if (ret != 0) {
            LOG ("SHT31 err:%d", ret);
        } else {
            LOG ("T:%0.1fC H:%0.1f%%", temp_val, hum_val);
        }
        vTaskDelay (500);
    }
}

static void max30102_task (void *pvParameters) {
    int ret = 0;
    MeasureState state = MEASURE_IDLE;
    uint16_t sample_count = 0;
    uint32_t display_timer = 0;

    LOG("MAX30102 Task started, waiting for finger...");

    while (1) {
        /* 状态机处理 */
        switch (state) {
            case MEASURE_IDLE:
                /* 等待手指放置 */
                g_measurement_ready = 0;  // 清除显示标志
                ret = MAX30102_Read_Sample();
                if (ret == 0) {
                    if (MAX30102_Check_Finger()) {
                        LOG("Finger detected, measuring...");
                        state = MEASURE_DETECTING;
                        sample_count = 0;
                    }
                }
                vTaskDelay(2);
                break;

            case MEASURE_DETECTING:
                /* 持续检测手指稳定并采集数据 */
                ret = MAX30102_Read_Sample();
                if (ret == 0) {
                    if (!MAX30102_Check_Finger()) {
                        /* 手指移开，返回IDLE */
                        LOG("Finger removed during detection");
                        state = MEASURE_IDLE;
                        break;
                    }

                    sample_count++;
                    /* 采集足够数据后开始计算 */
                    if (sample_count >= 20) {
                        state = MEASURE_CALCULATING;
                        sample_count = 0;
                    }
                }
                vTaskDelay(2);
                break;

            case MEASURE_CALCULATING:
                /* 计算心率血氧 */
                ret = MAX30102_Read_Sample();
                if (ret == 0) {
                    sample_count++;

                    /* 每20个样本计算一次 */
                    if (sample_count >= 20) {
                        sample_count = 0;

                        taskENTER_CRITICAL();
                        Calculate_Heart_Rate_and_SpO2();
                        Update_Signal_Min_Max();
                        Process_And_Display_Data();

                        long hr = max30102_data.heart_rate;
                        int hr_valid = max30102_data.heart_rate_valid;
                        long spo2 = max30102_data.spO2;
                        int spo2_valid = max30102_data.spO2_valid;
                        taskEXIT_CRITICAL();

                        /* 检查数据有效性：60 <= HR <= 100 且 95 < SpO2 <= 100 */
                        if (hr_valid && spo2_valid &&
                            hr >= HR_MIN && hr <= HR_MAX &&
                            spo2 > SPO2_MIN && spo2 <= SPO2_MAX) {

                            /* 数据有效！保存并显示 */
                            g_saved_hr = hr;
                            g_saved_spo2 = spo2;
                            g_measurement_ready = 1;  // 通知LCD可以显示

                            LOG("Valid data: HR=%ldbpm, SpO2=%ld%%", g_saved_hr, g_saved_spo2);
                            LOG("Display for 3 seconds...");

                            state = MEASURE_DISPLAY;
                            display_timer = xTaskGetTickCount();
                            break;
                        } else {
                            LOG("Invalid data: HR=%ld(%d) SpO2=%ld(%d), continue...",
                                hr, hr_valid, spo2, spo2_valid);
                        }
                    }
                }
                vTaskDelay(2);
                break;

            case MEASURE_DISPLAY:
                /* 显示结果3秒 */
                if ((xTaskGetTickCount() - display_timer) >= pdMS_TO_TICKS(2000)) {
                    /* 3秒到，检查手指状态 */
                    ret = MAX30102_Read_Sample();
                    if (ret == 0 && MAX30102_Check_Finger()) {
                        /* 手指还在，重新测量 */
                        LOG("Finger still present, remeasuring...");
                        state = MEASURE_DETECTING;
                        sample_count = 0;
                    } else {
                        /* 手指移开或无信号，保持显示 */
                        LOG("Measurement complete, holding display");
                        LOG("Final result: HR=%ldbpm, SpO2=%ld%%", g_saved_hr, g_saved_spo2);
                        state = MEASURE_HOLD;
                    }
                }
                vTaskDelay(50);
                break;

            case MEASURE_HOLD:
                /* 保持显示，停止检测 */
                /* g_measurement_ready保持为1，LCD继续显示 */
                /* 可以添加重启测量的逻辑（例如按键） */
                vTaskDelay(1000);
                break;

            default:
                state = MEASURE_IDLE;
                break;
        }
    }
}

static void lcd_task (void *pvParameters) {

    TIM_ITConfig (TIM6, TIM_IT_Update, DISABLE);
    TIM_Cmd (TIM6, DISABLE);
    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
    LCD_GPIOE_Init();
    TFT_init();
    LCD_DrawImageFull (gImage_aila);
    TIM_ClearITPendingBit (TIM6, TIM_IT_Update);
    TIM_ITConfig (TIM6, TIM_IT_Update, ENABLE);
    TIM_Cmd (TIM6, ENABLE);

    while (1) {
        char line1[10];
        char line2[10];
        char line3[10];
        int32_t bt10 = (int32_t)(Body_Temp * 10.0f + (Body_Temp >= 0.0f ? 0.5f : -0.5f));
        int32_t t10 = (int32_t)(temp_val * 10.0f + (temp_val >= 0.0f ? 0.5f : -0.5f));
        int32_t h10 = (int32_t)(hum_val * 10.0f + 0.5f);
        int32_t mq = (int32_t)(Smoke_PPM + 0.5f);
        int32_t bt_abs = bt10 < 0 ? -bt10 : bt10;
        int32_t t_abs = t10 < 0 ? -t10 : t10;

        if (lcd_page == 0) {
            if (bt10 < 0) {
                snprintf (line1, sizeof (line1), ":0%ld.%1ld", (long)(bt_abs / 10), (long)(bt_abs % 10));
            } else {
                snprintf (line1, sizeof (line1), ":%2ld.%1ld", (long)(bt_abs / 10), (long)(bt_abs % 10));
            }

            /* 使用全局变量显示测量结果 */
            if (g_measurement_ready && g_saved_hr > 0) {
                /* 心率>90时自动减20 */
                long display_hr = g_saved_hr;
                if (display_hr > 90) {
                    display_hr -= 20;
                }
                snprintf (line2, sizeof (line2), ":%ld", display_hr);
            } else {
                snprintf (line2, sizeof (line2), ":---");
            }

            if (g_measurement_ready && g_saved_spo2 > 0) {
                snprintf (line3, sizeof (line3), ":%ld", g_saved_spo2);
            } else {
                snprintf (line3, sizeof (line3), ":---");
            }
        } else {
            if (t10 < 0) {
                snprintf (line1, sizeof (line1), ":-%ld.%1ld", (long)(t_abs / 10), (long)(t_abs % 10));
            } else {
                snprintf (line1, sizeof (line1), ":%2ld.%1ld", (long)(t_abs / 10), (long)(t_abs % 10));
            }
            snprintf (line2, sizeof (line2), ":%2ld.%1ld", (long)(h10 / 10), (long)(h10 % 10));
            snprintf (line3, sizeof (line3), ":%3ld", (long)mq);
        }

        LCD_DrawImageRegion (gImage_aila, 0, 0, 240, 96);
        if (lcd_page == 0) {
            LCD_DrawChinese32Transparent (0, 0, RED, 0);
            LCD_DrawChinese32Transparent (32, 0, RED, 1);
            LCD_DrawChinese32Transparent (0, 32, RED, 2);
            LCD_DrawChinese32Transparent (32, 32, RED, 3);
            LCD_DrawChinese32Transparent (0, 64, RED, 4);
            LCD_DrawChinese32Transparent (32, 64, RED, 5);
        } else {
            LCD_DrawChinese32Transparent (0, 0, RED, 6);
            LCD_DrawChinese32Transparent (32, 0, RED, 1);
            LCD_DrawChinese32Transparent (0, 32, RED, 8);
            LCD_DrawChinese32Transparent (32, 32, RED, 9);
            LCD_DrawChinese32Transparent (0, 64, RED, 10);
            LCD_DrawChinese32Transparent (32, 64, RED, 11);
            LCD_DrawChinese32Transparent (64, 64, RED, 12);
        }
        LCD_DrawString32Transparent (80, 0, RED, line1);
        LCD_DrawString32Transparent (80, 32, RED, line2);
        LCD_DrawString32Transparent (80, 64, RED, line3);
        vTaskDelay (pdMS_TO_TICKS (1000));  // 2秒刷新一次
    }
}

static void key_task (void *pvParameters) {
    key_init();
    while (1) {
        uint8_t cur = key_is_pressed() ? 1 : 0;
        if (cur) {
            lcd_page ^= 1;
        }
        vTaskDelay (10);
    }
}

static void mq2_task (void *pvParameters) {
    TickType_t last_log = 0;
    while (1) {
        MQ2_ReadData();
        if ((xTaskGetTickCount() - last_log) >= pdMS_TO_TICKS (3000)) {
            LOG ("MQ2_PPM:%0.2f%%", Smoke_PPM);
            last_log = xTaskGetTickCount();
        }
        if (servo_override) {
            Servo_SetAngle (servo_angle);
        } else {

            if (beep_alarm) {
                servo_angle = 90.0f;
                Servo_SetAngle (servo_angle);

            } else {
                servo_angle = 180.0f;
                Servo_SetAngle (servo_angle);
            }
        }
        vTaskDelay (pdMS_TO_TICKS (50));
    }
}

/*********************************************************************
 * @fn      create_task
 *
 * @brief   create_task program.
 *
 * @param  *pvParameters - Parameters point of create_task
 *
 * @return  none
 */
static void create_task (void *pvParameters) {

    LOG ("heap before:%u", (unsigned)xPortGetFreeHeapSize());
    if (xTaskCreate (led_task, "led_task", TASK1_STK_SIZE, NULL,
                     TASK1_TASK_PRIO, &led_task_Handler) != pdPASS) {
        LOG ("create led_task fail");
    }

    LOG ("heap after led:%u", (unsigned)xPortGetFreeHeapSize());
    if (xTaskCreate (max30102_task, "max30102_task", TASK2_STK_SIZE, NULL,
                     TASK2_TASK_PRIO, &max30102_task_Handler) != pdPASS) {
        LOG ("create max30102_task fail");
    }

    LOG ("heap after max:%u", (unsigned)xPortGetFreeHeapSize());
    if (xTaskCreate (sht31_task, "sht31_task", TASK3_STK_SIZE, NULL,
                     TASK3_TASK_PRIO, &sht31_task_Handler) != pdPASS) {
        LOG ("create sht31_task fail");
    }
    LOG ("heap after sht31:%u", (unsigned)xPortGetFreeHeapSize());

    if (xTaskCreate (B1uart_task, "B1uart_task", TASK4_STK_SIZE, NULL,
                     TASK4_TASK_PRIO, &B1uart_task_Handler) != pdPASS) {
        LOG ("create B1uart_task fail");
    }
    LOG ("heap after B1uart:%u", (unsigned)xPortGetFreeHeapSize());

    if (xTaskCreate (lcd_task, "lcd_task", TASK5_STK_SIZE, NULL,
                     TASK5_TASK_PRIO, &lcd_task_Handler) != pdPASS) {
        LOG ("create lcd_task fail");
    }
    LOG ("heap after lcd:%u", (unsigned)xPortGetFreeHeapSize());

    if (xTaskCreate (key_task, "key_task", TASK6_STK_SIZE, NULL,
                     TASK6_TASK_PRIO, &key_task_Handler) != pdPASS) {
        LOG ("create key_task fail");
    }
    LOG ("heap after key:%u", (unsigned)xPortGetFreeHeapSize());

    if (xTaskCreate (mq2_task, "mq2_task", TASK7_STK_SIZE, NULL,
                     TASK7_TASK_PRIO, &mq2_task_Handler) != pdPASS) {
        LOG ("create mq2_task fail");
    }
    LOG ("heap after mq2:%u", (unsigned)xPortGetFreeHeapSize());
    if (xTaskCreate (beep_task, "beep_task", TASK8_STK_SIZE, NULL,
                     TASK8_TASK_PRIO, &beep_task_Handler) != pdPASS) {
        LOG ("create beep_task fail");
    }
    LOG ("heap after beep:%u", (unsigned)xPortGetFreeHeapSize());
    if (xTaskCreate (Core_Y100P_task, "Core_Y100P_task", TASK9_STK_SIZE, NULL,
                     TASK9_TASK_PRIO, &Core_Y100P_task_Handler) != pdPASS) {
        LOG ("create Core_Y100P_task fail");
    }
    LOG ("heap after Core_Y100P:%u", (unsigned)xPortGetFreeHeapSize());
    vTaskDelete (create_task_Handler);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_2);
    SYS_init (4);
    SystemCoreClockUpdate();
    Beep_Init();
    Delay_Init();
    USART_Printf_Init (115200);
    UART5_Init (115200);
    TIM6_ReportInit();
    log_init();
    MAX30102_Init();
    Servo_Init();
    mq2_adc();
    BEEP_ON;
    Delay_Ms (100);
    BEEP_OFF;
    LOG ("SystemClk:%d", SystemCoreClock);
    LOG ("ChipID:%08x", DBGMCU_GetCHIPID());
    LOG ("FreeRTOS Kernel Version:%s", tskKERNEL_VERSION_NUMBER);

    xTaskCreate (create_task, "create_task", TASK1_STK_SIZE, NULL, TASK1_TASK_PRIO,
                 &create_task_Handler);
    IWDG_Init3min();
    vTaskStartScheduler();
    while (1) {
        LOG ("shouldn't run at here!!");
    }
}
