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
#define prioNotifyTask   (tskIDLE_PRIORITY + 4) // High Priority
#define prioNotifiedTask  (tskIDLE_PRIORITY + 3) // Low Priority

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
#endif

extern int notificationTest();

/* Handle for Tasks (optional for Linux) */
TaskHandle_t xHandleNotifyTask = NULL;
TaskHandle_t xHandleNotifiedTask = NULL;

uint8_t uiSharedValue = 0x00;

int notificationTest()
{
    SemaphoreHandle_t xSemaphore;

    /* Initializing console */
    console_init();
    
    /* Seed random numbers */
    srand(time(0));
    
    /* Creating tasks */
    xTaskCreate(prvHighPriorityTask,                    /* Task Function */
                "NOTIFY_TASK",                          /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,          /* Memory Stack */
                NULL,                             /* Used to pass a parameter to the task */
                prioNotifyTask,                         /* Priority of Task */
                &xHandleNotifyTask);                    /* Microcontroller use a Task Handle (optional on Linux) */

    xTaskCreate(prvLowPriorityTask,                     /* Task Function */
                "NOTIFIED_TASK",                        /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,          /* Memory Stack */
                NULL,                             /* Used to pass a parameter to the task */
                prioNotifiedTask,                       /* Priority of Task */
                &xHandleNotifiedTask);                  /* Microcontroller use a Task Handle (optional on Linux) */

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
    
    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("\n\n");
        console_print("[NOTIFY TASK] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));
        
        uiSharedValue = rand() % 9;
        console_print("[NOTIFY TASK] - The value are %d \n\n", uiSharedValue);

        /* Send notification to lower priority task */
        //xTaskNotifyGiveIndexed( xHandleNotifiedTask, 0 );
        xTaskNotifyAndQuery( xHandleNotifiedTask,
                            0xfff,
                            eSetValueWithoutOverwrite,
                            NULL );
    }
}

static void prvLowPriorityTask(void *pvParameters)
{
   uint32_t ulNotifiedValue;
   for(;;)
   {
        /* Wait notification */
     //   ulTaskNotifyTakeIndexed( 0, pdTRUE, portMAX_DELAY );

        xTaskNotifyWaitIndexed( 0,         /* Wait for 0th notification. */
                                0x00,      /* Don’t clear any notification bits on entry. */
                                ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                &ulNotifiedValue, /* Notified value pass out in
                                                     ulNotifiedValue. */
                                portMAX_DELAY );  /* Block indefinitely. */
        console_print("[NOTIFIED TASK] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));
        console_print("[NOTIFIED TASK] - The value are %d \n",   uiSharedValue);
        console_print("[NOTIFIED TASK] - The value are %d \n\n", ulNotifiedValue);
   }
}
