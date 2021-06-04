/*=============================================================================
 * Copyright (c) 2021, Martin Duarte <duartemartn@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: XX/XX/XXXX
 * Version: v1.0
 *===========================================================================*/

/*==================[inlcusiones]============================================*/
#include "keys.h"

//#include "cooperative_os_scheduler_03_seos_Pont2014.h"   // <= own header (optional)
#include "sapi.h"        // <= sAPI header
#include "seos_pont_2014_isr.h"       // <= dispatcher and task management header
#include "seos_pont_2014_scheduler.h" // <= scheduler and system initialization header
#define LED_PERIOD 2000
/*==================[definiciones y macros]==================================*/
typedef enum
{
    STATE_OFF,
    STATE_ON
} led_state_t;

/*==================[definiciones de datos internos]=========================*/
led_state_t led_state;

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

/*==================[funcion principal]======================================*/

int main( void )
{
   /*   Board initialization */
    boardConfig();

    /* Initialize scheduler */
    schedulerInit();

    /* Initialize keys first state */
    keys_init( );
   
    led_state = STATE_OFF;

    /* Add a periodically task to the scheduler */
    
   schedulerAddTask( task_tecla,   // -> void (* pFunction)(void)   <- | Void pointer to function |
                      0,            // -> Parameters of the task     <- No parameters for the task
                      0,            // -> Execution offset in ticks  <- No offset to be executed
                      DEBOUNCE_TIME // -> Value of the period        <- Equal to 40ms
                    );
   
      schedulerAddTask( task_led,   // -> void (* pFunction)(void)   <- | Void pointer to function |
                      0,            // -> Parameters of the task     <- No parameters for the task
                      0,            // -> Execution offset in ticks  <- No offset to be executed
                      LED_PERIOD // -> Value of the period        <- Equal to 40ms
                    );

   
    /*   Execute interrupt with period of 1ms to check ready tasks  */
    schedulerStart( 1 );

    while( TRUE )
    {
        /*  The tasks marked as ready are executed.
            Then the system goes low consumption waiting for the next interrupt. */
        
        schedulerDispatchTasks();
    }

    return 0;
}

/*==================[definiciones de funciones internas]=====================*/
void task_led( void* param )
{
    if( led_state == STATE_OFF )
    {
        tick_t key_time_diff = keys_get_diff( );
       
       /*   Set bound to key_time_diff to provent breaking the period   */
       if ( (key_time_diff == KEYS_INVALID_TIME) ||
            (key_time_diff >= LED_PERIOD))
       
       /*   Limit to T-100ms  */
       key_time_diff = LED_PERIOD-100;
       
       
        /*  Toggle blue led */
        gpioToggle( LEDB );
       
        /* Change the state of led enumeration */
        led_state = STATE_ON;
       
        /* Add shutdown task of the led */
        schedulerAddTask( task_led,      // -> void (* pFunction)(void)   <- | Void pointer to function |  
                          0,             // -> Parameters of the task     <- No parameters for the task
                          key_time_diff, // -> Execution offset in ticks  <- Offset will be button pressed time.
                          0              //  -> Value of the period       <- One shot task. No periodicity.
                        );
    }
        else if( led_state == STATE_ON )
    {
        /* Toggle blue led */
        gpioToggle( LEDB );

        /* Change enumeration variable = led state */
        led_state = STATE_OFF;

        /*  Clear time registered while button pressed */
        //keys_clear_diff();
    }

    
}
/*==================[definiciones de funciones externas]=====================*/
void task_tecla( void* param )
{
    event = keys_update(  );

    if( event == KEYS_EVENT_KEY_DOWN )
    {
        /* Do nothing */
    }
    else if( event == KEYS_EVENT_KEY_UP )
    {
        /*  Add another task to the scheduler                                                    */
        schedulerAddTask(  task_led,         /* Create task called "Led-Task'                    */
                           0,                /* No parameters for the task                       */
                           0,                /* Execute according to time while button pressed   */
                           0                 /* Period of 1 second                               */
                        );
    }
}

/*==================[fin del archivo]========================================*/
