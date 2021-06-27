/*
 * utypes.h
 *
 *  Created on: 14 jun. 2021
 *      Author: martin
 */

#ifndef CESE_PCSE_PMIC_INC_UTYPES_H_
#define CESE_PCSE_PMIC_INC_UTYPES_H_

#include "stdint.h"
#include "uart.h"

#define BYTE_COUNT_SIZE		2
#define FIFO_BUFFER			6
#define UART2_MAX_LEN  		8096
#define FRAME_HIGH_HEADER   0x5A
#define FRAME_LOW_HEADER	0xA5
//<Frame Header H>	<Frame Header L>	<Byte Count>	<Command>	[<Data>...]	[<CRC H> <CRC L>]





typedef struct{
	//uint16_t header = 0x5AA5;
	uint8_t	 highbyteCount
			,lowbyteCount
			,highCmdId
			,lowCmdId
			,highPID
			,lowPID
			,highregadd
			,lowregadd
			,highvalue
			,lowvalue
			,crc;
}frame_t;

frame_t rxPacket, txPacket;




#endif /* CESE_PCSE_PMIC_INC_UTYPES_H_ */
