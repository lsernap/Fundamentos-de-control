/*
 * gpio_driver_hal.h
 *
 *  Created on: 14 sept. 2023
 *      Author: luciana
 */

#ifndef GPIO_DRIVER_HAL_H_
#define GPIO_DRIVER_HAL_H_


#include <stdint.h>
#include <stm32f4xx.h>

/*
 * Descripcion de cada uno de los registros del periferico
 */

/*
 * GPIO bit SET and bit RESET
 */

enum{
GPIO_PIN_RESET = 0,
GPIO_PIN_SET
};

/*
 * 8.4.1 GPIOx_MODER (dos bit por cada pin)
 */

enum{
GPIO_MODE_IN = 0,
GPIO_MODE_OUT,
GPIO_MODE_ALTFN,
GPIO_MODE_ANALOG
};

/*
 * 8.4.2 GPIOx_OTYPE (un bit por cada pin)
 */


enum{
GPIO_OTYPE_PUSHPULL =0,
GPIO_OTYPE_OPENDRAIN
};

/*
 * 8.4.3 GPIOx_OSPEED (dos bit por cada pin)
 */

enum{
GPIO_OSPEED_LOW =0,
GPIO_OSPEED_MEDIUM,
GPIO_OSPEED_FAST,
GPIO_OSPEED_HIGH
};

/*
 * 8.4.4 GPIOx_PUPDR (dos bit por cada pin)
 */


enum{
GPIO_PUPDR_NOTHING = 0,
GPIO_PUPDR_PULLUP,
GPIO_PUPDR_PULLDOWN,
GPIO_PUPDR_RESERVED
};

/*
 * 8.4.5 GPIOx_IDR  (dos bit por cada pin)
 */
/*
 * 8.4.5 GPIOx_ODR (dos bit por cada pin)
 */


/*
 * Definicion de los nombres de los pines
 */

enum {
PIN_0 =0,
PIN_1,
PIN_2,
PIN_3,
PIN_4,
PIN_5,
PIN_6,
PIN_7,
PIN_8,
PIN_9,
PIN_10,
PIN_11,
PIN_12,
PIN_13,
PIN_14,
PIN_15
};

/*
 * Definicion de las funciones alternativas
 *
 */

enum{
AF0 = 0b0000,
AF1 =0b0001,
AF2 =0b0010,
AF3 =0b0011,
AF4 =0b0100,
AF5 =0b0101,
AF6 =0b0110,
AF7 =0b0111,
AF8 =0b1000,
AF9= 0b1001,
AF10= 0b1010,
AF11 =0b1011,
AF12 =0b1100,
AF13 =0b1101,
AF14 =0b1110,
AF15 =0b1111
};


#define GPIO_PIN_MASK 0x0FU //Pin mask for assert test

typedef struct{
uint32_t GPIO_PinNumber; //working pin
uint32_t GPIO_PinMode; //Config mode: input,output,analog,alternata func
uint32_t GPIO_PinOutputSpeed; // output speed for working oin
uint32_t GPIO_PinPuPdControl;//Turn on-off the resistor push up and pull down for working pin
uint32_t GPIO_PinOutputType; //select output type: Push-pull or opendrain
uint32_t GPIO_PinAltFunMode; // Type of alternative fuction to owrking pin
}GPIO_PinConfig_t;

/*
 * Pin Handler definition.
 *
 * this handler is used to configure the port at witch the selected pin is working
 * Configuration structure: GPIO_PinConfig_t
 */


typedef struct{

GPIO_TypeDef *GPIOx;
GPIO_PinConfig_t pinConfig;


}GPIO_Handler_t;


/*
 * For testing assert parameters - cheking basic configurations
 */

#define IS_GPIO_PIN_ACTION(ACTION) (((ACTION)==GPIO_PIN_RESET)|| ((ACTION )==GPIO_PIN_SET))

#define IS_GPIO_PIN(PIN) (((uint32_t)PIN)<=GPIO_PIN_MASK)

#define IS_GPIO_MODE(MODE) (((MODE)==GPIO_MODE_IN)||\
(MODE)==GPIO_MODE_OUT)||\
(MODE)==GPIO_MODE_ALTFN)||\
(MODE)==GPIO_MODE_ANALOG))

#define IS_GPIO_OUTPUT_TYPE(OUTPUT) (((OUTPUT)==GPIO_OTYPE_PUSHPULL)||((OUTPUT)== GPIO_OTYPE_OPENDRAIN))

#define IS_GPIO_OSPEED(SPEED) (((SPEED)==GPIO_OSPEED_LOW)||\
(SPEED)==GPIO_OSPEED_MEDIUM)||\
(SPEED)==GPIO_OSPEED_FAST)||\
(SPEED)==GPIO_OSPEED_HIGH))

#define IS_GPIO_PUPD(PULL) (((PULL)==GPIO_PUPDR_NOTHING)||\
(PULL)==GPIO_PUPDR_PULLUP)||\
(PULL)==GPIO_PUPDR_PULLDOWN)||\
(PULL)==GPIO_PUPDR_RESERVED))

/*
 * Header definitions for the *public functions of gpio driver hal
 */

void gpio_Config(GPIO_Handler_t *pGPIOHandler);
void gpio_WritePin(GPIO_Handler_t *pPinHandler,uint8_t newState);
void gpio_TooglePin(GPIO_Handler_t *pPinHandler);
uint32_t gpio_ReadPin(GPIO_Handler_t *pPinHandler);


#endif /* GPIO_DRIVER_HAL_H_ */
