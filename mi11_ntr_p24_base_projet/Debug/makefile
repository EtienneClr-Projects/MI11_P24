################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := mi11_ntr_p24_base_projet
BUILD_ARTIFACT_EXTENSION :=
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: main-build

# Main-build Target
main-build: mi11_ntr_p24_base_projet

# Tool invocations
mi11_ntr_p24_base_projet: $(OBJS) $(USER_OBJS) makefile $(OPTIONAL_TOOL_DEPS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-none-eabi-gcc -nostdlib -T ../ld.x -mcpu=cortex-m7 -mfloat-abi=softfp -o "mi11_ntr_p24_base_projet" $(OBJS) $(USER_OBJS) $(LIBS) -lgcc
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) mi11_ntr_p24_base_projet
	-@echo ' '

.PHONY: all clean dependents main-build

-include ../makefile.targets
