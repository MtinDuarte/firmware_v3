/*=============================================================================
 * Copyright (c) 2021, Martin Duarte <duartemartn@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: XX/XX/XXXX
 * Version: v1.0
 *===========================================================================*/



#include "keys.h"

/* Define the number of TASKS adjusted for the execution of the project.*/
#define SCHEDULER_MAX_TASKS   (3)


//#include "cooperative_os_scheduler_03_seos_Pont2014.h"   // <= own header (optional)
#include "sapi.h"        // <= sAPI header


/* Dispatcher and task management header  */
#include "seos_pont_2014_isr.h"       

/* Scheduler and system initialization header */
#include "seos_pont_2014_scheduler.h" 

/* Define enumeration of led state  */
typedef enum
{
    STATE_OFF,
    STATE_ON
} led_state_t;


/*==================[declaraciones de funciones internas]====================*/
led_state_t led_state;

/*==================[declaraciones de funciones externas]====================*/
void task_tecla( void* param );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
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


void task_led( void* param )
{
    if( led_state == STATE_OFF )
    {
        tick_t key_time_diff = keys_get_diff( );

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
        keys_clear_diff();
    }
}

void task_tecla( void* param )
{
    event = keys_update( );

    if( event == KEYS_EVENT_KEY_DOWN )
    {
        /* Do nothing */
    }
    else if( event == KEYS_EVENT_KEY_UP )
    {
        /*  Add another task to the scheduler                              */
        schedulerAddTask(  task_led,      /* Create task called "Led-Task' */
                           0,             /* No parameters for the task    */
                           0,             /* Execute the task inmediatly   */
                           0              /* One shot task                 */
                        );
    }
}