/*
 * uart.h
 *
 *  Created on: 8 jun. 2021
 *      Author: martin
 */

#ifndef CESE_PCSE_PMIC_4_COMUNICLAYER_UART_H_
#define CESE_PCSE_PMIC_4_COMUNICLAYER_UART_H_

#include "sapi.h"
#include "utypes.h"
#include "string.h"
#include "stdint.h"

bool_t uartSendByte(uint8_t sendingByte);
bool_t uartRecieveByte();



void headerDetect( void * Param);

void uartCallbackSet( uartMap_t uart, uartEvents_t event,
                      callBackFuncPtr_t callbackFunc, void* callbackParam );

void printInit( print_t printer, uartMap_t uart, uint32_t baudRate );

bool_t g_uartInitflag;
bool_t uartInitialization(void);


#endif /* CESE_PCSE_PMIC_4_COMUNICLAYER_UART_H_ */


