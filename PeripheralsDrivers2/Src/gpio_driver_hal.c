/*
 * gpio_driver_hal.c
 *
 *  Created on: 14 sept. 2023
 *      Author: luciana
 */

#include <stm32f4xx.h>
#include <gpio_driver_hal.h>
#include <stm32_assert.h>

/*
 * Headers for private functions
 */

static void gpio_enable_clock_peripheral(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_mode(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_output_type(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_output_speed(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_pullup_pulldown(GPIO_Handler_t *pGPIOHandler);
static void gpio_config_alternate_function(GPIO_Handler_t *pGPIOHandler);

/*
 * Para cualquier periferico hay qie seguir siertos pasos importantes para poder
 * que el sistema permita configurar el periférico x.
 *
 * Lo primero y más importante es activar la señal de reloj principal hacia ese
 * elemento especificado(relacionado con el periferico RCC)
 */
void gpio_Config(GPIO_Handler_t *pGPIOHandler){
/*
* Verificamos que el pin seleccionado es correcto
*/
assert_param(IS_GPIO_PIN(pGPIOHandler->pinConfig.GPIO_PinNumber));
//Activamos el periferico
gpio_enable_clock_peripheral(pGPIOHandler);
//Configuramos el registro GPIOx_MODER
gpio_config_mode(pGPIOHandler);
//Configuramos el registro GPIOx_OTYPE
gpio_config_output_type(pGPIOHandler);
//Configuramos el registro de la velocidad
gpio_config_output_speed(pGPIOHandler);
//Configuramos si se desea pull-up, pull-down o flotante
gpio_config_pullup_pulldown(pGPIOHandler);
//Configuracion de las funciones alternativas
gpio_config_alternate_function(pGPIOHandler);

}

void gpio_enable_clock_peripheral(GPIO_Handler_t *pGPIOHandler){
//Verificamos que el puerto configurado si es permitido
assert_param(IS_GPIO_ALL_INSTANCE(pGPIOHandler->GPIOx));

if(pGPIOHandler->GPIOx == GPIOA){
//Escribimos 1(set) en la posicion correspondiente al GPIOA
RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
}
else if(pGPIOHandler->GPIOx == GPIOB){
//Escribimos 1(set) en la posicion correspondiente al GPIOB
RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN);
}
else if(pGPIOHandler->GPIOx == GPIOC){
//Escribimos 1(set) en la posicion correspondiente al GPIOC
RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOCEN);
}
else if(pGPIOHandler->GPIOx == GPIOD){
//Escribimos 1(set) en la posicion correspondiente al GPIOD
RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN);
}
else if(pGPIOHandler->GPIOx == GPIOE){
//Escribimos 1(set) en la posicion correspondiente al GPIOE
RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOEEN);
}
else if(pGPIOHandler->GPIOx == GPIOH){
//Escribimos 1(set) en la posicion correspondiente al GPIOH
RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOHEN);
}
}

//Configures the mode in which the pin will work
/*
 * Input
 * Output
 * Analog
 * Alternate Function
 */



void gpio_config_mode(GPIO_Handler_t *pGPIOHandler){
uint32_t auxConfig =0;
//Verificamos si el modo que se ha seleccionado es permitido
assert_param(IS_GPIO_MODE(pGPIOHandler->pinConfig.GPIO_PinMode));
//Leemos la configuracion del PinNumber
auxConfig = (pGPIOHandler->pinConfig.GPIO_PinMode<< 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);
//Limpiamos el registro antes de escribirle el nuevo valor
pGPIOHandler->GPIOx->MODER &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);
// Cargamos a auxConfig en el registro MODER
pGPIOHandler->GPIOx->MODER |= auxConfig;
}

//Configures witch type of output the pinX will use:
// push-pull
//openDrain

void gpio_config_output_type(GPIO_Handler_t *pGPIOHandler){
uint32_t auxConfig =0;
//verificamos que el tipo de salida corresponda a los que se pueden usar
assert_param(IS_GPIO_OUTPUT_TYPE(pGPIOHandler->pinConfig.GPIO_PinOutputType));
auxConfig = (pGPIOHandler->pinConfig.GPIO_PinOutputType<< (pGPIOHandler->pinConfig.GPIO_PinNumber));
//Limpiamos antes de cargar
pGPIOHandler->GPIOx->OTYPER &= ~(1 << (pGPIOHandler->pinConfig.GPIO_PinNumber));
// Cargamos a auxConfig en el registro MODER
pGPIOHandler->GPIOx->OTYPER |= auxConfig;
}

/*
 * Selelects between four different possible speeds for output pinx
 * Low,mediium,fast,highSpeed
 */

void gpio_config_output_speed(GPIO_Handler_t *pGPIOHandler){
uint32_t auxConfig =0;
//verificamos que el tipo de salida corresponda a los que se pueden usar
assert_param(IS_GPIO_OSPEED(pGPIOHandler->pinConfig.GPIO_PinOutputSpeed));
auxConfig = (pGPIOHandler->pinConfig.GPIO_PinOutputSpeed<< 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);
//Limpiamos antes de cargar
pGPIOHandler->GPIOx->OSPEEDR &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);
// Cargamos a auxConfig en el registro MODER
pGPIOHandler->GPIOx->OSPEEDR |= auxConfig;
}

/*
 * Turns ON/OFF the pull-up and pull-down resistor for each pinx in selected GPIO port
 */

void gpio_config_pullup_pulldown(GPIO_Handler_t *pGPIOHandler){
uint32_t auxConfig =0;
//verificamos que el tipo de salida corresponda a los que se pueden usar
assert_param(IS_GPIO_PURDR(pGPIOHandler->pinConfig.GPIO_PinPuPdControl));
auxConfig = (pGPIOHandler->pinConfig.GPIO_PinPuPdControl<< 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);
//Limpiamos antes de cargar
pGPIOHandler->GPIOx->PUPDR &= ~(0b11 << 2 * pGPIOHandler->pinConfig.GPIO_PinNumber);
// Cargamos a auxConfig en el registro MODER
pGPIOHandler->GPIOx->PUPDR |= auxConfig;
}

/*
 * Allows to configure other functions(more specialized) in the selected pinx
 */

void gpio_config_alternate_function(GPIO_Handler_t *pGPIOHandler){
uint32_t auxPosition =0;
if(pGPIOHandler->pinConfig.GPIO_PinMode == GPIO_MODE_ALTFN){
if(pGPIOHandler->pinConfig.GPIO_PinNumber < 8){
//Seleccionamos primero si se debe utilizar el registro AFRL o AFRH
auxPosition = 4* pGPIOHandler->pinConfig.GPIO_PinNumber;
pGPIOHandler->GPIOx->AFR[0] &= ~(0b1111 << auxPosition);
//Limpiamos la posicion del registro que deseamos escribir
pGPIOHandler->GPIOx->AFR[0] |= (pGPIOHandler->pinConfig.GPIO_PinAltFunMode << auxPosition);
}
else{
//Seleccionamos primero si se debe utilizar el registro AFRL o AFRH
auxPosition = 4* (pGPIOHandler->pinConfig.GPIO_PinNumber -8);
pGPIOHandler->GPIOx->AFR[1] &= ~(0b1111 << auxPosition);
//Limpiamos la posicion del registro que deseamos escribir
pGPIOHandler->GPIOx->AFR[1] |= (pGPIOHandler->pinConfig.GPIO_PinAltFunMode << auxPosition);
}
}
}

void gpio_WritePin(GPIO_Handler_t *pPinHandler,uint8_t newState){
/* verificamos si la accion que deseamos realizar es permitida*/
assert_param(IS_GPIO_PIN_ACTION(newState));
//Limpiamos la posicion que desamos
if(newState== SET){
//Trabajamos con la parte baja del registro
pPinHandler->GPIOx->BSRR |= (SET << pPinHandler->pinConfig.GPIO_PinNumber);
}
else{
//Trabajamos con la parte alta del registro
pPinHandler->GPIOx->BSRR |= (SET << (pPinHandler->pinConfig.GPIO_PinNumber + 16));
}
}


uint32_t gpio_ReadPin(GPIO_Handler_t *pPinHandler){
//creamos una variable auxiliar la cual luego retornaremos
uint32_t pinValue = 0;
//Cargamos el valor del registro IDR,Desplazando a derecha tantas veces como la ubicacion
/*
* Necesitamos desplazar el valor que se encuentra en IDR(input data register) al bit 0 porque con esto al usar la compuerta AND me compare el primer valor y de entrada
* me entregue el valor que estoy esperando
*
*/
pinValue = (pPinHandler->GPIOx->IDR >> pPinHandler->pinConfig.GPIO_PinNumber);
pinValue &= 0b1; // Comparamos el valor que obtiene el pin con 1 en una compuerta AND para mantener el valor del pin
return pinValue;
}


void gpio_TooglePin(GPIO_Handler_t *pPinHandler){
//Como deseamos invertir la el valor del estado del pin lo que hacemos es usar la compuerta XOR, en donde si son iguales me retorna un cero, pero si son diferentes me retorna un 1
//ademas usamos una mascara de un pin en la posicion que vamos a invertir.
pPinHandler->GPIOx->ODR ^= (SET << pPinHandler->pinConfig.GPIO_PinNumber);
}
