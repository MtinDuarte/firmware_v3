/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.0
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#include "keys.h"

/*=====[ Definitions of private data types ]===================================*/


/*=====[Definition macros of private constants]==============================*/

/*=====[Prototypes (declarations) of private functions]======================*/


/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of public global variables]==============================*/
keys_ButtonState_t state;   //state of the key
tick_t time_down;		//timestamp of the last High to Low transition of the key
tick_t time_up;		    //timestamp of the last Low to High transition of the key
tick_t time_diff;	    //las time difference

const gpioMap_t tecla = TEC1;

/*=====[prototype of private functions]=================================*/


/*=====[Implementations of public functions]=================================*/
/**
   @brief Obtiene el tiempo de la ultima pulsacion de la tecla indicada.

   @param index
   @return TickType_t
 */
tick_t keys_get_diff(  )
{
    return time_diff;
}

/**
   @brief Borra el tiempo de la ultima pulsacion de la tecla indicada.

   @param index
 */
void keys_clear_diff(  )
{
    time_diff = KEYS_INVALID_TIME;
}

/**
   @brief Inicializa el driver

 */
void keys_init( void )
{
    state          = BUTTON_UP;  // Set initial state
    time_down      = KEYS_INVALID_TIME;
    time_up        = KEYS_INVALID_TIME;
    time_diff      = KEYS_INVALID_TIME;
}

// keys_ Update State Function
/**
   @brief Keys SM for polled operation

 */
uint32_t  keys_update( )
{
    uint32_t event = KEYS_EVENT_NONE ;

    switch( state )
    {
        case STATE_BUTTON_UP:
            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( tecla ) )
            {
                state = STATE_BUTTON_FALLING;
            }
            break;

        case STATE_BUTTON_FALLING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */
            if( !gpioRead( tecla ) )
            {
                state = STATE_BUTTON_DOWN;

                /* ACCION DEL EVENTO !*/
                event = KEYS_EVENT_KEY_DOWN ;

                time_down = tickRead();
            }
            else
            {
                state = STATE_BUTTON_UP;
            }

            /* LEAVE */
            break;

        case STATE_BUTTON_DOWN:
            /* CHECK TRANSITION CONDITIONS */
            if( gpioRead( tecla ) )
            {
                state = STATE_BUTTON_RISING;
            }
            break;

        case STATE_BUTTON_RISING:
            /* ENTRY */

            /* CHECK TRANSITION CONDITIONS */

            if( gpioRead( tecla ) )
            {
                state = STATE_BUTTON_UP;

                /* ACCION DEL EVENTO ! */
                event = KEYS_EVENT_KEY_UP ;

                tick_t current_tick_count = tickRead();

                /* calculo el tiempo de pulsacion */
                if( time_down < current_tick_count )
                {
                    time_diff  = current_tick_count - time_down;
                }
                else
                {
                    time_diff  = current_tick_count - time_down + 0xFFFFFFFF;
                }
            }
            else
            {
                state = STATE_BUTTON_DOWN;
            }

            /* LEAVE */
            break;
    }

    return event;
}

/*=====[Implementations of private functions]================================*/

/*=====[Implementations of private functions]=================================*/

