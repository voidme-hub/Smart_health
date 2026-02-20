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
#include "lcd.h"
#include "key.h"
#include "servo.h"
#include "mq2.h"


/* Global define */
#define TASK1_TASK_PRIO 5
#define TASK1_STK_SIZE 256

#define TASK2_TASK_PRIO 5
#define TASK2_STK_SIZE 384

#define TASK3_TASK_PRIO 5
#define TASK3_STK_SIZE 384

#define TASK4_TASK_PRIO 5
#define TASK4_STK_SIZE 256

#define TASK5_TASK_PRIO 5
#define TASK5_STK_SIZE 384

#define TASK6_TASK_PRIO 5
#define TASK6_STK_SIZE 256

#define TASK7_TASK_PRIO 5
#define TASK7_STK_SIZE 256

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
TaskHandle_t lcd_task_Handler;
TaskHandle_t key_task_Handler;
TaskHandle_t mq2_task_Handler;

TaskHandle_t log_task_Handler;
QueueHandle_t log_queue;

typedef struct {
  char msg[LOG_MSG_MAX_LEN];
} log_item_t;



static void log_printf(const char *fmt, ...) {
  log_item_t item;
  va_list args;

  va_start(args, fmt);
  vsnprintf(item.msg, LOG_MSG_MAX_LEN, fmt, args);
  va_end(args);

  xQueueSend(log_queue, &item, 0);
}

static void log_task(void *pvParameters) {
  log_item_t item;
  while(1) {
    if (xQueueReceive(log_queue, &item, portMAX_DELAY) == pdPASS) {
      printf("%s\r\n", item.msg);
    }
  }
}

static int log_init(void) {
  log_queue = xQueueCreate(LOG_QUEUE_LENGTH, sizeof(log_item_t));
  if (log_queue == 0)  return 0;
  
  if (xTaskCreate(log_task, "log_task", LOG_TASK_STK_SIZE, NULL,
                  LOG_TASK_PRIO, &log_task_Handler) != pdPASS)   return 0;
  return 1;
}

#define LOG(...) log_printf(__VA_ARGS__)



static void led_task(void *pvParameters) {
  GPIO_OUT_PP(GPIO_PORT_C, 0);
  while (1) {
    GPIO_SetBits(GPIOC, GPIO_Pin_0);
    vTaskDelay(500);
    GPIO_ResetBits(GPIOC, GPIO_Pin_0);
    vTaskDelay(500);
  }
}



static void B1uart_task(void *pvParameters) {
  USART3_Init();
  while (1) {
    Take_Temperature();
    LOG("Body_Temp:%.2f\r\n", Body_Temp);
    vTaskDelay(500);
  }
}

static void sht31_task(void *pvParameters) {
  int ret = 0;
  IIC_Software_Init();
  vTaskDelay(20);
  LOG("sht31_task start");
  while (1) {
    ret = SHT31_ReadData();
    if (ret != 0) {
      LOG("SHT31 err:%d", ret);
    } else {
      LOG("T:%0.1fC H:%0.1f%%", temp_val, hum_val);
    }
    vTaskDelay(500);
  }
}

static void max30102_task(void *pvParameters) {
  uint8_t part_id = 0;
  uint8_t rev_id = 0;
  int ret = 0;
  LOG("max30102_task start");
  vTaskDelay(200);

  while (1) {
    part_id = max30102_Bus_Read(REG_PART_ID);
    rev_id = max30102_Bus_Read(REG_REV_ID);
    ret = MAX30102_Read_Data();
    if (ret != 0) {
      LOG("MAX30102 read timeout");
      vTaskDelay(200);
      continue;
    }
    Calculate_Heart_Rate_and_SpO2();
    Update_Signal_Min_Max();
    Process_And_Display_Data();
    // LOG("MAX30102 PART:0x%02X REV:0x%02X HR:%ldbpm(%d) SPO2:%ld%%(%d)",
    //     part_id, rev_id, max30102_data.heart_rate,
    //     max30102_data.heart_rate_valid, max30102_data.spO2,
    //     max30102_data.spO2_valid);
    vTaskDelay(200);
  }
}

static void lcd_task(void *pvParameters){ 
	
	IO_init();
	SPI_SCK_0;
	SPI_BLK_1;
	TFT_init();

	while(1)
	{
		char line1[9];
		char line2[9];
		char line3[9];
		char line4[9];
		char line5[9];
		char line6[9];
		int32_t bt10 = (int32_t)(Body_Temp * 10.0f + (Body_Temp >= 0.0f ? 0.5f : -0.5f));
		int32_t t10 = (int32_t)(temp_val * 10.0f + (temp_val >= 0.0f ? 0.5f : -0.5f));
		int32_t h10 = (int32_t)(hum_val * 10.0f + 0.5f);
		int32_t mq = (int32_t)(Smoke_PPM + 0.5f);
		int32_t bt_abs = bt10 < 0 ? -bt10 : bt10;
		int32_t t_abs = t10 < 0 ? -t10 : t10;

		if (bt10 < 0) {
			snprintf(line1, sizeof(line1), "BT:-%ld.%1ld", (long)(bt_abs / 10), (long)(bt_abs % 10));
		} else {
			snprintf(line1, sizeof(line1), "BT:%2ld.%1ld", (long)(bt_abs / 10), (long)(bt_abs % 10));
		}
		if (t10 < 0) {
			snprintf(line2, sizeof(line2), "T:-%ld.%1ld", (long)(t_abs / 10), (long)(t_abs % 10));
		} else {
			snprintf(line2, sizeof(line2), "T:%2ld.%1ld", (long)(t_abs / 10), (long)(t_abs % 10));
		}
		snprintf(line3, sizeof(line3), "H:%2ld.%1ld", (long)(h10 / 10), (long)(h10 % 10));
		if (max30102_data.heart_rate_valid) {
			snprintf(line4, sizeof(line4), "HR:%3ld", (long)max30102_data.heart_rate);
		} else {
			snprintf(line4, sizeof(line4), "HR:---");
		}
		if (max30102_data.spO2_valid) {
			snprintf(line5, sizeof(line5), "S:%3ld", (long)max30102_data.spO2);
		} else {
			snprintf(line5, sizeof(line5), "S:---");
		}
		snprintf(line6, sizeof(line6), "MQ:%3ld", (long)mq);

		LCD_FillRect(0, 0, 64, 64, WHITE);
		LCD_DrawString(0, 0, BLUE, WHITE, line1);
		LCD_DrawString(0, 8, BLUE, WHITE, line2);
		LCD_DrawString(0, 16, BLUE, WHITE, line3);
		LCD_DrawString(0, 24, BLUE, WHITE, line4);
		LCD_DrawString(0, 32, BLUE, WHITE, line5);
		LCD_DrawString(0, 40, BLUE, WHITE, line6);
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

static void key_task(void *pvParameters) {
  key_init();
  Servo_Init();
  Servo_SetAngle(0.0f);
  vTaskDelay(20);
  while (1) {
    if (key_is_pressed()) {
      Servo_SetAngle(90.0f);
    } else {
      Servo_SetAngle(0.0f);
    }
    vTaskDelay(20);
  }
}

static void mq2_task(void *pvParameters) {
  
  while(1){
    MQ2_ReadData();
    LOG("MQ2_PPM:%0.2f%%",Smoke_PPM);
    vTaskDelay(500);
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
static void create_task(void *pvParameters) {

  LOG("heap before:%u", (unsigned)xPortGetFreeHeapSize());
  if (xTaskCreate(led_task, "led_task", TASK1_STK_SIZE, NULL,
                  TASK1_TASK_PRIO, &led_task_Handler) != pdPASS) {
    LOG("create led_task fail");
  }
  
  LOG("heap after led:%u", (unsigned)xPortGetFreeHeapSize());
  if (xTaskCreate(max30102_task, "max30102_task", TASK2_STK_SIZE, NULL,
                  TASK2_TASK_PRIO, &max30102_task_Handler) != pdPASS) {
    LOG("create max30102_task fail");
  }

  LOG("heap after max:%u", (unsigned)xPortGetFreeHeapSize());
  if (xTaskCreate(sht31_task, "sht31_task", TASK3_STK_SIZE, NULL,
                  TASK3_TASK_PRIO, &sht31_task_Handler) != pdPASS) {
    LOG("create sht31_task fail");
  }
  LOG("heap after sht31:%u", (unsigned)xPortGetFreeHeapSize());

  if (xTaskCreate(B1uart_task, "B1uart_task", TASK4_STK_SIZE, NULL,
                  TASK4_TASK_PRIO, &B1uart_task_Handler) != pdPASS) {
    LOG("create B1uart_task fail");
  }
  LOG("heap after B1uart:%u", (unsigned)xPortGetFreeHeapSize());

  if (xTaskCreate(lcd_task, "lcd_task", TASK5_STK_SIZE, NULL,
                  TASK5_TASK_PRIO, &lcd_task_Handler) != pdPASS) {
    LOG("create lcd_task fail");
  }
    LOG("heap after lcd:%u", (unsigned)xPortGetFreeHeapSize());

  if (xTaskCreate(key_task, "key_task", TASK6_STK_SIZE, NULL,
                  TASK6_TASK_PRIO, &key_task_Handler) != pdPASS) {
    LOG("create key_task fail");
  }
  LOG("heap after key:%u", (unsigned)xPortGetFreeHeapSize());

  if (xTaskCreate(mq2_task, "mq2_task", TASK7_STK_SIZE, NULL,
                  TASK7_TASK_PRIO, &mq2_task_Handler) != pdPASS) {
    LOG("create mq2_task fail");
  }
  LOG("heap after mq2:%u", (unsigned)xPortGetFreeHeapSize());

  vTaskDelete(create_task_Handler);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void) {

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SYS_init(4);
  SystemCoreClockUpdate();
  Delay_Init();
  USART_Printf_Init(115200);
  log_init();
  MAX30102_Init();
  mq2_adc();

  LOG("SystemClk:%d", SystemCoreClock);
  LOG("ChipID:%08x", DBGMCU_GetCHIPID());
  LOG("FreeRTOS Kernel Version:%s", tskKERNEL_VERSION_NUMBER);

  xTaskCreate(create_task, "create_task", TASK1_STK_SIZE, NULL, TASK1_TASK_PRIO,
              &create_task_Handler);

  vTaskStartScheduler();
  while (1) {
    LOG("shouldn't run at here!!");
  }
}
