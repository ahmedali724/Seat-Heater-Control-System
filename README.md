# Seat Heater Control System Using FreeRTOS

## Project Overview
This project implements a **Seat Heater Control System** for both driver and passenger seats using the **TM4C123GH6PM** microcontroller and **FreeRTOS** for real-time task management. The system adjusts the heater intensity based on real-time temperature readings, utilizing a potentiometer (POT) to simulate temperature sensors.

## Features
- **Heating Levels**: The system supports four heating modes – **Off**, **Low**, **Medium**, and **High**.
- **Temperature Control**: Adjusts heater intensity using **LEDs** to reflect the difference between current and target temperatures.
- **Diagnostics**: Error detection for invalid temperature readings, with a **red LED** indicating sensor failures.
- **AUTOSAR & MCAL Drivers**: Implements **Dio**, **Port**, **ADC**, **GPTM**, **NVIC**, and **UART** drivers.
- **FreeRTOS Integration**: Manages multiple tasks for sensor reading, button control, heater management, and diagnostics.

## Hardware Components
- **TM4C123GH6PM** microcontroller
- **Potentiometer** (POT) simulates the LM35 temperature sensor
- **LEDs** to indicate heater levels and errors
- **Buttons** for controlling heater levels

## Task Breakdown
- **Sensor Tasks**: Read and validate temperature from the POT.
- **Button Tasks**: Handle user inputs for adjusting the heating level.
- **Heater Tasks**: Adjust the heater intensity based on temperature differences (simulated by LEDs).
- **Diagnostic Tasks**: Monitor and report errors if temperature sensors fail or readings are out of range.

## How It Works
1. **Temperature Simulation**: The **POT** simulates temperature values between **5°C and 40°C**.
2. **Heating Intensity**: Based on the difference between the current and target temperature, the system adjusts the heater intensity:
   - **Low**: Green LED
   - **Medium**: Blue LED
   - **High**: Cyan LED
3. **Error Handling**: If the simulated temperature falls outside the valid range, the system disables the heater and alerts the user via a **red LED**.
4. **Real-Time Management**: All tasks are scheduled and managed by **FreeRTOS** to ensure efficient performance with minimal CPU load (~36%).

## Setup Instructions
1. **Hardware Setup**:
   - Connect the **POT** to simulate the temperature sensor (0V-3.3V corresponds to 0°C-45°C).
   - Wire the **LEDs** to represent the heater intensities.
   - Connect **buttons** to control the heating level.
2. **Software**:
   - Clone or download the project.
   - Flash the project to the **TM4C123GH6PM** microcontroller.
   - Monitor UART for system status and diagnostics.
