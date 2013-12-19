################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/lib/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Dip-WS2812B.axf

# Tool invocations
Dip-WS2812B.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"Z:\SolderSplash\Software\DipWS2812B\CMSISv2p10_LPC13Uxx\Debug" -Xlinker -Map="Dip-WS2812B.map" -Xlinker --gc-sections -mcpu=cortex-m3 -mthumb -T "Dip-NeoPixel_Debug.ld" -o "Dip-WS2812B.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Dip-WS2812B.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "Dip-WS2812B.axf"; arm-none-eabi-objcopy -O binary "Dip-WS2812B.axf" "Dip-WS2812B.bin" ; checksum -p LPC1347 -d "Dip-WS2812B.bin";
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
