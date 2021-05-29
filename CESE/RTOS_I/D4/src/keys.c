/*==================[ Inclusions ]============================================*/
#include "keys.h"

/*=====[ Definitions of private data types ]===================================*/
const t_key_config  keys_config[] = { TEC1 };
#define KEY_COUNT 1

/*=====[Definition macros of private constants]==============================*/
/* Debounce time to 40 ms  */
#define DEBOUNCE_TIME   40
/* Convert 40ms to ticks   */
#define DEBOUNCE_TIME_MS pdMS_TO_TICKS(DEBOUNCE_TIME)
/*=====[Prototypes (declarations) of private functions]======================*/


static void keys_ButtonError( uint32_t index );
static void buttonPressed( uint32_t index );
static void buttonReleased( uint32_t index );

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of public global variables]==============================*/
t_key_data keys_data[KEY_COUNT];
extern SemaphoreHandle_t sem_btn;
/*=====[prototype of private functions]=================================*/
void task_tecla( void* taskParmPtr );

/*=====[Implementations of public functions]=================================*/
TickType_t get_diff( uint32_t index )
{
    TickType_t tiempo;

    taskENTER_CRITICAL();
    tiempo = keys_data[index].time_diff;
    taskEXIT_CRITICAL();

    return tiempo;
}

void clear_diff( uint32_t index )
{
    taskENTER_CRITICAL();
    keys_data[index].time_diff = KEYS_INVALID_TIME;
    taskEXIT_CRITICAL();
}

void keys_Init( void )
{
    for (uint32_t i = 0 ; i < KEY_COUNT ; i++ )
    {
       /* Initialization states  */
        keys_data[i].state          = BUTTON_UP;  
        keys_data[i].time_down      = KEYS_INVALID_TIME;
        keys_data[i].time_up        = KEYS_INVALID_TIME;
        keys_data[i].time_diff      = KEYS_INVALID_TIME;
    }
    
    BaseType_t res;
    // Create a task
    res = xTaskCreate (
              task_tecla,					      // -> void (* pFunction)(void)      <- | Void pointer to function |
              ( const char * )"task_tecla",	// -> (const char *) "string"       <-
              configMINIMAL_STACK_SIZE*2,	   // -> Stack necessary for the task  <-   
              0,							         // -> Parameters of the task        <- No need of parameters
              tskIDLE_PRIORITY+1,			   // -> Priority of the task          <-
              0							         // -> Pointer to the task           <-
          );

    /*   Verify if the task was correctly created. */
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

/* Do this if the key was pressed   */

static void buttonPressed( uint32_t index )
{
    TickType_t current_tick_count = xTaskGetTickCount();
    
   /*   Secure current tick count assign */
    taskENTER_CRITICAL();
    keys_data[index].time_down = current_tick_count;
    taskEXIT_CRITICAL();
}

   /* Button released function   */
static void buttonReleased( uint32_t index )
{
    TickType_t current_tick_count = xTaskGetTickCount();

    taskENTER_CRITICAL();
    keys_data[index].time_up    = current_tick_count;
    keys_data[index].time_diff  = keys_data[index].time_up - keys_data[index].time_down;
    taskEXIT_CRITICAL();

    /*   If any key was pressed for a time more than 0, give semaphore .   */
    if ( keys_data[index].time_diff  > 0 )
    {
        xSemaphoreGive( sem_btn );
    }
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
