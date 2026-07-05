#include "max30102_app.h"

/*
 * Version: 1.0.0
 * Date: 2025-12-28
 * Description: Ported to CH32V203.
 */

// Init data structure
MAX30102_Data max30102_data = {
    .buffer_length = BUFFER_LENGTH,
    .min_value = 0x3FFFF,
    .max_value = 0,
    .brightness = 0,
    .finger_detected = 0,
    .finger_stable_count = 0
};

static int32_t hr_filtered = 0;
static int32_t spo2_filtered = 0;
static uint8_t hr_filtered_valid = 0;
static uint8_t spo2_filtered_valid = 0;

// 滚动缓冲区索引
static uint16_t buffer_index = 0;
static uint8_t buffer_filled = 0;

/********************************** Filter Variables *************************************************/

// Moving Average Buffer
int hr_buffer[WINDOW_SIZE] = {0};   
int spo2_buffer[WINDOW_SIZE] = {0}; 
int hr_index = 0, spo2_index = 0;   

// Low Pass Filter previous values
int prev_hr = 0, prev_spo2 = 0; 

/********************************** Filter Algorithms *************************************************/

// Moving Average
int SmoothData(int new_value, int *buffer, int *index)
{
  buffer[*index] = new_value;          
  *index = (*index + 1) % WINDOW_SIZE; 

  int sum = 0;
  for (int i = 0; i < WINDOW_SIZE; i++)
  {
    sum += buffer[i]; 
  }

  return sum / WINDOW_SIZE; 
}

// Low Pass Filter
int LowPassFilter(int new_value, int previous_filtered_value)
{
  return (int)(ALPHA * new_value + (1 - ALPHA) * previous_filtered_value); 
}

/********************************** Functions *************************************************/

// 检测手指是否稳定放置
// 返回: 1=手指稳定检测到, 0=无手指或不稳定
uint8_t MAX30102_Check_Finger(void)
{
  static uint32_t last_ir = 0;
  uint32_t current_ir;

  /* 获取当前IR值（最新样本）*/
  if (buffer_index > 0)
    current_ir = max30102_data.ir_buffer[buffer_index - 1];
  else
    current_ir = max30102_data.ir_buffer[BUFFER_LENGTH - 1];

  /* 检查IR信号是否超过阈值 */
  if (current_ir > FINGER_THRESHOLD)
  {
    /* 检查信号稳定性（变化不超过20%）*/
    if (last_ir > 0)
    {
      uint32_t delta = (current_ir > last_ir) ? (current_ir - last_ir) : (last_ir - current_ir);
      uint32_t threshold = last_ir / 5;  // 20%变化阈值

      if (delta < threshold)
      {
        /* 信号稳定，增加计数 */
        if (max30102_data.finger_stable_count < FINGER_STABLE_COUNT_MAX)
          max30102_data.finger_stable_count++;

        /* 达到稳定要求 */
        if (max30102_data.finger_stable_count >= FINGER_STABLE_COUNT)
        {
          max30102_data.finger_detected = 1;
          last_ir = current_ir;
          return 1;
        }
      }
      else
      {
        /* 信号不稳定，重置计数 */
        max30102_data.finger_stable_count = 0;
        max30102_data.finger_detected = 0;
      }
    }

    last_ir = current_ir;
  }
  else
  {
    /* IR信号过低，无手指 */
    max30102_data.finger_stable_count = 0;
    max30102_data.finger_detected = 0;
    last_ir = 0;
  }

  return 0;
}

// 读取新样本并添加到滚动缓冲区（非阻塞）
// 返回: 0=成功添加样本, -1=无新数据, -2=超时
int MAX30102_Read_Sample(void)
{
  uint8_t temp[6];
  static uint32_t last_timeout = 0;

  /* 检查是否有新数据 */
  if (MAX30102_INT == 1)
  {
    if (++last_timeout > 100)  // 快速超时检测
    {
      last_timeout = 0;
      return -2;
    }
    return -1;  // 无新数据
  }

  last_timeout = 0;

  /* 读取FIFO数据 */
  max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

  /* 提取数据并存入滚动缓冲区 */
  uint32_t red = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];
  uint32_t ir = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

  max30102_data.red_buffer[buffer_index] = red;
  max30102_data.ir_buffer[buffer_index] = ir;

  /* 更新最小最大值 */
  if (max30102_data.min_value > red)
    max30102_data.min_value = red;
  if (max30102_data.max_value < red)
    max30102_data.max_value = red;

  /* 更新索引 */
  buffer_index++;
  if (buffer_index >= BUFFER_LENGTH)
  {
    buffer_index = 0;
    buffer_filled = 1;
  }

  return 0;
}

// 兼容旧版本的阻塞采集（仅用于初始填充）
int MAX30102_Read_Data(void)
{
  uint8_t temp[6];
  int status = 0;

  /* 快速采集初始数据 */
  for (int i = 0; i < BUFFER_LENGTH; i++)
  {
    uint32_t timeout = 0;
    while (MAX30102_INT == 1)
    {
      if (++timeout > 200000U)
      {
        status = -1;
        break;
      }
    }
    if (status != 0)
    {
      return status;
    }

    /* 直接读取FIFO */
    max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

    max30102_data.red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];
    max30102_data.ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

    if (max30102_data.min_value > max30102_data.red_buffer[i])
      max30102_data.min_value = max30102_data.red_buffer[i];
    if (max30102_data.max_value < max30102_data.red_buffer[i])
      max30102_data.max_value = max30102_data.red_buffer[i];
  }

  buffer_index = 0;
  buffer_filled = 1;

  return 0;
}

// Calculate HR and SpO2
void Calculate_Heart_Rate_and_SpO2(void)
{
  maxim_heart_rate_and_oxygen_saturation(max30102_data.ir_buffer, max30102_data.buffer_length,
                                         max30102_data.red_buffer, &max30102_data.spO2, &max30102_data.spO2_valid,
                                         &max30102_data.heart_rate, &max30102_data.heart_rate_valid);
}

// Update Signal Min/Max and Filter
void Update_Signal_Min_Max(void)
{
  uint32_t un_prev_data = max30102_data.red_buffer[max30102_data.buffer_length - 1];

  // Process data from index 100? (Original logic preserved)
  for (int i = 100; i < max30102_data.buffer_length; i++)
  {
    // Shift data
    max30102_data.red_buffer[i - 100] = max30102_data.red_buffer[i];
    max30102_data.ir_buffer[i - 100] = max30102_data.ir_buffer[i];

    // Low Pass Filter
    max30102_data.red_buffer[i - 100] = LowPassFilter(max30102_data.red_buffer[i - 100], un_prev_data);
    max30102_data.ir_buffer[i - 100] = LowPassFilter(max30102_data.ir_buffer[i - 100], un_prev_data);

    // Smooth
    max30102_data.red_buffer[i - 100] = SmoothData(max30102_data.red_buffer[i - 100], hr_buffer, &hr_index);
    max30102_data.ir_buffer[i - 100] = SmoothData(max30102_data.ir_buffer[i - 100], spo2_buffer, &spo2_index);

    // Update Min/Max
    if (max30102_data.min_value > max30102_data.red_buffer[i - 100])
      max30102_data.min_value = max30102_data.red_buffer[i - 100];
    if (max30102_data.max_value < max30102_data.red_buffer[i - 100])
      max30102_data.max_value = max30102_data.red_buffer[i - 100];
  }
}

// Display Variables
uint8_t dis_hr = 0;   
uint8_t dis_spo2 = 0; 

// Process and Display
void Process_And_Display_Data(void)
{
  if (max30102_data.heart_rate_valid == 1 &&
      max30102_data.heart_rate >= 40 &&
      max30102_data.heart_rate <= 180)
  {
    if (hr_filtered_valid == 0) {
      hr_filtered = max30102_data.heart_rate;
      hr_filtered_valid = 1;
    } else {
      hr_filtered = (hr_filtered * 3 + max30102_data.heart_rate) / 4;
    }
    max30102_data.heart_rate = hr_filtered;
    max30102_data.heart_rate_valid = 1;
  }
  else
  {
    max30102_data.heart_rate_valid = 0;
  }

  if (max30102_data.spO2_valid == 1 &&
      max30102_data.spO2 >= 70 &&
      max30102_data.spO2 <= 100)
  {
    if (spo2_filtered_valid == 0) {
      spo2_filtered = max30102_data.spO2;
      spo2_filtered_valid = 1;
    } else {
      spo2_filtered = (spo2_filtered * 3 + max30102_data.spO2) / 4;
    }
    max30102_data.spO2 = spo2_filtered;
    max30102_data.spO2_valid = 1;
  }
  else
  {
    max30102_data.spO2_valid = 0;
  }

  if (max30102_data.heart_rate_valid == 1)
  {
    dis_hr = max30102_data.heart_rate;
  }
  if (max30102_data.spO2_valid == 1)
  {
    dis_spo2 = max30102_data.spO2;
  }
}
