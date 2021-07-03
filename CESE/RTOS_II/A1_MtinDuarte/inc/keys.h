/*=============================================================================
 * Copyright (c) 2021, Martin Duarte <duartemartn@gmail.com>
 * 					   
 * All rights reserved.
 * License: Free
 * Date: XX/XX/XXXX
 * Version: v1.0
 *============================================================================*/

/*==================[ Inclusions ]============================================*/
#ifndef KEYS_H_
#define KEYS_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "sapi.h"
/*=====[Definition macros of private constants]==============================*/

#define DEBOUNCE_TIME   40
#define DEBOUNCE_TIME_TICKS     pdMS_TO_TICKS(DEBOUNCE_TIME)
#define KEYS_INVALID_TIME   -1

typedef enum
{
    STATE_BUTTON_UP,
    STATE_BUTTON_DOWN,
    STATE_BUTTON_FALLING,
    STATE_BUTTON_RISING
} keys_ButtonState_t;

typedef struct
{
    gpioMap_t tecla;			//config
} t_key_config;

typedef struct{
uint8_t evento;
uint8_t index;
uint32_t value;
}Parameters_t;

typedef struct
{
    keys_ButtonState_t state;   //variables

    TickType_t time_down;		//timestamp of the last High to Low transition of the key
    TickType_t time_up;		    //timestamp of the last Low to High transition of the key
    TickType_t time_diff;	    //variables
} t_key_data;

/* methods ================================================================= */
void keys_Init( void );
   
TickType_t get_diff( uint32_t index );
void clear_diff( uint32_t index );

#endif

/* public macros==============================================================*/

/* types =====================================================================*/

