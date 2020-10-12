/* Microkernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "console.h"

/* Task prototypes */
static void prvTaskA(void *pvParameters);
static void prvTaskB(void *pvParameters);

/* Task Priorities */
#define prioTASK_A (tskIDLE_PRIORITY + 3) // Low Priority
#define prioTASK_B (tskIDLE_PRIORITY + 3) // High Priority

/* Some definitions */
#define TRUE 1
#define FALSE 0

#define isPeriodic FALSE

#if isPeriodic == TRUE
#define pTaskA pdMS_TO_TICKS(1000);
#define pTaskB pdMS_TO_TICKS(1500);
#endif

extern int taskTest();

int taskTest()
{
    console_init();

    /* Creating tasks */
    xTaskCreate(prvTaskA,                      /* Task Function */
                "Task A",                      /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10, /* Memory Stack */
                (void *)1,                     /* Used to pass a parameter to the task */
                prioTASK_A,                    /* Priority of Task */
                NULL);                         /* Microcontroller use a Task Handle (not used on Linux, so NULL) */

    xTaskCreate(prvTaskB,                      /* Task Function */
                "Task B",                      /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10, /* Memory Stack */
                (void *)2,                     /* Used to pass a parameter to the task */
                prioTASK_B,                    /* Priority of Task */
                NULL);                         /* Microcontroller use a Task Handle (not used on Linux, so NULL) */

    /* Initializing Scheduler */
    vTaskStartScheduler();
}

static void prvTaskA(void *pvParameters)
{

    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskA;
        xLastWakeTime = xTaskGetTickCount();
    #endif

    /* Checking parameter passed to  task */
    configASSERT(((uint32_t)pvParameters) == 1);

    for (;;)
    {

        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("Task A running... \n");
    }
}

static void prvTaskB(void *pvParameters)
{

    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskB;
        xLastWakeTime = xTaskGetTickCount();
    #endif

    /* Checking parameter passed to  task */
    configASSERT(((uint32_t)pvParameters) == 2);

    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("Task B running... \n");
    }
}