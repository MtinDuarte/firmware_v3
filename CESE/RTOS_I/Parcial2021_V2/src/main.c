/*=============================================================================
 * Copyright (c) 2021, Martin Duarte 
 * All rights reserved.
 * License: Free
 *===========================================================================*/

/*==================[inclusiones]============================================*/
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "random.h"
#include "sapi.h"
#include "keys.h"
/*==================[definiciones y macros]==================================*/


#define WAM_MOLE_SHOW_MAX_TIME      6000
#define WAM_MOLE_OUTSIDE_MAX_TIME   2000
#define WAM_MOLE_SHOW_MIN_TIME      1000
#define WAM_MOLE_OUTSIDE_MIN_TIME   500
#define WAM_GAMEPLAY_TIMEOUT        20000   //Gameplay time


#define WELCOME_MSG  "Ejercicio de Parcial. Martin Duarte.\r\n"
#define USED_UART UART_USB
#define UART_RATE 115200
#define QUEUE_DEPTH	8
#define GAMEPLAY_TIMEOUT_IN_MS		500
#define GAMEPLAY_TIMEOUT_IN_MS_TICKS pdMS_TO_TICKS(GAMEPLAY_TIMEOUT_IN_MS)

DEBUG_PRINT_ENABLE;
extern t_key_config* keys_config;


#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*==================[definiciones de datos internos]=========================*/
gpioMap_t leds_t[] = {LEDB, LED1, LED2,LED3};		/* Array de LEDS	*/
gpioMap_t gpio_t[] = {GPIO7,GPIO5,GPIO3,GPIO1};		/* Array de GPIOS	*/

/* Handlers */
QueueHandle_t colaMartillazos, score, matatopos[4];
TaskHandle_t xHandle[LED_COUNT+1];

/*==================[Variables globales a ser vistas desde funciones externas] =========================*/
TickType_t GameTime;


void gpio_init( void );

/*==================[declaraciones de funciones internas]====================*/
// Prototipos de funciones de las tareas
void tarea_topo (void* taskParmPtr );
void Tarea_Ppal (void* taskParmPtr);

/*==================[funcion principal]======================================*/


int main( void )
{

    boardConfig();									// Inicializar y configurar la plataforma

    gpio_init();

    debugPrintConfigUart( USED_UART, UART_RATE );		// UART for debug messages

    printf( WELCOME_MSG );

    BaseType_t res;

    // Crear tarea en freeRTOS
    for (uint32_t i = 0 ; i < LED_COUNT ; i++ )
    {
        res = xTaskCreate(
        		tarea_topo,                     // Funcion de la tarea a ejecutar
                  ( const char * )"tarea_topo",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
                  (uint32_t*)i,                          // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
                  &xHandle[i]                           // Puntero a la tarea creada en el sistema
              );
        // Gestion de errores
        configASSERT( res == pdPASS );
    }

       res = xTaskCreate(
    		   	   Tarea_Ppal,                     // Funcion de la tarea a ejecutar
                  ( const char * )"Tarea_Ppal",   // Nombre de la tarea como String amigable para el usuario
                  configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
				  0,               // Parametros de tarea
                  tskIDLE_PRIORITY+1,         // Prioridad de la tarea
				  &xHandle[LED_COUNT]                           // Puntero a la tarea creada en el sistema
              );

        // Gestion de errores
        configASSERT( res == pdPASS );
       


    // Inicializo driver de teclas
    keys_Init();


    /*	Cola que captará los martillazos */
    colaMartillazos = xQueueCreate(QUEUE_DEPTH, sizeof(t_key_data));

    /*	Assert!	*/
    configASSERT( colaMartillazos !=  NULL  );


   /*	Inicializar 4 colas para martillar	con sus respectivos assert	*/
    for (uint32_t i = 0 ; i <LED_COUNT ; i ++)
    {
    	matatopos[i] = xQueueCreate(QUEUE_DEPTH, sizeof(t_key_data));
    	configASSERT( matatopos [i]  !=  NULL  );
    }

    /*	Cola para informar puntajes!	con assert!	*/
	score = xQueueCreate(QUEUE_DEPTH, sizeof(int32_t));
    configASSERT( score !=  NULL  );

    /*	Suspender tasks, hasta que el driver de teclas las active [500 ms tecla apretada ] 	*/
   for (uint32_t i = 0;  i < LED_COUNT ; i++) vTaskSuspend(xHandle[i]);

    // Iniciar scheduler
    vTaskStartScheduler();					// Enciende tick | Crea idle y pone en ready | Evalua las tareas creadas | Prioridad mas alta pasa a running

    configASSERT( 0 );


    return TRUE;
}

/*==================[definiciones de funciones internas]=====================*/
void gpio_init( void )
{
    for( uint32_t i = 0 ; i < LED_COUNT; i++ )
    {
        gpioInit ( gpio_t[i], GPIO_OUTPUT );
    }
}



uint32_t whackamole_points_success( TickType_t tiempo_afuera,TickType_t tiempo_reaccion_usuario )
{

	//printf("TA: %d ; TR: %d \n \r",tiempo_afuera,tiempo_reaccion_usuario);

	return ( (uint32_t) (20  * (tiempo_afuera - tiempo_reaccion_usuario )/tiempo_afuera)  );

}

/**
   @brief devuelve el puntaje por haber perdido al mole

   @return int32_t
 */
int32_t whackamole_points_miss()
{
	return -10;
}

/**
   @brief devuelve el puntaje por haber martillado cuando no habia mole

   @return int32_t
 */
int32_t whackamole_points_no_mole()
{
	return -20;
}


/*	Tarea principal requisito del problema */
void Tarea_Ppal (void * taskParmPtr)
{

	  /*	Creación de una estructutura de tipo t_key_data */
	  t_key_data TecDriver;
	  /*	Variables para puntos	int32_t porque pueden ser negativos.*/
	  int32_t IncomingScore,acum_puntos=0;

	  uint32_t time_end_game=	GameTime+WAM_GAMEPLAY_TIMEOUT;

	  while (true)
	  {

		  /*	En el caso de que se detecte un dato en esta cola de driver de teclas enviarlo de inmediato	*/
		  if (pdTRUE==xQueueReceive(colaMartillazos,&TecDriver, 0) && (GameTime>=GAMEPLAY_TIMEOUT_IN_MS) )
		  {

			  /*	Al topo correspondiente	*/
			 xQueueSend(matatopos[TecDriver.index],&TecDriver,0);
		  }

		  /*	En el caso de recibir un dato por la cola de puntos, acumular y publicar por serial	*/
		  if (pdTRUE==xQueueReceive(score,&IncomingScore, 0))
		  {
			  acum_puntos += IncomingScore;

			  if (IncomingScore!=0)
			  {
				  taskENTER_CRITICAL();
				  printf("El puntaje acumulado es : %i \r\n",acum_puntos);
				  taskEXIT_CRITICAL();
			  }
		  }

		  if (GameTime >= time_end_game)
		  {
			  taskENTER_CRITICAL();
			  printf("El juego finalizo. El puntaje fue : %i \r\n",acum_puntos);
			  taskEXIT_CRITICAL();

			  for (uint32_t i = 0 ; i < LED_COUNT ; i++ ) vTaskDelete(xHandle[i]);
		  }
	  }
}



// Implementacion de funcion de la tarea
void tarea_topo( void* taskParmPtr )
{
	/*	Los topos saldrán [Leds prenderán]	conforme al casteo de esta tarea	*/
    uint32_t 	index = ( uint32_t ) taskParmPtr;

    /*	Variables de tiempo para los topos y de juego	*/
	TickType_t	tiempo_aparicion,tiempo_afuera, xInitCount=0, Gametime=0;

	t_key_data topoStruct= {0};
	random_seed_freertos();
	uint32_t time_end_game=	GameTime+WAM_GAMEPLAY_TIMEOUT;

	xQueueReset(matatopos[index]);

	/*Tiempo actual  <= 	20 segundos		*/
    while(GameTime <= time_end_game)
    {
	    /*	ThirdParty lib para randomizar tiempos	*/

		tiempo_aparicion = random( WAM_MOLE_SHOW_MIN_TIME, WAM_MOLE_SHOW_MAX_TIME );
		tiempo_afuera    = random( WAM_MOLE_OUTSIDE_MIN_TIME, WAM_MOLE_OUTSIDE_MAX_TIME );

		/*	Si recibo un martillazo desde driver de teclas, durante el tiempo en que el topo
		 *  se encuentra escondido, restar 20 puntos.	*/
		if(pdTRUE == xQueueReceive(matatopos[index],&topoStruct, tiempo_aparicion) )
		{
			taskENTER_CRITICAL();
			topoStruct.score =  whackamole_points_no_mole();
			taskEXIT_CRITICAL();
		}
		else //if(pdFALSE == xQueueReceive(matatopos[index],&topoStruct, tiempo_aparicion))
		{
			/*	Encender leds!	*/
			gpioWrite(leds_t[index], ON);
			gpioWrite(gpio_t[index], ON);


			/*	Comenzar la cuenta para determinar el tiempo de reacción	*/
			xInitCount = xTaskGetTickCount();
			if ( pdTRUE == xQueueReceive(matatopos[index], &topoStruct, tiempo_afuera) )
			{
				taskENTER_CRITICAL();
				topoStruct.time_diff = topoStruct.time_down - xInitCount ;		//	Calculos de tiempos reacción[mayor] - contador inicial [menor->Driver teclas]
				topoStruct.score = whackamole_points_success(tiempo_afuera,topoStruct.time_diff);
				printf("PRESIONADO! Tiempo de reaccion: %d ms  Tecla: %i \r\n",topoStruct.time_diff,topoStruct.index);
				taskEXIT_CRITICAL();

				/*	Apagar los leds de inmediato debido al martillazo	*/
				gpioWrite( leds_t[index], OFF );
				gpioWrite( gpio_t[index], OFF );
			}
			else
			{
				/*	Si estoy aquí significa que fallé al presionar la tecla y expiró el tiempo, resta 10	*/
				taskENTER_CRITICAL();
				topoStruct.score = whackamole_points_miss();
				printf(" Miss! -10 \r\n");
				taskEXIT_CRITICAL();
			}

		}
		/*	Enviar puntaje final	*/
		xQueueSend(score,&topoStruct.score,0);
		clear_diff(topoStruct.index);
		gpioWrite( leds_t[index], OFF );
		gpioWrite( gpio_t[index], OFF );
		GameTime= xTaskGetTickCount();

	}
   /*	Si estoy aquí significa que el tiempo de juego expiró, eliminar tareas con handler y apagar todos los leds	*/
   /*	Juego finalizado	*/

   }




