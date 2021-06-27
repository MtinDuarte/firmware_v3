/*
 * app.c
 *
 *  Created on: 8 jun. 2021
 *      Author: martin
 */
#include "app.h"


char test_writeString[20]= "PCSE 2021 Project...";
char test_writeByte[1]='A';
char test_readstring[];
char test_readByte[1];

char dato[];

void runTasks(void)
{


    if(  uartReadByte( UART_USB, &dato ) )
    {


    }


    uartWriteByte( UART_USB, dato );




		/*	Set the time */
		read_Buffer(&test_readstring, sizeof(test_readstring))
		{

		}

		write_Buffer(&test_writeString,sizeof(test_writeString));
		{

		}

}


void main()
{
	while(1)
	{
		runTasks():
	}
}

