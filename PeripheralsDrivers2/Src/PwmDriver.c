/*
 * PwmDriver.c
 *
 *  Created on: 3 nov. 2023
 *      Author: luciana
 */




#include <PwmDriver.h>

static void pwm_enable_clock_peripheral(PWM_Handler_t *ptrPwmHandler);
static void pwm_mode(PWM_Handler_t *ptrPwmHandler, uint8_t state);
static void pwm_CCxS_config(PWM_Handler_t *ptrPwmHandler);
static void pwm_output_compare(PWM_Handler_t *ptrPwmHandler);
static void pwm_preLoad_bit(PWM_Handler_t *ptrPwmHandler);


void pwm_Config(PWM_Handler_t *ptrPwmHandler){
	// 1. Activar la señal de reloj del periférico requerido
	pwm_enable_clock_peripheral(ptrPwmHandler);

	//2. Cargamos la frecuencia deseada
	setFrequency(ptrPwmHandler);

	// 3. Cargamos el valor del dutty-Cycle
	setDuttyCycle(ptrPwmHandler);

	//3a. Estamos en UP_Mode, el limite se carga en ARR y se comienza en 0
	// Configuramos modo up
	pwm_mode(ptrPwmHandler, PWM_UP_COUNTER);

	//Cargamos el limite del contador
	//ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo - 1;

	// reiniciamos el registro de counter
	ptrPwmHandler -> ptrTIMx -> CNT = 0;

	//4. Configuramos el registro CCxS de modo que quede en salida.
	pwm_CCxS_config(ptrPwmHandler);

	//5. Configuramos el output compare
	pwm_output_compare(ptrPwmHandler);

	//6.Activamos el preload bit
	pwm_preLoad_bit(ptrPwmHandler);

	//7. Activamos o desactivamos el canal para dar salida.
	enableOutput(ptrPwmHandler,PWM_OUTPUT_ENABLE);
}
void pwm_enable_clock_peripheral(PWM_Handler_t *ptrPwmHandler){
	if(ptrPwmHandler->ptrTIMx == TIM2){
		RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN ;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM3){
		RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN ;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM4){
		RCC -> APB1ENR |= RCC_APB1ENR_TIM4EN ;
	}
	else if(ptrPwmHandler->ptrTIMx == TIM5){
		RCC -> APB1ENR |= RCC_APB1ENR_TIM5EN ;
	}
	else{
		__NOP();
	}
}
/*
 *La frecuencia esta definada como el prescaler y el registro maximo del timer
 *
 */
void setFrequency(PWM_Handler_t *ptrPwmHandler){
	// Cargamos el valor del prescaler
	ptrPwmHandler->ptrTIMx->PSC = ptrPwmHandler->config.prescaler;
	// Cargamos el valor del ARR, el cual es el límite de incrementos del Timer
	ptrPwmHandler->ptrTIMx->ARR = ptrPwmHandler->config.periodo - 1;
}

void setDuttyCycle(PWM_Handler_t *ptrPwmHandler){
	// Seleccionamos el canal para configurar su dutty
	//Como el dutty esta dado en porcentaje (%), entonces dividimos por 100
	switch(ptrPwmHandler->config.channel){
		case PWM_CHANNEL_1:{
			ptrPwmHandler->ptrTIMx->CCR1 = (ptrPwmHandler->config.duttyCicle);
			break;
		}
		case PWM_CHANNEL_2: {
			ptrPwmHandler->ptrTIMx->CCR2 = (ptrPwmHandler->config.duttyCicle);
			break;
		}
		case PWM_CHANNEL_3: {
			ptrPwmHandler->ptrTIMx->CCR3 = (ptrPwmHandler->config.duttyCicle);
			break;
		}
		case PWM_CHANNEL_4: {
			ptrPwmHandler->ptrTIMx->CCR4 = (ptrPwmHandler->config.duttyCicle);
			break;
		}
		default:{
			break;
		}
	}
}

void pwm_mode(PWM_Handler_t *ptrPwmHandler, uint8_t state){

	switch (state) {
		case PWM_UP_COUNTER:{
			ptrPwmHandler ->ptrTIMx -> CR1 &= ~(TIM_CR1_DIR);
			break;
		}
		case PWM_DOWN_COUNTER:{
			ptrPwmHandler ->ptrTIMx -> CR1 |= (TIM_CR1_DIR);
			break;
		}
		default:{
			break;
		}
	}
}

void pwm_CCxS_config(PWM_Handler_t *ptrPwmHandler){
	switch(ptrPwmHandler->config.channel){
		case PWM_CHANNEL_1:{
			// Seleccionamos como salida el canal
			ptrPwmHandler->ptrTIMx->CCMR1 &= ~(TIM_CCMR1_CC1S);
			break;
		}
		case PWM_CHANNEL_2: {
			// Seleccionamos como salida el canal
			ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_CC2S;
			break;
		}
		case PWM_CHANNEL_3: {
			// Seleccionamos como salida el canal
			ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_CC3S;
			break;
		}
		case PWM_CHANNEL_4: {
			// Seleccionamos como salida el canal
			ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_CC4S;
			break;
		}
		default:{
			break;
		}

	}
}

void pwm_output_compare(PWM_Handler_t *ptrPwmHandler){
	switch(ptrPwmHandler->config.channel){
		case PWM_CHANNEL_1:{
			// Configuramos el canal como PWM
			ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC1M_0;
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1M_1;
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1M_2;
			break;
		}
		case PWM_CHANNEL_2: {
			// Configuramos el canal como PWM
			ptrPwmHandler->ptrTIMx->CCMR1 &= ~TIM_CCMR1_OC2M_0;
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2M_1;
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2M_2;
			break;
		}
		case PWM_CHANNEL_3: {
			// Configuramos el canal como PWM
			ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC3M_0;
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3M_1;
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3M_2;
			break;
		}
		case PWM_CHANNEL_4: {
			// Configuramos el canal como PWM
			ptrPwmHandler->ptrTIMx->CCMR2 &= ~TIM_CCMR2_OC4M_0;
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4M_1;
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4M_2;
			break;
		}
		default:{
			break;
		}

	}
}

void pwm_preLoad_bit(PWM_Handler_t *ptrPwmHandler){

	switch(ptrPwmHandler->config.channel){
		case PWM_CHANNEL_1:{
			// Activamos la comparacion en modo fast
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1FE;

			// Activamos la funcionalidad de pre-load
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC1PE;
			break;
		}
		case PWM_CHANNEL_2: {
			// Activamos la comparacion en modo fast
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2FE;

			// Activamos la funcionalidad de pre-load
			ptrPwmHandler->ptrTIMx->CCMR1 |= TIM_CCMR1_OC2PE;
			break;
		}
		case PWM_CHANNEL_3: {
			// Activamos la comparacion en modo fast
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3FE;

			// Activamos la funcionalidad de pre-load
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC3PE;
			break;
		}
		case PWM_CHANNEL_4: {
			// Activamos la comparacion en modo fast
			ptrPwmHandler->ptrTIMx->CCMR2 |= TIM_CCMR2_OC4FE;

			// Activamos la funcionalidad de pre-load
			ptrPwmHandler->ptrTIMx->CCMR2 |=  TIM_CCMR2_OC4PE;
			break;
		}
		default:{
			break;
		}

	}
}

void enableOutput(PWM_Handler_t *ptrPwmHandler,uint8_t state) {
	switch (ptrPwmHandler->config.channel) {
		case PWM_CHANNEL_1: {
			if (state == PWM_OUTPUT_DISABLE) {
				//Desactivamos la salida del canal 1
				ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC1E;
			}
			else{
				// Activamos la salida del canal 1
				ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC1E;
			}
			break;
		}
		case PWM_CHANNEL_2: {
			if (state == PWM_OUTPUT_DISABLE) {
				//Desactivamos la salida del canal 2
				ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC2E;
			}
			else{
				// Activamos la salida del canal 2
				ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC2E;
			}
			break;
		}
		case PWM_CHANNEL_3: {
			if (state == PWM_OUTPUT_DISABLE) {
				//Desactivamos la salida del canal 3
				ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC3E;
			}
			else{
				// Activamos la salida del canal 3
				ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC3E;
			}
			break;
		}
		case PWM_CHANNEL_4: {
			if (state == PWM_OUTPUT_DISABLE) {
				//Desactivamos la salida del canal 4
				ptrPwmHandler->ptrTIMx->CCER &= ~TIM_CCER_CC4E;
			}
			else{
				// Activamos la salida del canal 4
				ptrPwmHandler->ptrTIMx->CCER |= TIM_CCER_CC4E;
			}
			break;
		}
		default: {
			break;
		}
	}
}
// Funcion para activar el Timer y activar el módulo PWM
void startPwmSignal(PWM_Handler_t *ptrPwmHandler) {

	ptrPwmHandler->ptrTIMx->CR1 |= TIM_CR1_CEN;
}
//Funcion para desactivar el Timer y detener el módulo PWM
void stopPwmSignal(PWM_Handler_t *ptrPwmHandler) {

	ptrPwmHandler->ptrTIMx->CR1 &= ~(TIM_CR1_CEN);

}
// Funcion para actualizar la frecuencia, funciona de la mano con setFrequency
void updateFrequency(PWM_Handler_t *ptrPwmHandler, uint16_t periodo){
	// Actualizamos el registro que manipula el periodo

    ptrPwmHandler->config.periodo = periodo;
	// Llamamos a la fucnión que cambia la frecuencia
    setFrequency(ptrPwmHandler);
}
// Función para actualizar el Dutty, funciona de la mano con setDuttyCycle
void updateDuttyCycle(PWM_Handler_t *ptrPwmHandler, uint16_t newDutty){
	// Actualizamos el registro que manipula el dutty

	ptrPwmHandler->config.duttyCicle = newDutty;
	// Llamamos a la fucnión que cambia el dutty y cargamos el nuevo valor
	setDuttyCycle(ptrPwmHandler);
}
