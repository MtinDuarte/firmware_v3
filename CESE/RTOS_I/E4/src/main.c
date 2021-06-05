/*=============================================================================
 * Copyright (c) 2021, Martin Duarte <duartemartn@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: XX/XX/XXXX
 * Version: v1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"

#define MODO 1

#include "keys.h"

/*=====[Definition & macros of public constants]==============================*/
#define RATE                    2000
#define LED_RATE_TICKS          pdMS_TO_TICKS(RATE)

/*=====[Definitions of extern global functions]==============================*/

// Prototipo de funcion de la tarea
void task_led0( void* taskParmPtr );
void task_led1( void* taskParmPtr );
void keys_service_task( void* taskParmPtr );

/*=====[Definitions of public global variables]==============================*/



/*=====[Main function, program entry point after power on or reset]==========*/

int main( void )
{
    BaseType_t res;

    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();  // Inicializar y configurar la plataforma

    gpioInit( GPIO7, GPIO_OUTPUT );
    gpioInit( GPIO8, GPIO_OUTPUT );

    printf( "Ejercicio E4_ MtinDuarte  \n" );
   
    /*  uint32_t Counter initialization   */
    uint32_t contador = 500, *pcontador = &contador;

    // Create first task of led 1
    res = xTaskCreate (
              task_led0,					      // Funcion de la tarea a ejecutar
              ( const char * )"task_led0",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	   // Cantidad de stack de la tarea
			  pcontador,							   // Parametros de tarea
              tskIDLE_PRIORITY+1,			   // Prioridad de la tarea
              0							         // Puntero a la tarea creada en el sistema
          );

    res = xTaskCreate (
              task_led1,					// Funcion de la tarea a ejecutar
              ( const char * )"task_led1",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  pcontador,					// Parametros de tarea
              tskIDLE_PRIORITY+1,			// Prioridad de la tarea
              0								// Puntero a la tarea creada en el sistema
          );

    // Gestión de errores
    configASSERT( res == pdPASS );

    /* inicializo driver de teclas */
    keys_init(pcontador);

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    /* realizar un assert con "false" es equivalente al while(1) */
    configASSERT( 0 );
    return 0;
}

void task_led0( void* taskParmPtr )
{
    uint32_t *contador = ( uint32_t *) taskParmPtr;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t  LED_RATE;

    while( 1 )
    {

    	LED_RATE = pdMS_TO_TICKS(*contador);

       	gpioWrite( LED1, ON );
       	gpioWrite( GPIO7, ON );
        vTaskDelay(LED_RATE/2);
       	gpioWrite( GPIO7, OFF );
        gpioWrite( LED1, OFF );

        // Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
        vTaskDelayUntil( &xLastWakeTime, LED_RATE);
    }
}

void task_led1( void* taskParmPtr )
{
   /* Cast incoming parameter */
    uint32_t *contador = ( uint32_t *) taskParmPtr;

    TickType_t xLastWakeTime = xTaskGetTickCount();
      
    TickType_t  LED_RATE;

    while( 1 )
    {
       /*   Update LED_RATE   */
    	LED_RATE = pdMS_TO_TICKS(*contador);
   
      /* Turn on leds   */
        gpioWrite( LED2, ON );
        gpioWrite( GPIO8, ON );
       
         /* Wait 2 * counter seconds  */
        vTaskDelay(LED_RATE*2);
       
         /* Shutdown leds  */
        gpioWrite( GPIO8, ON );
        gpioWrite( LED2, OFF );

       // Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
       
       *contador -= 100;

       (*contador < 100) ? (*contador= 100) :  ( *contador -= 100);
       
       
       /*   Period equal of 2 seconds =   LED_RATE_TICKS  */
        vTaskDelayUntil( &xLastWakeTime, LED_RATE_TICKS );
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( "Malloc Failed Hook!\n" );
    configASSERT( 0 );
}