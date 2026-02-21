################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Hardware/Src/B1uart.c \
../Hardware/Src/algorithm.c \
../Hardware/Src/beep.c \
../Hardware/Src/iwdg.c \
../Hardware/Src/key.c \
../Hardware/Src/lcd.c \
../Hardware/Src/max30102.c \
../Hardware/Src/max30102_app.c \
../Hardware/Src/mq2.c \
../Hardware/Src/myiic.c \
../Hardware/Src/servo.c \
../Hardware/Src/sht31.c 

C_DEPS += \
./Hardware/Src/B1uart.d \
./Hardware/Src/algorithm.d \
./Hardware/Src/beep.d \
./Hardware/Src/iwdg.d \
./Hardware/Src/key.d \
./Hardware/Src/lcd.d \
./Hardware/Src/max30102.d \
./Hardware/Src/max30102_app.d \
./Hardware/Src/mq2.d \
./Hardware/Src/myiic.d \
./Hardware/Src/servo.d \
./Hardware/Src/sht31.d 

OBJS += \
./Hardware/Src/B1uart.o \
./Hardware/Src/algorithm.o \
./Hardware/Src/beep.o \
./Hardware/Src/iwdg.o \
./Hardware/Src/key.o \
./Hardware/Src/lcd.o \
./Hardware/Src/max30102.o \
./Hardware/Src/max30102_app.o \
./Hardware/Src/mq2.o \
./Hardware/Src/myiic.o \
./Hardware/Src/servo.o \
./Hardware/Src/sht31.o 

DIR_OBJS += \
./Hardware/Src/*.o \

DIR_DEPS += \
./Hardware/Src/*.d \

DIR_EXPANDS += \
./Hardware/Src/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Hardware/Src/%.o: ../Hardware/Src/%.c
	@	riscv-none-embed-gcc -march=rv32imafcxw -mabi=ilp32f -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant -Wunused -Wuninitialized -g -I"f:/EVT/CH32V307-FreeRTOS/Debug" -I"f:/EVT/CH32V307-FreeRTOS/Core" -I"f:/EVT/CH32V307-FreeRTOS/User" -I"f:/EVT/CH32V307-FreeRTOS/Peripheral/inc" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/include" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -I"f:/EVT/CH32V307-FreeRTOS/Hardware/Inc" -I"f:/EVT/CH32V307-FreeRTOS/Hardware/Src" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

