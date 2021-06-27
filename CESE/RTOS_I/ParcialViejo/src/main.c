/*=============================================================================
 * Copyright (c) 2021, Martin Duarte 
 * All rights reserved.
 * License: Free
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "keys.h"
/*==================[definiciones y macros]==================================*/
#define RATE 200
#define LED_RATE pdMS_TO_TICKS(RATE)

#define PRINT_DELAY 1
#define PRINT_D pdMS_TO_TICKS(RATE)

#define WELCOME_MSG  "Ejercicio de Parcial.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200

#define BOTON_PRESIONADO   1
#define BOTON_NO_DETECTADO 0




/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDB, LED1};
gpioMap_t gpio_t[] = {GPIO7,GPIO5,GPIO3, GPIO1};

/* Handlers */
QueueHandle_t Cola200ms, Cola_UART;
SemaphoreHandle_t mutex;

/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;


#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*==================[declaraciones de funciones internas]====================*/
void gpio_init( void );
/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_led( void* taskParmPtr );
void tarea_print( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

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
                  tarea_print,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_print",   // Nombre de la tarea como String amigable para el usuario
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

    
    mutex = xSemaphoreCreateMutex();
      
    // Crear cola de longitud 1
    /*   QUiero encolar elementos que sean del tipo ticktype*/
    Cola200ms = xQueueCreate(10, sizeof(t_key_data));
    Cola_UART = xQueueCreate(10, sizeof(t_key_data));
    // Gestion de errores de semaforos
    configASSERT( Cola200ms !=  NULL  );
    configASSERT( Cola_UART !=  NULL  );
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
	uint32_t i;

    gpioInit ( GPIO7, GPIO_OUTPUT );
    gpioWrite ( GPIO7, ON );
    gpioInit ( GPIO5, GPIO_OUTPUT );
    gpioWrite ( GPIO5, ON );
    gpioInit ( GPIO3, GPIO_OUTPUT );
    gpioInit ( GPIO1, GPIO_OUTPUT );


}

// Implementacion de funcion de la tarea
void tarea_led( void* taskParmPtr )
{


	t_key_data Tarea_led;

    while( TRUE )
    {
         
      
       if (xQueueReceive( Cola200ms, &Tarea_led, portMAX_DELAY) == pdTRUE )
       {
    	   if (Tarea_led.time_diff != KEYS_INVALID_TIME)
    	   {
				gpioWrite( LED2, ON );
				gpioWrite( GPIO3, ON );


				vTaskDelay(LED_RATE);

				gpioWrite( LED2, OFF );
				//gpioWrite( gpio_t[Tarea_led.index], OFF );
				gpioWrite( GPIO3, OFF );

				xQueueSend(Cola_UART,&Tarea_led,portMAX_DELAY);

				clear_diff(Tarea_led.index);
			}
		   else
		   {
			    gpioWrite( GPIO1, ON );
				gpioWrite( leds_t[Tarea_led.index], ON );
				//gpioWrite( gpio_t[1], ON );

				vTaskDelay( LED_RATE );
				gpioWrite( GPIO1, OFF );
				gpioWrite( leds_t[Tarea_led.index], OFF );
				//gpioWrite( gpio_t[1], OFF );
		   }
       }
    }
}


void tarea_print( void* taskParmPtr )
{
    
	t_key_data Tarea_led;

      while (true)
      {
         xQueueReceive(Cola_UART, &Tarea_led , portMAX_DELAY);
         
          printf("%d : %d : %d ms \r\n",Tarea_led.index,!Tarea_led.index,Tarea_led.time_diff);
         

      }
   }  

