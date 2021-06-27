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
#include "keys.h"
/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)

#define SEPARACION_MS 150
#define T_SEPARACION pdMS_TO_TICKS(SEPARACION_MS)

#define WELCOME_MSG  "Ejercicio F_1.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_SEM "Error al crear los semaforos.\r\n"
#define MSG_ERROR_QUE "Error al crear colas\r \n" 
#define LED_ERROR LEDR
#define N_QUEUE 1
#define MSG_COUNT 5
#define MSG_LENGTH 50
/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDB,LED1};
gpioMap_t gpio_t[] = {GPIO7,GPIO5};

//QueueHandle_t queue_A_B, queue_B_A;

QueueHandle_t queue_tec_pulsada;
/* Handler   */
SemaphoreHandle_t mutex;

//SemaphoreHandle_t sem_btn;
/*==================[definiciones de datos externos]=========================*/
DEBUG_PRINT_ENABLE;

extern t_key_config* keys_config;

//#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
#define LED_COUNT 1
/*==================[declaraciones de funciones internas]====================*/
void gpio_init( void );
/*==================[declaraciones de funciones externas]====================*/
TickType_t get_diff();
void clear_diff();

// Prototipo de funcion de la tarea
void tarea_led( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );
void tarea_led1(void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();
   
  
   
    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages
    printf( WELCOME_MSG );

    //BaseType_t res;
    uint32_t i;

   
    
for (i=0; i<LED_COUNT ; i++)
{
// Crear tarea en freeRTOS
BaseType_t  resB = xTaskCreate(
                  tarea_led,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_led",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  (uint32_t*)i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );
configASSERT( resB == pdPASS );

// Crear tarea en freeRTOS
   BaseType_t  resC = xTaskCreate(
                  tarea_led1,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_led1",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  (uint32_t*)i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );
configASSERT( resC == pdPASS );

}

    

    // Inicializo driver de teclas
    keys_Init();

    // Crear cola de longitud 1
    /*   QUiero encolar elementos que sean del tipo ticktype*/
    /*
    queue_A_B = xQueueCreate(N_QUEUE, sizeof(TickType_t));
   
    queue_B_A = xQueueCreate(N_QUEUE, sizeof(TickType_t));
    
    configASSERT( queue_B_A !=  NULL  );
    configASSERT( queue_A_B !=  NULL  );
*/
    
     queue_tec_pulsada = xQueueCreate(N_QUEUE, sizeof(TickType_t));
    // Gestion de errores de semaforos
    configASSERT( queue_tec_pulsada !=  NULL  );

    
    
    mutex = xSemaphoreCreateMutex();
    
    configASSERT( mutex !=  NULL  );
    
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

    for( i = 0 ; i <= LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT );
    }
}
/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void tarea_led( void* taskParmPtr )
{
    uint32_t index = ( uint32_t ) taskParmPtr;

    
    TickType_t xPeriodicity = LED_RATE;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t dif=0;
    /*   Arreglo de 50 chars  */
    //char mensaje [MSG_LENGTH];
   
   
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {

      xSemaphoreTake(mutex, portMAX_DELAY);
   
       gpioWrite (gpio_t[index]+1,ON);
       gpioWrite (leds_t[index]+1, ON);
       vTaskDelay(LED_RATE/2);
       gpioWrite (leds_t[index]+1, OFF);       
       gpioWrite (gpio_t[index]+1,OFF);       
       
      xSemaphoreGive(mutex);       
            vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
       
   /**    
      if(xQueueReceive(queue_tec_pulsada, &dif, 0) == pdTRUE )
         {
            if (dif > xPeriodicity ) dif = xPeriodicity;
         }     
     
      if (dif > 0 )
      {   
        gpioWrite( leds_t[index], ON );
        gpioWrite( gpio_t[index], ON );
        vTaskDelay( dif );
      }
        gpioWrite( leds_t[index], OFF );
        gpioWrite( gpio_t[index], OFF );
                     vTaskDelayUntil( &xLastWakeTime , xPeriodicity );

   **/
      }
       
       
       
       
      //xQueueReceive(queue_B_A, &mensaje, portMAX_DELAY);
     
      //gpioWrite (GPIO7, OFF);  
      //gpioWrite (GPIO5, ON);
       
      // xSemaphoreTake(mutex, portMAX_DELAY);
       
      // printf("[A] Tarea B dijo %s \r\n",mensaje);
       
       
      // xSemaphoreGive(mutex);
       
      // gpioWrite(LED2, ON);
       //vTaskDelay(xPeriodicity/2);
       //xQueueSend(queue_A_B, &mensaje, portMAX_DELAY);
       
       
       //vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
       
       
    
         
}

void tarea_led1( void* taskParmPtr )

{
	uint32_t index = (uint32_t) taskParmPtr;

    // ---------- CONFIGURACIONES ------------------------------
	TickType_t xPeriodicity = LED_RATE; // Tarea periodica cada 1000 ms
	TickType_t xLastWakeTime = xTaskGetTickCount();
	TickType_t dif = 0;
   
   while(true)
   {
      xQueueReceive( queue_tec_pulsada , &dif,  portMAX_DELAY );			// Esperamos tecla

		xSemaphoreTake( mutex , portMAX_DELAY );			//abrir seccion critica
      
      gpioWrite( leds_t[index] , OFF );
		gpioWrite( gpio_t[index] , OFF );
		vTaskDelay( T_SEPARACION );

		gpioWrite( leds_t[index] , ON );
		gpioWrite( gpio_t[index], ON );
   
		vTaskDelay( dif );
   
		gpioWrite( leds_t[index] , OFF );
		gpioWrite( gpio_t[index], OFF );
   
		vTaskDelay( T_SEPARACION );
   
		xSemaphoreGive( mutex );	
   }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( MALLOC_ERROR );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
