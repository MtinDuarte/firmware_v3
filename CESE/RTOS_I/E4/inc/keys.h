/*=============================================================================
 * Copyright (c) 2021, Franco Bucafusco <franco_bucafusco@yahoo.com.ar>
 * 					   Martin N. Menendez <mmenendez@fi.uba.ar>
 * All rights reserved.
 * License: Free
 * Date: 2021/10/03
 * Version: v1.2
 *===========================================================================*/

#ifndef KEYS_H_
#define KEYS_H_

#include "FreeRTOS.h"
#include "sapi.h"

/* public macros ================================================================= */

#if MODO==0
/* con problemas */
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START
#define CRITICAL_END
#endif
#if MODO==1
/* enter y exit critical */
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START      taskENTER_CRITICAL();
#define CRITICAL_END        taskEXIT_CRITICAL();
#endif
#if MODO==2
/* suspend / resume all  */
#define CRITICAL_DECLARE
#define CRITICAL_CONFIG
#define CRITICAL_START      vTaskSuspendAll();
#define CRITICAL_END        xTaskResumeAll();
#endif
#if MODO==3
/* mutex  */
#define CRITICAL_DECLARE    SemaphoreHandle_t mutex
#define CRITICAL_CONFIG     mutex = xSemaphoreCreateMutex(); \
                            configASSERT( mutex != NULL );
#define CRITICAL_START      xSemaphoreTake( mutex , portMAX_DELAY )
#define CRITICAL_END        xSemaphoreGive( mutex )
#endif

#define KEYS_INVALID_TIME   -1
/* types ================================================================= */
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

typedef struct
{
    keys_ButtonState_t state;   //variables

    uint32_t time;			//timestamp of the last High to Low transition of the key

} t_key_data;

/* methods ================================================================= */
void keys_init       (void*    taskParmPtr);
void keys_get_diff   (uint32_t *contador  );
void keys_clear_diff (uint32_t index      );

#endif /* PDM_ANTIRREBOTE_MEF_INC_DEBOUNCE_H_ */