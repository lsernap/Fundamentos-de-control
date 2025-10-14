/*
 * i2c_driver_hal.h
 *
 *  Created on: 26 nov. 2023
 *      Author: luciana
 */

#ifndef I2C_DRIVER_HAL_H_
#define I2C_DRIVER_HAL_H_


#include <stm32f4xx.h>
#include <systick_driver_hal.h>
enum{
	I2C_WRITE_DATA = 0,
	I2C_READ_DATA
};

enum{
	I2C_MAIN_CLOCK_4MHz = 4,
	I2C_MAIN_CLOCK_16MHz = 16,
	I2C_MAIN_CLOCK_20MHz = 20
};

enum{
	I2C_MODE_SM = 0, // SM means Standard Mode
	I2C_MODE_FM // FM means Fast Mode
};

enum{
	I2C_MODE_SM_SPEED_100kHz = 80,
	I2C_MODE_FM_SPEED_400kHz = 14
};

enum{
	I2C_MAX_RISE_TIME_SM = 17,
	I2C_MAX_RISE_TIME_FM = 5
};

typedef struct{
	I2C_TypeDef *ptrI2Cx;
	uint8_t slaveAddress;
	uint8_t modeI2C;
	uint8_t dataI2C;
}I2C_Handler_t;

/* Public Function Prototypes */
void i2c_Config(I2C_Handler_t *ptrI2C_Handler);
void i2c_StartTransaction(I2C_Handler_t *ptrI2C_Handler);
void i2c_RestartTransaction(I2C_Handler_t *ptrI2C_Handler);
void i2c_SendSlaveAddress_RW(I2C_Handler_t *ptrI2C_Handler, uint8_t slaveAddress,uint8_t read0rWrite);
void i2c_SendMemoryAddress(I2C_Handler_t *ptrI2C_Handler, uint8_t memAddress);
void i2c_SendDataByte(I2C_Handler_t *ptrI2C_Handler, uint8_t dataToWrite);
uint8_t i2c_ReadDataByte(I2C_Handler_t *ptrI2C_Handler);
void i2c_StopTransaction(I2C_Handler_t *ptrI2C_Handler);
void i2c_Send_Ack(I2C_Handler_t *ptrI2C_Handler);
void i2c_Send_NoAck(I2C_Handler_t *ptrI2C_Handler);

uint8_t i2c_ReadSingleRegister(I2C_Handler_t *ptrI2C_Handler, uint8_t regToRead);
void i2c_WriteSingleRegister(I2C_Handler_t *ptrI2C_Handler, uint8_t regToRead, uint8_t newValue);
void i2c_readBurstSequenceRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToRead,uint8_t *ptrData,uint8_t number_of_register);


#endif /* I2C_DRIVER_HAL_H_ */
