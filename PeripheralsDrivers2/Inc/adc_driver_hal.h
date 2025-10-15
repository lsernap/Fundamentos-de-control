/*
 * adc_driver_hal.h
 *
 *  Created on: 13 oct. 2023
 *      Author: luciana
 */

#ifndef ADC_DRIVER_HAL_H_
#define ADC_DRIVER_HAL_H_

#include <stm32f4xx.h>
#include <stdint.h>
#include <PwmDriver.h>

enum{

ADC_CHANNEL_0 = 0,
ADC_CHANNEL_1,
ADC_CHANNEL_2,
ADC_CHANNEL_3,
ADC_CHANNEL_4,
ADC_CHANNEL_5,
ADC_CHANNEL_6,
ADC_CHANNEL_7,
ADC_CHANNEL_8,
ADC_CHANNEL_9,
ADC_CHANNEL_10,
ADC_CHANNEL_11,
ADC_CHANNEL_12,
ADC_CHANNEL_13,
ADC_CHANNEL_14,
ADC_CHANNEL_15,
};

enum{
ADC_RESOLUTION_12_BIT = 0,
ADC_RESOLUTION_10_BIT,
ADC_RESOLUTION_8_BIT,
ADC_RESOLUTION_6_BIT
};


enum{
ADC_ALIGNMENT_RIGHT = 0,
ADC_ALIGNMENT_LEFT
};

enum{
SCAN_OFF = 0,
SCAN_ON
};

enum{
ADC_INT_DISABLE = 0,
ADC_INT_ENABLE
};

enum{
TRIGGER_AUTO = 0,
TRIGGER_MANUAL,
TRIGGER_EXT
};

enum{
ADC_OFF = 0,
ADC_ON
};

enum{
SAMPLING_PERIOD_3_CYCLES = 0b000,
SAMPLING_PERIOD_15_CYCLES = 0b001,
SAMPLING_PERIOD_28_CYCLES = 0b010,
SAMPLING_PERIOD_56_CYCLES = 0b011,
SAMPLING_PERIOD_84_CYCLES = 0b100,
SAMPLING_PERIOD_112_CYCLES = 0b101,
SAMPLING_PERIOD_144_CYCLES = 0b110,
SAMPLING_PERIOD_480_CYCLES = 0b111
};

enum{
TIMMER_1_CHANNEL_1 = 0,
TIMMER_1_CHANNEL_2,
TIMMER_1_CHANNEL_3,
TIMMER_2_CHANNEL_2,
TIMMER_2_CHANNEL_3,
TIMMER_2_CHANNEL_4,
TIMMER_2_TRGO,
TIMMER_3_CHANNEL_1,
TIMMER_3_TRGO,
TIMMER_4_CHANNEL_4,
TIMMER_5_CHANNEL_1,
TIMMER_5_CHANNEL_2,
TIMMER_5_CHANNEL_3
};

enum{
TRIGGER_DISABLED = 0,
TRIGGER_RISING_EDGE,
TRIGGER_FALLING_EDGE,
TRIGGER_RISING_FALLING_EDGE
};


typedef struct
{
uint8_t channel; // Canal ADC que será utilizado para la conversión ADC
uint8_t resolution; // Precisión con la que el ADC hace la adquisición del dato
uint16_t samplingPeriod; // Tiempo deseado para hacer la adquisición del dato
uint8_t dataAlignment; // Alineación a la izquierda o a la derecha
uint16_t adcData; //Dato de la conversión
uint8_t interrupState; // Para configurar si se desea trabajar con la interrupcion o no
uint8_t triggerState;   // EL modo para configurar el trigger
}ADC_Config_t;

void adc_ConfigSingleChannel(ADC_Config_t *adcConfig);
void adc_ConfigAnalogPin(uint8_t adcChannel);
void adc_CompleteCallback(void);
void adc_StartSingleConv(void);
void adc_ScanMode(uint8_t state);
void adc_StartContinuousConv(void);
void adc_StopContinuousConv(void);
void adc_peripheralOnOFF(uint8_t state);
uint16_t adc_GetValue(void);
void adc_resetConfigAnalogPin(ADC_Config_t *adcConfig,uint8_t adcChannel);

//Cpnfiguracion avanzada del ADC
void adc_ConfigMultichannel(ADC_Config_t *adcConfig[16], uint8_t numeroDeCanales);
void adc_ConfigTrigger(uint8_t sourceType, PWM_Handler_t *triggerSignal);

#endif /* ADC_DRIVER_HAL_H_ */
