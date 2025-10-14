/*
 * oled_driver_hal.h
 *
 *  Created on: 5 dic. 2023
 *      Author: luciana
 */

#ifndef OLED_DRIVER_HAL_H_
#define OLED_DRIVER_HAL_H_

#include <stm32f4xx.h>
#include "i2c_driver_hal.h"

// paquetes de 8 bits para escrbir carcateres
// Definición de la dirección del esclavo y bytes de control para la OLED
#define OLED_ADRESS 0x3C // direccion del esclavo
#define OLED_CONTROLBYTE_CONFIG  0x00 // configurara la oled,  oled, tamaño, posicion,etc,
#define OLED_CONTROLBYTE_DISPLAY 0x40 // comando para escribir



enum{

OLED_PAGE_0 = 0,
OLED_PAGE_1,
OLED_PAGE_2,
OLED_PAGE_3,
OLED_PAGE_4,
OLED_PAGE_5,
OLED_PAGE_6,
OLED_PAGE_7

};


void sendDataBytes(I2C_Handler_t *ptrHandlerI2C, char *dataToSent, uint8_t sizeArray);
void sendCommandByte ( I2C_Handler_t *ptrHandlerI2C, char command);

//Funciones para la pantalla OLED

void startOLED(I2C_Handler_t *ptrHandlerI2Ctr);
void stopOLED(I2C_Handler_t *ptrHandlerI2Ctr);
void setPage (I2C_Handler_t *ptrHandlerI2Ctr, uint8_t page);
void setColumnAddress (I2C_Handler_t *ptrHandlerI2Ctr, uint8_t page);
void clearDisplay (I2C_Handler_t *ptrHandlerI2Ctr);
void setLineAddress (I2C_Handler_t *ptrHandlerI2Ctr, uint8_t moveNum);
void drawMSG(I2C_Handler_t *ptrHandlerI2Ctr,char *msg);
void drawSinglePageMSG (I2C_Handler_t *ptrHandlerI2Ctr, char *msg,uint8_t page);
char *letterTochar (uint8_t character);

#endif /* OLED_DRIVER_HAL_H_ */
