################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: UART2I2C.axf

# Tool invocations
UART2I2C.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"E:\LPCXpresso\workspace\CMSIS_CORE_LPC8xx\Release" -Xlinker -Map="UART2I2C.map" -Xlinker --gc-sections -mcpu=cortex-m0 -mthumb -T "UART2I2C_Release.ld" -o "UART2I2C.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) UART2I2C.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "UART2I2C.axf" ; arm-none-eabi-objcopy -O binary "UART2I2C.axf" "UART2I2C.bin" ; arm-none-eabi-objcopy -O ihex "UART2I2C.axf" "UART2I2C.hex" ; #checksum -p LPC810 -d "UART2I2C.bin"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
