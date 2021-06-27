/*
 * writeParser.c
 *
 *  Created on: 8 jun. 2021
 *      Author: martin
 */

#include "writeParser.h"
//	<FHH> <FHL> <BC> <CMD> [<DATA>...] [<CRCH> <CRCL>]

#define header 0x5AA5
#define	write_id_command	0x80

bool_t writeParser(char* dataPointer)
{
	static char writeBuffer[10] =
}
