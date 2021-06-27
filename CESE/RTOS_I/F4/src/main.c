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

#define WELCOME_MSG  "Ejercicio F_4.\r\n"
#define MSG_QUEUEA  "Hola soy Cola A.\r\n"
#define MSG_QUEUEB  "Hola soy Cola B.\r\n"
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

/* Creación de las dos colas  */
QueueHandle_t queue_A_B, queue_B_A;

//QueueHandle_t queue_tec_pulsada;
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
void tarea1( void* taskParmPtr );
void tarea2(void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );


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
                  tarea1,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea1",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  (uint32_t*)i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );
   configASSERT( resB == pdPASS );


// Crear tarea en freeRTOS
      BaseType_t  resC = xTaskCreate(
                     tarea2,                     // Funcion de la tarea a ejecutar
                     ( const char * )"tarea2",   // Nombre de la tarea como String amigable para el usuario
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
    /*   Quiero encolar elementos que sean del tipo ticktype   */
    
    queue_A_B = xQueueCreate(N_QUEUE, sizeof(TickType_t));
   
    queue_B_A = xQueueCreate(N_QUEUE, sizeof(TickType_t));
    

    
    //queue_tec_pulsada = xQueueCreate(N_QUEUE, sizeof(TickType_t));
   
    // Gestion de errores de semaforos
    //configASSERT( queue_tec_pulsada !=  NULL  );

    
    
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
void tarea1( void* taskParmPtr )
{
    uint32_t index = ( uint32_t ) taskParmPtr;
   
    TickType_t xPeriodicity = LED_RATE;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //TickType_t dif=0;
    /*   Arreglo de 50 chars  */
    char init_msg[MSG_LENGTH] = "start!";
   
    xQueueSend(queue_B_A, init_msg,portMAX_DELAY );

    
    char mensaje_buffer [MSG_LENGTH];
    char mensaje_tarea1 [MSG_LENGTH] = "Mensaje enviado de tarea 1 a cola 1_2" ;
   
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
       if ( xQueueReceive(queue_B_A, &mensaje_buffer, portMAX_DELAY)==pdTRUE);
       {   
            gpioWrite(leds_t[0], ON);
            
            xSemaphoreTake(mutex, portMAX_DELAY);
            printf("%s \n\r",mensaje_buffer);
            xSemaphoreGive(mutex);  
            
            vTaskDelay( LED_RATE / 2 );
          
            xQueueSend(queue_A_B, &mensaje_tarea1, portMAX_DELAY); 

            gpioWrite(leds_t[0], OFF);
          
            vTaskDelayUntil( &xLastWakeTime , xPeriodicity );

         }
      }
   }

void tarea2( void* taskParmPtr )

{
	    
    TickType_t xPeriodicity = LED_RATE;
    TickType_t xLastWakeTime = xTaskGetTickCount();
   
    char mensaje_buffer [MSG_LENGTH];
    char mensaje_tarea1 [MSG_LENGTH] = "Mensaje enviado de tarea 2 a cola 2_1" ;
   
   while(true)
   {
      
      if (xQueueReceive( queue_A_B , &mensaje_buffer,  portMAX_DELAY ) == pdTRUE)			// Esperamos tecla
      {
        	
         gpioWrite(leds_t[1], ON);
          
         xSemaphoreTake( mutex , portMAX_DELAY );	
         printf("El mensaje de la tarea A: %s \n\r",mensaje_buffer);
         xSemaphoreGive(mutex); 
         
         vTaskDelay( LED_RATE / 2 );
         gpioWrite(leds_t[1], OFF);
       
         xQueueSend(queue_B_A, &mensaje_tarea1, portMAX_DELAY); 
         
         vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
      }
      
   }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( MALLOC_ERROR );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
