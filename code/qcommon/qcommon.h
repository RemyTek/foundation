#ifndef FOUNDATION_QCOMMON_QCOMMON_H
#define FOUNDATION_QCOMMON_QCOMMON_H

#include "q_shared.h"

typedef struct zone_stats_s {
	int zoneSegments;
	int zoneBlocks;
	int zoneBytes;
	int botlibBytes;
	int rendererBytes;
	int freeBytes;
	int freeBlocks;
	int freeSmallest;
	int freeLargest;
} zone_stats_t;

void Com_InitZoneMemory( void );
void *Z_Malloc( int size );
void Z_Free( const void *ptr );
int Z_AvailableMemory( void );
void Z_Stats( zone_stats_t *stats );
char *Z_StrDup( const char *str );

#endif