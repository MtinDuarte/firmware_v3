/*=============================================================================
 * Copyright (c) 2021, Martin Duarte
 * All rights reserved.
 * License: Free
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#include "keys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "sapi.h"

/*=====[ Definitions of private data types ]===================================*/
const t_key_config  keys_config[] = { TEC1 , TEC2, TEC3, TEC4};

#define LED_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
extern TickType_t GameTime;
extern TaskHandle_t xHandle[4];

#define GAMEPLAY_TIMEOUT_IN_MS		500
#define GAMEPLAY_TIMEOUT_IN_MS_TICKS pdMS_TO_TICKS(GAMEPLAY_TIMEOUT_IN_MS)

#define KEY_COUNT   sizeof(keys_config)/sizeof(keys_config[0])
/*=====[Definition macros of private constants]==============================*/
#define DEBOUNCE_TIME   40
#define DEBOUNCE_TIME_MS pdMS_TO_TICKS(DEBOUNCE_TIME)
/*=====[Prototypes (declarations) of private functions]======================*/

static void keys_ButtonError( uint32_t index );
static void buttonPressed	( uint32_t index );
static void buttonReleased	( uint32_t index );

/*=====[Definitions of private global variables]=============================*/

t_key_data keys_data[KEY_COUNT];

/*	Recursos declarados en otros archivos	*/
extern QueueSetHandle_t colaMartillazos;

/*=====[prototype of private functions]=================================*/
void task_tecla( void* taskParmPtr );

void clear_diff( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].time_diff = KEYS_INVALID_TIME;
    taskEXIT_CRITICAL();
}

void keys_Init( void )
{
    BaseType_t res;
    uint32_t i;

    for ( i = 0 ; i < KEY_COUNT ; i++ )
    {
        keys_data[i].state          = BUTTON_UP;  // Set initial state
        keys_data[i].time_down      = KEYS_INVALID_TIME;
        keys_data[i].time_up        = KEYS_INVALID_TIME;
        keys_data[i].time_diff      = KEYS_INVALID_TIME;
    }
    // Crear tareas en freeRTOS
    res = xTaskCreate (
              task_tecla,					// Funcion de la tarea a ejecutar
              ( const char * )"task_tecla",	// Nombre de la tarea como String amigable para el usuario
              configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
              0,							// Parametros de tarea
              tskIDLE_PRIORITY+1,			// Prioridad de la tarea
              0								// Puntero a la tarea creada en el sistema
          );

    // Gestión de errores
    configASSERT( res == pdPASS );
}

// keys_ Update State Function
void keys_Update( uint32_t index )
{
    switch( keys_data[index].state )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_DOWN;

                /* ACCION DEL EVENTO !*/
                buttonPressed( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_UP;
            }

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:
            /* CHECK TRANSITION CONDITIONS */
            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_RISING;
            }
            break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( gpioRead( keys_config[index].tecla ) )
            {
                keys_data[index].state = STATE_BUTTON_UP;

                /* ACCION DEL EVENTO ! */
                buttonReleased( index );
            }
            else
            {
                keys_data[index].state = STATE_BUTTON_DOWN;
            }

            /* LEAVE */
            break;

        default:
            keys_ButtonError( index );
            break;
    }
}

/*=====[Implementations of private functions]================================*/

/* Acción del evento tecla pulsada	*/
static void buttonPressed( uint32_t index )
{
	/*	Proteger las variables de tiempo	*/
	taskENTER_CRITICAL();
	/*	Tomar el tiempo desde aquí y guardarlo en la estructura keys_data	*/
    TickType_t current_tick_count = xTaskGetTickCount();
    keys_data[index].time_down= current_tick_count;
    keys_data[index].index=index;
    taskEXIT_CRITICAL();

}

/* accion de el evento de tecla liberada */
static void buttonReleased( uint32_t index )
{
	/*	Se utilizará una flag local con static, para recordar el estado anterior	*/
	static bool_t SuspendON=true;
	/*	Proteger las variables	*/

	taskENTER_CRITICAL();
	TickType_t current_tick_count = xTaskGetTickCount();
	keys_data[index].time_up    = current_tick_count;
	keys_data[index].time_diff  =  keys_data[index].time_up - keys_data[index].time_down;
    taskEXIT_CRITICAL();

    xQueueSend(colaMartillazos,&keys_data[index], 0);
    /*	Si las tareas no estas suspendidas y la diferencia de tiempo de botón apretado es mayor que 500ms	*/
    if ( ( true== SuspendON)  && (keys_data[index].time_diff > GAMEPLAY_TIMEOUT_IN_MS_TICKS) )
    	{
    		SuspendON=false;
    		GameTime= xTaskGetTickCount();
    		/*	Resumir todas las tareas puestas en pausa	*/
    		for (uint32_t i = 0;  i < LED_COUNT ; i++) vTaskResume(xHandle[i]);
    	}

    /*	En el caso de haber detectado un martillazo, enviar el dato por colaMartillazo inmediatamente	*/

}

static void keys_ButtonError( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].state = BUTTON_UP;
    taskEXIT_CRITICAL();
}

/*=====[Implementations of private functions]=================================*/
void task_tecla( void* taskParmPtr )
{
    uint32_t i;
    while( TRUE )
    {
        for ( i = 0 ; i < KEY_COUNT ; i++ )
        {
            keys_Update( i );
        }
        vTaskDelay( DEBOUNCE_TIME_MS );
    }
}
