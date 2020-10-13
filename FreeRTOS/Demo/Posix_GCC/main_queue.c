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
#include "queue.h"

/* Task prototypes */
static void prvProducer(void *pvParameters);
static void prvConsumer(void *pvParameters);

/* Task Priorities */
#define prioPRODUCER (tskIDLE_PRIORITY + 4) // HIgh Priority
#define prioCONSUMER (tskIDLE_PRIORITY + 3) // Low Priority

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

extern int queueTest();

/* Handle for Tasks (optional for Linux) */
TaskHandle_t xHandleProducer = NULL;
TaskHandle_t xHandleConsumer = NULL;

/* Struct message for queue */
typedef struct A_Message
{
    char ucMessageID;
    char ucData[20];
} AMessage;

int queueTest()
{
    QueueHandle_t xQueue;

    /* Initializing console */
    console_init();
    
    /* Seed random numbers */
    srand(time(0));

    /* Creating queue */
    console_print("Creating Queues... \n");
    xQueue = xQueueCreate(10, sizeof(AMessage));
    
    if (xQueue == NULL)
    {
        console_print("Failed on create queue (memory), the program has stopped. Ctrl + C to finish. \n"); 
        for (;;);
    } else
    {
        /* Setting name to Queue */
        vQueueAddToRegistry( xQueue, "Queue-01" );
        console_print("Queue created... \n");
    }
    
    /* Creating tasks */
    xTaskCreate(prvProducer,                      /* Task Function */
                "PRODUCER",                       /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,    /* Memory Stack */
                xQueue,                           /* Used to pass a parameter to the task */
                prioPRODUCER,                     /* Priority of Task */
                &xHandleProducer);                /* Microcontroller use a Task Handle (optional on Linux) */

    xTaskCreate(prvConsumer,                      /* Task Function */
                "CONSUMER",                       /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10,    /* Memory Stack */
                xQueue,                           /* Used to pass a parameter to the task */
                prioCONSUMER,                     /* Priority of Task */
                &xHandleConsumer);                /* Microcontroller use a Task Handle (optional on Linux) */

    console_print("Starting scheduling, use Ctrl + C on any moment to finish ... \n");

    /* Initializing Scheduler */
    vTaskStartScheduler();

    console_print("Failed to start the scheduler. Ctrl + C to finish. \n");
    for(;;);
}

static void prvProducer(void *pvParameters)
{
    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskA;
        xLastWakeTime = xTaskGetTickCount();
    #endif

    /* Queue variables */
    QueueHandle_t xQueue = NULL;
    AMessage *pxMessage;

    xQueue = (QueueHandle_t) pvParameters; // Restoring xQueue from higher context.
    

    /* Checking parameter passed to task */
    configASSERT(xQueue != NULL);

    console_print("\n******* %s STATS *******", pcTaskGetName(xHandleProducer));
    console_print("\n Task Priority: %d", uxTaskPriorityGet(xHandleProducer));
    console_print("\n Queue Name: %s", pcQueueGetName(xQueue));
    console_print("\n Queue Space Used: %d", uxQueueMessagesWaiting(xQueue));
    console_print("\n Queue Space Avaliable: %d", uxQueueSpacesAvailable(xQueue));
    console_print("\n******************************\n\n");

    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("\n\n");
        console_print("[PRODUCER] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));
        
        /* Mount package */
        pxMessage->ucMessageID= rand() % 9;
        randomString(pxMessage->ucData, 4);
        
        console_print("-Sending- ");
        console_print(" Message ID : [%d]", pxMessage->ucMessageID);
        console_print(" Message String : [%s] \n", pxMessage->ucData);

        /* Send the message. If the queue are full, overwrite into last position (10) */
        if(xQueueSend(xQueue, ( void * ) &pxMessage, pdMS_TO_TICKS(0)) == pdFALSE){
            console_print("\n Full Queue. Overwrite the last item on queue\n");
            xQueueOverwrite( xQueue, &pxMessage );
        }
        
    }
}

static void prvConsumer(void *pvParameters)
{

    #if isPeriodic == TRUE
        TickType_t xLastWakeTime;
        const TickType_t xFrequency = pTaskB;
        xLastWakeTime = xTaskGetTickCount();
    #endif

    /* Queue variables */
    QueueHandle_t xQueue = NULL;
    AMessage *pxMessage;
        
    xQueue = (QueueHandle_t) pvParameters; // Restoring xQueue from higher context.
        
    /* Checking parameter passed to  task */
    configASSERT(xQueue != NULL);

    console_print("\n******* %s STATS *******", pcTaskGetName(xHandleConsumer));
    console_print("\n Task Priority: %d", uxTaskPriorityGet(xHandleConsumer));
    console_print("\n Queue Name: %s", pcQueueGetName(xQueue));
    console_print("\n Queue Space Used: %d", uxQueueMessagesWaiting(xQueue));
    console_print("\n Queue Space Avaliable: %d", uxQueueSpacesAvailable(xQueue));
    console_print("\n******************************\n\n");

    for (;;)
    {
        #if isPeriodic == TRUE
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        #endif

        console_print("\n\n");
        console_print("[CONSUMER] running at %lld ms after vTaskStartScheduler() called. \n", pdTICKS_TO_MS(xTaskGetTickCount()));

        if(xQueueReceive( xQueue,( void * ) &pxMessage, pdMS_TO_TICKS(0)) == pdTRUE){
            console_print("-Received- ");
            console_print(" Message ID : [%d]", pxMessage->ucMessageID);
            console_print(" Message String : [%s] \n", pxMessage->ucData);
        }

        
    }
}

/* Creating a random string to send as message to queue */
void randomString(char *s, const int len)
{
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

    for (int i = 0; i < len; ++i) 
    {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}