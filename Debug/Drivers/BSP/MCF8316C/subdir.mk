################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/MCF8316C/mcf8316c.c 

OBJS += \
./Drivers/BSP/MCF8316C/mcf8316c.o 

C_DEPS += \
./Drivers/BSP/MCF8316C/mcf8316c.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/MCF8316C/%.o Drivers/BSP/MCF8316C/%.su Drivers/BSP/MCF8316C/%.cyclo: ../Drivers/BSP/MCF8316C/%.c Drivers/BSP/MCF8316C/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/ST7735 -I../Drivers/BSP/LSM6DS -I../Drivers/BSP/MCF8316C -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-MCF8316C

clean-Drivers-2f-BSP-2f-MCF8316C:
	-$(RM) ./Drivers/BSP/MCF8316C/mcf8316c.cyclo ./Drivers/BSP/MCF8316C/mcf8316c.d ./Drivers/BSP/MCF8316C/mcf8316c.o ./Drivers/BSP/MCF8316C/mcf8316c.su

.PHONY: clean-Drivers-2f-BSP-2f-MCF8316C

