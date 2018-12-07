################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
pressureComputation/BMP280_driver-master\ (1)/BMP280_driver-master/bmp280.obj: ../pressureComputation/BMP280_driver-master\ (1)/BMP280_driver-master/bmp280.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/bin/cl430" -vmspx --code_model=large --data_model=restricted --near_data=none --use_hw_mpy=F5 --include_path="C:/ti/ccsv7/ccs_base/msp430/include" --include_path="C:/Users/David/workspace_v7/GEW-FFHUB" --include_path="C:/Users/David/workspace_v7/GEW-FFHUB/driverlib/MSP430FR2xx_4xx" --include_path="C:/ti/ccsv7/tools/compiler/ti-cgt-msp430_16.9.6.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR2433__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="pressureComputation/BMP280_driver-master (1)/BMP280_driver-master/bmp280.d_raw" --obj_directory="pressureComputation/BMP280_driver-master (1)/BMP280_driver-master" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


