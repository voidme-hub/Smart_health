#ifndef __MAX30102_APP_H
#define __MAX30102_APP_H

#include "max30102.h"
#include "algorithm.h"

#define BUFFER_LENGTH BUFFER_SIZE
#define WINDOW_SIZE 5
#define ALPHA 0.2f
#define HEART_RATE_COMPENSATION 0

/* 手指检测阈值和参数 */
#define FINGER_THRESHOLD 10000      // IR信号阈值，低于此值认为无手指
#define FINGER_STABLE_COUNT 50      // 稳定计数（50样本 = 0.5秒 @ 100Hz）
#define FINGER_STABLE_COUNT_MAX 100 // 最大稳定计数（1秒）

/* 数据有效性范围 */
#define HR_MIN 60                   // 心率最小值
#define HR_MAX 90                  // 心率最大值
#define SPO2_MIN 95                 // 血氧最小值（不含）
#define SPO2_MAX 100                // 血氧最大值

/* 测量模式 */
typedef enum {
    MEASURE_IDLE,                   // 空闲，等待手指
    MEASURE_DETECTING,              // 检测手指稳定
    MEASURE_CALCULATING,            // 计算中
    MEASURE_DISPLAY,                // 显示结果，等待3秒
    MEASURE_HOLD                    // 保持显示（手指移开）
} MeasureState;

typedef struct
{
    int32_t buffer_length;
    uint32_t red_buffer[BUFFER_LENGTH];
    uint32_t ir_buffer[BUFFER_LENGTH];
    uint32_t min_value;
    uint32_t max_value;
    uint8_t brightness;
    int32_t spO2;
    int8_t spO2_valid;
    int32_t heart_rate;
    int8_t heart_rate_valid;

    /* 手指检测状态 */
    uint8_t finger_detected;        // 手指是否检测到
    uint16_t finger_stable_count;   // 稳定计数器
} MAX30102_Data;

extern MAX30102_Data max30102_data;

uint8_t MAX30102_Check_Finger(void);  // 检测手指是否稳定放置
int MAX30102_Read_Sample(void);       // 非阻塞读取单个样本
int MAX30102_Read_Data(void);         // 阻塞采集（仅用于初始化）
void Calculate_Heart_Rate_and_SpO2(void);
void Update_Signal_Min_Max(void);
void Process_And_Display_Data(void);


#endif
