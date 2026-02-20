################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS/portable/MemMang/heap_4.c 

C_DEPS += \
./FreeRTOS/portable/MemMang/heap_4.d 

OBJS += \
./FreeRTOS/portable/MemMang/heap_4.o 

DIR_OBJS += \
./FreeRTOS/portable/MemMang/*.o \

DIR_DEPS += \
./FreeRTOS/portable/MemMang/*.d \

DIR_EXPANDS += \
./FreeRTOS/portable/MemMang/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS/portable/MemMang/%.o: ../FreeRTOS/portable/MemMang/%.c
	@	riscv-none-embed-gcc -march=rv32imafcxw -mabi=ilp32f -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant -Wunused -Wuninitialized -g -I"f:/EVT/CH32V307-FreeRTOS/Debug" -I"f:/EVT/CH32V307-FreeRTOS/Core" -I"f:/EVT/CH32V307-FreeRTOS/User" -I"f:/EVT/CH32V307-FreeRTOS/Peripheral/inc" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/include" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -I"f:/EVT/CH32V307-FreeRTOS/Hardware/Inc" -I"f:/EVT/CH32V307-FreeRTOS/Hardware/Src" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

