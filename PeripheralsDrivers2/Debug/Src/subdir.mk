################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/HDC1080_sensor.c \
../Src/PwmDriver.c \
../Src/adc_driver_hal.c \
../Src/exti_driver_hal.c \
../Src/gpio_driver_hal.c \
../Src/i2c_driver_hal.c \
../Src/oled_driver_hal.c \
../Src/pll_driver_hal.c \
../Src/systick_driver_hal.c \
../Src/timer_driver_hal.c \
../Src/usart_driver_hal.c 

OBJS += \
./Src/HDC1080_sensor.o \
./Src/PwmDriver.o \
./Src/adc_driver_hal.o \
./Src/exti_driver_hal.o \
./Src/gpio_driver_hal.o \
./Src/i2c_driver_hal.o \
./Src/oled_driver_hal.o \
./Src/pll_driver_hal.o \
./Src/systick_driver_hal.o \
./Src/timer_driver_hal.o \
./Src/usart_driver_hal.o 

C_DEPS += \
./Src/HDC1080_sensor.d \
./Src/PwmDriver.d \
./Src/adc_driver_hal.d \
./Src/exti_driver_hal.d \
./Src/gpio_driver_hal.d \
./Src/i2c_driver_hal.d \
./Src/oled_driver_hal.d \
./Src/pll_driver_hal.d \
./Src/systick_driver_hal.d \
./Src/timer_driver_hal.d \
./Src/usart_driver_hal.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DNUCLEO_F411RE -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -DSTM32F4xx -c -I"/home/luciana/GitHub/Fundamentos-de-control/PeripheralsDrivers2/Inc" -I/home/luciana/STM32CubeIDE/Fcontrol/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Include -I/home/luciana/STM32CubeIDE/Fcontrol/CMSIS-repo/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/HDC1080_sensor.d ./Src/HDC1080_sensor.o ./Src/HDC1080_sensor.su ./Src/PwmDriver.d ./Src/PwmDriver.o ./Src/PwmDriver.su ./Src/adc_driver_hal.d ./Src/adc_driver_hal.o ./Src/adc_driver_hal.su ./Src/exti_driver_hal.d ./Src/exti_driver_hal.o ./Src/exti_driver_hal.su ./Src/gpio_driver_hal.d ./Src/gpio_driver_hal.o ./Src/gpio_driver_hal.su ./Src/i2c_driver_hal.d ./Src/i2c_driver_hal.o ./Src/i2c_driver_hal.su ./Src/oled_driver_hal.d ./Src/oled_driver_hal.o ./Src/oled_driver_hal.su ./Src/pll_driver_hal.d ./Src/pll_driver_hal.o ./Src/pll_driver_hal.su ./Src/systick_driver_hal.d ./Src/systick_driver_hal.o ./Src/systick_driver_hal.su ./Src/timer_driver_hal.d ./Src/timer_driver_hal.o ./Src/timer_driver_hal.su ./Src/usart_driver_hal.d ./Src/usart_driver_hal.o ./Src/usart_driver_hal.su

.PHONY: clean-Src

