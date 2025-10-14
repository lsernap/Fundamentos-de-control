/*
 * trimer_driver_hal.c
 *
 *  Created on: Sep 26, 2023
 *      Author: luciana
 */
#include "stm32f4xx.h"
#include "stm32_assert.h"

#include <timer_driver_hal.h>

/* Variable que guarda la referencia del periferico que se esta utilizando*/
TIM_TypeDef *ptrTimerUsed;

/* === Headers for private functions === */
static void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler);
static void timer_set_prescaler(Timer_Handler_t *pTimerHandler);
static void timer_set_period(Timer_Handler_t *pTimerHandler);
static void timer_set_mode(Timer_Handler_t *pTimerHandler);
static void timer_config_interrupt(Timer_Handler_t *pTimerHandler);

/* Funcion en la que cargamos la configuracion del timer
 * Recordar que siempre se debe comenzar con activar la señal de reloj
 * del periferico que se esta utilizando. Ademas, en este caso, debemos
 * ser cuidadosos al momento de utilizar las interrupciones.
 * Los Timer estan conectados directamente al elemento NVIC del Cortex-Mx
 * Debemos configurar y7o utilizar:
 *  - TIMx_CR1 (control Register 1)
 *  - TIMx_SMCR (slave mode control register) -> mantener en 0 para modo Timer Básico
 *  - TIMx_DMA and Interrupt enable register)
 *  - TIMx_SR (Status register)
 *  - TIMx_CNT (Counter)
 *  - TIMx_PSC (Pre-scaler)
 *  - TIMx_ARR (Auto-reload register)
 *
 *  Como vamos a trabajar con interrupciones, antes de configurar una nueva, debemos
 *  desactivar el sistema global de interrupciones, activar la IQR especifica y luego
 *  volver a encender el sistema.
 */
void timer_Config(Timer_Handler_t *pTimerHandler){
	//Guardamos una referencia al periferico que estamos utilizando...
	ptrTimerUsed = pTimerHandler->pTIMx;

	/* 0. Desactivamos las interrupciones globales mientras configuramos el sistema.*/
	__disable_irq();

	/* 1. Activar la señal de reloj del periferico requerido */
	timer_enable_clock_peripheral(pTimerHandler);

	/* 2. Configuramos el Pre-scaler*/
	timer_set_prescaler(pTimerHandler);

	/* 3. Configuramos si UP_CONT o DOWN_COUNT (mode) */
	timer_set_mode(pTimerHandler);

	/* 4. Configuramos el Auto-reload */
	timer_set_period(pTimerHandler);

	/* 5. Configuramos la interrupcion.*/
	timer_config_interrupt(pTimerHandler);

	/* x. Volvemos a activar la interrupciones del sistema */
	__enable_irq();

	/* El timer inicia apagado */
	timer_SetState(pTimerHandler,TIMER_OFF);
}

/**
 *
 */

void timer_enable_clock_peripheral(Timer_Handler_t *pTimerHandler){

	// Verificamos que es un timer permitido
	assert_param(IS_TIM_CC1_INSTANCE(pTimerHandler->pTIMx));

	if (pTimerHandler->pTIMx == TIM2){
		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	}
	else if (pTimerHandler->pTIMx == TIM3){
		RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
	}
	else if (pTimerHandler->pTIMx == TIM4){
		RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	}
	else if (pTimerHandler->pTIMx == TIM5){
		RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	}
	else if (pTimerHandler->pTIMx == TIM9){
		RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
	}
	else if (pTimerHandler->pTIMx == TIM10){
		RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
	}
	else if (pTimerHandler->pTIMx == TIM11){
		RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
	}
	else if (pTimerHandler->pTIMx == TIM1){
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	}
	else{
		__NOP();
	}
}

/**
 * El prescaler nos configura la velocidad a la que se incrementa el registro
 * CNT del Timer.
 */
void timer_set_prescaler(Timer_Handler_t *pTimerHandler){
	// Verificamos que el valor del prescaler es valido
	assert_param(IS_TIMER_PRESC(pTimerHandler->TIMx_Config.TIMx_Prescaler));

	//Configuramos el valor del prescaler.
	pTimerHandler->pTIMx->PSC = pTimerHandler->TIMx_Config.TIMx_Prescaler - 1; // (16000000)
}

/**
 * Esta funcion configura el limite hasta donde cuentael Timer para generar un
 * evento "update" (cuando esta contando de forma ascendente), o configura
 * el valor desde donde se comienza a contar, cuando el sistema funciona de
 * forma decendente.
 */
void timer_set_period(Timer_Handler_t *pTimerHandler){

	// Verificamos que el valor que genera el periodo es valido
	assert_param(IS_TIMER_PERIOD(pTimerHandler->TIMx_Config.TIMx_Prescaler));

	//Configuramos el valor del autoreload
	pTimerHandler->pTIMx->ARR = pTimerHandler->TIMx_Config.TIMx_Period  - 1;
}

/**
 * Upcounter or Downcounter
 *
 */
void timer_set_mode(Timer_Handler_t *pTimerHandler){

	// Verificamos que el modo de funcionamiento es correcto.
	assert_param(IS_TIMER_MODE(pTimerHandler->TIMx_Config.TIMx_mode));

	// Verificamos cual es el modo que se desea configurar.
	if(pTimerHandler->TIMx_Config.TIMx_mode == TIMER_UP_COUNTER){
		// configuramos en modo Upcounter DIR = 0 )
		pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_DIR;
	}
	else{
		// Configuramos en modo downcounter DIR = 1)
		pTimerHandler->pTIMx->CR1 |= TIM_CR1_DIR;
	}
}
/*
 *
 */
void timer_config_interrupt(Timer_Handler_t *pTimerHandler){

	// Verificamos el posible valor configurado
	assert_param(IS_TIMER-INTERRUP(pTimerHandler->TIMx_Config.TIMx_InterruptEnable ));

	if (pTimerHandler->TIMx_Config.TIMx_InterruptEnable == TIMER_INT_ENABLE){
		/* Activamos la interrupcion debida al timerx utilizado*/
		pTimerHandler->pTIMx->DIER |= TIM_DIER_UIE;

		/* Activamos el canal del sistema NVIC para que lea la interrupcion*/
		if(pTimerHandler->pTIMx == TIM2){
			NVIC_EnableIRQ(TIM2_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM3){
			NVIC_EnableIRQ(TIM3_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM4){
			NVIC_EnableIRQ(TIM4_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM5){
			NVIC_EnableIRQ(TIM5_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM9){
			NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM10){
			NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM11){
			NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
		}
		else if(pTimerHandler->pTIMx == TIM1){
			NVIC_EnableIRQ(TIM1_CC_IRQn);
		}
		else{
			__NOP();
		}
	}
	else{
		/* Desactivamos la interrupcion debida al timerx utilizado*/
		pTimerHandler->pTIMx->DIER &= ~TIM_DIER_UIE;

		/* Desactivamos el canal del sistema NVIC para que lea la interrupcion*/
		if (pTimerHandler->pTIMx == TIM2) {
			NVIC_DisableIRQ(TIM2_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM3) {
			NVIC_DisableIRQ(TIM3_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM4) {
			NVIC_DisableIRQ(TIM4_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM5) {
			NVIC_DisableIRQ(TIM5_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM9) {
			NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM10) {
			NVIC_DisableIRQ(TIM1_UP_TIM10_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM11) {
			NVIC_DisableIRQ(TIM1_TRG_COM_TIM11_IRQn);
		}
		else if (pTimerHandler->pTIMx == TIM1) {
			NVIC_DisableIRQ(TIM1_CC_IRQn);
		}
		else {
			__NOP();
		}
	}
}

/**
 *
 */
void timer_SetState(Timer_Handler_t *pTimerHandler, uint8_t newState){
	// Verificamos que el estado ingresado es adecuado
	assert_param(IS_TIMER_STATE(nexState));

	/* 4. Reiniciamos el registro counter*/
	pTimerHandler->pTIMx->CNT = 0;

	if(newState == TIMER_ON){
		/* 5a. Activamos el timer (el CNT debe comenzar a contar)*/
		pTimerHandler->pTIMx->CR1 |= TIM_CR1_CEN;
	}
	else{
		/* 5b. Desactivamos el timer (el CNT debe detenerse)*/
		pTimerHandler->pTIMx->CR1 &= ~TIM_CR1_CEN;
	}
}

/**/
__attribute__((weak)) void timer2_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer3_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer4_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer5_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer9_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer10_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer11_Callback(void){
	__NOP();
}
__attribute__((weak)) void timer1_Callback(void){
	__NOP();
}
/*
 * Esta es la funcion a la que apunta el sistema en el vector de interrupciones.
 * se debe utilizar usando exactamente el mismo nombre definido en el vector de interrupcion
 * al hacerlo correctamente, el sistema apunta a esta funcion y cuando la interrupcion se la
 * el sistema inmediatamente salta a este lugar en la memoria
 */
void TIM2_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM2->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer2_Callback();
}
void TIM3_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM3->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer3_Callback();
}
void TIM4_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM4->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer4_Callback();
}
void TIM5_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM5->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer5_Callback();
}
void TIM1_BRK_TIM9_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM9->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer9_Callback();
}
void TIM1_UP_TIM10_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM10->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer10_Callback();
}
void TIM1_TRG_COM_TIM11_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM11->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer11_Callback();
}
void TIM1_CC_IRQHandler(void){
	/* limpiamos la bandera que indica que la interrupcion se ha generado */
	TIM1->SR &= ~TIM_SR_UIF;

	/* Llamamos a la funcion que se debe encargar de hacer algo con esta interrupcion */
	timer1_Callback();
}
