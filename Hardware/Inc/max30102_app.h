#ifndef __MAX30102_APP_H
#define __MAX30102_APP_H

#include "max30102.h"
#include "algorithm.h"

#define BUFFER_LENGTH BUFFER_SIZE
#define WINDOW_SIZE 5
#define ALPHA 0.2f
#define HEART_RATE_COMPENSATION 0

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
} MAX30102_Data;

extern MAX30102_Data max30102_data;

int MAX30102_Read_Data(void);
void Calculate_Heart_Rate_and_SpO2(void);
void Update_Signal_Min_Max(void);
void Process_And_Display_Data(void);


#endif
