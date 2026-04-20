#ifndef FOUNDATION_QCOMMON_L_CRC_H
#define FOUNDATION_QCOMMON_L_CRC_H

#include "q_shared.h"

typedef unsigned short crc_t;

void CRC_Init( unsigned short *crcvalue );
void CRC_ProcessByte( unsigned short *crcvalue, byte data );
unsigned short CRC_Value( unsigned short crcvalue );
unsigned short CRC_ProcessString( const unsigned char *data, int length );
void CRC_ContinueProcessString( unsigned short *crc, const char *data, int length );

#endif