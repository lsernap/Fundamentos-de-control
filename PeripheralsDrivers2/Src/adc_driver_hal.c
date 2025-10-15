/*
 * adc_driver_hal.c
 *
 *  Created on: 13 oct. 2023
 *      Author: luciana
 */

#include <adc_driver_hal.h>
#include <gpio_driver_hal.h>
#include <stm32f4xx.h>
#include <stm32_assert.h>

//   Headers for private functions

static void adc_enable_clock_peripheral(void);
static void adc_set_resolution(ADC_Config_t *adcConfig);
static void adc_set_alignment(ADC_Config_t *adcConfig);
static void adc_set_sampling_and_hold(ADC_Config_t *adcConfig);
static void adc_set_one_channel_sequence(ADC_Config_t *adcConfig);
static void adc_config_interrupt(uint8_t state);
static void adc_sequence_config(ADC_Config_t *adcConfig[16], uint8_t numeroDeCanales);
//static void adc_ConfigMultichannel(ADC_Config_t *adcConfig,uint8_t sequenceNumber);
void adc_config_sampling_and_sequience(ADC_Config_t *adcConfig,uint8_t sequenceNumber);

// Variables y elementos que necesita internamente el driver para funcionar adecuadamente

GPIO_Handler_t handlerADCPin = {0};
uint16_t adc_RawData = 0;


void adc_ConfigSingleChannel(ADC_Config_t *adcConfig){
	// 1. Configuramos el PinX para que cumpla la función de canal análogo deseado
	adc_ConfigAnalogPin(adcConfig->channel);

	// 2. Activamos la señal de reloj para el periférico ADC1 en el APB2
	adc_enable_clock_peripheral();

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	//Comenzamos la configuracion de ADC1

	//3.Resolucion del ADC
	adc_set_resolution(adcConfig);

	//4. Configuramos el modo scan como desactivado
	adc_ScanMode(SCAN_OFF);

	//5. Configuramos la aliniacion de los datos (derecha o izquierda)
	adc_set_alignment(adcConfig);

	//6. Desactivamos el "continuos mode"
	adc_StopContinuousConv();

	//7.Configuracion del sampling
	adc_set_sampling_and_hold(adcConfig);

	//8.Configuramos la secuencia y cuantos elementos hay en la secuencia
	adc_set_one_channel_sequence(adcConfig);

	// 9.Configuramos el preescaler del ADC en 2:1 (el mas rápido que se puede tener
	ADC->CCR &= ~(ADC_CCR_ADCPRE);

	// 10.Desactivamos las interrupciones globales
	__disable_irq();

	// 11.Configuramos la interrupción (si se encuentra activa), ademas de inscribir/remover
	//la interrupcion en el NVIC
	adc_config_interrupt(ADC_INT_ENABLE);

	//12.Activamos el modulo ADC
	adc_peripheralOnOFF(ADC_ON);

	__enable_irq();
}

void adc_ConfigAnalogPin(uint8_t adcChannel){

	// Con este switch seleccionamos el canal y lo configuramos como analogo.
	switch (adcChannel) {
	case ADC_CHANNEL_0: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_1: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_2: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		break;
	}
	case ADC_CHANNEL_3: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		break;
	}
	case ADC_CHANNEL_4: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		break;
	}
	case ADC_CHANNEL_5: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		break;
	}
	case ADC_CHANNEL_6: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_6;
		break;
	}
	case ADC_CHANNEL_7: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_7;
		break;
	}
	case ADC_CHANNEL_8: {
		handlerADCPin.GPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_9: {
		handlerADCPin.GPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_10: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_11: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_12: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		break;
	}
	case ADC_CHANNEL_13: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		break;
	}
	case ADC_CHANNEL_14: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		break;
	}
	case ADC_CHANNEL_15: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		break;
	}
	default:{
		break;
	}
	}
	// Despues de configurar el canal cargamos la configuracion con el driver del GPIOx
	handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
	handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
	gpio_Config(&handlerADCPin);
}

void adc_resetConfigAnalogPin(ADC_Config_t *adcConfig,uint8_t adcChannel){

	adc_config_interrupt(ADC_INT_DISABLE);

	// Con este switch seleccionamos el canal y ponemos en modo de entrada con el fin de reiniciarlo
	switch (adcChannel) {
	case ADC_CHANNEL_0: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		break;
	}
	case ADC_CHANNEL_1: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_2: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		break;
	}
	case ADC_CHANNEL_3: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		break;
	}
	case ADC_CHANNEL_4: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		break;
	}
	case ADC_CHANNEL_5: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		break;
	}
	case ADC_CHANNEL_6: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_6;
		break;
	}
	case ADC_CHANNEL_7: {
		handlerADCPin.GPIOx = GPIOA;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_7;
		break;
	}
	case ADC_CHANNEL_8: {
		// Cofiguramos los registro
		handlerADCPin.GPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case ADC_CHANNEL_9: {
		handlerADCPin.GPIOx = GPIOB;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		break;
	}
	case ADC_CHANNEL_10: {
		// Cofiguramos los registro
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_0;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case ADC_CHANNEL_11: {
		// Cofiguramos los registro
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_1;
		handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_ANALOG;
		handlerADCPin.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_NOTHING;
		gpio_Config(&handlerADCPin);
		break;
	}
	case ADC_CHANNEL_12: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_2;
		break;
	}
	case ADC_CHANNEL_13: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_3;
		break;
	}
	case ADC_CHANNEL_14: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_4;
		break;
	}
	case ADC_CHANNEL_15: {
		handlerADCPin.GPIOx = GPIOC;
		handlerADCPin.pinConfig.GPIO_PinNumber = PIN_5;
		break;
	}
	default:{
		break;
	}
	}
	// Despues de configurar el canal cargamos la configuracion con el driver del GPIOx
	handlerADCPin.pinConfig.GPIO_PinMode = GPIO_MODE_IN;
	gpio_Config(&handlerADCPin);
}

void adc_enable_clock_peripheral(void){
	//Escribimos un 1 en el registro del periferico
	RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN ;
}

void adc_set_resolution(ADC_Config_t *adcConfig){
	//En esta funcion asignsmos la cantidad de bits de resolucion
	//Para mas informacion mirar la pagina 229 del manual de referencias (Tabla 11.12.2)
	switch(adcConfig->resolution){
	case ADC_RESOLUTION_12_BIT:
	{//00
		ADC1->CR1 &= ~(ADC_CR1_RES_0);
		ADC1->CR1 &= ~(ADC_CR1_RES_1);
		break;
	}
	//01
	case ADC_RESOLUTION_10_BIT:
	{
		ADC1->CR1 |= ADC_CR1_RES_0;
		ADC1->CR1 &= ~(ADC_CR1_RES_1);
		break;
	}
	//10
	case ADC_RESOLUTION_8_BIT:
	{
		ADC1->CR1 &= ~(ADC_CR1_RES_0);
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}
	//11
	case ADC_RESOLUTION_6_BIT:
	{
		ADC1->CR1 |= ADC_CR1_RES_0;
		ADC1->CR1 |= ADC_CR1_RES_1;
		break;
	}
	default:{
		ADC1->CR1 &= ~(ADC_CR1_RES_0);
		ADC1->CR1 &= ~(ADC_CR1_RES_1);
		break;
	}
	}
}

void adc_ScanMode(uint8_t state){
	if (state == SCAN_ON) {
		//Ponemos el registro en 1 para usar el scan
		ADC1 -> CR1 |= ADC_CR1_SCAN;
	}
	else{
		//ponemos el registro en cero para desactivar
		ADC1 -> CR1 &= ~(ADC_CR1_SCAN);
	}

}

void adc_set_alignment(ADC_Config_t *adcConfig){
	if(adcConfig->dataAlignment == ADC_ALIGNMENT_RIGHT){
		// Alineación a la derecha (esta es la forma "natural")
		ADC1 -> CR2 &= ~(ADC_CR2_ALIGN);
	}
	else{
		// Alineación a la izquierda (para algunos cálculos matemáticos)
		ADC1 -> CR2 |= (ADC_CR2_ALIGN);
	}
}


void adc_StopContinuousConv(void){

	ADC1 -> CR2 &= ~(ADC_CR2_CONT);
}

void adc_set_sampling_and_hold(ADC_Config_t *adcConfig){

	switch (adcConfig->channel) {// switch para el canal que se selecciono
	case ADC_CHANNEL_0:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP0_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP0_2);
			break;
		}
		default:{
			break;
		}
		}
		break;
	}
	case ADC_CHANNEL_1:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP1_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP1_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_2:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP2_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP2_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_3:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP3_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP3_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_4:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP4_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP4_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_5:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP5_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP5_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_6:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP6_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP6_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_7:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP7_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP7_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_8:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP8_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP8_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_9:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_0);//escribimos 1 en
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_0);//escribimos cero en todos
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_0);
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR2 &= ~(ADC_SMPR2_SMP9_0);//escribimos cero en todos
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_0);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_1);
			ADC1 -> SMPR2 |= (ADC_SMPR2_SMP9_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_10:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_0);//escribimos 1 en
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_0);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP10_0);//escribimos cero en todos
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP10_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_11:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_0);//escribimos 1 en
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_0);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP11_0);//escribimos cero en todos
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP11_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_12:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_0);//escribimos 1 en
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_0);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP12_0);//escribimos cero en todos
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP12_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_13:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_0);//escribimos 1 en
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_0);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP13_0);//escribimos cero en todos
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP13_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_14:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_0);//escribimos 1 en
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_0);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP14_0);//escribimos cero en todos
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP14_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	case ADC_CHANNEL_15:{
		switch (adcConfig->samplingPeriod) {// activamos el registro dependiendo de la cantidad de ciclos
		case SAMPLING_PERIOD_3_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_15_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_0);//escribimos 1 en
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_28_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_56_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_84_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_0);//escribimos cero en todos
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_112_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_0);
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_144_CYCLES:{
			ADC1 -> SMPR1 &= ~(ADC_SMPR1_SMP15_0);//escribimos cero en todos
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_2);
			break;
		}
		case SAMPLING_PERIOD_480_CYCLES:{
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_0);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_1);
			ADC1 -> SMPR1 |= (ADC_SMPR1_SMP15_2);
			break;
		}
		default:{
			break;
		}
		}

		break;
	}
	default:{
		break;
	}
	}
}

void adc_set_one_channel_sequence(ADC_Config_t *adcConfig){
	// //seleccionamos solo 1 elemento de la secuencia para el conteo
	// ADC1->SQR1 = 0;
	//
	// // Asignamos el canal de la conversion a la primera elemento en la secuencia
	// ADC1->SQR3 |= (adcConfig->channel << 0);
	/* 8. Configuramos la secuencia y cuantos elementos hay en las secuencias */

	/* Como solo será un elemento configuraremos el Length a un solo elemnto*/
	ADC1->SQR1 &= ~ADC_SQR1_L;

	/* Limpiamos el resgistro primero*/
	ADC1->SQR3 &= ~(ADC_SQR3_SQ1);

	/* Ahora configuraremos la cada canal de acuerdo a la elección
	 * de un solo elemento en la secuencia
	 */

	switch(adcConfig->channel){
	case ADC_CHANNEL_0:{
		ADC1->SQR3 &= ~ADC_SQR3_SQ1;
		break;
	}
	case ADC_CHANNEL_1:{
		ADC1->SQR3 |= ADC_SQR3_SQ1_0;
		break;
	}
	case ADC_CHANNEL_2:{
		ADC1->SQR3 |= ADC_SQR3_SQ1_1;
		break;
	}
	case ADC_CHANNEL_3:{
		ADC1->SQR3 |= 3 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_4:{
		ADC1->SQR3 |= ADC_SQR3_SQ1_2;
		break;
	}
	case ADC_CHANNEL_5:{
		ADC1->SQR3 |= 5 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_6:{
		ADC1->SQR3 |= 6 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_7:{
		ADC1->SQR3 |= 7 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_8:{
		ADC1->SQR3 |= ADC_SQR3_SQ1_3;
		break;
	}
	case ADC_CHANNEL_9:{
		ADC1->SQR3 |= 9 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_10:{
		ADC1->SQR3 |= 10 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_11:{
		ADC1->SQR3 |= 11 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_12:{
		ADC1->SQR3 |= 12 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_13:{
		ADC1->SQR3 |= 13 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_14:{
		ADC1->SQR3 |= 14 << ADC_SQR3_SQ1_Pos;
		break;
	}
	case ADC_CHANNEL_15:{
		ADC1->SQR3 |= 15 << ADC_SQR3_SQ1_Pos;
		break;
	}
	default:{
		__NOP();
		break;
	}
	}

}


void adc_config_interrupt(uint8_t state){

	if (state == ADC_INT_ENABLE) {
		ADC1 -> CR1 |= ADC_CR1_EOCIE;

		__NVIC_EnableIRQ(ADC_IRQn);

		__NVIC_SetPriority(ADC_IRQn, 4);
	}
	else{
		ADC1 -> CR1 &= ~ADC_CR1_EOCIE;
		/* Debemos desmatricular la interrupción en el NVIC */
		__NVIC_DisableIRQ(ADC_IRQn);
		__NVIC_SetPriority(ADC_IRQn, 4);
	}
}

void adc_peripheralOnOFF(uint8_t state){
	if (state == ADC_ON) {
		ADC1 -> CR2 |= ADC_CR2_ADON;
	}
	else{
		ADC1 -> CR2 &= ~(ADC_CR2_ADON);
	}

}

uint16_t adc_GetValue(void){
	return adc_RawData;
}

void adc_StartSingleConv(void){
	// Desactivamos el modo continuo de ADC
	ADC1 -> CR2 &= ~(ADC_CR2_CONT);

	// Iniciamos un ciclo de conversión ADC (CR2)
	ADC1 -> CR2 |= ADC_CR2_SWSTART;

	//Revisamos que la bandera  del overrun no este en 1, si esta esto significa que ya tenemos el dato
	//por lo que debemos bajar esa bandera y dar por terminado la conversion para poder que el sistema siga con el ADC
	if (ADC1->SR & ADC_SR_OVR){
		ADC1->SR &= ~(ADC_SR_OVR);
		ADC1->SR |= ADC_SR_EOC;
	}
}

void adc_StartContinuousConv(void){
	// Iniciamos un ciclo de conversión ADC (CR2)
	ADC1 -> CR2 |= ADC_CR2_SWSTART;
	// Activamos el modo continuo de ADC
	ADC1 -> CR2 |= (ADC_CR2_CONT);

}

void ADC_IRQHandler(){


	if(ADC1->SR & ADC_SR_EOC){
		// Leemos el resultado de la conversión ADC y lo cargamos en una variable auxiliar
		adc_RawData = ADC1 -> DR;
		// Hacemos el llamado a la función que se ejecutará en el main
		adc_CompleteCallback();
	}
	else if(ADC1->SR & ADC_SR_OVR){

		ADC1->SR &= ~ADC_SR_OVR;
		// Leemos el resultado de la conversión ADC y lo cargamos en una variable auxiliar
		adc_RawData = ADC1 -> DR;
		// Hacemos el llamado a la función que se ejecutará en el main
		adc_CompleteCallback();
	}
}

__attribute__((weak)) void adc_CompleteCallback(void) {
	__NOP();
}

void adc_ConfigMultichannel (ADC_Config_t *adcConfig[16], uint8_t numeroDeCanales){
	for(int i = 0; i < numeroDeCanales; i++){
		/* 1. Configuramos el PinX para que cumpla la función de canal análogo deseado. */
		adc_ConfigAnalogPin(adcConfig[i]->channel);
	}

	// 2. Activamos la señal de reloj para el periférico ADC1 en el APB2
	adc_enable_clock_peripheral();

	// Limpiamos los registros antes de comenzar a configurar
	ADC1->CR1 = 0;
	ADC1->CR2 = 0;

	//Comenzamos la configuracion de ADC1

	//3.Resolucion del ADC
	adc_set_resolution(adcConfig[0]);
	//4. Configuramos el modo scan como desactivado
	adc_ScanMode(SCAN_ON);

	//5. Configuramos la aliniacion de los datos (derecha o izquierda)
	adc_set_alignment(adcConfig[0]);

	//6. Desactivamos el "continuos mode"
	adc_StopContinuousConv();

	//7.Limpiamos los registros para poder hacer una secuencia
	ADC1->SQR1 = 0;
	ADC1->SQR2 = 0;
	ADC1->SQR3 = 0;

	adc_sequence_config(adcConfig, numeroDeCanales);

	// 9.Configuramos el preescaler del ADC en 2:1 (el mas rápido que se puede tener
	ADC->CCR &= ~(ADC_CCR_ADCPRE);

	// 10.Desactivamos las interrupciones globales
	__disable_irq();

	//11. Activamos la interrupción debida a la finalización de una conversión EOC (CR1)
	ADC1->CR1 |= ADC_CR1_EOCIE;

	// Configurar interrupción cuando acaba con cada canal
	ADC1->CR2 |= ADC_CR2_EOCS;
	//la interrupcion en el NVIC
	adc_config_interrupt(ADC_INT_ENABLE);

	//12.Activamos el modulo ADC
	adc_peripheralOnOFF(ADC_ON);

	__enable_irq();


}
void adc_sequence_config(ADC_Config_t *adcConfig[16], uint8_t numeroDeCanales){

	for(int i = 0; i < numeroDeCanales; i++){
		// adc_set_sampling_and_hold(adcConfig[i]);

		// 8. Configuramos la secuencia y cuantos elementos hay
		// adc_set_multi_ADC_CHANNEL_sequence(adcConfig[i],i);

		if(adcConfig[i]->channel < ADC_CHANNEL_9){
			ADC1->SMPR2 |= adcConfig[i]->samplingPeriod << (3 * adcConfig[i]->channel);
		}
		else{
			ADC1->SMPR1 |= adcConfig[i]->samplingPeriod << (3 * (adcConfig[i]->channel-10));
		}

		/* 8. Configuramos la secuencia y cuantos elementos hay en la secuencia */
		ADC1->SQR1 = i << ADC_SQR1_L_Pos;
		// Asignamos el canal de la conversión a la posición en la secuencia
		if(i < 6){
			ADC1->SQR3 |= (adcConfig[i]->channel << (5*i));
		}
		else if(i < 12){
			ADC1->SQR2 |= (adcConfig[i]->channel << (5*(i - 6)));
		}
		else{
			ADC1->SQR1 |= (adcConfig[i]->channel << (5*(i - 12)));
		}
	}
}


void adc_ConfigTrigger(uint8_t sourceType, PWM_Handler_t *triggerSignal){
	// Activación por medio externo de ADC
	switch (sourceType) {
	case TRIGGER_DISABLED:{
		ADC1->CR2 &= ~ADC_CR2_EXTEN_0;
		ADC1->CR2 &= ~ADC_CR2_EXTEN_1;

		break;
	}
	case TRIGGER_RISING_EDGE:{
		ADC1->CR2 |= ADC_CR2_EXTEN_0;
		ADC1->CR2 &= ~ADC_CR2_EXTEN_1;

		break;
	}
	case TRIGGER_FALLING_EDGE:{
		ADC1->CR2 &= ~ADC_CR2_EXTEN_0;
		ADC1->CR2 |= ADC_CR2_EXTEN_1;

		break;
	}
	case TRIGGER_RISING_FALLING_EDGE:{
		ADC1->CR2 |= ADC_CR2_EXTEN_0;
		ADC1->CR2 |= ADC_CR2_EXTEN_1;

		break;
	}
	default:{
		ADC1->CR2 |= ADC_CR2_EXTEN_0;
		ADC1->CR2 &= ~ADC_CR2_EXTEN_1;
		break;
	}
	}

	//Revisamos cual es el timmer configurado para ese PWM
	if (triggerSignal->ptrTIMx == TIM1) {
		switch (triggerSignal->config.channel) {
		case PWM_CHANNEL_1:{
			ADC1->CR2 |= TIMMER_1_CHANNEL_1 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case PWM_CHANNEL_2:{
			ADC1->CR2 |= TIMMER_1_CHANNEL_2 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case PWM_CHANNEL_3:{
			ADC1->CR2 |= TIMMER_1_CHANNEL_3 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		default:{
			break;
		}
		}
	}
	else if(triggerSignal->ptrTIMx == TIM2) {
		switch (triggerSignal->config.channel) {
		case PWM_CHANNEL_2:{
			ADC1->CR2 |= TIMMER_2_CHANNEL_2 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case PWM_CHANNEL_3:{
			ADC1->CR2 |= TIMMER_2_CHANNEL_3 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case PWM_CHANNEL_4:{
			ADC1->CR2 |= TIMMER_2_CHANNEL_4 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case 4:{
			ADC1->CR2 |= TIMMER_2_TRGO << ADC_CR2_EXTSEL_Pos;
			break;
		}
		default:{
			break;
		}
		}
	}
	else if(triggerSignal->ptrTIMx == TIM3) {
		switch (triggerSignal->config.channel) {
		case PWM_CHANNEL_1:{
			ADC1->CR2 |= TIMMER_3_CHANNEL_1 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case 4:{
			ADC1->CR2 |= TIMMER_3_TRGO << ADC_CR2_EXTSEL_Pos;
			break;
		}
		default:{
			break;
		}
		}
	}
	else if(triggerSignal->ptrTIMx == TIM4) {
		switch (triggerSignal->config.channel) {
		case PWM_CHANNEL_4:{
			ADC1->CR2 |= TIMMER_4_CHANNEL_4 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		default:{
			break;
		}
		}
	}
	else if(triggerSignal->ptrTIMx == TIM5) {
		switch (triggerSignal->config.channel) {
		case PWM_CHANNEL_1:{
			ADC1->CR2 |= TIMMER_5_CHANNEL_1 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case PWM_CHANNEL_2:{
			ADC1->CR2 |= TIMMER_5_CHANNEL_2 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		case PWM_CHANNEL_3:{
			ADC1->CR2 |= TIMMER_5_CHANNEL_3 << ADC_CR2_EXTSEL_Pos;
			break;
		}
		default:{
			break;
		}
		}
	}
}
