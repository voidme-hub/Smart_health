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
    .brightness = 0                 
};

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

// Read Data from MAX30102 (Blocking 500 samples)
int MAX30102_Read_Data(void)
{
  volatile uint32_t un_prev_data = 0;
  uint8_t temp[6];
  int status = 0;

  for (int i = 0; i < max30102_data.buffer_length; i++)
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

    max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);

    max30102_data.red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2];
    max30102_data.ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

    if (max30102_data.min_value > max30102_data.red_buffer[i])
      max30102_data.min_value = max30102_data.red_buffer[i];
    if (max30102_data.max_value < max30102_data.red_buffer[i])
      max30102_data.max_value = max30102_data.red_buffer[i];
  }

  un_prev_data = max30102_data.red_buffer[max30102_data.buffer_length - 1];
  max30102_data.heart_rate += HEART_RATE_COMPENSATION;

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
  if (max30102_data.heart_rate_valid == 1 && max30102_data.heart_rate < 120)
  {
    dis_hr = max30102_data.heart_rate;
    dis_spo2 = max30102_data.spO2;
    printf("dis_hr:%d  ,dis_spo2:%d\r\n",dis_hr,dis_spo2);
  }
}

