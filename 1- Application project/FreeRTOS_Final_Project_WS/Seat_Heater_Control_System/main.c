/*
 ============================================================================
 Name        : Seat Heater Control System
 Module Name : Main Control Module
 Author      : Ahmed Ali
 Date        : 22 Sept. 2024
 Description : Controls driver and passenger seat heating based on temperature
 ============================================================================
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "queue.h"

/* MCAL includes. */
#include "adc.h"
#include "uart0.h"
#include "Dio.h"
#include "Port.h"
#include "Mcu.h"
#include "GPTM.h"

/* HAL includes. */
#include "lm35.h"
#include "Button.h"
#include "Led.h"

/* Other includes. */
#include "tm4c123gh6pm_registers.h"

/* Event bits for button interrupts: SW1 and SW3 for the driver, SW2 for the passenger */
#define mainSW1_INTERRUPT_BIT       (1UL << 0UL) /* Bit for SW1 */
#define mainSW2_INTERRUPT_BIT       (1UL << 0UL) /* Bit for SW2 */
#define mainSW3_INTERRUPT_BIT       (1UL << 1UL) /* Bit for SW3 */

/* Heater state definitions for the heating system */
#define mainHEATER_STATE_OFF        0  /* Heater is off */
#define mainHEATER_STATE_LOW        1  /* Low intensity */
#define mainHEATER_STATE_MEDIUM     2  /* Medium intensity */
#define mainHEATER_STATE_HIGH       3  /* High intensity */

/* Heating level definitions for desired intensity */
#define mainHEATING_LEVEL_OFF       0  /* Off */
#define mainHEATING_LEVEL_LOW       1  /* Low */
#define mainHEATING_LEVEL_MEDIUM    2  /* Medium */
#define mainHEATING_LEVEL_HIGH      3  /* High */

/* Maximum and minimum valid temperature ranges */
#define mainTEMP_MAX_VALID_RANGE    40 /* Max: 40�C */
#define mainTEMP_MIN_VALID_RANGE    5  /* Min: 5�C */

/* Desired temperature levels for the heating system */
#define mainDESIRED_TEMP_OFF        0   /* Off (0�C) */
#define mainDESIRED_TEMP_LOW        25  /* Low (25�C) */
#define mainDESIRED_TEMP_MEDIUM     30  /* Medium (30�C) */
#define mainDESIRED_TEMP_HIGH       35  /* High (35�C) */

/* Error codes for diagnostic handling */
#define mainTEMP_OVER_RANGE_FAIL    0x44
#define mainTEMP_UNDER_RANGE_FAIL   0x55
#define mainDRIVER_SEAT_FAIL        0x66
#define mainPASSENGER_SEAT_FAIL     0x77

/* Diagnostic array size and element count */
#define mainDIAGNOSTIC_SIZE         5   /* Max size of the diagnostic array */

/*
 * Task delays for periodic tasks in the system:
 * - mainSENSOR_TASK_DELAY: 100 ms for sensor readings.
 * - mainHEATER_TASK_DELAY: 250 ms for heater control.
 * - mainDISPLAY_TASK_DELAY: 500 ms for display updates.
 * - mainRUNTIME_TASK_DELAY: 5000 ms for runtime measurements.
 */
#define mainSENSOR_TASK_DELAY           pdMS_TO_TICKS(100)
#define mainHEATER_TASK_DELAY           pdMS_TO_TICKS(250)
#define mainDISPLAY_TASK_DELAY          pdMS_TO_TICKS(500)
#define mainRUNTIME_TASK_DELAY          (5000U)

/* Define thresholds for temperature differences */
#define mainTEMP_DIFF_LOW_THRESHOLD         2   /* Threshold for low heating state (2�C) */
#define mainTEMP_DIFF_MEDIUM_THRESHOLD      5   /* Threshold for medium heating state (5�C) */
#define mainTEMP_DIFF_HIGH_THRESHOLD        10  /* Threshold for high heating state (10�C) */

/* Define the total number of heating levels */
#define mainTOTAL_HEATING_LEVELS            4  /* Total heating levels available */

#define NUMBER_OF_ITERATIONS_PER_ONE_MILI_SECOND 369

void Delay_MS(unsigned long long n)
{
    volatile unsigned long long count = 0;
    while (count++ < (NUMBER_OF_ITERATIONS_PER_ONE_MILI_SECOND * n))
        ;
}

/* Structure to log failure information for temperature sensors */
typedef struct xFailureLog
{
    uint32 uiTimeStamp; /* Timestamp of the failure event */
    uint8 ucFailureCode; /* Code indicating the type of failure */
    uint8 ucFailureSeat; /* Identifier for the seat where the failure occurred */
    uint8 ucHeatingLevel; /* Current heating level at the time of failure */
} xFailureLog;

/* Global Variables for heating system state management */

/* Driver heating level (0: OFF, 1: LOW, 2: MEDIUM, 3: HIGH) */
uint8 ucDriverHeatingLevel = 0;

/* Passenger heating level (0: OFF, 1: LOW, 2: MEDIUM, 3: HIGH) */
uint8 ucPassengerHeatingLevel = 0;

/* Desired temperature settings for the driver (in degrees) */
uint8 ucDriverDesiredTemperature = 0;

/* Desired temperature settings for the passenger (in degrees) */
uint8 ucPassengerDesiredTemperature = 0;

/* Current temperature readings for the driver (in degrees) */
uint8 ucDriverTemperatureValue = 0;

/* Current temperature readings for the passenger (in degrees) */
uint8 ucPassengerTemperatureValue = 0;

/* Current state of the driver�s heater (initially set to OFF) */
uint8 ucDriverHeaterState = mainHEATER_STATE_OFF;

/* Current state of the passenger�s heater (initially set to OFF) */
uint8 ucPassengerHeaterState = mainHEATER_STATE_OFF;

/* Flags for error report */
uint8 ucDriverErrorFlag = pdFALSE;
uint8 ucPassengerErrorFlag = pdFALSE;

/* Diagnostic array section in RAM */
xFailureLog xDiagnosticArray[mainDIAGNOSTIC_SIZE];
uint8 ucDiagnosticIndex = 0;

/* Arrays for storing runtime measurements of tasks (includes +1 for the Idle Task) */
uint32 ullTasksOutTime[mainTOTAL_NUMBER_OF_TASKS + 1]; /* Timestamps for task exit times */
uint32 ullTasksInTime[mainTOTAL_NUMBER_OF_TASKS + 1]; /* Timestamps for task entry times */
uint32 ullTasksTotalTime[mainTOTAL_NUMBER_OF_TASKS + 1]; /* Total execution time for each task */

/* The HW setup function */
static void prvSetupHardware(void);

/* FreeRTOS tasks */
void vDriverSensorProcessTask(void *pvParameters);
void vPassengerSensorsProcessTask(void *pvParameters);

void vDriverButtonsProcessTask(void *pvParameters);
void vPassengerButtonProcessTask(void *pvParameters);

void vDriverDiagnosticTask(void *pvParameters);
void vPassengerDiagnosticTask(void *pvParameters);

void vDriverHeaterProcessTask(void *pvParameters);
void vPassengerHeatersProcessTask(void *pvParameters);

void vDisplayScreenTask(void *pvParameters);
void vRunTimeMeasurementsTask(void *pvParameters);

/* Tasks Handles */
TaskHandle_t xDriverSensorsProcessHandle;
TaskHandle_t xPassengerSensorsProcessHandle;

TaskHandle_t xDriverButtonsProcessHandle;
TaskHandle_t xPassengerButtonProcessHandle;

TaskHandle_t xDriverHeaterProcessHandle;
TaskHandle_t xPassengerHeaterProcessHandle;

TaskHandle_t xDriverDiagnosticHandle;
TaskHandle_t xPassengerDiagnosticHandle;

TaskHandle_t xDisplayScreenHandle;
TaskHandle_t xRunTimeMeasurementsHandle;

/* FreeRTOS Events Group */
EventGroupHandle_t xDriverButtonsEventGroup;
EventGroupHandle_t xPassengerButtonEventGroup;

/* FreeRTOS Mutexes */
xSemaphoreHandle xDisplayScreenMutex;
xSemaphoreHandle xDriverHeatingLevelMutex;
xSemaphoreHandle xPassengerHeatingLevelMutex;

xSemaphoreHandle xDriverDesiredTempMutex;
xSemaphoreHandle xPassengerDesiredTempMutex;

xSemaphoreHandle xDriverTempValueMutex;
xSemaphoreHandle xPassengerTempValueMutex;

xSemaphoreHandle xDriverHeaterStateMutex;
xSemaphoreHandle xPassengerHeaterStateMutex;

/* FreeRTOS Binary Semaphores */
xSemaphoreHandle xDriverErrorReportSemaphore;
xSemaphoreHandle xPassengerErrorReportSemaphore;

/* FreeRTOS Queues */
QueueHandle_t xDriverDiagnosticQueue;
QueueHandle_t xPassengerDiagnosticQueue;

/*-------------------------------------------------------------------------------------------------------------------------------------*/

int main(void)
{
    /*
     * Initialize hardware components for the Tiva C board.
     * This includes configuring GPIOs, UART, ADCs, and other peripherals.
     * Hardware setup is essential before creating tasks or starting the FreeRTOS scheduler.
     */
    prvSetupHardware();

    /* Create a mutexs */
    xDisplayScreenMutex = xSemaphoreCreateMutex();

    xDriverDesiredTempMutex = xSemaphoreCreateMutex();
    xDriverHeaterStateMutex = xSemaphoreCreateMutex();
    xDriverHeatingLevelMutex = xSemaphoreCreateMutex();
    xDriverTempValueMutex = xSemaphoreCreateMutex();

    xPassengerDesiredTempMutex = xSemaphoreCreateMutex();
    xPassengerHeaterStateMutex = xSemaphoreCreateMutex();
    xPassengerHeatingLevelMutex = xSemaphoreCreateMutex();
    xPassengerTempValueMutex = xSemaphoreCreateMutex();

    /* Create event groups */
    xDriverButtonsEventGroup = xEventGroupCreate();
    xPassengerButtonEventGroup = xEventGroupCreate();

    /* Create binary semaphores */
    xDriverErrorReportSemaphore = xSemaphoreCreateBinary();
    xPassengerErrorReportSemaphore = xSemaphoreCreateBinary();

    /* Create diagnostic queues */
    xDriverDiagnosticQueue = xQueueCreate(3, sizeof(xFailureLog));
    xPassengerDiagnosticQueue = xQueueCreate(3, sizeof(xFailureLog));

    /*
     * Create FreeRTOS tasks for system functionalities, each with a specific role.
     * Tasks include:
     * - Driver Sensor: Reads temperature data from the driver�s LM35 sensor.
     * - Passenger Sensor: Reads temperature data from the passenger�s LM35 sensor.
     * - Driver Button: Monitors input from the driver�s buttons and manages events.
     * - Passenger Button: Monitors input from the passenger�s buttons and manages events.
     * - Driver Diagnostic: Handles error reporting for the driver's temperature sensor.
     * - Passenger Diagnostic: Handles error reporting for the passenger's temperature sensor.
     * - Driver Heater: Controls the driver�s heating system based on temperature readings.
     * - Passenger Heater: Controls the passenger�s heating system based on temperature readings.
     * - Display Screen: Updates the display with temperature and heater status information.
     * - Run Time Measurements: Collects and reports runtime data for monitoring CPU load and task performance.
     */
    xTaskCreate(vDriverSensorProcessTask, "Driver Sensor", 64, NULL, 4, &xDriverSensorsProcessHandle);
    xTaskCreate(vPassengerSensorsProcessTask, "Passenger Sensor", 64, NULL, 4, &xPassengerSensorsProcessHandle);

    xTaskCreate(vDriverButtonsProcessTask, "Driver Button", 64, NULL, 3, &xDriverButtonsProcessHandle);
    xTaskCreate(vPassengerButtonProcessTask, "Passenger Button", 64, NULL, 3, &xPassengerButtonProcessHandle);

    xTaskCreate(vDriverDiagnosticTask, "Driver Diagnostic", 64, NULL, 2, &xDriverDiagnosticHandle);
    xTaskCreate(vPassengerDiagnosticTask, "Passenger Diagnostic", 64, NULL, 2, &xPassengerDiagnosticHandle);

    xTaskCreate(vDriverHeaterProcessTask, "Driver Heater", 128, NULL, 1, &xDriverHeaterProcessHandle);
    xTaskCreate(vPassengerHeatersProcessTask, "Passenger Heater", 128, NULL, 1, &xPassengerHeaterProcessHandle);

    xTaskCreate(vDisplayScreenTask, "Display Screen", 64, NULL, 1, &xDisplayScreenHandle);
    xTaskCreate(vRunTimeMeasurementsTask, "Run Time", 64, NULL, 1, &xRunTimeMeasurementsHandle);

    /* Set application task tags for runtime measurement */
    vTaskSetApplicationTaskTag(xDriverSensorsProcessHandle, (TaskHookFunction_t) 1);
    vTaskSetApplicationTaskTag(xPassengerSensorsProcessHandle, (TaskHookFunction_t) 2);
    vTaskSetApplicationTaskTag(xDriverButtonsProcessHandle, (TaskHookFunction_t) 3);
    vTaskSetApplicationTaskTag(xPassengerButtonProcessHandle, (TaskHookFunction_t) 4);
    vTaskSetApplicationTaskTag(xDriverDiagnosticHandle, (TaskHookFunction_t) 5);
    vTaskSetApplicationTaskTag(xPassengerDiagnosticHandle, (TaskHookFunction_t) 6);
    vTaskSetApplicationTaskTag(xDriverHeaterProcessHandle, (TaskHookFunction_t) 7);
    vTaskSetApplicationTaskTag(xPassengerHeaterProcessHandle, (TaskHookFunction_t) 8);
    vTaskSetApplicationTaskTag(xDisplayScreenHandle, (TaskHookFunction_t) 9);
    vTaskSetApplicationTaskTag(xRunTimeMeasurementsHandle, (TaskHookFunction_t) 10);

    /*
     * Start the FreeRTOS scheduler to begin task execution.
     * Once the scheduler starts, tasks will begin running based on their assigned priorities.
     * It is crucial that this function is called in supervisor mode to ensure proper operation.
     */
    vTaskStartScheduler();

    /*
     * If the scheduler starts successfully, this line will never be reached.
     * If execution reaches here, it indicates that there was insufficient heap memory
     * to create the idle task, which is a critical failure.
     */
    for (;;)
        ; /* Infinite loop to prevent any further execution */

}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Function to initialize hardware components.
 * This includes setting up microcontroller peripherals such as GPIOs, ADCs, and UARTs.
 * Specific initialization routines will depend on the hardware configuration needed
 * for the application to function correctly.
 */
static void prvSetupHardware(void)
{
    /*
     * Call hardware initialization functions to configure various peripherals.
     * This could involve initializing the microcontroller, setting up port configurations,
     * configuring digital I/O, enabling interrupts for buttons, and initializing the ADC
     * for temperature readings, as well as setting up UART for communication.
     */
    Mcu_Init(); /* Initialize the microcontroller settings */
    Port_Init(&Port_Configuration); /* Initialize GPIO ports according to configuration */
    Dio_Init(&Dio_Configuration); /* Initialize digital I/O settings */
    GPIO_SetupButtonsInterrupt(); /* Configure interrupt handling for button inputs */
    ADC_Init(); /* Initialize ADC for temperature sensor readings */
    UART0_Init(); /* Initialize UART0 for serial communication */
    GPTM_WTimer0Init(); /* Initialize Timer0 for timing process */
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to periodically process sensor data for driver temperature readings.
 * This task retrieves the temperature values from LM35 sensor at regular intervals.
 * - The driver's temperature is read from SENSOR0_CHANNEL_ID.
 * The task runs indefinitely with a delay between each iteration.
 */
void vDriverSensorProcessTask(void *pvParameters)
{
    TickType_t xSensorLastWakeTime = xTaskGetTickCount(); /* Initialize the variable for precise periodic delays */

    for (;;)
    {
        /*
         * Retrieve the current temperature for the driver by reading from LM35 sensor
         * connected to SENSOR0_CHANNEL_ID.
         * The value is stored in ucDriverTemperatureValue.
         */
        xSemaphoreTake(xDriverTempValueMutex, portMAX_DELAY);

        ucDriverTemperatureValue = LM35_getTemperature(SENSOR0_CHANNEL_ID);

        xSemaphoreGive(xDriverTempValueMutex);

        /*
         * Check if the driver's temperature is outside the acceptable range (5�C to 40�C).
         * If the temperature exceeds 40�C or falls below 5�C, it indicates a potential fault
         * in the temperature reading or an abnormal condition.
         */
        if (ucDriverTemperatureValue > mainTEMP_MAX_VALID_RANGE || ucDriverTemperatureValue < mainTEMP_MIN_VALID_RANGE)
        {
            /* If the condition is met, the semaphore xDriverErrorReportSemaphore is given to signal
             * that an error condition has occurred for the driver's temperature, allowing error task
             * waiting on this semaphore to take appropriate action.
             * */
            if (!ucDriverErrorFlag)
            {
                xFailureLog xlog; /* Create a log for driver failure data */

                /* Capture the current system timestamp */
                xlog.uiTimeStamp = GPTM_WTimer0Read();

                /* Log the current driver heating level */
                xlog.ucHeatingLevel = ucDriverHeatingLevel;

                /* Determine if the driver temperature is out of valid range and set failure code */
                if (ucDriverTemperatureValue > mainTEMP_MAX_VALID_RANGE)
                {
                    xlog.ucFailureCode = mainTEMP_OVER_RANGE_FAIL; /* Overheating detected */
                }
                else if (ucDriverTemperatureValue < mainTEMP_MIN_VALID_RANGE)
                {
                    xlog.ucFailureCode = mainTEMP_UNDER_RANGE_FAIL; /* Temperature too low */
                }

                /* Mark the failure as related to the driver's seat */
                xlog.ucFailureSeat = mainDRIVER_SEAT_FAIL;

                /* Set error flag for the driver */
                ucDriverErrorFlag = pdTRUE;

                /* Send the failure log to the driver diagnostic queue */
                xQueueSend(xDriverDiagnosticQueue, &xlog, portMAX_DELAY);

                /* Signal the error reporting task via semaphore */
                xSemaphoreGive(xDriverErrorReportSemaphore);

            }
        }
        else if (ucDriverErrorFlag)
        {
            ucDriverErrorFlag = pdFALSE;
            Led_RED1_SetOff();
        }

        /*
         * Delay until 100ms has passed since the task's last execution to ensure consistent periodic timing.
         */
        vTaskDelayUntil(&xSensorLastWakeTime, mainSENSOR_TASK_DELAY); /* 100ms delay */
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to periodically process sensor data for passenger temperature readings.
 * This task retrieves the temperature values from LM35 sensor at regular intervals.
 * - The passenger's temperature is read from SENSOR1_CHANNEL_ID.
 * The task runs indefinitely with a delay between each iteration.
 */
void vPassengerSensorsProcessTask(void *pvParameters)
{
    TickType_t xSensorLastWakeTime = xTaskGetTickCount(); /* Initialize the variable for precise periodic delays */

    for (;;)
    {
        /*
         * Retrieve the current temperature for the passenger by reading from LM35 sensor
         * connected to SENSOR1_CHANNEL_ID.
         * The value is stored in ucDriverTemperatureValue.
         */
        xSemaphoreTake(xPassengerTempValueMutex, portMAX_DELAY);

        ucPassengerTemperatureValue = LM35_getTemperature(SENSOR1_CHANNEL_ID);

        xSemaphoreGive(xPassengerTempValueMutex);
        /*
         * Check if the passenger's temperature is outside the acceptable range (5�C to 40�C).
         * Similar to the driver check, if the temperature exceeds 40�C or falls below 5�C,
         * it indicates a fault or abnormal condition for the passenger's temperature reading.
         */
        if (ucPassengerTemperatureValue > mainTEMP_MAX_VALID_RANGE || ucPassengerTemperatureValue < mainTEMP_MIN_VALID_RANGE)
        {
            /* If the condition is met, the semaphore xPassengerErrorReportSemaphore is given,
             * signaling an error condition for the passenger's temperature. This allows error task
             * that are waiting on this semaphore to respond appropriately.
             * */
            if (!ucPassengerErrorFlag)
            {
                xFailureLog xlog; /* Create a log to store failure information */

                /* Capture the current system time for diagnostics */
                xlog.uiTimeStamp = GPTM_WTimer0Read();

                /* Record the current heating level for the passenger */
                xlog.ucHeatingLevel = ucPassengerHeatingLevel;

                /* Check if the passenger's temperature is out of range and set failure code */
                if (ucPassengerTemperatureValue > mainTEMP_MAX_VALID_RANGE)
                {
                    xlog.ucFailureCode = mainTEMP_OVER_RANGE_FAIL; /* Overheating detected */
                }
                else if (ucPassengerTemperatureValue < mainTEMP_MIN_VALID_RANGE)
                {
                    xlog.ucFailureCode = mainTEMP_UNDER_RANGE_FAIL; /* Temperature too low */
                }

                /* Identify the failure as coming from the passenger seat */
                xlog.ucFailureSeat = mainPASSENGER_SEAT_FAIL;

                /* Set error flag for passenger */
                ucPassengerErrorFlag = pdTRUE;

                /* Send the failure log to the diagnostic queue */
                xQueueSend(xPassengerDiagnosticQueue, &xlog, portMAX_DELAY);

                /* Notify the error reporting task via semaphore */
                xSemaphoreGive(xPassengerErrorReportSemaphore);

            }
        }
        else if (ucPassengerErrorFlag)
        {
            ucPassengerErrorFlag = pdFALSE;
            Led_RED2_SetOff();
        }

        /*
         * Delay until 100ms has passed since the task's last execution to ensure consistent periodic timing.
         */
        vTaskDelayUntil(&xSensorLastWakeTime, mainSENSOR_TASK_DELAY); /* 100ms delay */
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to periodically process button inputs.
 * This function waits for specific event bits to be set in an event group,
 * which are triggered by external interrupts (button presses).
 * Based on the event bits, the function adjusts the desired temperature
 * settings for driver and based on their respective heating levels.
 */
void vDriverButtonsProcessTask(void *pvParameters)
{
    /* Variable to store the event bits that are set when an interrupt occurs */
    EventBits_t xDriverButtonsEventGroupValue;

    /* Define which event bits we are waiting for (button interrupts) */
    const EventBits_t xBitsToWaitFor = (mainSW1_INTERRUPT_BIT | mainSW3_INTERRUPT_BIT);

    for (;;)
    {
        /*
         * Wait for any of the specified event bits to be set in the event group.
         * - xEventGroupWaitBits() will block until one or more of the bits in xBitsToWaitFor is set.
         * - pdTRUE: Clear the event bits before returning.
         * - pdFALSE: Wait for any bit to be set, not necessarily all bits.
         * - portMAX_DELAY: Block indefinitely (no timeout).
         */
        xDriverButtonsEventGroupValue = xEventGroupWaitBits(xDriverButtonsEventGroup, /* The event group being waited on */
                                                            xBitsToWaitFor, /* The bits to wait for */
                                                            pdTRUE, /* Clear the bits after they are read */
                                                            pdFALSE, /* Wait for any one of the bits */
                                                            portMAX_DELAY); /* No timeout */

        /*
         * Check if interrupts related to SW1 (PF4) or SW3 (PB0) have occurred.
         * These interrupts will trigger adjustments to the driver's desired temperature.
         */
        if (((xDriverButtonsEventGroupValue & mainSW1_INTERRUPT_BIT) != 0) || ((xDriverButtonsEventGroupValue & mainSW3_INTERRUPT_BIT) != 0))
        {
            /*
             * Adjust the driver's desired temperature based on the current heating level.
             * The desired temperature is set to a predefined value depending on the heating level:
             *  - Level 0: Heating feature is off
             *  - Level 1: Low heating (25�C)
             *  - Level 2: Medium heating (30�C)
             *  - Level 3: High heating (35�C)
             */
            xSemaphoreTake(xDriverDesiredTempMutex, portMAX_DELAY);

            if (ucDriverHeatingLevel == mainHEATING_LEVEL_OFF)
            {
                ucDriverDesiredTemperature = mainDESIRED_TEMP_OFF; /* Heating feature is off */
            }
            else if (ucDriverHeatingLevel == mainHEATING_LEVEL_LOW)
            {
                ucDriverDesiredTemperature = mainDESIRED_TEMP_LOW; /* Low heating: 25�C */
            }
            else if (ucDriverHeatingLevel == mainHEATING_LEVEL_MEDIUM)
            {
                ucDriverDesiredTemperature = mainDESIRED_TEMP_MEDIUM; /* Medium heating: 30�C */
            }
            else if (ucDriverHeatingLevel == mainHEATING_LEVEL_HIGH)
            {
                ucDriverDesiredTemperature = mainDESIRED_TEMP_HIGH; /* High heating: 35�C */
            }

            xSemaphoreGive(xDriverDesiredTempMutex);
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to periodically process button inputs.
 * This function waits for specific event bits to be set in an event group,
 * which are triggered by external interrupts (button presses).
 * Based on the event bits, the function adjusts the desired temperature
 * settings for passenger based on their respective heating levels.
 */
void vPassengerButtonProcessTask(void *pvParameters)
{
    /* Variable to store the event bits that are set when an interrupt occurs */
    EventBits_t xPassengerButtonEventGroupValue;

    /* Define which event bits we are waiting for (button interrupts) */
    const EventBits_t xBitsToWaitFor = (mainSW2_INTERRUPT_BIT);

    for (;;)
    {
        /*
         * Wait for any of the specified event bits to be set in the event group.
         * - xEventGroupWaitBits() will block until one or more of the bits in xBitsToWaitFor is set.
         * - pdTRUE: Clear the event bits before returning.
         * - pdFALSE: Wait for any bit to be set, not necessarily all bits.
         * - portMAX_DELAY: Block indefinitely (no timeout).
         */
        xPassengerButtonEventGroupValue = xEventGroupWaitBits(xPassengerButtonEventGroup, /* The event group being waited on */
                                                              xBitsToWaitFor, /* The bits to wait for */
                                                              pdTRUE, /* Clear the bits after they are read */
                                                              pdFALSE, /* Wait for any one of the bits */
                                                              portMAX_DELAY); /* No timeout */

        /*
         * Check if the interrupt related to SW2 (PF0) has occurred.
         * This interrupt will trigger an adjustment to the passenger's desired temperature.
         */
        if ((xPassengerButtonEventGroupValue & mainSW2_INTERRUPT_BIT) != 0)
        {
            /*
             * Adjust the passenger's desired temperature based on the current heating level.
             * Similar to the driver, the passenger's desired temperature is set according to the heating level:
             *  - Level 0: Heating feature is off
             *  - Level 1: Low heating (25�C)
             *  - Level 2: Medium heating (30�C)
             *  - Level 3: High heating (35�C)
             */
            xSemaphoreTake(xPassengerDesiredTempMutex, portMAX_DELAY);

            if (ucPassengerHeatingLevel == mainHEATING_LEVEL_OFF)
            {
                ucPassengerDesiredTemperature = mainDESIRED_TEMP_OFF; /* Heating feature is off */
            }
            else if (ucPassengerHeatingLevel == mainHEATING_LEVEL_LOW)
            {
                ucPassengerDesiredTemperature = mainDESIRED_TEMP_LOW; /* Low heating: 25�C */
            }
            else if (ucPassengerHeatingLevel == mainHEATING_LEVEL_MEDIUM)
            {
                ucPassengerDesiredTemperature = mainDESIRED_TEMP_MEDIUM; /* Medium heating: 30�C */
            }
            else if (ucPassengerHeatingLevel == mainHEATING_LEVEL_HIGH)
            {
                ucPassengerDesiredTemperature = mainDESIRED_TEMP_HIGH; /* High heating: 35�C */
            }

            xSemaphoreGive(xPassengerDesiredTempMutex);
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Optimized Task function to periodically control and passenger heating systems.
 * This task manages the heater's intensity based on the difference between the current and desired temperatures.
 * The heating intensity is simulated using LEDs.
 * - Low intensity: Green LED
 * - Medium intensity: Blue LED
 * - High intensity: Both Green and Blue LEDs (Cyan)
 * If the current temperature exceeds the desired temperature, the heater is disabled.
 */
void vDriverHeaterProcessTask(void *pvParameters)
{
    /* Get the current tick count to set up a periodic delay for the task */
    TickType_t xDriverHeaterLastWakeTime = xTaskGetTickCount();

    /* Variables to track the previous heater states for driver
     * This helps to avoid unnecessary updates to the LEDs if the state doesn't change
     */
    uint8 ucPrevDriverHeaterState = 0xFF;

    for (;;)
    {
        /* ------------- Process Driver Heating ------------- */
        xSemaphoreTake(xDriverDesiredTempMutex, portMAX_DELAY);
        xSemaphoreTake(xDriverHeaterStateMutex, portMAX_DELAY);
        xSemaphoreTake(xDriverHeatingLevelMutex, portMAX_DELAY);
        xSemaphoreTake(xDriverTempValueMutex, portMAX_DELAY);

        /* Check if driver heating is enabled (heating level != 0) */
        if ((ucDriverHeatingLevel != mainHEATING_LEVEL_OFF) && (ucDriverErrorFlag == pdFALSE))
        {
            /* Check if the desired temperature is greater than or equal to the current temperature */
            if (ucDriverDesiredTemperature >= ucDriverTemperatureValue)
            {
                /* Calculate the temperature difference */
                uint8 driverTempDiff = ucDriverDesiredTemperature - ucDriverTemperatureValue;

                /* Adjust the heater state based on the temperature difference */
                if (driverTempDiff >= mainTEMP_DIFF_LOW_THRESHOLD && driverTempDiff < mainTEMP_DIFF_MEDIUM_THRESHOLD)
                {
                    /* Small difference, set the heater to low */
                    ucDriverHeaterState = mainHEATER_STATE_LOW;
                }
                else if (driverTempDiff >= mainTEMP_DIFF_MEDIUM_THRESHOLD && driverTempDiff < mainTEMP_DIFF_HIGH_THRESHOLD)
                {
                    /* Moderate difference, set the heater to medium */
                    ucDriverHeaterState = mainHEATER_STATE_MEDIUM;
                }
                else if (driverTempDiff >= mainTEMP_DIFF_HIGH_THRESHOLD)
                {
                    /* Large difference, set the heater to high */
                    ucDriverHeaterState = mainHEATER_STATE_HIGH;
                }
            }
            else
            {
                /* If the desired temperature is less than the current, turn off the heater */
                ucDriverHeaterState = mainHEATER_STATE_OFF;
            }
        }
        else
        {
            /* If heating level is 0, ensure the heater is off */
            ucDriverHeaterState = mainHEATER_STATE_OFF;
        }

        /* Update the driver's heater LEDs only if the heater state has changed */
        if ((ucPrevDriverHeaterState != ucDriverHeaterState) || (ucDriverErrorFlag == pdTRUE))
        {
            /* Store the new state to prevent redundant updates */
            ucPrevDriverHeaterState = ucDriverHeaterState;

            /* Control the LEDs based on the current heater state */
            switch (ucDriverHeaterState)
            {
            case mainHEATER_STATE_LOW:
                /* Low heat - turn on the green LED, turn off the blue LED */
                Led_GREEN1_SetOn();
                Led_BLUE1_SetOff();
                break;
            case mainHEATER_STATE_MEDIUM:
                /* Medium heat - turn off the green LED, turn on the blue LED */
                Led_GREEN1_SetOff();
                Led_BLUE1_SetOn();
                break;
            case mainHEATER_STATE_HIGH:
                /* High heat - turn on both green and blue LEDs */
                Led_GREEN1_SetOn();
                Led_BLUE1_SetOn();
                break;
            case mainHEATER_STATE_OFF:
            default:
                /* Heater off - turn off both LEDs */
                Led_GREEN1_SetOff();
                Led_BLUE1_SetOff();
                break;
            }
        }

        xSemaphoreGive(xDriverTempValueMutex);
        xSemaphoreGive(xDriverHeatingLevelMutex);
        xSemaphoreGive(xDriverHeaterStateMutex);
        xSemaphoreGive(xDriverDesiredTempMutex);

        /* Delay the task for a period of 250ms to achieve periodic execution */
        vTaskDelayUntil(&xDriverHeaterLastWakeTime, mainHEATER_TASK_DELAY);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Optimized Task function to periodically control passenger heating systems.
 * This task manages the heater's intensity based on the difference between the current and desired temperatures.
 * The heating intensity is simulated using LEDs.
 * - Low intensity: Green LED
 * - Medium intensity: Blue LED
 * - High intensity: Both Green and Blue LEDs (Cyan)
 * If the current temperature exceeds the desired temperature, the heater is disabled.
 */
void vPassengerHeatersProcessTask(void *pvParameters)
{
    /* Get the current tick count to set up a periodic delay for the task */
    TickType_t xPassengerHeaterLastWakeTime = xTaskGetTickCount();

    /* Variables to track the previous heater states for passenger
     * This helps to avoid unnecessary updates to the LEDs if the state doesn't change
     */
    uint8 ucPrevPassengerHeaterState = 0xFF;

    for (;;)
    {
        /* ------------- Process Passenger Heating ------------- */
        xSemaphoreTake(xPassengerDesiredTempMutex, portMAX_DELAY);
        xSemaphoreTake(xPassengerHeaterStateMutex, portMAX_DELAY);
        xSemaphoreTake(xPassengerHeatingLevelMutex, portMAX_DELAY);
        xSemaphoreTake(xPassengerTempValueMutex, portMAX_DELAY);
        /* Check if passenger heating is enabled (heating level != 0) */
        if ((ucPassengerHeatingLevel != mainHEATING_LEVEL_OFF) && (ucPassengerErrorFlag == pdFALSE))
        {
            /* Check if the desired temperature is greater than or equal to the current temperature */
            if (ucPassengerDesiredTemperature >= ucPassengerTemperatureValue)
            {
                /* Calculate the temperature difference */
                uint8 passengerTempDiff = ucPassengerDesiredTemperature - ucPassengerTemperatureValue;

                /* Adjust the heater state based on the temperature difference */
                if (passengerTempDiff >= mainTEMP_DIFF_LOW_THRESHOLD && passengerTempDiff < mainTEMP_DIFF_MEDIUM_THRESHOLD)
                {
                    /* Small difference, set the heater to low */
                    ucPassengerHeaterState = mainHEATER_STATE_LOW;
                }
                else if (passengerTempDiff >= mainTEMP_DIFF_MEDIUM_THRESHOLD && passengerTempDiff < mainTEMP_DIFF_HIGH_THRESHOLD)
                {
                    /* Moderate difference, set the heater to medium */
                    ucPassengerHeaterState = mainHEATER_STATE_MEDIUM;
                }
                else if (passengerTempDiff >= mainTEMP_DIFF_HIGH_THRESHOLD)
                {
                    /* Large difference, set the heater to high */
                    ucPassengerHeaterState = mainHEATER_STATE_HIGH;
                }
            }
            else
            {
                /* If the desired temperature is less than the current, turn off the heater */
                ucPassengerHeaterState = mainHEATER_STATE_OFF;
            }
        }
        else
        {
            /* If heating level is 0, ensure the heater is off */
            ucPassengerHeaterState = mainHEATER_STATE_OFF;
        }

        /* Update the passenger's heater LEDs only if the heater state has changed */
        if ((ucPrevPassengerHeaterState != ucPassengerHeaterState) || (ucPassengerErrorFlag == pdTRUE))
        {
            /* Store the new state to prevent redundant updates */
            ucPrevPassengerHeaterState = ucPassengerHeaterState;

            /* Control the LEDs based on the current heater state */
            switch (ucPassengerHeaterState)
            {
            case mainHEATER_STATE_LOW:
                /* Low heat - turn on the green LED, turn off the blue LED */
                Led_GREEN2_SetOn();
                Led_BLUE2_SetOff();
                break;
            case mainHEATER_STATE_MEDIUM:
                /* Medium heat - turn off the green LED, turn on the blue LED */
                Led_GREEN2_SetOff();
                Led_BLUE2_SetOn();
                break;
            case mainHEATER_STATE_HIGH:
                /* High heat - turn on both green and blue LEDs */
                Led_GREEN2_SetOn();
                Led_BLUE2_SetOn();
                break;
            case mainHEATER_STATE_OFF:
            default:
                /* Heater off - turn off both LEDs */
                Led_GREEN2_SetOff();
                Led_BLUE2_SetOff();
                break;
            }
        }

        xSemaphoreGive(xPassengerTempValueMutex);
        xSemaphoreGive(xPassengerHeatingLevelMutex);
        xSemaphoreGive(xPassengerHeaterStateMutex);
        xSemaphoreGive(xPassengerDesiredTempMutex);

        /* Delay the task for a period of 250ms to achieve periodic execution */
        vTaskDelayUntil(&xPassengerHeaterLastWakeTime, mainHEATER_TASK_DELAY);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to handle error reporting for driver temperature sensors.
 * This task monitors error conditions based on semaphores signaling temperature faults.
 * - When an error is detected for the driver, it disables the driver's heater and turns on a red LED.
 */
void vDriverDiagnosticTask(void *pvParameters)
{
    xFailureLog xlog; /* Structure to hold diagnostic information */

    for (;;)
    {
        /*
         * Block indefinitely until the xDriverErrorReportSemaphore is given by another task,
         * which signals that the driver's temperature sensor has detected a fault (i.e., temperature
         * outside of the acceptable range defined in the system).
         */
        xSemaphoreTake(xDriverErrorReportSemaphore, portMAX_DELAY);

        /*
         * Upon receiving the semaphore, immediately disable the driver's heater to ensure
         * that the heating element does not remain active under potentially hazardous conditions.
         * This ensures the safety of the system in case of temperature sensor faults.
         */
        xSemaphoreTake(xDriverHeaterStateMutex, portMAX_DELAY);

        ucDriverHeaterState = mainHEATER_STATE_OFF;

        xSemaphoreGive(xDriverHeaterStateMutex);

        /*
         * Retrieve diagnostic data from the xDriverDiagnosticQueue to log details about the error.
         * The queue contains information about the seat where the failure occurred, a failure code,
         * a timestamp of the error event, and the heating level at the time of the failure.
         */
        if (xQueueReceive(xDriverDiagnosticQueue, &xlog, portMAX_DELAY))
        {
            /* Store the diagnostic data in the uiDiagnosticArray for future reference and analysis. */
            xDiagnosticArray[ucDiagnosticIndex].ucFailureSeat = xlog.ucFailureSeat; /* Seat number */
            xDiagnosticArray[ucDiagnosticIndex].ucFailureCode = xlog.ucFailureCode; /* Failure code */
            xDiagnosticArray[ucDiagnosticIndex].uiTimeStamp = xlog.uiTimeStamp; /* Timestamp */
            xDiagnosticArray[ucDiagnosticIndex].ucHeatingLevel = xlog.ucHeatingLevel; /* Heating level */
            ucDiagnosticIndex++; /* Increment the diagnostic index for the next entry */
        }

        /*
         * Turn on the driver's red LED to indicate that an error has occurred in the temperature control system.
         * This visual indication warns the driver that their heater is no longer active due to a sensor fault,
         * alerting them to the issue so it can be addressed.
         */
        Led_RED1_SetOn();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to handle error reporting for passenger temperature sensors.
 * This task monitors error conditions based on semaphores signaling temperature faults.
 * - When an error is detected for the passenger, it disables the passenger's heater and turns on a separate red LED.
 */
void vPassengerDiagnosticTask(void *pvParameters)
{
    xFailureLog xlog; /* Structure to store diagnostic information */

    for (;;)
    {
        /*
         * Wait indefinitely (blocking) for the xPassengerErrorReportSemaphore to become available.
         * This semaphore is given by another task when the passenger's temperature sensor detects a fault
         * (e.g., temperature outside the valid range). The task will not proceed until this event occurs.
         */
        xSemaphoreTake(xPassengerErrorReportSemaphore, portMAX_DELAY);

        /*
         * Once the semaphore is taken, the system reads diagnostic data from the xPassengerDiagnosticQueue.
         * The queue contains information like the type of failure (seat and code), the timestamp,
         * and the last heater level before the fault occurred.
         */
        if (xQueueReceive(xPassengerDiagnosticQueue, &xlog, portMAX_DELAY))
        {
            /* Store the diagnostic data in the uiDiagnosticArray for future reference and analysis. */
            xDiagnosticArray[ucDiagnosticIndex].ucFailureSeat = xlog.ucFailureSeat; /* Seat number */
            xDiagnosticArray[ucDiagnosticIndex].ucFailureCode = xlog.ucFailureCode; /* Failure code */
            xDiagnosticArray[ucDiagnosticIndex].uiTimeStamp = xlog.uiTimeStamp; /* Timestamp */
            xDiagnosticArray[ucDiagnosticIndex].ucHeatingLevel = xlog.ucHeatingLevel; /* Heating level */
            ucDiagnosticIndex++; /* Increment the diagnostic index for the next entry */
        }

        /*
         * Disable the passenger's heater to ensure that no heating continues while the system is
         * in an error state, preventing any potential safety risks.
         */
        xSemaphoreTake(xPassengerHeaterStateMutex, portMAX_DELAY);

        ucPassengerHeaterState = mainHEATER_STATE_OFF;

        xSemaphoreGive(xPassengerHeaterStateMutex);
        /*
         * Turn on the red LED to indicate a fault in the passenger's heating system. This visual
         * notification signals that the passenger heating is deactivated due to an error,
         * prompting the user to check or fix the issue.
         */
        Led_RED2_SetOn();
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to periodically update and display system status on the UART screen.
 * The displayed information includes desired and current temperatures for both driver and passenger,
 * as well as the current state of the heater (Off, Low, Medium, High).
 * This task uses a mutex (xDisplayScreenMutex) to ensure exclusive access to the UART for sending data.
 */
void vDisplayScreenTask(void *pvParameters)
{
    TickType_t xDisplayLastWakeTime = xTaskGetTickCount(); /* Initialize for periodic delay */
    uint8 ucPrevDriverHeaterState = 0xFF, ucPrevPassengerHeaterState = 0xFF;
    uint8 ucPrevDriverTemp = 0xFF, ucPrevPassengerTemp = 0xFF;
    uint8 ucPrevDriverHeatingLevel = 0xFF, ucPrevPassengerHeatingLevel = 0xFF;

    for (;;)
    {
        /* Check if any data has changed before sending updates */
        if ((ucDriverHeatingLevel != ucPrevDriverHeatingLevel) || (ucDriverTemperatureValue != ucPrevDriverTemp) || (ucDriverHeaterState != ucPrevDriverHeaterState)
                || (ucPassengerHeatingLevel != ucPrevPassengerHeatingLevel) || (ucPassengerTemperatureValue != ucPrevPassengerTemp) || (ucPassengerHeaterState != ucPrevPassengerHeaterState))
        {
            /* Attempt to take the mutex for screen display to ensure exclusive access to the UART. */
            xSemaphoreTake(xDisplayScreenMutex, portMAX_DELAY);

            /* Display the header line for separating display updates. */
            UART0_SendString("------------------------------------------------------------\r\n");

            /* Display the driver's current temperature. */
            UART0_SendString("Driver Current Temperature: ");
            UART0_SendInteger(ucDriverTemperatureValue); /* Send the temperature value. */
            UART0_SendString("�C\r\n");

            /* Display the driver's heating level. */
            UART0_SendString("Driver Heating Level: ");
            switch (ucDriverHeatingLevel)
            {
            case mainHEATING_LEVEL_OFF:
                UART0_SendString("Off");
                break;
            case mainHEATING_LEVEL_LOW:
                UART0_SendString("Low");
                break;
            case mainHEATING_LEVEL_MEDIUM:
                UART0_SendString("Medium");
                break;
            case mainHEATING_LEVEL_HIGH:
                UART0_SendString("High");
                break;
            } /* Send the heating level. */
            UART0_SendString("\r\n");

            /* Display the driver's heater state. */
            UART0_SendString("Driver Heater State: ");
            switch (ucDriverHeaterState)
            {
            case mainHEATER_STATE_OFF: /* Heater is off */
                UART0_SendString("Off");
                break;
            case mainHEATER_STATE_LOW: /* Heater is set to low intensity */
                UART0_SendString("Low");
                break;
            case mainHEATER_STATE_MEDIUM: /* Heater is set to medium intensity */
                UART0_SendString("Medium");
                break;
            case mainHEATER_STATE_HIGH: /* Heater is set to high intensity */
                UART0_SendString("High");
                break;
            }

            UART0_SendString("\r\n\r\n"); /* Double newline to separate driver and passenger sections */

            /* Display the passenger's current temperature. */
            UART0_SendString("Passenger Current Temperature: ");
            UART0_SendInteger(ucPassengerTemperatureValue); /* Send the temperature value. */
            UART0_SendString("�C\r\n");

            /* Display the passenger's heating level. */
            UART0_SendString("Passenger Heating Level: ");
            switch (ucPassengerHeatingLevel)
            {
            case mainHEATING_LEVEL_OFF:
                UART0_SendString("Off");
                break;
            case mainHEATING_LEVEL_LOW:
                UART0_SendString("Low");
                break;
            case mainHEATING_LEVEL_MEDIUM:
                UART0_SendString("Medium");
                break;
            case mainHEATING_LEVEL_HIGH:
                UART0_SendString("High");
                break;
            } /* Send the heating level. */
            UART0_SendString("\r\n");

            /* Display the passenger's heater state. */
            UART0_SendString("Passenger Heater State: ");
            switch (ucPassengerHeaterState)
            {
            case mainHEATER_STATE_OFF: /* Heater is off */
                UART0_SendString("Off");
                break;
            case mainHEATER_STATE_LOW: /* Heater is set to low intensity */
                UART0_SendString("Low");
                break;
            case mainHEATER_STATE_MEDIUM: /* Heater is set to medium intensity */
                UART0_SendString("Medium");
                break;
            case mainHEATER_STATE_HIGH: /* Heater is set to high intensity */
                UART0_SendString("High");
                break;
            }
            UART0_SendString("\r\n"); /* End of the display update */

            /* Release the mutex to allow other tasks to access the UART. */
            xSemaphoreGive(xDisplayScreenMutex);

            /* Save current states to detect future changes */
            ucPrevDriverHeatingLevel = ucDriverHeatingLevel;
            ucPrevDriverTemp = ucDriverTemperatureValue;
            ucPrevDriverHeaterState = ucDriverHeaterState;
            ucPrevPassengerHeatingLevel = ucPassengerHeatingLevel;
            ucPrevPassengerTemp = ucPassengerTemperatureValue;
            ucPrevPassengerHeaterState = ucPassengerHeaterState;
        }

        /* Delay for 500ms before checking for updates again */
        vTaskDelayUntil(&xDisplayLastWakeTime, mainDISPLAY_TASK_DELAY);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * Task function to measure and report CPU load in real-time.
 * The task calculates the CPU load by comparing the total time spent in all tasks
 * to the total runtime of the system, and periodically reports the result via UART.
 * This task runs indefinitely with a delay between each measurement.
 */
void vRunTimeMeasurementsTask(void *pvParameters)
{
    TickType_t xRunTimeLastWakeTime = xTaskGetTickCount(); /* Capture the current tick count for accurate periodic delay. */

    for (;;)
    {
        uint8 ucCounter; /* Variable to store task index. */
        uint32 ullTotalTasksTime = 0; /* Variable to accumulate the total runtime for all tasks. */

        /* Delay to maintain consistent runtime measurements. */
        vTaskDelayUntil(&xRunTimeLastWakeTime, mainRUNTIME_TASK_DELAY);

        /* Attempt to take the mutex for screen display to ensure exclusive access to the UART. */
        xSemaphoreTake(xDisplayScreenMutex, portMAX_DELAY);

        /* Display the header line for separating display updates. */
        UART0_SendString("------------------------------------------------------------\r\n");

        /* Sum the total execution time for tasks (excluding Idle Task) */
        for (ucCounter = 1; ucCounter <= mainTOTAL_NUMBER_OF_TASKS; ucCounter++)
        {
            ullTotalTasksTime += ullTasksTotalTime[ucCounter];
        }

        /*
         * Calculate CPU load as a percentage of the total tasks runtime compared to the system's total elapsed time
         * since startup, as measured by the wide timer.
         */
        uint32 ulCurrentSystemTime = GPTM_WTimer0Read(); // Get the current system time for CPU load calculation
        uint8 ucCPU_Load = (ullTotalTasksTime * 100) / ulCurrentSystemTime;

        UART0_SendString("\r\nCPU Load is ");
        UART0_SendInteger(ucCPU_Load);
        UART0_SendString("% \r\n");

        /* Release the mutex to allow other tasks to access the UART. */
        xSemaphoreGive(xDisplayScreenMutex);
    }
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * ISR for handling interrupts from Port F.
 * This handler manages button presses for PF0 and PF4, which control the passenger and driver heating levels respectively.
 * When either button is pressed, the heating level is cycled through predefined settings, and an event group bit is set to notify tasks.
 */
void GPIO_PORTF_Handler(void)
{
    /* Variable to indicate if a higher priority task was woken by the event group operation */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Check if PF0 (SW2 button) triggered the interrupt */
    if (GPIO_PORTF_RIS_REG & PF0)
    {
        /*
         * Increment the passenger heating level control.
         * The heating levels are cycled through:
         *  - 0: Heating feature is off
         *  - 1: Low heating (25�C)
         *  - 2: Medium heating (30�C)
         *  - 3: High heating (35�C)
         * Modulo 4 is used to wrap back to 0 after reaching level 3.
         */
        ucPassengerHeatingLevel = (ucPassengerHeatingLevel + 1) % mainTOTAL_HEATING_LEVELS;

        /*
         * Set the event group bit associated with SW2.
         * This will notify the task waiting on this event that the button was pressed.
         */
        xEventGroupSetBitsFromISR(xPassengerButtonEventGroup, mainSW2_INTERRUPT_BIT, &xHigherPriorityTaskWoken);

        /* Clear the interrupt flag for PF0 to acknowledge that the interrupt has been handled */
        GPIO_PORTF_ICR_REG |= PF0;
    }

    /* Check if PF4 (SW1 button) triggered the interrupt */
    if (GPIO_PORTF_RIS_REG & PF4)
    {
        /*
         * Increment the driver heating level control.
         * The heating levels are cycled through the same steps as the passenger heating:
         *  - 0: Heating feature is off
         *  - 1: Low heating (25�C)
         *  - 2: Medium heating (30�C)
         *  - 3: High heating (35�C)
         * Modulo 4 is used to reset to 0 after reaching level 3.
         */
        ucDriverHeatingLevel = (ucDriverHeatingLevel + 1) % mainTOTAL_HEATING_LEVELS;

        /*
         * Set the event group bit associated with SW1.
         * This will notify the task waiting on this event that the button was pressed.
         */
        xEventGroupSetBitsFromISR(xDriverButtonsEventGroup, mainSW1_INTERRUPT_BIT, &xHigherPriorityTaskWoken);

        /* Clear the interrupt flag for PF4 to acknowledge that the interrupt has been handled */
        GPIO_PORTF_ICR_REG |= PF4;
    }

    /*
     * If setting the event group bit caused a higher priority task to be woken, yield to that task.
     * portYIELD_FROM_ISR() ensures the FreeRTOS scheduler switches context if needed.
     */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

/*
 * ISR for handling interrupts from Port B.
 * This handler manages the button press for PB0, which controls the driver heating level.
 * When the button is pressed, the heating level is cycled, and an event group bit is set to notify tasks.
 */
void GPIO_PORTB_Handler(void)
{
    /* Variable to indicate if a higher priority task was woken by the event group operation */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Check if PB0 (SW3 button) triggered the interrupt */
    if (GPIO_PORTB_RIS_REG & PB1)
    {
        /*
         * Increment the driver heating level control.
         * The heating levels are cycled through:
         *  - 0: Heating feature is off
         *  - 1: Low heating (25�C)
         *  - 2: Medium heating (30�C)
         *  - 3: High heating (35�C)
         * Modulo 4 is used to cycle the levels back to 0 after reaching 3.
         */
        ucDriverHeatingLevel = (ucDriverHeatingLevel + 1) % mainTOTAL_HEATING_LEVELS;

        /*
         * Set the event group bit associated with SW3.
         * This will notify the task waiting on this event that the button was pressed.
         */
        xEventGroupSetBitsFromISR(xDriverButtonsEventGroup, mainSW3_INTERRUPT_BIT, &xHigherPriorityTaskWoken);

        /* Clear the interrupt flag for PB0 to acknowledge that the interrupt has been handled */
        GPIO_PORTB_ICR_REG |= PB1;
    }

    /*
     * Yield to a higher priority task if setting the event group bit caused it to be woken.
     * This ensures that FreeRTOS schedules the higher priority task to run.
     */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*-------------------------------------------------------------------------------------------------------------------------------------*/

