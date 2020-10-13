/*Libs to generate random includes*/
#include <stdio.h> 
#include <stdlib.h> 
#include<time.h> 

/* Microkernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "console.h"
#include "semphr.h"

/* Task prototypes */
static void prvHighPriorityTask(void *pvParameters);
static void prvLowPriorityTask(void *pvParameters);

/* Task Priorities */
#define prioHighPriorityTask (tskIDLE_PRIORITY + 4) // High Priority
#define prioLowPriorityTask  (tskIDLE_PRIORITY + 3) // Low Priority

/* Some definitions */
#define TRUE 1
#define FALSE 0
#define isPeriodic TRUE
#define pdTICKS_TO_MS( xTicks ) ( ( xTicks * 1000 ) / configTICK_RATE_HZ )

/* Some aux functions */
void randomString(char *s, const int len);

/* Define if tasks are periodic */
#if isPeriodic == TRUE
    #define pTaskA pdMS_TO_TICKS(1000);
    #define pTaskB pdMS_TO_TICKS(1500);
#endif

extern int mutexTest();

/* Handle for Tasks (optional for Linux) */
TaskHandle_t xHandleHighPriority = NULL;
TaskHandle_t xHandleLowPriority = NULL;

volatile uint8_t uiSharedInteger = 0;

int mutexTest()
{
    SemaphoreHandle_t xSemaphore;

    /* Initializing console */
    console_init();
    
    /* Seed random numbers */
    srand(time(0));

    /* Create a mutex type semaphore. */
    xSemaphore = xSemaphoreCreateMutex();
    
    if (xSemaphore == NULL)
    {
        console_print("Failed on create mutex (memory), the program has stopped. Ctrl + C to finish. \n"); 
        for (;;);
    } else
    {
        /* Setting name to mutex.  */
        vQueueAddToRegistry( xSemaphore, "Mutex-01" );
        console_print("Mutex created... \n");
    }
    
    /* Creating tasks */
    xTaskCreate(prvHighPriorityTask,                    /* Task Function */
                "HIGH_PRIORITY",                        /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,          /* Memory Stack */
                xSemaphore,                             /* Used to pass a parameter to the task */
                prioHighPriorityTask,                   /* Priority of Task */
                &xHandleHighPriority);                  /* Microcontroller use a Task Handle (optional on Linux) */

    xTaskCreate(prvLowPriorityTask,                     /* Task Function */
                "LOW_PRIORITY",                         /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,          /* Memory Stack */
                xSemaphore,                             /* Used to pass a parameter to the task */
                prioLowPriorityTask,                    /* Priority of Task */
                &xHandleLowPriority);                   /* Microcontroller use a Task Handle (optional on Linux) */

    console_print("Starting scheduling, use Ctrl + C on any moment to finish ... \n");

    /* Initializing Scheduler */
    vTaskStartScheduler();

    console_print("Failed to start the scheduler. Ctrl + C to finish. \n");
    for(;;);
}

static void prvHighPriorityTask(void *pvParameters)
{
    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskA;
        xLastWakeTime = xTaskGetTickCount();
    #endif

    /* Semaphore variables */
    SemaphoreHandle_t xSemaphore = NULL;
    xSemaphore = (SemaphoreHandle_t) pvParameters; // Restoring xSemaphore from higher context.
    
    /* Checking parameter passed to task */
    configASSERT(xSemaphore != NULL);

    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("\n\n");
        console_print("[HIGH PRIORITY] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));
        
        /* Hold Mutex */
        xSemaphoreTake(xSemaphore, portMAX_DELAY); 
            console_print("[HIGH PRIORITY] - Take xSemaphore \n");
            uiSharedInteger = rand() % 9;
            console_print("[HIGH PRIORITY] - Give xSemaphore \n\n");
        /*Give Mutex*/
        xSemaphoreGive(xSemaphore); 
    }
}

static void prvLowPriorityTask(void *pvParameters)
{

    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskB;
        xLastWakeTime = xTaskGetTickCount();
    #endif

    uint8_t uiloadLocalInteger = 0;

    /* Semaphore variables */
    SemaphoreHandle_t xSemaphore = NULL;
    xSemaphore = (SemaphoreHandle_t) pvParameters; // Restoring xSemaphore from higher context.
        
    /* Checking parameter passed to  task */
    configASSERT(xSemaphore != NULL);

    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("\n\n");
        console_print("[LOW PRIORITY] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));

        /* Hold Mutex */
        xSemaphoreTake(xSemaphore, portMAX_DELAY); 
            console_print("[LOW PRIORITY] - Take xSemaphore \n");
            uiloadLocalInteger = uiSharedInteger;
            console_print("[LOW PRIORITY] - Give xSemaphore \n\n");
        /*Give Mutex*/
        xSemaphoreGive(xSemaphore); 
    }
}
