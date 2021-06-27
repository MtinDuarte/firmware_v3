

/*==================[inclusions]=============================================*/


#include "uart.h"



int main(void){

	boardConfig();

	if ( !uartInitialization() )
		/*	Initialization flag	*/
		return -1;


   while(true)
   {

	   uartWriteString(UART_USB, "Welcome to my PCSE project! [MtinDuarte 2021] \n\r");
	   delay(60000);
   }

   	   return 0;

}
