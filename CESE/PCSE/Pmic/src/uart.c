/*
 * uart.c
 *
 *  Created on: 8 jun. 2021
 *      Author: martin
 */


/**
A Frame (or packet) structure follows this format:
<Frame Header H>	<Frame Header L>	<Byte Count>	<Command>	[<Data>...]	[<CRC H> <CRC L>]

Or, in abbreviated notation:
<FHH> <FHL> <BC> <CMD> [<DATA>...] [<CRCH> <CRCL>]

Frame Header: Is used to identify a new incoming package, and is used to identify the product.
Byte Count  : This field resumes the size of the packet excluding the header and this byte [Counting starts from command byte]
Command		: Defines the command to be executed
Data		: Include the address, lengths and values
CRC			: Optional if needed.
**/


/*	Necessary header files	*/
#include "uart.h"
#include "read_parser.h"

const char COMPLETE_HEADER[4] = {'5','A','A','5'};




char       rxHeader[FIFO_BUFFER],rxData[15];

char	   *ptrHead=rxHeader , *ptrTail=rxHeader, *p_rxData=rxData;

static 		bool_t g_PackinRx;


void headerDetect( void * Param)
{

	char	   R8data= uartRxRead(UART_USB);

	static uint32_t i,m=0;

	uint16_t   byte_count=0;

	uint32_t   hdrflag = -1;

	   if (true == g_PackinRx)
	   {
		   /*	Save incoming data	*/
			   *p_rxData = R8data;

			    switch(m)
			    {
			    case 0:
			    	rxPacket.highCmdId	=	R8data;		// High byte command ID value,
			    	break;
			    case 1:
			    	rxPacket.lowCmdId	=	R8data;		// Low byte command ID value.
			    	break;
			    case 2:
			    	rxPacket.highPID	=	R8data;		// High byte ID page.
			    	break;
			    case 3:
			    	rxPacket.lowPID		=	R8data;		// Low byte ID page.
			    	break;
			    case 4:
			    	rxPacket.highregadd =	R8data;		// High byte register address
			    	break;
			    case 5:
			    	rxPacket.lowregadd  =	R8data;		// Low byte register address
			    	break;
			    case 6:
			    	rxPacket.highvalue	=	R8data;		// High byte data value
			    	break;
			    case 7:
			    	rxPacket.lowvalue	=	R8data;		// Low byte data value
			    	incomingPacketParser();
			    	break;
			    default:								// Nothing to do here.
			    	break;

			    }

			    m++;

			    p_rxData++;								//Advance vector

			    //verify end of packet
			    g_PackinRx = (m >= 2*(rxPacket.highbyteCount + rxPacket.lowbyteCount)) ? false : true ;

	   }
	   else
	   {
		   /*	Load next value	, header not detected	*/
		  *ptrHead = R8data;
		   ptrHead++;
		   i++;
	   }


       if(i >= FIFO_BUFFER)
     {
      /*	Enter here if only there are enough bits to be compare as header	*/


      /*	strcmp returns 0 when both strings are equal	*/
    	 hdrflag = strncmp(rxHeader,COMPLETE_HEADER,FIFO_BUFFER-BYTE_COUNT_SIZE);

       	 if (0 == hdrflag )
       	 {
       		/*	Reset iteration variable	*/
       		 i=0;

        		/*	Reset head - tail pointers to the start of the array	*/
       		 ptrTail=ptrHead=rxHeader;

       		 /*	Packet arrived signaling! */
       		 uartWriteString( UART_USB,"Packet Arrived");
       		 uartWriteString( UART_USB, "\r\n" );


       		/*	From char to int to calculate byte count len	*/

       		rxPacket.highbyteCount   =  rxHeader[4];

       		rxPacket.lowbyteCount    =  rxHeader[5];

       		g_PackinRx=true;

       	 }
       	 else
       	 {
       		 i=1;
       		do
       		{
       			/*	Shift 1 element in the array	*/
       			*ptrTail = *(ptrTail+i);
       			/*	Advance pointer to shift the next element	*/
       			ptrTail++;

       		}
       		while(ptrTail<ptrHead);

       		/*	Reset Head pointer position to memory add of last element -1*/
       		ptrHead=rxHeader+(FIFO_BUFFER-1);

       		/*	Set tail pointer to the start of the array	*/
          	ptrTail=rxHeader;

          	/*	Advance iteration variable	to check immediately */
          	i=FIFO_BUFFER-1;
       	 }
       }
 }


bool_t uartInitialization(void)
{

	g_uartInitflag = true;
	/*	Set baudrate	*/
	uartInit(UART_USB,115200);

	/* Set callback event and enable interrupt	 */
	uartCallbackSet(UART_USB,UART_RECEIVE,headerDetect,NULL);
	/* Enable all interrupts UART_USB */
	uartInterrupt(UART_USB, true);

	uartWriteString(UART_USB, "UART initialized \n\r");

	return g_uartInitflag;
}

/*	Global functions	*/

