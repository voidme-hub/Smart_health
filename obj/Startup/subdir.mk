################################################################################
# MRS Version: 2.3.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../Startup/startup_ch32v30x_D8C.S 

S_UPPER_DEPS += \
./Startup/startup_ch32v30x_D8C.d 

OBJS += \
./Startup/startup_ch32v30x_D8C.o 

DIR_OBJS += \
./Startup/*.o \

DIR_DEPS += \
./Startup/*.d \

DIR_EXPANDS += \
./Startup/*.234r.expand \


# Each subdirectory must supply rules for building sources it contributes
Startup/%.o: ../Startup/%.S
	@	riscv-none-embed-gcc -march=rv32imafcxw -mabi=ilp32f -msmall-data-limit=8 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant -Wunused -Wuninitialized -g -x assembler-with-cpp -I"f:/EVT/CH32V307-FreeRTOS/Startup" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/include" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/Common" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable/MemMang" -I"f:/EVT/CH32V307-FreeRTOS/FreeRTOS/portable" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

