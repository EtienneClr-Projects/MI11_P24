################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FIFO.c \
../SEM.c \
../TERMINAL.c \
../cortex.c \
../delay.c \
../init.c \
../noyau_file_prio.c \
../noyau_prio.c \
../noyau_test_prio.c \
../ouvrier_taches.c \
../serialio.c \
../stm_gpio.c \
../stm_uart.c 

S_UPPER_SRCS += \
../vectors.S 

C_DEPS += \
./FIFO.d \
./SEM.d \
./TERMINAL.d \
./cortex.d \
./delay.d \
./init.d \
./noyau_file_prio.d \
./noyau_prio.d \
./noyau_test_prio.d \
./ouvrier_taches.d \
./serialio.d \
./stm_gpio.d \
./stm_uart.d 

OBJS += \
./FIFO.o \
./SEM.o \
./TERMINAL.o \
./cortex.o \
./delay.o \
./init.o \
./noyau_file_prio.o \
./noyau_prio.o \
./noyau_test_prio.o \
./ouvrier_taches.o \
./serialio.o \
./stm_gpio.o \
./stm_uart.o \
./vectors.o 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -O0 -g -Wall -c -fmessage-length=0 -ffreestanding -mcpu=cortex-m7 -mfloat-abi=softfp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.S subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Assembler'
	arm-none-eabi-gcc -c -ggdb -mcpu=cortex-m7 -mfloat-abi=softfp -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean--2e-

clean--2e-:
	-$(RM) ./FIFO.d ./FIFO.o ./SEM.d ./SEM.o ./TERMINAL.d ./TERMINAL.o ./cortex.d ./cortex.o ./delay.d ./delay.o ./init.d ./init.o ./noyau_file_prio.d ./noyau_file_prio.o ./noyau_prio.d ./noyau_prio.o ./noyau_test_prio.d ./noyau_test_prio.o ./ouvrier_taches.d ./ouvrier_taches.o ./serialio.d ./serialio.o ./stm_gpio.d ./stm_gpio.o ./stm_uart.d ./stm_uart.o ./vectors.o

.PHONY: clean--2e-

