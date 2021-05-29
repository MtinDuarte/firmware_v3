/*==================[inclusions]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/* Header files of this project*/
#include "FreeRTOSConfig.h"
#include "keys.h"
#include "sapi.h"

/*==================[definiciones y macros]==================================*/
/* Rate in miliseconds  */
#define RATE 1000

/* Conversion from ms to ticks   */
#define LED_RATE pdMS_TO_TICKS(RATE)


//#define N_SEM 			1
#define WELCOME_MSG  "Ejercicio D_4 - Martin Duarte.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define MALLOC_ERROR "Malloc Failed Hook!\n"
#define MSG_ERROR_SEM "Error al crear los semaforos.\r\n"
#define LED_ERROR LEDR


#define LED_COUNT 1
/* Global array's of leds and GPIO for logic analyzer */

gpioMap_t leds_t[] = {LEDG,LED_ERROR};//,LED2,LED3};
gpioMap_t gpio_t[] = {GPIO7,GPIO5}; //,GPIO3,GPIO1};

/* Instance semaphore */
SemaphoreHandle_t sem_btn;

/*==================[definiciones de datos externos]=========================*/

/* Enable debugging interface*/
DEBUG_PRINT_ENABLE;

/* Pointer to t_key_config structure */
extern t_key_config* keys_config;


int main( void )
{
    /*   Initialization of the board   */
    boardConfig();		

    /*   Initialization of gpio's*/
    gpio_init();

    /*   Configure UART interface   */
    debugPrintConfigUart( USED_UART, UART_RATE );	
    /*   Print welcome message   */
    printf( WELCOME_MSG );

    /*   Creation of status variable for assert verification  */
    BaseType_t res;

    /*   Task creation  */
   
    for (uint32_t i = 0 ; i < LED_COUNT ; i++ )
    {
        res = xTaskCreate(
                  tarea_led,                    // -> void (* pFunction)(void)      <- | Void pointer to function |
                  ( const char * )"tarea_led",  // -> (const char *) "string"       <-
                  configMINIMAL_STACK_SIZE*2,   // -> Stack necessary for the task  <-
                  (uint32_t *)i,                // -> Parameters of the task        <- It's force to cast an int
                  tskIDLE_PRIORITY+1,           // -> Priority of the task          <- 
                  0                             // -> Pointer to created function   <- No need.
              );

        /* If the task was correctly created, same as while (0), if any error ocurred during task creation, behave as while (1)  */
       
        configASSERT( res == pdPASS );
    }

    /*   Initialization of keys */
    keys_Init();

    /*   Create binarty semaphore   */
    sem_btn = xSemaphoreCreateBinary();

    /*Assert for semaphore creation */
    configASSERT( sem_btn !=  NULL  );

    /*   Start scheduler   */
    /* This means: 
      -Turn on Systick.
      -Check for created task.
      -Set idle task and set ready to the corresponding tasks.
      -Check which task has highest priority */
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running


    configASSERT( 0 );

    return TRUE;
}

void gpio_init( void )
{
    
    for( uint32_t i = 0 ; i < LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT );
    }
}


/* Implementation of led task */

void tarea_led( void* taskParmPtr )
{
    /*   Cast incoming parameter pointer to uint32_t index  */
    uint32_t index = ( uint32_t ) taskParmPtr;


    /*   Define the periodicity of led toggle   */
   
    TickType_t xPeriodicity = LED_RATE; 
   
    /*   Start LastWake time tick variable      */
    TickType_t xLastWakeTime = xTaskGetTickCount();
   
    
    while( TRUE )
    {
       /*   Verify if semaphore was taken with no block time needed  */
    	if ( xSemaphoreTake( sem_btn, 0 ) == pdTRUE )
    	{
         /* Turn ON led GREEN & GPIO7 for 1/4 of LED_RATE  = 250ms   */
    	 gpioWrite( leds_t[0], ON );
		 gpioWrite( gpio_t[0], ON );
       vTaskDelay( LED_RATE/4 );
       gpioWrite( gpio_t[0], OFF );
    	 gpioWrite( leds_t[0], OFF );
    	}
    	else
    	{
       /* Turn ON led RED & GPIO5 for 1/4 of LED_RATE  = 250ms   */
       gpioWrite( leds_t[1], ON );
       gpioWrite( gpio_t[1], ON );
       vTaskDelay( LED_RATE/4 );
       gpioWrite( gpio_t[1], OFF );
    	 gpioWrite( leds_t[1], OFF );
    	}
    	
      vTaskDelayUntil(&xLastWakeTime,xPeriodicity);

    }
}

   /* No need of malloc failed hook function */
void vApplicationMallocFailedHook()
{
    printf( MALLOC_ERROR );
    configASSERT( 0 );
}
/*==================[fin del archivo]========================================*/
