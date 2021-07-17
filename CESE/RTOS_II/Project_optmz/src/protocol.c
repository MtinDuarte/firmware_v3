#include "protocol.h"

uartMap_t         uart_used;
SemaphoreHandle_t new_frame_signal;
SemaphoreHandle_t mutex;

char buffer[FRAME_MAX_SIZE];
uint16_t index;


/**
   @brief Manejador del evento de recepcion de 1 byte via UART
   @param noUsado
 */
void protocol_rx_event( void *noUsado )
{
    ( void* ) noUsado;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* leemos el caracter recibido */
    char c = uartRxRead( UART_USB );

    BaseType_t signaled = xSemaphoreTakeFromISR( mutex, &xHigherPriorityTaskWoken );

    if( signaled )
    {
        if( FRAME_MAX_SIZE-1==index )
        {
            /* reinicio el paquete */
            index = 0;
        }

        if( c == SOM )
        {
            if( index==0 )
            {
                /* 1er byte del frame*/
            }
            else
            {
                /* fuerzo el arranque del frame (descarto lo anterior)*/
                index = 0;
            }

            buffer[index] = c;

            /* incremento el indice */
            index++;
        }
        else if( c == EOM )
        {
            /* solo cierro el fin de frame si al menos se recibio un start.*/
            if( index>=1 )
            {
                /* se termino el paquete - guardo el dato */
                buffer[index] = c;

                /* incremento el indice */
                index++;

                /* Deshabilito todas las interrupciones de UART_USB */
                uartCallbackClr( uart_used, UART_RECEIVE );

                /* señalizo a la aplicacion */
                xSemaphoreGiveFromISR( new_frame_signal, &xHigherPriorityTaskWoken );
            }
            else
            {
                /* no hago nada, descarto el byte */
            }
        }
        else
        {
            /* solo cierro el fin de frame si al menos se recibio un start.*/
            if( index>=1 )
            {
                /* guardo el dato */
                buffer[index] = c;

                /* incremento el indice */
                index++;
            }
            else
            {
                /* no hago nada, descarto el byte */
            }
        }

        xSemaphoreGiveFromISR( mutex, &xHigherPriorityTaskWoken );
    }

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/**
   @brief Inicializa el stack
   @param uart
   @param baudRate
 */
void procotol_x_init( uartMap_t uart, uint32_t baudRate )
{
    /* CONFIGURO EL DRIVER */

    uart_used = uart;

    /* Inicializar la UART_USB junto con las interrupciones de Tx y Rx */
    uartConfig( uart, baudRate );

    /* Seteo un callback al evento de recepcion y habilito su interrupcion */
    uartCallbackSet( uart, UART_RECEIVE, protocol_rx_event, NULL );

    /* Habilito todas las interrupciones de UART_USB */
    uartInterrupt( uart, true );

    /* CONFIGURO LA PARTE LOGICA */
    index = 0;
    new_frame_signal = xSemaphoreCreateBinary();
    mutex = xSemaphoreCreateMutex();
}

/**
   @brief Espera indefinidamente por un paquete.
 */
bool_t protocol_wait_frame()
{
    if ( pdTRUE == xSemaphoreTake( new_frame_signal, 0) )
    {
    	xSemaphoreTake( mutex, 0 );
    	return true;
    }
    else
    	return false;

}

/**
   @brief Devuelve la referencia al paquete recibido
   @param data
   @param size
 */


uint32_t protocol_get_frame_ref(char* SEC, char* IDC,  char* data, char* CRC,  uint16_t* size )
{
	uint32_t i= INIT_FRAME, Ecode=NO_PROBLEM;

	*size = index + (- INIT_FRAME- SEC_SIZE - IDC_SIZE - CRC_SIZE  - FIN_FRAME);

	if (index <= DATA_SIZE )		//Data frame control ask if package is out of bounds
	{

		for (i = INIT_FRAME ;i < SEC_SIZE + INIT_FRAME ;i++)	// Fill Sequence field (SEC).
		 {
			if ( (buffer[i] >='0') && (buffer[i] <= '9') )		// Assert of correct characters
			{
				SEC[i -INIT_FRAME] = buffer[i];
			}
			else
			{
				Ecode=WRONG_SECUENCE_CHARACTER;					//Error code
				return Ecode;
			}
	 }

	*IDC = buffer[SEC_SIZE + IDC_SIZE];
	// Code Field (IDC).
	if (  ( (*IDC == 'C') || (*IDC == 'P') || (*IDC == 'S') ) )	//Assert of IDC code
	{
		// Nothing to do here...
	}
	else
	{
		 //Ecode=WRONG_CODE_CHARACTER;
		 //return Ecode;
	}

	// Data field (DATA) - No need of assert.
	for (i = SEC_SIZE + IDC_SIZE + INIT_FRAME; i < (index - CRC_SIZE - INIT_FRAME) ; i++)
	 {
		data[i - INIT_FRAME - (SEC_SIZE + IDC_SIZE)] = buffer[i];
	 }

		// CRC field (CRC).	TBD assert!
		CRC[1] = buffer[index - CRC_SIZE];
		CRC[0] = buffer[index - CRC_SIZE - FIN_FRAME];
	}
	else
		Ecode=DATA_SIZE_EXCEED;		// data > DATA_SIZE discard packet.
	return Ecode;
}


/**
   @brief Una vez procesado el frame, descarta el paquete y permite una nueva recepcion.
 */
void protocol_discard_frame()
{
    /* indico que se puede inciar un paquete nuevo */
    index = 0;

    /* libero la seccion critica, para que el handler permita ejecutarse */
    xSemaphoreGive( mutex );

    /* limpio cualquier interrupcion que haya ocurrido durante el procesamiento */
    uartClearPendingInterrupt( uart_used );

    /* habilito isr rx  de UART_USB */
    uartCallbackSet( uart_used, UART_RECEIVE, protocol_rx_event, NULL );
}
