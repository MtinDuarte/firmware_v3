#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "protocol.h"
#include "semphr.h"
#include "string.h"
#include "queue.h"

xQueueHandle Layer2_TO_App,  App_TO_Layer2;
/*
 * Global Array's */

uint8_t PoolSizesArr[DATA_FRAME_SIZE]={ SEC_SIZE,
                                        IDC_SIZE,
                                        DATA_SIZE,
                                        CRC_SIZE};


bool_t PutTakenMemory(char* P1,char* P2,char* P3,char* P4)
{
	if (P1 != NULL && P2 != NULL && P3 != NULL && P4 != NULL)
	{
           QMPool_put(&data_frame[0].PoolHandlers, P1);
	       QMPool_put(&data_frame[1].PoolHandlers, P2);
	       QMPool_put(&data_frame[2].PoolHandlers, P3);
	       QMPool_put(&data_frame[3].PoolHandlers, P4);
	       P1=P2=P3=P4=NULL;
	       return true;
	}
	return false;
}
bool_t CammelCaseConversion (data_t* rxdata, char* dout)
{

	if ( (*(rxdata->DATA) >= 'A') && ((*(rxdata->DATA) <= 'Z'))) //Treat first letter
			{
				dout[0] = *(rxdata->DATA) + 32;	//To lower case
			}
			else
			{
				dout[0] = rxdata->DATA[0];		//Lower, just load to buffer.
			}

			bool_t symbol = 0;

			int charctr = 0;				//Data frame counter

			for(int i = 1; i < rxdata->data_size; i++)
			{
				if(symbol)					//Ask if symbol has been detected
				{
					symbol = 0;

					if ((rxdata->DATA[i] >= 'A') && (rxdata->DATA[i] <= 'Z'))
					{
						dout[i-charctr] = rxdata->DATA[i];		//Stay upper case
					}
					else
					{
						dout[i-charctr] = rxdata->DATA[i] - 32;	//Convert to upper case
					}
				}
				else if ((rxdata->DATA[i] == ' ') || (rxdata->DATA[i] == '_'))
				{
					symbol = 1;			// Activate symbol flag to skip this position and convert following char
					charctr++;
				}
				else
				{
					dout[i-charctr] = rxdata->DATA[i];		//Write same as buffer
				}
				//printf(" rxdata.DATA[i] = %c , dout[i] = %c , flag = %d \r\n", rxdata.DATA[i],dout[i-cont_sim],flag);
			}

			printf(" camelCase = %s \r\n", dout);
			return true;
}
bool_t PascalCaseConversion	(data_t* rxdata, char* dout)
{
	if ( (*(rxdata->DATA) >= 'A') && ((*(rxdata->DATA) <= 'Z')))
	{
		dout[0] = *(rxdata->DATA);
	}
	else
	{
		dout[0] = *(rxdata->DATA) - 32;
	}

	bool_t symbol = 0;

	int charctr = 0;

	for(int i = 1;i < rxdata->data_size;i++)
	{
		if(symbol == 1)
		{
			symbol = 0;

			if ((rxdata->DATA[i] >= 'A') && (rxdata->DATA[i] <= 'Z'))
			{
				dout[i-charctr] = rxdata->DATA[i] ;
			}
			else
			{
				dout[i-charctr] = rxdata->DATA[i] - 32;
			}
		}
		else if ((rxdata->DATA[i] == ' ') || (rxdata->DATA[i] == '_'))
		{
			symbol = 1;
			charctr++;
		}
		else
		{
			dout[i-charctr] = rxdata->DATA[i];
		}
	}
	printf(" PascalCase = %s \r\n", dout);	//This will print data in pascal case!
	return true;
}
bool_t SnakeCaseConversion	(data_t* rxdata, char* dout)
{
	if ( (*(rxdata->DATA) >= 'A') && ((*(rxdata->DATA) <= 'Z')))
		{
			dout[0] = *(rxdata->DATA) + 32;

		}
		else
		{
			dout[0] = *(rxdata->DATA);

		}

	bool_t symbol = 0;  uint32_t charctr = 0;

	for(int i = 1;i < rxdata->data_size;i++)
	{
		if ((rxdata->DATA[i] >= 'A') && (rxdata->DATA[i] <= 'Z'))
		{
			dout[i] = '_';
			charctr++;
			dout[i+charctr] = rxdata->DATA[i]+	32;
		}
		else if ((rxdata->DATA[i]== ' ') || (rxdata->DATA[i] == '_'))
		{
			dout[i+charctr] = '_';
		}
		else
		{
			dout[i+charctr] = rxdata->DATA[i];
		}

		//printf(" rxdata.DATA[i] = %c , dout[i] = %c , cont_sim = %d \r\n", rxdata.DATA[i],dout[i+cont_sim],cont_sim);

	}
	printf(" snake_case = %s \r\n", dout);
	return true;
}

void Layer3AppControl( void* pvParameters )	//3rd Layer task
{

	data_t 		rxdata;
	bool_t 		symbol = 0;
	char 		dout[FRAME_MAX_SIZE];


    while( TRUE )
    {
    	strncpy(dout,'\0',FRAME_MAX_SIZE-1);

    	if ( pdTRUE == xQueueReceive(Layer2_TO_App, &rxdata, portMAX_DELAY) )	//Wait for Layer2 frame
		{
			switch (*rxdata.OPT)	//Switch function of 'C' 'S' 'P'
			{
				case camelCase:
					if (true == CammelCaseConversion (&rxdata, dout));
					break;

				case PascalCase:

					if (true == PascalCaseConversion(&rxdata, dout));
					break;

				case snake_case:
					if (true == SnakeCaseConversion(&rxdata, dout));
				   break;
			}
			xQueueSend(App_TO_Layer2,&rxdata,0);
			vTaskDelay( 1/portTICK_RATE_MS );	//Retardo 1 ms
		}
	//   if (true == PutTakenMemory(rxdata.SEC,rxdata.OPT,rxdata.DATA,rxdata.CRC))
	// 	 {
		   //printf("Memory put executed sucessfuly \r\n");
	//   }
	//   else;
    //	 printf("Error while memPut at application layer\r\n");

    }
}
void Layer2Control( void* pvParameters )	//2nd Layer Task
{

uint8_t Ecode;	uint16_t size;	splitBuffer splitdata;
data_t	app2layer2;
char_ptr pckptr[DATA_FRAME_SIZE*16];
uint32_t FrameCounter=1;

   while( TRUE )
   {
	   Ecode=NO_PROBLEM;
	   if ( pdTRUE== xQueueReceive(App_TO_Layer2, &app2layer2,0))	//Always check if there is response
	   {
		   /*
		   //Write an echo
		   	    uartWriteByte( UART_USB,40);	// '('
	   			uartWriteString(UART_USB,app2layer2.SEC);
	   			uartWriteByte(UART_USB,*app2layer2.OPT);
	   			uartWriteString(UART_USB,app2layer2.DATA);
	   			uartWriteString(UART_USB,app2layer2.CRC);
	   			uartWriteByte(UART_USB, 41);	// ')'
	   			uartWriteByte(UART_USB, 10);	// \r
	   			uartWriteByte(UART_USB, 13);	// \n
	   			strncpy(app2layer2.DATA,'\0',FRAME_MAX_SIZE-1);
	*/
			if ( PutTakenMemory(app2layer2.SEC,app2layer2.OPT,app2layer2.DATA,app2layer2.CRC) )
			{
				//Nothing to do here...
			}
	   }

	   if (true == protocol_wait_frame())	//There is a frame at UART buffer.
	   {
		   // Ask for memory on every pool, and save at main pool pointer & assert pointers!
		   for(int i = 0; i <  DATA_FRAME_SIZE ;i++)
		   {
			   pckptr[i+(FrameCounter*DATA_FRAME_SIZE)] = (char*)QMPool_get(&data_frame[i].PoolHandlers,0);

			   configASSERT( pckptr[i+(FrameCounter*DATA_FRAME_SIZE)] != NULL);
		   }


		   if (  xPortGetFreeHeapSize() > 0 )
		   {
			   // Passing every package pointer, to save incoming data frame
			   Ecode = protocol_get_frame_ref(	pckptr[ID_SEC+(FrameCounter*DATA_FRAME_SIZE)],
												pckptr[ID_OPT+(FrameCounter*DATA_FRAME_SIZE)],
												pckptr[ID_DATA+(FrameCounter*DATA_FRAME_SIZE)],
												pckptr[ID_CRC+(FrameCounter*DATA_FRAME_SIZE)],
												&size);

			   if (Ecode == NO_PROBLEM)			//Send to application layer!
			   {

					splitdata.SEC  = pckptr[ID_SEC +(FrameCounter*DATA_FRAME_SIZE)];
					splitdata.OPT  = pckptr[ID_OPT +(FrameCounter*DATA_FRAME_SIZE)];
					splitdata.DATA = pckptr[ID_DATA+(FrameCounter*DATA_FRAME_SIZE)];
					splitdata.CRC  = pckptr[ID_CRC +(FrameCounter*DATA_FRAME_SIZE)];
					splitdata.data_size  = size;

					xQueueSend(Layer2_TO_App,
							   &splitdata,
							   0);

					FrameCounter++;
					FrameCounter= ( FrameCounter >= 16) ? INIT_FRAME : FrameCounter;

			   }
			   else
			   {
				   if (true == PutTakenMemory(pckptr[ID_SEC +(FrameCounter*DATA_FRAME_SIZE)],
										  	  pckptr[ID_OPT +(FrameCounter*DATA_FRAME_SIZE)],
											  pckptr[ID_DATA+(FrameCounter*DATA_FRAME_SIZE)],
											  pckptr[ID_CRC +(FrameCounter*DATA_FRAME_SIZE)]));	//Put memory pool's back
				   // printf("The program exited at layer 2 with error code : %d \r\n",Ecode);
			   else
			   {
				   //printf("Error! Couldn't put memory back to pools on layer 2 \r\n");
				   while(true);
			   }

			   }
		   }
		   protocol_discard_frame();
	   }
   }
}
void TasksQueuesInits (void)
{
    BaseType_t res;

    res = xTaskCreate(
    	Layer2Control,                  // Funcion de la tarea a ejecutar
        ( const char * )"SplitFrame", // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*4,   // Cantidad de stack de la tarea
        0,                            // Parametros de tarea
        tskIDLE_PRIORITY+1,           // Prioridad de la tarea
        0                             // Puntero a la tarea creada en el sistema
    );
    configASSERT(res);

    res= xTaskCreate(
    	Layer3AppControl,                  // Funcion de la tarea a ejecutar
        ( const char * )"App", // Nombre de la tarea como String amigable para el usuario
        configMINIMAL_STACK_SIZE*4,   // Cantidad de stack de la tarea
        0,                            // Parametros de tarea
        tskIDLE_PRIORITY+1,           // Prioridad de la tarea
        0                             // Puntero a la tarea creada en el sistema
    );
    configASSERT(res);

	/* Se crea una cola para enviar el mensaje de la capa de separacion a la capa de aplicacion */
    Layer2_TO_App = xQueueCreate( 16, sizeof( splitBuffer ) );
    App_TO_Layer2 = xQueueCreate( 16, sizeof( splitBuffer ) );
	// Gestión de errores
	configASSERT( Layer2_TO_App  != NULL );
	configASSERT( App_TO_Layer2  != NULL );
}
void Inits(void)
{
	for (int i = 0 ; i < DATA_FRAME_SIZE ; i ++ )
	{
		data_frame[i].PoolHandlers=MyPools[i];												//Pool 		definitions [ 4 ]
		data_frame[i].PoolPointers= (char *) pvPortMalloc(sizeof(char)*PoolSizesArr[i]);	//Pool Pointers	definitions [ 4 ]

		configASSERT( data_frame[i].PoolPointers	!= NULL );								// Assert

 	  	QMPool_init(&data_frame[i].PoolHandlers,											// Initialization of mem.Pools
				    (char *) data_frame[i].PoolPointers,
		  	        sizeof(char*)*PoolSizesArr[i]*16,
				    sizeof(char*)*PoolSizesArr[i]);
	}
}
int	 main( void )
{
    boardConfig();

    procotol_x_init( UART_USB, 115200 );

    //printf("Ing. Alvarado Jose David - Ing. Duarte Martin  [ RTOS II ]  \r\n");

    TasksQueuesInits();				//Task and Queue Creation

    Inits();						// Memory pools - Pointers - Initializations

    vTaskStartScheduler();

    return 0;
}
