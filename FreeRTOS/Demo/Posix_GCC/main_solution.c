
/* Socket includes */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>


/* Microkernel includes */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "console.h"

#define BUFFER_SIZE     1000

/* Task prototypes */
static void prvActuator( void *pvParameters );
static void prvQuery( void *pvParameters );
static void prvKeyboard( void *pvParameters );

/* Function prototypes */
static int createLocalSocket(void);
struct sockaddr_in createDestinyAddress(char *destiny, int port);
void sendMessage(int localSocket, struct sockaddr_in destinyAddress, char *msg);
int receiveMessage(int localSocket, char *buffer, int bufferSize);


/* Global variables (Mutexes) */
SemaphoreHandle_t xSemaphore;

int solution()
{

    /* Initializing the console of FreeRTOS */
    console_init();

    /* Creating tasks */
    xTaskCreate(prvActuator, /* Task Function */
                "Actuator", /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10, /* Memory Stack */
                NULL, /* Used to pass a parameter to the task */
                7, /* Priority of Task */
                NULL); /* Microcontroller use a Task Handle (not used on Linux, so NULL) */

    xTaskCreate(prvQuery, /* Task Function */
                "Query", /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10, /* Memory Stack */
                NULL, /* Used to pass a parameter to the task */
                7, /* Priority of Task */
                NULL); /* Microcontroller use a Task Handle (not used on Linux, so NULL) */

    xTaskCreate(prvKeyboard, /* Task Function */
                "keyboard", /* Name of task (for debugging propose only) */
                configMINIMAL_STACK_SIZE * 10, /* Memory Stack */
                NULL, /* Used to pass a parameter to the task */
                6, /* Priority of Task */
                NULL); /* Microcontroller use a Task Handle (not used on Linux, so NULL) */

    /* Creating mutexes */
    xSemaphore = xSemaphoreCreateMutex();
    if(xSemaphore == NULL) for(;;); /* Stop here if not memory allocated for mutex */

    /* Starting the scheduler */
    vTaskStartScheduler();

    return 0;
}



static void prvActuator(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(5000UL);
    xLastWakeTime = xTaskGetTickCount();
    
    struct sockaddr_in destinyAddress;
    int localSocket;

    char msgSent[1000]="ani12.4";
    char msgReceived[1000];
    int nrec;

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        xSemaphoreTake( xSemaphore, portMAX_DELAY);
        
        localSocket = createLocalSocket();
        destinyAddress = createDestinyAddress("127.0.0.1", 5000);
        
        sendMessage(localSocket, destinyAddress, msgSent);
        
        close(localSocket);
        xSemaphoreGive( xSemaphore );
    }
}

static void prvQuery(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(5500UL);
    xLastWakeTime = xTaskGetTickCount();

    struct sockaddr_in destinyAddress;
    int localSocket;

    char msgSent[1000]="sta0";
    char msgReceived[1000];
    int nrec;

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        xSemaphoreTake( xSemaphore, portMAX_DELAY);
        
        localSocket = createLocalSocket();
        destinyAddress = createDestinyAddress("127.0.0.1", 5000);

        sendMessage(localSocket, destinyAddress, msgSent);
        nrec = receiveMessage(localSocket,msgReceived, BUFFER_SIZE);
        console_print("Mensagem de resposta com %d bytes >>>%s\n", nrec, msgReceived);

        close(localSocket);
        xSemaphoreGive( xSemaphore );        
    }
}

static void prvKeyboard(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(5000UL);
    xLastWakeTime = xTaskGetTickCount();

    uint16_t refAngle = 0;
    char refAngleString[64];
      char name[10];

    for (;;)
    {
        scanf("%s", refAngleString);
        refAngle = (uint16_t)atoi(refAngleString);

        if(refAngle != 0){
            console_print("Angle is: %d\n", refAngle);
            refAngle = 0;
            refAngleString[0] = '\0';
            refAngleString[1] = '\0';
            refAngleString[2] = '\0';
            refAngleString[3] = '\0';
        }
    }
}

/********************************* Communication f120unctions **********************************/

static int createLocalSocket(void)
{
    int localSocket; /* Socket used on communication */

    localSocket = socket(PF_INET, SOCK_DGRAM, 0);

    if (localSocket < 0)
    {
        console_print(" -- Error on create socket -- \n");
        return 0;
    }
    return localSocket;
}

struct sockaddr_in createDestinyAddress(char *destiny, int port)
{
    struct sockaddr_in server;    /* Server address including ip and port */
    struct hostent *destInternet; /* Destination address in own format */
    struct in_addr destIp;        /* Destination address in numeric ip format */

    if (inet_aton(destiny, &destIp))
        destInternet = gethostbyaddr((char *)&destIp, sizeof(destIp), AF_INET);
    else
        destInternet = gethostbyname(destiny);

    if (destInternet == NULL)
    {
        console_print("Invalid network address \n");
        exit(1);
    }

    memset((char *)&server, 0, sizeof(server));
    memcpy(&server.sin_addr, destInternet->h_addr_list[0], sizeof(server.sin_addr));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    return server;
}

void sendMessage(int localSocket, struct sockaddr_in destinyAddress, char *msg)
{
    if (sendto(localSocket, msg, strlen(msg) + 1, 0, (struct sockaddr *)&destinyAddress, sizeof(destinyAddress)) < 0)
    {
        console_print("Error on send message \n");
        return;
    }
}

int receiveMessage(int localSocket, char *buffer, int bufferSize){
    int receivedBytes;		/* Number of received bytes */

	/* Wait from response server message */
	receivedBytes = recvfrom(localSocket, buffer, bufferSize, 0, NULL, 0);
	if (receivedBytes < 0)
	{
		console_print("Error on receive message \n");
	}

	return receivedBytes;
}