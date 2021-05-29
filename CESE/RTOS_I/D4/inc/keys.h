#ifndef KEYS_H_
#define KEYS_H_

/* Include header files*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sapi.h"

/* public macros ================================================================= */
#define KEYS_INVALID_TIME   -1
/* types ================================================================= */

/* Debouncing corrector states   */
typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} keys_ButtonState_t;

/* Structure for switch */
typedef struct
{
    gpioMap_t tecla;	//config
} t_key_config;

typedef struct
{
    keys_ButtonState_t state;   //variables

    TickType_t time_down;		//timestamp of the last High to Low transition of the key
    TickType_t time_up;		    //timestamp of the last Low to High transition of the key
    TickType_t time_diff;	    //variables

    SemaphoreHandle_t sem_tec_pulsada; //semaforo
} t_key_data;

/* methods ================================================================= */

/* Initialization of gpios */
void gpio_init( void );
void keys_Init( void );

/* Declaration of time functions */
TickType_t get_diff();
void clear_diff();

/* task function prototypes */
void tarea_led( void* taskParmPtr );
void tarea_tecla( void* taskParmPtr );



#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */
