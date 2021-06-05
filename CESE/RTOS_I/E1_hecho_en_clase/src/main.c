/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"

#include "FreeRTOSConfig.h"

/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)

#define WELCOME_MSG  "Ejercicio D_1.\r\n"
#define USED_UART UART_USB
#define UART_RATE 9600
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_SEM "Error al crear los semaforos.\r\n"
#define LED_ERROR LEDR

#define MODO 3

#if MODO==0
/* con problemas */
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START
#define CRITICAL_END
#endif
#if MODO==1
/* enter y exit critical */
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START      taskENTER_CRITICAL();
#define CRITICAL_END        taskEXIT_CRITICAL();
#endif
#if MODO==2
/* suspend / resume all  */
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START      vTaskSuspendAll();
#define CRITICAL_END        xTaskResumeAll();
#endif
#if MODO==3
/* mutex  */
#define CRITICAL_DECLARE    SemaphoreHandle_t mutex
#define CRITICAL_CONFIG     mutex = xSemaphoreCreateMutex(); \
                            configASSERT( mutex != NULL );
#define CRITICAL_START      xSemaphoreTake( mutex , portMAX_DELAY )
#define CRITICAL_END        xSemaphoreGive( mutex )
#endif

/*==================[definiciones de datos internos]=========================*/
typedef struct
{
    char*   nombre;
    uint32_t periodicidad;
} task_parm_t;


task_parm_t params[] =
{
    { .nombre = "tarea 1", .periodicidad = 33 },
    { "tarea 2", 55 },
    { "tarea 3", 77},
    { "tarea 4", 20 },
};

CRITICAL_DECLARE;

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;


/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void tarea_printf( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();									// Inicializar y configurar la plataforma

    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages


    BaseType_t res;
    uint32_t i;

    // Crear tarea en freeRTOS
    for ( i = 0 ; i < 4 ; i++ )
    {
        res = xTaskCreate(
                  tarea_printf,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_printf",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2,       // Cantidad de stack de la tarea
                  &params[i],                       // Parametros de tarea
                  tskIDLE_PRIORITY+1,               // Prioridad de la tarea
                  0                                 // Puntero a la tarea creada en el sistema
              );

        // Gestion de errores
        configASSERT( res == pdPASS );
    }

    CRITICAL_CONFIG;

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    // ---------- REPETIR POR SIEMPRE --------------------------
    configASSERT( 0 );

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_printf( void* taskParmPtr )
{
    task_parm_t* param = ( task_parm_t* ) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = pdMS_TO_TICKS( param->periodicidad ) ; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t dif;

    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        CRITICAL_START;
        TickType_t time = xTaskGetTickCount();
        printf( "%08u Hola soy la Tarea %s\n", time, param->nombre );
        CRITICAL_END;

        vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
    }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( MALLOC_ERROR );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
