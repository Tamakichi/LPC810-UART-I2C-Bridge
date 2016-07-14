################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/UART2I2C.c \
../src/cr_startup_lpc8xx.c \
../src/crp.c \
../src/i2c.c \
../src/mrt.c \
../src/uart.c 

OBJS += \
./src/UART2I2C.o \
./src/cr_startup_lpc8xx.o \
./src/crp.o \
./src/i2c.o \
./src/mrt.o \
./src/uart.o 

C_DEPS += \
./src/UART2I2C.d \
./src/cr_startup_lpc8xx.d \
./src/crp.d \
./src/i2c.d \
./src/mrt.d \
./src/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__CODE_RED -DCORE_M0PLUS -D__MTB_DISABLE -D__MTB_BUFFER_SIZE=0 -D__USE_CMSIS=CMSIS_CORE_LPC8xx -D__LPC8XX__ -D__REDLIB__ -I"E:\LPCXpresso\workspace\CMSIS_CORE_LPC8xx\inc" -I"E:\LPCXpresso\workspace\lpc800_driver_lib\inc" -O0 -g3 -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m0 -mthumb -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


