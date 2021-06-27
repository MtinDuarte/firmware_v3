/*
 * read_parser.h
 *
 *  Created on: 9 jun. 2021
 *      Author: martin
 */

#ifndef CESE_PCSE_PMIC_3_FUNCLAYER_READ_PARSER_H_
#define CESE_PCSE_PMIC_3_FUNCLAYER_READ_PARSER_H_

#include "utypes.h"



frame_t getRxFrameStruct( void );


void incomingPacketParser ( void );


void buildACK(void);
char* CharShift_x4(char* Vec , uint32_t len );

#endif /* CESE_PCSE_PMIC_3_FUNCLAYER_READ_PARSER_H_ */
