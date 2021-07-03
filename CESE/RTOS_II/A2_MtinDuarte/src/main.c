/*=============================================================================
 * Copyright (c) 2021, Martin Duarte <duartemartn@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: XX/XX/XXXX
 * Version: v1.0
 *===========================================================================*/

/*==================[inlcusiones]============================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "sapi.h"
#include "keys.h"
#include "stdlib.h"


/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)

#define WELCOME_MSG  "Ejercicio 1 - Martin Duarte.\r\n"

#define USED_UART UART_USB
#define UART_RATE 115200


/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDB, LED1, LED2, LED3};
gpioMap_t gpio_t[] = {GPIO7, GPIO5, GPIO3, GPIO1};

/* Handlers */
QueueHandle_t Cola1;//, Cola2;

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;



#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

TickType_t get_diff();
void clear_diff();

// Function prototypes
void tarea_led( void* taskParmPtr );
void tarea_C( void* taskParmPtr );
void gpio_init();
/*==================[funcion principal]======================================*/

int main( void )
{

    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();									// Gpio initialization


    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages

    printf( WELCOME_MSG );

    BaseType_t res;

    // Crear tarea en freeRTOS
    for (uint32_t i = 0 ; i < 1 ; i++ )
    {
        res = xTaskCreate(
                  tarea_led,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_led",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  (uint32_t*)i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );

        // Gestion de errores
        configASSERT( res == pdPASS );

       res = xTaskCreate(
                  tarea_C,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_C",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  (uint32_t*)i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );

        // Gestion de errores
        configASSERT( res == pdPASS );

    }

    // Inicializo driver de teclas
    keys_Init();

    // Crear cola de longitud 1
    /*   QUiero encolar elementos que sean del tipo ticktype*/
    Cola1 = xQueueCreate(5, sizeof(char*));
    //Cola2 = xQueueCreate(10, sizeof(Parameters_t));
    // Gestion de errores de semaforos
    configASSERT( Cola1 !=  NULL  );

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
void gpio_init( void )
{
    for( uint32_t i = 0 ; i < LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT);
    }
}

bool_t sendLedText (char* msgpointer)
{



	return true;
}

void tarea_led( void* taskParmPtr )
{
    uint32_t index = ( uint32_t ) taskParmPtr;
    // ---------- CONFIGURACIONES ------------------------------
    TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while( TRUE )
    {
    	char *D_pointer;

    	D_pointer=(char*)pvPortMalloc(sizeof(char)*8);

		// ASSERT puntero NULL
		if ( (D_pointer != NULL)  )
		{
			gpioWrite(leds_t[index], ON);

			sprintf(D_pointer,"LED ON!");

			vTaskDelay(LED_RATE/2);

			gpioWrite(leds_t[index], OFF);

			xQueueSend(Cola1,&D_pointer, 0);

			vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
		}
		else
			printf("No se pudo asignar memoria, puntero nulo");
    }
}


void tarea_C( void* taskParmPtr )
{

	 char*	P_pointer=NULL;
	 uint32_t memoria_consumida,memoria_total;

      while (true)
      {
    	  if ( pdTRUE == xQueueReceive(Cola1,&P_pointer, portMAX_DELAY) )
    	  {
    		memoria_consumida = xPortGetFreeHeapSize();

    		taskENTER_CRITICAL();
			uartWriteString(UART_USB, P_pointer);
			printf("\r\n");
			taskEXIT_CRITICAL();

			memoria_consumida = xPortGetFreeHeapSize();

    	  }
    	  vPortFree(P_pointer);	P_pointer=NULL;
    	  memoria_total= xPortGetFreeHeapSize() - memoria_consumida;
    	  printf("Consumo memoria : %d \r\n",memoria_total); 	//Incluir esta línea para determinar el consumo de memoria.
      }
}
