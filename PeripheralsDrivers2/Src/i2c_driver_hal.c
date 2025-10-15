/*
 * i2c_driver_hal.c
 *
 *  Created on: 26 nov. 2023
 *      Author: luciana
 */

#include <stm32f4xx.h>
#include <i2c_driver_hal.h>



/* Remember to configure the pins for I2C (SDA y SCL),
 * as alternate function in the GPIO module.
 *
 * This pins must be open-drain outputs with pull-up resistors.
 */

/* Private function prototypes */
static void i2c_enable_clock_peripheral(I2C_Handler_t *ptrI2C_Handler);
static void i2c_reset_peripheral(I2C_Handler_t *ptrI2C_Handler);
static void i2c_main_clock_speed(I2C_Handler_t *ptrI2C_Handler);
static void i2c_mode(I2C_Handler_t *ptrI2C_Handler);

void i2c_Config(I2C_Handler_t *ptrI2C_Handler){

	/* Activate the clock signal for the I2C module selected */
	i2c_enable_clock_peripheral(ptrI2C_Handler);

	/* Reset peripheral before use it */
	i2c_reset_peripheral(ptrI2C_Handler);

	/* Set the main clock rate to generate the clock signal for the I2C bus */
	i2c_main_clock_speed(ptrI2C_Handler);

	/*
	* Set I2C mode
	*/
	i2c_mode(ptrI2C_Handler);

	/* Activate the I2C module */
	ptrI2C_Handler->ptrI2Cx->CR1 |= I2C_CR1_PE;

}

/* Private function definitions */
static void i2c_enable_clock_peripheral(I2C_Handler_t *ptrI2C_Handler){
	/* Activate the clock signal for the I2C module selected */
	if(ptrI2C_Handler->ptrI2Cx == I2C1){

		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	}
	else if(ptrI2C_Handler->ptrI2Cx == I2C2){

		RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

	}
	else if(ptrI2C_Handler->ptrI2Cx == I2C3){

		RCC->APB1ENR |= RCC_APB1ENR_I2C3EN;

	}

}
static void i2c_reset_peripheral(I2C_Handler_t *ptrI2C_Handler){
	/* Reset peripheral before use it */

	ptrI2C_Handler->ptrI2Cx->CR1 |= (I2C_CR1_SWRST); // Activate Reset peripheral
	__NOP();
	ptrI2C_Handler->ptrI2Cx->CR1 &= ~(I2C_CR1_SWRST); // Deactivate Reset peripheral
}
static void i2c_main_clock_speed(I2C_Handler_t *ptrI2C_Handler){
	/* Set the main clock rate to generate the clock signal for the I2C bus */
	ptrI2C_Handler->ptrI2Cx->CR2 &= ~(0b111111 << I2C_CR2_FREQ_Pos); // Clear bits
	ptrI2C_Handler->ptrI2Cx->CR2 |= (I2C_MAIN_CLOCK_16MHz << I2C_CR2_FREQ_Pos);
}

static void i2c_mode(I2C_Handler_t *ptrI2C_Handler){
	/*
	* Set I2C mode
	*
	* Here must be included the clock rate and T-rise
	* First, reset both register to zero
	*
	*/
	ptrI2C_Handler->ptrI2Cx->CCR = RESET;
	ptrI2C_Handler->ptrI2Cx->TRISE = RESET;

	if(ptrI2C_Handler->modeI2C == I2C_MODE_SM){ // For the standard mode

		ptrI2C_Handler->ptrI2Cx->CCR &= ~(I2C_CCR_FS); // To select Standard Mode

		// Configures the clock signal generator register
		ptrI2C_Handler->ptrI2Cx->CCR |= (I2C_MODE_SM_SPEED_100kHz << I2C_CCR_CCR_Pos);

		// Configures the max rise time register (T-Rise)
		ptrI2C_Handler->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_SM;

	}
	else{ // For the fast mode

		ptrI2C_Handler->ptrI2Cx->CCR |= (I2C_CCR_FS); // To select Fast Mode

		// Configures the clock signal generator register
		ptrI2C_Handler->ptrI2Cx->CCR |= (I2C_MODE_FM_SPEED_400kHz << I2C_CCR_CCR_Pos);

		// Configures the max rise time register (T-Rise)
		ptrI2C_Handler->ptrI2Cx->TRISE |= I2C_MAX_RISE_TIME_FM;

	}
}

/* Public function definitions */
void i2c_StartTransaction(I2C_Handler_t *ptrI2C_Handler){

	ptrI2C_Handler->ptrI2Cx->CR1 &= ~I2C_CR1_STOP;

	while(ptrI2C_Handler->ptrI2Cx->SR2 & I2C_SR2_BUSY){
		__NOP();
	}

	/* Set start condition */
	ptrI2C_Handler->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* Wait until the start flag is up, while waiting the SB value is 0
	* then the reverse value (!) is 1 */
	while(!(ptrI2C_Handler->ptrI2Cx->SR1 & I2C_SR1_SB)){
		__NOP();
	}
}
void i2c_RestartTransaction(I2C_Handler_t *ptrI2C_Handler){
	/* Set start condition */
	ptrI2C_Handler->ptrI2Cx->CR1 |= I2C_CR1_START;

	/* Wait until the start flag is up, while waiting the SB value is 0
	* then the reverse value (!) is 1 */
	while(!(ptrI2C_Handler->ptrI2Cx->SR1 & I2C_SR1_SB)){
		__NOP();
	}
}

void i2c_SendSlaveAddress_RW(I2C_Handler_t *ptrI2C_Handler, uint8_t slaveAddress,uint8_t read0rWrite){
	/* Define an auxiliary variable */
	uint8_t auxByte = 0;
	(void) auxByte;

	/*Send the slave address and the bit that tells -write zero(0)- */
	ptrI2C_Handler->ptrI2Cx->DR = (slaveAddress << 1) | read0rWrite;


// esperamos a que la direccion de memoria se termine de transmitir
	delay_ms(7);
	ptrI2C_Handler->ptrI2Cx->SR1 &= ~ I2C_SR1_AF;

	if (ptrI2C_Handler->ptrI2Cx->CR1 & I2C_CR1_STOP){
		ptrI2C_Handler->ptrI2Cx->CR1 &= ~I2C_CR1_STOP;
	}
	else{
		ptrI2C_Handler->ptrI2Cx->CR1 &= ~I2C_CR1_STOP;

	}
	/* Wait until the ADDR flag is up.
	* That means the address was send correctly.
	*/

	while(!(ptrI2C_Handler->ptrI2Cx->SR1 & I2C_SR1_ADDR)){
		__NOP();
	}

	/* The ACK reception flag of the ADDR must be cleared,
	* this will be accomplished by read first I2C_SR1 and then I2C_SR2
	*/
	auxByte = ptrI2C_Handler->ptrI2Cx->SR1;
	auxByte = ptrI2C_Handler->ptrI2Cx->SR2;
}
void i2c_SendMemoryAddress(I2C_Handler_t *ptrI2C_Handler, uint8_t memAddress){
	/* Send the desire memory address to read */
	ptrI2C_Handler->ptrI2Cx->DR = memAddress;

	/* Wait until the byte is transmitted */
	while(!(ptrI2C_Handler->ptrI2Cx->SR1 & I2C_SR1_TXE)){
		__NOP();
	}
}
void i2c_SendDataByte(I2C_Handler_t *ptrI2C_Handler, uint8_t dataToWrite){
	/* Load the value to be wrote */
	ptrI2C_Handler->ptrI2Cx->DR = dataToWrite;

	/* Wait until the byte is transmitted */
	while(!(ptrI2C_Handler->ptrI2Cx->SR1 & I2C_SR1_BTF)){
		__NOP();
	}
}
uint8_t i2c_ReadDataByte(I2C_Handler_t *ptrI2C_Handler){
	/* Wait until the byte is received */
	while(!(ptrI2C_Handler->ptrI2Cx->SR1 & I2C_SR1_RXNE)){
		__NOP();
	}

	ptrI2C_Handler->dataI2C = ptrI2C_Handler->ptrI2Cx->DR;
	return ptrI2C_Handler->dataI2C;
}
void i2c_StopTransaction(I2C_Handler_t *ptrI2C_Handler){
	/* Set stop condition */
	ptrI2C_Handler->ptrI2Cx->CR1 |= I2C_CR1_STOP;

	delay_ms(10);
}
void i2c_Send_Ack(I2C_Handler_t *ptrI2C_Handler){
	/* Write 1 at the ACK bit on the CR1 */
	ptrI2C_Handler->ptrI2Cx->CR1 |= I2C_CR1_ACK;
}
void i2c_Send_NoAck(I2C_Handler_t *ptrI2C_Handler){
	/* Write zero at the ACK bit on the CR1 */
	ptrI2C_Handler->ptrI2Cx->CR1 &= ~(I2C_CR1_ACK);
}

uint8_t i2c_ReadSingleRegister(I2C_Handler_t *ptrI2C_Handler, uint8_t regToRead){
	/* Create a auxiliary variable to receive the writing data */
	uint8_t auxRead = 0;

	/* Set start condition */
	i2c_StartTransaction(ptrI2C_Handler);

	/* Send the slave address and tell it to write */
	i2c_SendSlaveAddress_RW(ptrI2C_Handler, ptrI2C_Handler->slaveAddress, I2C_WRITE_DATA);
	/*Send the desire memory address to read */
	i2c_SendMemoryAddress(ptrI2C_Handler, regToRead);

	/* Set a restart condition */
	i2c_RestartTransaction(ptrI2C_Handler);
	/* Send the slave address and tell it to read*/
	i2c_SendSlaveAddress_RW(ptrI2C_Handler, ptrI2C_Handler->slaveAddress, I2C_READ_DATA);
	/* Set the NoAck condition in order to the master don't answer and the slave sends 1 byte */
	i2c_Send_NoAck(ptrI2C_Handler);
	/* Set the Stop condition in order to the slave stops itself after 1 byte */
	i2c_StopTransaction(ptrI2C_Handler);
	/* Read the incoming data from the slave */
	auxRead = i2c_ReadDataByte(ptrI2C_Handler);

	return auxRead;
}

void i2c_readBurstSequenceRegister(I2C_Handler_t *pHandlerI2C, uint8_t regToRead,uint8_t *ptrData,uint8_t number_of_register){

	/* 0. Creamos una variable auxiliar para recribir el dato que leemos*/
	uint8_t auxRead = 0;

	/* 1. Generamos la condicion Start */
	i2c_StartTransaction(pHandlerI2C);

	/* 2. Enviamos la direccion del esclavo y la direccion de ESCRIBIR */
	i2c_SendSlaveAddress_RW(pHandlerI2C, pHandlerI2C->slaveAddress, I2C_WRITE_DATA);

	/* 3. Enviamos la direccion de memoria que deseamos leer */
	i2c_SendMemoryAddress(pHandlerI2C, regToRead);

	/* 4. Creamos una condicion re Start */
	i2c_RestartTransaction(pHandlerI2C);

	/* 5. Enviamos la direccion del esclavo y la indicacion de LEER */
	i2c_SendSlaveAddress_RW(pHandlerI2C, pHandlerI2C->slaveAddress, I2C_READ_DATA);

	for (int i = 0; i < number_of_register; i++) {
	/*Leemos la primera posicion debido a que cuando enviamos una secuencia
	* el primer ACK lo genera el esclavo, despues debemos generar nosotros
	* por cada byte que leemos el ACK
	* */
	auxRead = i2c_ReadDataByte(pHandlerI2C);
	ptrData[i] = auxRead ;
	/*Enviamos un ACK para que siga enviando datos*/
	i2c_Send_Ack(pHandlerI2C);

	}

	/* 6. Generamos la condicion de NoACK, para que el Master no responda y el slave solo envie 1 byte*/
	i2c_Send_NoAck(pHandlerI2C);

	/* 7. Generamos la condicion Stop, para que el slave se detenga despues de 1 byte */
	i2c_StopTransaction(pHandlerI2C);

	// /* 8. Leemos el dato que envia el esclavo */
	// auxRead = i2c_readDataByte(pHandlerI2C);
	//
	// return auxRead;
}

void i2c_WriteSingleRegister(I2C_Handler_t *ptrI2C_Handler, uint8_t regToRead, uint8_t newValue){
	/* Set start condition */
	i2c_StartTransaction(ptrI2C_Handler);

	/* Send the slave address and tell it to write */
	i2c_SendSlaveAddress_RW(ptrI2C_Handler, ptrI2C_Handler->slaveAddress, I2C_WRITE_DATA);
	/* Send the desire memory address to write */
	i2c_SendMemoryAddress(ptrI2C_Handler, regToRead);
	/* Send the value to be write on the selected register */
	i2c_SendDataByte(ptrI2C_Handler, newValue);

	/* Set stop condition in order to the slave stops itself after 1 byte */
	i2c_StopTransaction(ptrI2C_Handler);
}
