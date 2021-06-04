/*=============================================================================
 * Copyright (c) 2021, Martin Duarte <duartemartn@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: XX/XX/XXXX
 * Version: v1.0
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/

#include "sapi.h"
/*=====[ Definitions of private data types ]===================================*/
#include "keys.h"

/*=====[Definition macros of private constants]==============================*/

keys_ButtonState_t state;        /* Initialization of button state variable   */
tick_t time_down;                /* timestamp of the last High to Low transition of the key */
tick_t time_up;		            /* timestamp of the last Low to High transition of the key */
tick_t time_diff;	               /* last time difference */

const gpioMap_t tecla = TEC1;

/*=====[Prototypes (declarations) of private functions]======================*/

/* Return the difference of the time */
tick_t keys_get_diff(  )
{
    return time_diff;
}

/* Clear the time difference buffer */
void keys_clear_diff(  )
{
    time_diff = KEYS_INVALID_TIME;
}


void keys_init( void )
{
    state          = STATE_BUTTON_UP;           /* Set initial state of the button */
    time_down      = KEYS_INVALID_TIME;   /* Set first time pressed buffer to -1 */
    time_up        = KEYS_INVALID_TIME;   /* Set time when released buffer to -1 */
    time_diff      = KEYS_INVALID_TIME;   /* Set difference time buffer to -1 */
}


/* State machine for debouncing corrector */

uint32_t  keys_update( )
{
    event = KEYS_EVENT_NONE;

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

                /* Change event to button pressed  */
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

                /* Change the event to button released */
                event = KEYS_EVENT_KEY_UP ;

                tick_t current_tick_count = tickRead();

                /* Calculate the time of button while pressed */
                if( time_down < current_tick_count )
                {
                    time_diff  = current_tick_count - time_down;
                }
                else
                {  /*                                           -    1        */
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

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of public global variables]==============================*/

/*=====[Prototype of private functions]=================================*/

/*=====[Implementations of public functions]=================================*/

/*=====[Implementations of private functions]================================*/

/*=====[Implementations of private functions]=================================*/

