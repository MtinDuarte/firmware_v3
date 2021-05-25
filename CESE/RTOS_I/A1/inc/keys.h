/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.0
 *===========================================================================*/

/*==================[ Inclusions ]============================================*/
#ifndef KEYS_H_
#define KEYS_H_
#include "sapi.h"

/* public macros ================================================================= */
#define KEYS_INVALID_TIME   -1
#define DEBOUNCE_TIME       40

/* types ================================================================= */
typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} keys_ButtonState_t;

#define KEYS_EVENT_NONE     0
#define KEYS_EVENT_KEY_DOWN 1
#define KEYS_EVENT_KEY_UP   2

/* methods ================================================================= */

tick_t keys_get_diff(  );
void keys_clear_diff(  );

void keys_init( void );



#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
