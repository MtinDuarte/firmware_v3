/*
 * read_parser.c
 *
 *  Created on: 8 jun. 2021
 *      Author: martin
 */

#include "read_parser.h"


frame_t* p_rxPacket = &rxPacket;

static frame_t inFrameStruct;


char ACK_HEADER[4] = {'4','F','4','B'};


frame_t getRxFrameStruct( void )
{
	return rxPacket;
}


void incomingPacketParser ( void )
{
	inFrameStruct = getRxFrameStruct();

	if ( (inFrameStruct.highCmdId == '8') && (inFrameStruct.lowCmdId=='0' ) )
	{
  		 uartWriteString( UART_USB," Write register instruction \n\r");

  		 buildACK();
	}

}


void buildACK(void)
{


	char Tx_vec[13] = {'5','A','A','5','0','3','8','0','4','F','4','B','\n'};

	uartWriteString(UART_USB,Tx_vec);

			/**
	txPacket.highbyteCount=	0;
	txPacket.lowbyteCount=	4;
	txPacket.highCmdId=		8;
	txPacket.lowCmdId=		1;
	txPacket.highPID=		0;
	txPacket.lowPID=		0;
	txPacket.highregadd=	0;
	txPacket.lowregadd=		1;
	txPacket.highvalue=		0;
	txPacket.lowvalue=		1;
**/
}






