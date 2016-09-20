/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


/**
 * @file
 * A simple demo application.
 *
 * @author Jernej Kovacic
 */


#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "task.h"

#include "app_config.h"
#include "serial.h"


/*
 * This diagnostic pragma will suppress the -Wmain warning,
 * raised when main() does not return an int
 * (which is perfectly OK in bare metal programming!).
 *
 * More details about the GCC diagnostic pragmas:
 * https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#pragma GCC diagnostic ignored "-Wmain"

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName );
void vApplicationTickHook( void );
void vApplicationIdleHook( void );

extern void vPortUnknownInterruptHandler( void *pvParameter );
extern void vPortInstallInterruptHandler( void (*vHandler)(void *), void *pvParameter, unsigned long ulVector, unsigned char ucEdgeTriggered, unsigned char ucPriority, unsigned char ucProcessorTargets );

/* Struct with settings for each task */
typedef struct _paramStruct
{
    portCHAR* text;                  /* text to be printed by the task */
    UBaseType_t  delay;              /* delay in milliseconds */
} paramStruct;

/* Default parameters if no parameter struct is available */
static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000;


/* Task function - may be instantiated in multiple tasks */
void vTaskFunction( void *pvParameters )
{
    const portCHAR* taskName;
    UBaseType_t  delay;
    paramStruct* params = (paramStruct*) pvParameters;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    for( ; ; )
    {
        /* Print out the name of this task. */

        vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)taskName, strlen(taskName));

        vTaskDelay( delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}


/* Fixed frequency periodic task function - may be instantiated in multiple tasks */
void vPeriodicTaskFunction(void* pvParameters)
{
    const portCHAR* taskName;
    UBaseType_t delay;
    paramStruct* params = (paramStruct*) pvParameters;
    TickType_t lastWakeTime;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    /*
     * This variable must be initialized once.
     * Then it will be updated automatically by vTaskDelayUntil().
     */
    lastWakeTime = xTaskGetTickCount();

    for( ; ; )
    {
        /* Print out the name of this task. */

        vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)taskName, strlen(taskName));

        /*
         * The task will unblock exactly after 'delay' milliseconds (actually
         * after the appropriate number of ticks), relative from the moment
         * it was last unblocked.
         */
        vTaskDelayUntil( &lastWakeTime, delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}


/* Parameters for two tasks */
paramStruct tParam[2] =
{
    { .text="Task1\r\n", .delay=1000 },
    { .text="Periodic task\r\n", .delay=3000 }
};

/* Startup function that creates and runs two FreeRTOS tasks */
void main(void)
{

    unsigned long ulVector = 0UL;
    unsigned long ulValue = 0UL;
    char cAddress[32];

    portDISABLE_INTERRUPTS();
    
    /* Install the Spurious Interrupt Handler to help catch interrupts. */
    for ( ulVector = 0; ulVector < portMAX_VECTORS; ulVector++ )
    	vPortInstallInterruptHandler( vPortUnknownInterruptHandler, (void *)ulVector, ulVector, pdTRUE, configMAX_SYSCALL_INTERRUPT_PRIORITY, 1 );
    
    /* Init of print related tasks: */
    vUARTInitialise( mainPRINT_PORT, mainPRINT_BAUDRATE, 64);

    portENABLE_INTERRUPTS();

    /*
     * I M P O R T A N T :
     * Make sure (in startup.s) that main is entered in Supervisor mode.
     * When vTaskStartScheduler launches the first task, it will switch
     * to System mode and enable interrupt exceptions.
     */
    vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)("= = = T E S T   S T A R T E D = = =\r\n\r\n"), strlen("= = = T E S T   S T A R T E D = = =\r\n\r\n"));

    /* And finally create two tasks: */
    if ( pdPASS != xTaskCreate(vTaskFunction, "task1", 128, (void*) &tParam[0],
                               PRIOR_PERIODIC, NULL) )
    {
        vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)("Could not create task1\r\n"), strlen("Could not create task1\r\n"));
	while(1);
    }

    if ( pdPASS != xTaskCreate(vPeriodicTaskFunction, "task2", 128, (void*) &tParam[1],
                               PRIOR_FIX_FREQ_PERIODIC, NULL) )
    {
        vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)("Could not create task2\r\n"), strlen("Could not create task2\r\n"));
	while(1);
    }

    vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)("A text may be entered using a keyboard.\r\n"), strlen("A text may be entered using a keyboard.\r\n"));
    vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)("It will be displayed when 'Enter' is pressed.\r\n\r\n"), strlen("It will be displayed when 'Enter' is pressed.\r\n\r\n"));

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /*
     * If all goes well, vTaskStartScheduler should never return.
     * If it does return, typically not enough heap memory is reserved.
     */

    vSerialPutString((xComPortHandle)configUART_PORT, (const signed char * const)("Could not start the scheduler!!!\r\n"), strlen("Could not start the scheduler!!!\r\n"));
    while(1);

    /* just in case if an infinite loop is somehow omitted in FreeRTOS_Error */
    for ( ; ; );
}

void vApplicationMallocFailedHook( void )
{
	__asm volatile (" smc #0 ");
}
/*----------------------------------------------------------------------------*/

extern void vAssertCalled( char *file, int line )
{
	printf("Assertion failed at %s, line %d\n\r",file,line);
	taskDISABLE_INTERRUPTS();
	for( ;; );
}

void vApplicationTickHook( void )
{
	static int ticks = 0;
	ticks++;

	if (ticks % 1000 == 0)
		printf("Time : %d sec\r\n\n", ticks / 1000);
}
/*----------------------------------------------------------------------------*/

void vApplicationIdleHook( void )
{
signed char cChar;
	if ( pdTRUE == xSerialGetChar( (xComPortHandle)mainPRINT_PORT, &cChar, 0UL ) )
	{
		(void)xSerialPutChar( (xComPortHandle)mainPRINT_PORT, cChar, 0UL );
		(void)xSerialPutChar( (xComPortHandle)mainPRINT_PORT, cChar, 0UL );
	}
}
/*----------------------------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	( void ) pxTask;
	( void ) pcTaskName;
	 printf("StackOverflowHook\n");
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	identify which task has overflowed its stack. */
	for( ;; );
}
