################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
HAL/Button/%.obj: ../HAL/Button/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ccs1271/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/Common" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/HAL" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/HAL/Temperatrue Sensor" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/HAL/Button" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/HAL/Led" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL/NVIC" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL/ADC" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL/Dio" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL/Port" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL/UART" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/MCAL/GPTM" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/FreeRTOS/Source/include" --include_path="D:/Study/M.T Embedded Course/Advanced/RTOS/FreeRTOS Final Project/FreeRTOS_Final_Project_WS/Seat_Heater_Control_System/FreeRTOS/Source/portable/CCS/ARM_CM4F" --include_path="C:/ti/ccs1271/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="HAL/Button/$(basename $(<F)).d_raw" --obj_directory="HAL/Button" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


