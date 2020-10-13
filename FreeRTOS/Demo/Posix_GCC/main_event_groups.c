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
#include "event_groups.h"

/* Task prototypes */
static void prvHighPriorityTaskEvent(void *pvParameters);
static void prvLowPriorityTaskEvent(void *pvParameters);

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

extern int eventGroupsTest();

/* Handle for Tasks (optional for Linux) */
TaskHandle_t xHandleNotifyTaskEvent = NULL;
TaskHandle_t xHandleNotifiedTaskEvent = NULL;


#define BIT_0	( 1 << 0 )
#define BIT_4	( 1 << 4 )


int eventGroupsTest()
{
    EventGroupHandle_t xEventGroup;

    /* Initializing console */
    console_init();
    
    /* Seed random numbers */
    srand(time(0));
    
    /* Event Group Creation */
    xEventGroup = xEventGroupCreate();
    
    if( xEventGroup == NULL )
    {
        console_print("Failed on create Event Group (memory), the program has stopped. Ctrl + C to finish. \n"); 
        for (;;);
    }

    /* Creating tasks */
    xTaskCreate(prvHighPriorityTaskEvent,                   /* Task Function */
                "NOTIFY_TASK",                              /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,              /* Memory Stack */
                xEventGroup,                                /* Used to pass a parameter to the task */
                prioNotifyTask,                             /* Priority of Task */
                &xHandleNotifyTaskEvent);                   /* Microcontroller use a Task Handle (optional on Linux) */

    xTaskCreate(prvLowPriorityTaskEvent,                    /* Task Function */
                "NOTIFIED_TASK",                            /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,              /* Memory Stack */
                xEventGroup,                                /* Used to pass a parameter to the task */
                prioNotifiedTask,                           /* Priority of Task */
                &xHandleNotifiedTaskEvent);                 /* Microcontroller use a Task Handle (optional on Linux) */

    console_print("Starting scheduling, use Ctrl + C on any moment to finish ... \n");

    /* Initializing Scheduler */
    vTaskStartScheduler();

    console_print("Failed to start the scheduler. Ctrl + C to finish. \n");
    for(;;);
}

static void prvHighPriorityTaskEvent(void *pvParameters)
{
    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskA;
        xLastWakeTime = xTaskGetTickCount();
    #endif
    
    EventGroupHandle_t xEventGroup = NULL;

    xEventGroup = (EventGroupHandle_t) pvParameters;
    EventBits_t uxBits;
    
    /* Checking event group passed to task */
    configASSERT(xEventGroup != NULL);

    uint8_t uiRandomValue = 0x00;

    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("[NOTIFY TASK] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));
        
        uiRandomValue = rand() % 3;

        if (uiRandomValue == 0)
        {
            uxBits = xEventGroupSetBits(
                xEventGroup,    /* The event group being updated. */
                BIT_0 | BIT_4); /* The bits being set. */
        }
        else if (uiRandomValue == 1)
        {
            uxBits = xEventGroupSetBits(
                xEventGroup, /* The event group being updated. */
                BIT_0);      /* The bits being set. */
        }
        else
        {
            uxBits = xEventGroupSetBits(
                xEventGroup, /* The event group being updated. */
                BIT_4);      /* The bits being set. */
        }
    }
}

static void prvLowPriorityTaskEvent(void *pvParameters)
{
    EventGroupHandle_t xEventGroup = NULL;
    EventBits_t uxBits;

    xEventGroup = (EventGroupHandle_t) pvParameters;

    /* Checking event group passed to task */
    configASSERT(xEventGroup != NULL);

    for (;;)
    {
        /* Wait notification */
        uxBits = xEventGroupWaitBits(
            xEventGroup,        /* The event group being tested. */
            BIT_0 | BIT_4,      /* The bits within the event group to wait for. */
            pdTRUE,             /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,            /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY );    /* Wait a maximum for either bit to be set. */

        console_print("[NOTIFIED TASK] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));


        if ((uxBits & (BIT_0 | BIT_4)) == (BIT_0 | BIT_4))
        {
            console_print("BIT 0 AND BIT 4 \n");
        }
        else if ((uxBits & BIT_0) != 0)
        {
            console_print("BIT 0 \n");
        }
        else if ((uxBits & BIT_4) != 0)
        {
            console_print("BIT 4 \n");
        }
        else
        {
            console_print("NONE %d\n", uxBits);
        }
   }
}
