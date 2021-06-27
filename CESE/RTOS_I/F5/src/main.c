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
#include "string.h"
#include "sapi.h"

#include "FreeRTOSConfig.h"
#include "keys.h"
/*==================[definiciones y macros]==================================*/
#define RATE 1000
#define LED_RATE pdMS_TO_TICKS(RATE)

#define SEPARACION_MS 150
#define T_SEPARACION pdMS_TO_TICKS(SEPARACION_MS)
#define PRINT_RATE_MS 500
#define PRINT_RATE pdMS_TO_TICKS(PRINT_RATE_MS)
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

typedef struct {
   TickType_t Time_pressed;
   uint32_t index;
   char Event[15];
   
}queue_t;
  queue_t MyStruct;
/* Creación de las dos colas  */
QueueHandle_t Data_Queue, Print_Queue;

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
    
    
    MyStruct.index = 0;
    queue_t *p_main= &MyStruct;

// Crear tarea en freeRTOS
   BaseType_t  resB = xTaskCreate(
                  tarea_led,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_led",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  &p_main,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  0                           // Puntero a la tarea creada en el sistema
              );
   configASSERT( resB == pdPASS );


// Crear tarea en freeRTOS
      BaseType_t  resC = xTaskCreate(
                     tarea_led1,                     // Funcion de la tarea a ejecutar
                     ( const char * )"tarea_led1",   // Nombre de la tarea como String amigable para el usuario
                     configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                      0,                          // Parametros de tarea
                     tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                     0                           // Puntero a la tarea creada en el sistema
               );
   configASSERT( resC == pdPASS );


    

    // Inicializo driver de teclas
    keys_Init();

    // Crear cola de longitud 1
   
   /*   Quiero encolar elementos que sean del tipo ticktype   */
   
    Data_Queue = xQueueCreate(1, sizeof(TickType_t));
        configASSERT( Data_Queue !=  NULL  );

    Print_Queue = xQueueCreate(10, sizeof(queue_t));
      configASSERT( Print_Queue !=  NULL  );
    

    
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
void tarea_led( void* taskParmPtr  )
{
    queue_t* struct_pointer = ( void *) taskParmPtr;
   
    TickType_t xPeriodicity = LED_RATE;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t dif=0;
    
      
    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
       xQueueReceive(Data_Queue, &dif, portMAX_DELAY);
       
       //xSemaphoreTake(mutex, portMAX_DELAY);
   
       gpioWrite (gpio_t[MyStruct.index]+1,ON);
       gpioWrite (leds_t[MyStruct.index]+1, ON);
       
       vTaskDelay(dif);
       struct_pointer->Time_pressed=dif;
       struct_pointer->index=MyStruct.index;
       strcpy (MyStruct.Event, "TareaA_Lpressed");
       
   
       
       //strcpy(struct_pointer->Event,msg);
       
       
       gpioWrite (leds_t[MyStruct.index]+1,OFF);       
       gpioWrite (gpio_t[MyStruct.index]+1,OFF);       
       
       xQueueSend(Print_Queue, &struct_pointer, portMAX_DELAY);
       //xSemaphoreGive(mutex);       
       vTaskDelayUntil(&xLastWakeTime , xPeriodicity );
       

       }
}

void tarea_led1( void* taskParmPtr )

{
    // ---------- CONFIGURACIONES ------------------------------
	TickType_t xPeriodicity = PRINT_RATE; // Tarea periodica cada 1000 ms
	TickType_t xLastWakeTime = xTaskGetTickCount();
	//TickType_t dif = 0;
   
   while(true)
   {
      queue_t* s_pointer = &MyStruct;
    
      xQueueReceive( Print_Queue , &s_pointer ,  portMAX_DELAY );			// Esperamos tecla
         
      
		//xSemaphoreTake( mutex , portMAX_DELAY );			//abrir seccion critica
      printf("el valor del retardo: %i , y el evento : %s , el indice del led %i \n\r",s_pointer->Time_pressed,MyStruct.Event, s_pointer->index);
		//xSemaphoreGive( mutex );	
      
      vTaskDelayUntil( &xLastWakeTime , xPeriodicity );
   }
}

/* hook que se ejecuta si al necesitar un objeto dinamico, no hay memoria disponible */
void vApplicationMallocFailedHook()
{
    printf( MALLOC_ERROR );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
