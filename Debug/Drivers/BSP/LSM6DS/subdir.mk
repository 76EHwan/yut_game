################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/BSP/LSM6DS/lsm6ds3tr_c.c 

OBJS += \
./Drivers/BSP/LSM6DS/lsm6ds3tr_c.o 

C_DEPS += \
./Drivers/BSP/LSM6DS/lsm6ds3tr_c.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/BSP/LSM6DS/%.o Drivers/BSP/LSM6DS/%.su Drivers/BSP/LSM6DS/%.cyclo: ../Drivers/BSP/LSM6DS/%.c Drivers/BSP/LSM6DS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_PWR_LDO_SUPPLY -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Drivers/BSP/ST7735 -I../Drivers/BSP/LSM6DS -I../Drivers/BSP/MCF8316C -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-BSP-2f-LSM6DS

clean-Drivers-2f-BSP-2f-LSM6DS:
	-$(RM) ./Drivers/BSP/LSM6DS/lsm6ds3tr_c.cyclo ./Drivers/BSP/LSM6DS/lsm6ds3tr_c.d ./Drivers/BSP/LSM6DS/lsm6ds3tr_c.o ./Drivers/BSP/LSM6DS/lsm6ds3tr_c.su

.PHONY: clean-Drivers-2f-BSP-2f-LSM6DS

