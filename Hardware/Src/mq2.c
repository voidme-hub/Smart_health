#include "mq2.h"

void mq2_adc (void) {
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (GPIOA, &GPIO_InitStruct);


    ADC_InitTypeDef ADC_InitStructure = {0};
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;        // Single Channel
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;  // Single Conversion
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init (ADC1, &ADC_InitStructure);

    // Configure Channel 1
    ADC_RegularChannelConfig (ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);

    ADC_Cmd (ADC1, ENABLE);

    // Calibration
    ADC_ResetCalibration (ADC1);
    while (ADC_GetResetCalibrationStatus (ADC1));
    ADC_StartCalibration (ADC1);
    while (ADC_GetCalibrationStatus (ADC1));
}


/**
 * @brief  Read MQ2 Data
 * @retval Raw ADC Value
 */
float Smoke_PPM;
u16 MQ2_ReadData (void) {
    float Vol = 0;
    
    ADC_SoftwareStartConvCmd (ADC1, ENABLE);          
    while (!ADC_GetFlagStatus (ADC1, ADC_FLAG_EOC));  
    u16 adc_val = ADC_GetConversionValue(ADC1);
    
    Vol = (3.3f / 4095.0f * adc_val);
    
    // Simple mapping: 0-3.3V -> 0-1000ppm
    Smoke_PPM = (Vol / 3.3f) * 100.0f;
    

    return adc_val;
}

