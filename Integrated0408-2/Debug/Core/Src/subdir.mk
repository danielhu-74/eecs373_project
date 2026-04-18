################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/app_flow.c \
../Core/Src/collision.c \
../Core/Src/ddr_pad.c \
../Core/Src/final_stage.c \
../Core/Src/game_app.c \
../Core/Src/game_init.c \
../Core/Src/game_update.c \
../Core/Src/lcd_minimal.c \
../Core/Src/lcd_touch_demo.c \
../Core/Src/lcd_ui.c \
../Core/Src/main.c \
../Core/Src/mp3_control.c \
../Core/Src/paddle_shuttle.c \
../Core/Src/play_mode.c \
../Core/Src/scoring.c \
../Core/Src/spi.c \
../Core/Src/start_page.c \
../Core/Src/stm32l4xx_hal_msp.c \
../Core/Src/stm32l4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32l4xx.c \
../Core/Src/touch_xpt2046.c \
../Core/Src/wii_nunchuk.c 

OBJS += \
./Core/Src/app_flow.o \
./Core/Src/collision.o \
./Core/Src/ddr_pad.o \
./Core/Src/final_stage.o \
./Core/Src/game_app.o \
./Core/Src/game_init.o \
./Core/Src/game_update.o \
./Core/Src/lcd_minimal.o \
./Core/Src/lcd_touch_demo.o \
./Core/Src/lcd_ui.o \
./Core/Src/main.o \
./Core/Src/mp3_control.o \
./Core/Src/paddle_shuttle.o \
./Core/Src/play_mode.o \
./Core/Src/scoring.o \
./Core/Src/spi.o \
./Core/Src/start_page.o \
./Core/Src/stm32l4xx_hal_msp.o \
./Core/Src/stm32l4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32l4xx.o \
./Core/Src/touch_xpt2046.o \
./Core/Src/wii_nunchuk.o 

C_DEPS += \
./Core/Src/app_flow.d \
./Core/Src/collision.d \
./Core/Src/ddr_pad.d \
./Core/Src/final_stage.d \
./Core/Src/game_app.d \
./Core/Src/game_init.d \
./Core/Src/game_update.d \
./Core/Src/lcd_minimal.d \
./Core/Src/lcd_touch_demo.d \
./Core/Src/lcd_ui.d \
./Core/Src/main.d \
./Core/Src/mp3_control.d \
./Core/Src/paddle_shuttle.d \
./Core/Src/play_mode.d \
./Core/Src/scoring.d \
./Core/Src/spi.d \
./Core/Src/start_page.d \
./Core/Src/stm32l4xx_hal_msp.d \
./Core/Src/stm32l4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32l4xx.d \
./Core/Src/touch_xpt2046.d \
./Core/Src/wii_nunchuk.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L4R5xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/app_flow.cyclo ./Core/Src/app_flow.d ./Core/Src/app_flow.o ./Core/Src/app_flow.su ./Core/Src/collision.cyclo ./Core/Src/collision.d ./Core/Src/collision.o ./Core/Src/collision.su ./Core/Src/ddr_pad.cyclo ./Core/Src/ddr_pad.d ./Core/Src/ddr_pad.o ./Core/Src/ddr_pad.su ./Core/Src/final_stage.cyclo ./Core/Src/final_stage.d ./Core/Src/final_stage.o ./Core/Src/final_stage.su ./Core/Src/game_app.cyclo ./Core/Src/game_app.d ./Core/Src/game_app.o ./Core/Src/game_app.su ./Core/Src/game_init.cyclo ./Core/Src/game_init.d ./Core/Src/game_init.o ./Core/Src/game_init.su ./Core/Src/game_update.cyclo ./Core/Src/game_update.d ./Core/Src/game_update.o ./Core/Src/game_update.su ./Core/Src/lcd_minimal.cyclo ./Core/Src/lcd_minimal.d ./Core/Src/lcd_minimal.o ./Core/Src/lcd_minimal.su ./Core/Src/lcd_touch_demo.cyclo ./Core/Src/lcd_touch_demo.d ./Core/Src/lcd_touch_demo.o ./Core/Src/lcd_touch_demo.su ./Core/Src/lcd_ui.cyclo ./Core/Src/lcd_ui.d ./Core/Src/lcd_ui.o ./Core/Src/lcd_ui.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/mp3_control.cyclo ./Core/Src/mp3_control.d ./Core/Src/mp3_control.o ./Core/Src/mp3_control.su ./Core/Src/paddle_shuttle.cyclo ./Core/Src/paddle_shuttle.d ./Core/Src/paddle_shuttle.o ./Core/Src/paddle_shuttle.su ./Core/Src/play_mode.cyclo ./Core/Src/play_mode.d ./Core/Src/play_mode.o ./Core/Src/play_mode.su ./Core/Src/scoring.cyclo ./Core/Src/scoring.d ./Core/Src/scoring.o ./Core/Src/scoring.su ./Core/Src/spi.cyclo ./Core/Src/spi.d ./Core/Src/spi.o ./Core/Src/spi.su ./Core/Src/start_page.cyclo ./Core/Src/start_page.d ./Core/Src/start_page.o ./Core/Src/start_page.su ./Core/Src/stm32l4xx_hal_msp.cyclo ./Core/Src/stm32l4xx_hal_msp.d ./Core/Src/stm32l4xx_hal_msp.o ./Core/Src/stm32l4xx_hal_msp.su ./Core/Src/stm32l4xx_it.cyclo ./Core/Src/stm32l4xx_it.d ./Core/Src/stm32l4xx_it.o ./Core/Src/stm32l4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32l4xx.cyclo ./Core/Src/system_stm32l4xx.d ./Core/Src/system_stm32l4xx.o ./Core/Src/system_stm32l4xx.su ./Core/Src/touch_xpt2046.cyclo ./Core/Src/touch_xpt2046.d ./Core/Src/touch_xpt2046.o ./Core/Src/touch_xpt2046.su ./Core/Src/wii_nunchuk.cyclo ./Core/Src/wii_nunchuk.d ./Core/Src/wii_nunchuk.o ./Core/Src/wii_nunchuk.su

.PHONY: clean-Core-2f-Src

