/*
 * PwmDriver.h
 *
 *  Created on: 3 nov. 2023
 *      Author: luciana
 */

#ifndef PWM_DRIVER_HAL_H_
#define PWM_DRIVER_HAL_H_

#include <stm32f4xx.h>


enum{
PWM_CHANNEL_1 = 0,
PWM_CHANNEL_2,
PWM_CHANNEL_3,
PWM_CHANNEL_4
};

enum{
PWM_DUTTY_0_PERCENT = 0,
PWM_DUTTY_100_PERCENT = 100
};

enum{
PWM_UP_COUNTER = 0,
PWM_DOWN_COUNTER
};

enum{
PWM_OUTPUT_DISABLE = 0,
PWM_OUTPUT_ENABLE
};

typedef struct
{
uint8_t channel; // Canal PWM relacionado con el TIMER
uint32_t prescaler; // A qué velocidad se incrementa el Timer
uint16_t periodo; // Indica el número de veces que el Timer se incrementa, el periodo de la frecuencia viene dado por Time_Fosc * PSC * ARR
uint16_t duttyCicle; // Valor en porcentaje (%) del tiempo que la señal está en alto
}PWM_Config_t;

typedef struct
{
TIM_TypeDef *ptrTIMx; // Timer al que esta asociado el PWM
PWM_Config_t config; // Configuración inicial del PWM
}PWM_Handler_t;

/* Headers de las funciones */
void pwm_Config(PWM_Handler_t *ptrPwmHandler);
void setFrequency(PWM_Handler_t *ptrPwmHandler);
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t newFreq);
void setDuttyCycle(PWM_Handler_t *ptrPwmHandler);
void updateDuttyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDutty);
void enableOutput(PWM_Handler_t *ptrPwmHandler,uint8_t state);
void disableOutput(PWM_Handler_t *ptrPwmHandler);
void startPwmSignal(PWM_Handler_t *ptrPwmHandler);
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler);


#endif /* PWM_DRIVER_HAL_H_ */
