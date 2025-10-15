/*
 * trimer_driver_hal.h
 *
 *  Created on: Sep 26, 2023
 *      Author: luciana
 */

#ifndef INC_TIMER_DRIVER_HAL_H_
#define INC_TIMER_DRIVER_HAL_H_

#include "stm32f4xx.h"

enum
{
	TIMER_INT_DISABLE  = 0,
	TIMER_INT_ENABLE
};

enum
{
	TIMER_UP_COUNTER   = 0,
	TIMER_DOWN_COUNTER
};

enum
{
	TIMER_OFF = 0,
	TIMER_ON
};

enum
{
	TIMER_SPEED_LOW = 160,
	TIMER_SPEED_MEDIUM = 1600,
	TIMER_SPEED_HIGH = 16000
};

/* Estructura que contiene la configuracion minima necesaria para el manejo del timer */
typedef struct
{
	uint8_t   TIMx_mode;             // Up or down
	uint16_t  TIMx_Prescaler;        // Prescaler...
	uint32_t  TIMx_Period;           // Valor en ms del periodo del Timer
	uint8_t   TIMx_InterruptEnable;  // Activa o desactiva el modo interrupcion
} Timer_BasicConfig_t;

/* Handler para el Timer*/
typedef struct
{
	TIM_TypeDef  *pTIMx;
	Timer_BasicConfig_t TIMx_Config;
}Timer_Handler_t;

/* For testing assert parameters - checking basic configurations. */
#define IS_TIMER_INTERRUP(VALUE)  (((VALUE) == TIMER_INT_DISABLE) || ((VALUE) == TIMER_INT_ENABLE))

#define IS_TIMER_MODE(VALUE)  (((VALUE) == TIMER_UP_COUNTER) || ((VALUE) == TIMER_DOWN_COUNTER))

#define IS_TIMER_STATE(VALUE)  (((VALUE) == TIMER_OFF) || ((VALUE) == TIMER_ON))

#define IS_TIMER_PRESC(VALUE)  (((uint32_t)VALUE) > 1 && ((uint32_t)VALUE) < 0xFFFE)

#define IS_TIMER_PERIOD(PERIOD) (((uint32_t)PERIOD) > 1)

void timer_Config(Timer_Handler_t *pTimerHandler);
void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState);

/* Esta funcion debe ser sobre-escrita en el main para que el sistema funcione*/
void timer2_Callback(void);
void timer3_Callback(void);
void timer4_Callback(void);
void timer5_Callback(void);
void timer9_Callback(void);
void timer10_Callback(void);
void timer11_Callback(void);
void timer1_Callback(void);

#endif /* INC_TIMER_DRIVER_HAL_H_ */
