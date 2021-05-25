/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.0
 *===========================================================================*/

#include "keys.h"

// The maximum number of tasks required at any one time during the execution
// of the program. MUST BE ADJUSTED FOR EACH NEW PROJECT
#define SCHEDULER_MAX_TASKS   (3)

/*==================[inlcusiones]============================================*/

//#include "cooperative_os_scheduler_03_seos_Pont2014.h"   // <= own header (optional)
#include "sapi.h"        // <= sAPI header

#include "seos_pont_2014_isr.h"       // <= dispatcher and task management header
#include "seos_pont_2014_scheduler.h" // <= scheduler and system initialization header

/*==================[definiciones y macros]==================================*/
typedef enum
{
    STATE_OFF,
    STATE_ON
} led_state_t;

#define DEFAULT_PERIOD  1000

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/
uint32_t led_state;

/*==================[declaraciones de funciones externas]====================*/
void task_tecla( void* param );
void task_led( void* param );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------
    // Inicializar y configurar la plataforma
    boardConfig();

    /* inicializo el planificador */
    schedulerInit();

    /* inicializo el modulo de tecla */
    keys_init( );
    led_state = STATE_OFF;

    /* agregamos al planificador una tarea que "consulte" el estado de la tecla,
       de manera periodica */
    schedulerAddTask( task_tecla,   // funcion de tarea a agregar
                      0,            // parametro de la tarea
                      0,            // offset de ejecucion en ticks
                      DEBOUNCE_TIME // periodicidad de ejecucion en ticks
                    );

    /* planifico que la tarea de LED se ejecute en 0 ticks */
    schedulerAddTask(  task_led,      // funcion de tarea a agregar
                       0,             // parametro de la tarea
                       0,             // offset -> 0 = "ejecutate inmediatamente"
                       DEFAULT_PERIOD // periodicidad de ejecucion en ticks
                    );

    // FUNCION que inicializa la interrupcion que ejecuta el planificador de
    // tareas con tick cada 1ms.
    schedulerStart( 1 );

    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE )
    {
        // Se despachan (ejecutan) las tareas marcadas para su ejecucion.
        // Luego se pone el sistema en bajo consumo hasta que ocurra la
        // proxima interrupcion, en este caso la de Tick.
        // Al ocurrir la interrupcion de Tick se ejecutara el planificador
        // que revisa cuales son las tareas a marcar para su ejecucion.
        schedulerDispatchTasks();
    }

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return 0;
}

/*==================[definiciones de funciones internas]=====================*/
void task_led( void* param )
{
    if( led_state == STATE_OFF )
    {
        tick_t key_time_diff = keys_get_diff( );

        if(  key_time_diff==KEYS_INVALID_TIME || key_time_diff>= DEFAULT_PERIOD )
        {
            //uso el DEFAULT_PERIOD/2
            key_time_diff = DEFAULT_PERIOD/2 ;
        }

        /* toggle del led */
        gpioToggle( LEDB );

        /* cambio de estado al led */
        led_state = STATE_ON;

        /* planifico el apagado del led */
        schedulerAddTask( task_led,               // funcion de tarea a agregar
                          0,                    // parametro de la tarea
                          key_time_diff,        // offset de ejecucion en ticks
                          0                     // periodicidad de ejecucion en ticks
                        );
    }
    else if( led_state == STATE_ON )
    {
        /* toggle del led */
        gpioToggle( LEDB );

        /* cambio de estado al led */
        led_state = STATE_OFF;

    }
}

void task_tecla( void* param )
{
    uint32_t event = keys_update( );

    if( event == KEYS_EVENT_KEY_DOWN )
    {
        /* no hago nada */
    }
    else if( event == KEYS_EVENT_KEY_UP )
    {
        /* no hago nada */
    }
}
/*==================[definiciones de funciones externas]=====================*/


/*==================[fin del archivo]========================================*/
