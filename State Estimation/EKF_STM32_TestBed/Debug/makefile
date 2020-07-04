################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include USB_DEVICE/Target/subdir.mk
-include USB_DEVICE/App/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Core/Src/subdir.mk
-include Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/Drivers/subdir.mk
-include Core/Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
EKF_STM32F4.elf \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
EKF_STM32F4.list \

OBJCOPY_BIN += \
EKF_STM32F4.bin \


# All Target
all: EKF_STM32F4.elf secondary-outputs

# Tool invocations
EKF_STM32F4.elf: $(OBJS) $(USER_OBJS) C:\Users\phili\OneDrive\UAV\State\ Estimation\EKF_STM32_TestBed\STM32F405RGTX_FLASH.ld
	arm-none-eabi-gcc -o "EKF_STM32F4.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"C:\Users\phili\OneDrive\UAV\State Estimation\EKF_STM32_TestBed\STM32F405RGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="EKF_STM32F4.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -u _printf_float -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

EKF_STM32F4.list: $(EXECUTABLES)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "EKF_STM32F4.list"
	@echo 'Finished building: $@'
	@echo ' '

EKF_STM32F4.bin: $(EXECUTABLES)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "EKF_STM32F4.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) *
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_BIN)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
