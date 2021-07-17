
#ifndef PROTOCOL_H_
#define PROTOCOL_H_
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "semphr.h"
#include "sapi.h"
#include "qmpool.h"

#define FRAME_MAX_SIZE  220
#define INIT_FRAME 		1
#define FIN_FRAME  		1
#define SOM  			'('
#define EOM				')'
#define camelCase		'C'
#define PascalCase		'P'
#define snake_case		'S'
#define DATA_FRAME_SIZE	 4

#define DATA_SIZE		 200
#define	IDC_SIZE		 1
#define SEC_SIZE		 4
#define	CRC_SIZE		 2
/*
 * Typdef's */

typedef char* char_ptr;

typedef struct
{
	char_ptr       	PoolPointers;
	uint32_t		PoolSizes;
	QMPool 			PoolHandlers;
} data_frame_t;

data_frame_t data_frame[DATA_FRAME_SIZE], *dataframeptr;

QMPool MyPools[DATA_FRAME_SIZE];


typedef struct
{
	char_ptr	SEC;
	char_ptr    OPT;
	char_ptr 	DATA;
	char_ptr	CRC;
	uint16_t	data_size;
} splitBuffer;

/*
 * 	Handlers		*/

typedef struct
{
	char_ptr	SEC;
	char_ptr    OPT;
	char_ptr 	DATA;
	char_ptr	CRC;
	uint16_t	data_size;
} data_t;

enum ErrorCode
{
	DATA_SIZE_EXCEED,
	WRONG_SECUENCE_CHARACTER,
	WRONG_CODE_CHARACTER,
	NO_PROBLEM,
};

enum PckPkgID
{
	ID_SEC,
	ID_OPT,
	ID_DATA,
	ID_CRC,
};


void procotol_x_init( uartMap_t uart, uint32_t baudRate );
bool_t protocol_wait_frame();
uint32_t protocol_get_frame_ref(char* SEC, char* IDC,  char* data, char* CRC , uint16_t* size);
//void protocol_get_frame_ref( char** data, uint16_t* size );
void protocol_discard_frame();

#endif
