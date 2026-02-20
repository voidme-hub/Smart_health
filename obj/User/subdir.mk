################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/ch32v30x_it.c \
../User/com.c \
../User/main.c \
../User/system_ch32v30x.c 

C_DEPS += \
./User/ch32v30x_it.d \
./User/com.d \
./User/main.d \
./User/system_ch32v30x.d 

OBJS += \
./User/ch32v30x_it.o \
./User/com.o \
./User/main.o \
./User/system_ch32v30x.o 

DIR_OBJS += \
./User/*.o \

DIR_DEPS += \
./User/*.d \

DIR_EXPANDS += \
./User/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
User/%.o: ../User/%.c
	@	riscv-none-embed-gcc -march=rv32imafcxw -mabi=ilp32f -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant -Wunused -Wuninitialized -g -I"f:/EVT/CH32V307-FreeRTOS/Debug" -I"f:/EVT/CH32V307-FreeRTOS/Core" -I"f:/EVT/CH32V307-FreeRTOS/User" -I"f:/EVT/CH32V307-FreeRTOS/Peripheral/inc" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/include" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -I"f:/EVT/CH32V307-FreeRTOS/Hardware/Inc" -I"f:/EVT/CH32V307-FreeRTOS/Hardware/Src" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

