################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"/home/travis/ti/ccs1010/ccs/tools/compiler/ti-cgt-msp430_20.2.1.LTS/bin/cl430" -vmsp --use_hw_mpy=none --include_path="/home/travis/ti/ccs1010/ccs/ccs_base/msp430/include" --include_path="/home/travis/e-cribbage/Software/CCS_e-cribbage" --include_path="/home/travis/ti/ccs1010/ccs/tools/compiler/ti-cgt-msp430_20.2.1.LTS/include" --advice:power=all --define=__MSP430F2012__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


