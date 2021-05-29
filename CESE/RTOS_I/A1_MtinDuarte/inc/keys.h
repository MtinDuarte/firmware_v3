/*==================[ Inclusions ]============================================*/
#ifndef KEYS_H_
#define KEYS_H_
#include "sapi.h"

/* Define as a macro the initial state */
#define KEYS_INVALID_TIME   -1

/* Define debounce time macro to 40 ms */
#define DEBOUNCE_TIME       40

/* Define the states of the button while pressed. This states are for debouncing button corrector. */
typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} keys_ButtonState_t;


typedef enum
{
   KEYS_EVENT_NONE,
   KEYS_EVENT_KEY_DOWN,
   KEYS_EVENT_KEY_UP,
}Create_event_variable;

Create_event_variable event;

/* methods ================================================================= */

tick_t keys_get_diff(  );
void keys_clear_diff(  );

void keys_init( void );

uint32_t  keys_update( );


#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
