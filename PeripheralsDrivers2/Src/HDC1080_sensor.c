/*
 * HDC1080_sensor.c
 *
 *  Created on: 26 nov. 2023
 *      Author: luciana
 */

#include <stm32f4xx.h>
#include "HDC1080_sensor.h"
#include "i2c_driver_hal.h"
#include "gpio_driver_hal.h"
#include "systick_driver_hal.h"

//GPIO_Handler_t handlerI2C_SCL  = {0};
//GPIO_Handler_t handlerI2C_SDA  = {0};
//I2C_Handler_t handler_HDC1080  = {0};
//
//void initHDC1080 (void){
//	handlerI2C_SCL.GPIOx                         = GPIOB;
//	handlerI2C_SCL.pinConfig.GPIO_PinNumber      = PIN_8;
//	handlerI2C_SCL.pinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
//	handlerI2C_SCL.pinConfig.GPIO_PinAltFunMode  = AF4;
//	handlerI2C_SCL.pinConfig.GPIO_PinOutputType  = GPIO_OTYPE_OPENDRAIN;
//	handlerI2C_SCL.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
//	handlerI2C_SCL.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
//	gpio_Config(&handlerI2C_SCL);
//
//	handlerI2C_SDA.GPIOx                         = GPIOB;
//	handlerI2C_SDA.pinConfig.GPIO_PinNumber      = PIN_9;
//	handlerI2C_SDA.pinConfig.GPIO_PinMode        = GPIO_MODE_ALTFN;
//	handlerI2C_SDA.pinConfig.GPIO_PinAltFunMode  = AF4;
//	handlerI2C_SDA.pinConfig.GPIO_PinOutputType  = GPIO_OTYPE_OPENDRAIN;
//	handlerI2C_SDA.pinConfig.GPIO_PinPuPdControl = GPIO_PUPDR_PULLUP;
//	handlerI2C_SDA.pinConfig.GPIO_PinOutputSpeed = GPIO_OSPEED_FAST;
//	gpio_Config(&handlerI2C_SDA);
//
//	handler_HDC1080.pI2Cx      = I2C1;
//	handler_HDC1080.modeI2C    = I2C_MODE_FM;
//	handler_HDC1080.sAddress   = HDC1080_ADDRESS;
//	I2C_Config(&handler_HDC1080, FREQUENCY_16Mhz);
//}

uint8_t time_conversion=0;

/*Cabeceras de las funciones privadas */
uint16_t config_resolution(HDC1080_Handler_t *ptrHandlerHDC);


//Configuracion de la resolucion de los datos y el tipo de toma de datos
void hdc1080_config(HDC1080_Handler_t *ptrHandlerHDC){
	//generamos la configuracion que deseamos cargar
	uint16_t df_config=config_resolution(ptrHandlerHDC);
	//Enviamos una transaccion de configuracion
	// 1) Generamos una condicion de Start
	i2c_StartTransaction((ptrHandlerHDC->pI2Cx));
	// 2) Enviamos la direccion de memoria para configurar la resolcion de la toma de datos
	i2c_SendSlaveAddress_RW((ptrHandlerHDC->pI2Cx), HDC1080_ADDRESS,HDC_WRITE);
	// 3) Enviamos la direccion de memoria donde se quire cargar la configuracion
	i2c_SendMemoryAddress((ptrHandlerHDC->pI2Cx), HDC1080_CONFIGURATION);
	// 4) Enviamos los primeros 8 bits de configuracion enviando primero MSB
	i2c_SendDataByte(ptrHandlerHDC->pI2Cx, (uint8_t) (df_config>>8) );
	// 5) Enviamos la segundos 8 bits de configuracion
	i2c_SendDataByte(ptrHandlerHDC->pI2Cx, (uint8_t) df_config);
	// 6) Generamos el stop
	i2c_StopTransaction(ptrHandlerHDC->pI2Cx);
}
//Temperatura y humedad
uint32_t hdc1080_readTH(HDC1080_Handler_t *ptrHandlerHDC){
	// 0) Creamos dos varibles para cargar y guardar los valores recibidos
	uint32_t readData=0;
	uint8_t  bufferData[4]={0};
	// 1) Generamos un Restart
	i2c_RestartTransaction((ptrHandlerHDC->pI2Cx));
	// 2) Enviamos la direccion de meRegistermoria y R/W
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_WRITE);
	// 3) Enviamos la direccion de memoria que deseamos leer
	i2c_SendMemoryAddress(ptrHandlerHDC->pI2Cx, HDC1080_TEMPERATURE);
	// 4) Esperamos a que la conversion en el sensor se finali
	delay_ms(hdc1080_gettimeconversion());  //sumamos dos al tiempo para que no quede muy preciso
	// 5) Generamos un restar para inical la lectura
	i2c_RestartTransaction(ptrHandlerHDC->pI2Cx);
	// 6) Enviamos la direccion del sensor y la orden de leer
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_READ);
	// 7) leemos los primeros 8 bits de informacion relacionada ala temperatura
	bufferData[0]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 8) generamos un ACKM
	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	// 9) leemos el segundo byte de informacion relacionado ala temperatura
	bufferData[1]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 10) Generamos un ACKM
	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	// 11) Leemos el primer byte de informacion relacionado ala humedad
	bufferData[2]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 12) Generamos un ACKM
	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	// 13) Leemos el primer byte de informacion relacionado ala humedad
	bufferData[3]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 14) Finalizamos la lectura de datos con NACKM
	i2c_Send_NoAck(ptrHandlerHDC->pI2Cx);
	// 15) Por ultimo generamos el stop en la linea de datos
	i2c_StopTransaction(ptrHandlerHDC->pI2Cx);
	readData=(bufferData[0]<<24)|bufferData[1]<<16|bufferData[2]<<8|bufferData[3]<<0;
	return readData;
}

uint16_t hdc1080_readT(HDC1080_Handler_t *ptrHandlerHDC){
	// 0) Creamos dos varibles para cargar y guardar los valores recibidos
	uint16_t readData=0;
	uint8_t  bufferData[2]={0};
	// 1) Generamos un Restart
	i2c_RestartTransaction((ptrHandlerHDC->pI2Cx));
	// 2) Enviamos la direccion de memoria y R/W
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_WRITE);
	// 3) Enviamos la direccion de memoria que deseamos leer
	i2c_SendMemoryAddress(ptrHandlerHDC->pI2Cx, HDC1080_TEMPERATURE);
	// 4) Esperamos a que la conversion en el sensor se finali
	time_conversion=80;
	delay_ms(hdc1080_gettimeconversion());  //sumamos dos al tiempo para que no quede muy preciso

	// 5) Generamos un restar para inical la lectura
	i2c_RestartTransaction(ptrHandlerHDC->pI2Cx);
	// 6) Enviamos la direccion del sensor y la orden de leer
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_READ);
	// 7) leemos los primeros 8 bits de informacion relacionada ala temperatura
	bufferData[0]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);

	// 8) generamos un ACKM
	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	 // 9) leemos el segundo byte de informacion relacionado ala temperatura
	 bufferData[1]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	 // 14) Finalizamos la lectura de datos con NACKM
	 i2c_Send_NoAck(ptrHandlerHDC->pI2Cx);
	// 15) Por ultimo generamos el stop en la linea de datos
	 i2c_StopTransaction(ptrHandlerHDC->pI2Cx);
	readData=(bufferData[0]<<8|bufferData[1]<<0);
	return readData;
}

uint16_t hdc1080_readH(HDC1080_Handler_t *ptrHandlerHDC){
	// 0) Creamos dos varibles para cargar y guardar los valores recibidos
	uint16_t readData=0;
	uint8_t  bufferData[2]={0};
	// 1) Generamos un Restart
	i2c_StartTransaction((ptrHandlerHDC->pI2Cx));
	// 2) Enviamos la direccion de memoria y R/W
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_WRITE);
	// 3) Enviamos la direccion de memoria que deseamos leer
	i2c_SendMemoryAddress(ptrHandlerHDC->pI2Cx, HDC1080_HUMIDITY);
	// 4) Esperamos a que la conversion en el sensor se finali
//	time_conversion=80;
//	delay_ms(hdc1080_gettimeconversion());  //sumamos dos al tiempo para que no quede muy preciso

	// 5) Generamos un restar para inical la lectura
	i2c_RestartTransaction(ptrHandlerHDC->pI2Cx);
	// 6) Enviamos la direccion del sensor y la orden de leer
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_READ);

	// 7) leemos los primeros 8 bits de informacion relacionada ala temperatura
	bufferData[0]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 8) generamos un ACKM
	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	// 9) leemos el segundo byte de informacion relacionado ala temperatura
	bufferData[1]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 14) Finalizamos la lectura de datos con NACKM
	i2c_Send_NoAck(ptrHandlerHDC->pI2Cx);
	// 15) Por ultimo generamos el stop en la linea de datos
	i2c_StartTransaction(ptrHandlerHDC->pI2Cx);
	readData=(bufferData[0]<<8|bufferData[1]<<0);
	return readData;
}

uint16_t hdc1080_readID(HDC1080_Handler_t *ptrHandlerHDC){
	// 0) Creamos dos varibles para cargar y guardar los valores recibidos
	uint16_t readData=0;
	uint8_t  bufferData[3] = {0};
	// 1) Generamos un Restart
	i2c_StartTransaction((ptrHandlerHDC->pI2Cx));
	// 2) Enviamos la direccion de memoria y R/W
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_WRITE);
	// 3) Enviamos la direccion de memoria que deseamos leer
	i2c_SendMemoryAddress(ptrHandlerHDC->pI2Cx, HDC1080_SERIAL_ID_FIRST);
	// 4) Esperamos a que la conversion en el sensor se finali
//	time_conversion=80;
//	delay_ms(hdc1080_gettimeconversion());  //sumamos dos al tiempo para que no quede muy preciso

	// 5) Generamos un restar para inical la lectura
	i2c_RestartTransaction(ptrHandlerHDC->pI2Cx);
	// 6) Enviamos la direccion del sensor y la orden de leer
	i2c_SendSlaveAddress_RW(ptrHandlerHDC->pI2Cx, HDC1080_ADDRESS,HDC_READ);

	// 7) leemos los primeros 8 bits de informacion relacionada ala temperatura
	bufferData[0]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 8) generamos un ACKM
	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	// 9) leemos el segundo byte de informacion relacionado ala temperatura
	bufferData[1]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);

	i2c_Send_Ack(ptrHandlerHDC->pI2Cx);
	// 9) leemos el segundo byte de informacion relacionado ala temperatura
	bufferData[2]=i2c_ReadDataByte(ptrHandlerHDC->pI2Cx);
	// 14) Finalizamos la lectura de datos con NACKM
	i2c_Send_NoAck(ptrHandlerHDC->pI2Cx);
	// 15) Por ultimo generamos el stop en la linea de datos
	i2c_StopTransaction(ptrHandlerHDC->pI2Cx);
	readData=(bufferData[0]<<8|bufferData[1]<<0);
	return readData;
}

uint16_t config_resolution(HDC1080_Handler_t *ptrHandlerHDC){
	uint16_t df_config =0; //Definimos una variable para cargar la configuracion
	// Cargamos la resolucion  de humedad seleccionada en la variable
	switch (ptrHandlerHDC->HDC1080_Config.resolution_humi) {
		case HUMIDITY_RESOLUTION_14BIT:
			df_config&=~HDC1080_HUMIDITY_RES;
			time_conversion+=7;  //la conversion de la humedad se demora 7ms
			break;
		case HUMIDITY_RESOLUTION_11BIT:
			df_config|=HDC1080_HUMIDITY_RES_0;
			time_conversion+=4;   //la conversion de la humedad se demora 4ms
			break;
		case HUMIDITY_RESOLUTION_8BIT:
			df_config|=HDC1080_HUMIDITY_RES_1;
			time_conversion+=3;   //la conversion de la humedad se demora 3ms
			break;
		default:
			break;
	}
	switch (ptrHandlerHDC->HDC1080_Config.resolution_temp) {
		case TEMPERATURE_RESOLUTION_14BIT:
			df_config&=~HDC1080_TEMPERATURE_RES;
			time_conversion+=7;   //la conversion de la temperatura se demora 7ms
			break;
		case TEMPERATURE_RESOLUTION_11BIT:
			df_config|=HDC1080_TEMPERATURE_RES;
			time_conversion+=4;   //la conversion de la temperatura se demora 4ms
		default:
			break;
	}
	switch (ptrHandlerHDC->HDC1080_Config.type_take_data) {

		case TEMPERATURE_OR_HUMIDITY:
			df_config&=~HDC1080_TYPE_TAKE_DATA;
			break;
		case TEMPERATURE_AND_HUMIDITY:
			df_config|=HDC1080_TYPE_TAKE_DATA;
			break;
		default:
			break;
	}
	return df_config;
}

uint8_t hdc1080_gettimeconversion(void){
 return time_conversion;
}

