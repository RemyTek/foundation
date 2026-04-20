#include "cg_local.h"
#include "../qcommon/qcommon.h"

#define CG_ZONE_ALIGNMENT 8
#ifdef Q3_VM
#define CG_ZONE_POOL_SIZE (2 * 1024 * 1024)
#endif

typedef struct cg_zone_block_s {
	int size;
	int free;
	struct cg_zone_block_s *next;
	struct cg_zone_block_s *prev;
} cg_zone_block_t;

#ifdef Q3_VM
static byte cg_zone_pool[CG_ZONE_POOL_SIZE];
#endif

static cg_zone_block_t *cg_zone_head;

static int CG_ZoneAlign( int size ) {
	return ( size + ( CG_ZONE_ALIGNMENT - 1 ) ) & ~( CG_ZONE_ALIGNMENT - 1 );
}

static void CG_ZoneInit( void *memory, int size ) {
	cg_zone_head = (cg_zone_block_t *)memory;
	cg_zone_head->size = size - (int)sizeof( cg_zone_block_t );
	cg_zone_head->free = qtrue;
	cg_zone_head->next = NULL;
	cg_zone_head->prev = NULL;
}

static void CG_ZoneSplitBlock( cg_zone_block_t *block, int size ) {
	cg_zone_block_t *next;

	if ( block->size < size + (int)sizeof( cg_zone_block_t ) + CG_ZONE_ALIGNMENT ) {
		return;
	}

	next = (cg_zone_block_t *)((byte *)( block + 1 ) + size);
	next->size = block->size - size - (int)sizeof( cg_zone_block_t );
	next->free = qtrue;
	next->next = block->next;
	next->prev = block;

	if ( next->next ) {
		next->next->prev = next;
	}

	block->size = size;
	block->next = next;
}

static void CG_ZoneCoalesce( cg_zone_block_t *block ) {
	if ( block->next && block->next->free ) {
		block->size += (int)sizeof( cg_zone_block_t ) + block->next->size;
		block->next = block->next->next;
		if ( block->next ) {
			block->next->prev = block;
		}
	}

	if ( block->prev && block->prev->free ) {
		block->prev->size += (int)sizeof( cg_zone_block_t ) + block->size;
		block->prev->next = block->next;
		if ( block->next ) {
			block->next->prev = block->prev;
		}
	}
}

void Com_InitZoneMemory( void ) {
#ifdef Q3_VM
	CG_ZoneInit( cg_zone_pool, sizeof( cg_zone_pool ) );
#else
	cg_zone_head = NULL;
#endif
}

void *Z_Malloc( int size ) {
	cg_zone_block_t *block;
	int alignedSize;

	if ( size <= 0 ) {
		return NULL;
	}

	alignedSize = CG_ZoneAlign( size );

#ifndef Q3_VM
	block = (cg_zone_block_t *)malloc( sizeof( cg_zone_block_t ) + alignedSize );
	if ( !block ) {
		Com_Error( ERR_DROP, "Z_Malloc: failed on allocation of %i bytes", size );
		return NULL;
	}
	block->size = alignedSize;
	block->free = qfalse;
	block->next = cg_zone_head;
	block->prev = NULL;
	if ( cg_zone_head ) {
		cg_zone_head->prev = block;
	}
	cg_zone_head = block;
	Com_Memset( block + 1, 0, alignedSize );
	return block + 1;
#else
	if ( !cg_zone_head ) {
		Com_InitZoneMemory();
	}

	for ( block = cg_zone_head; block; block = block->next ) {
		if ( block->free && block->size >= alignedSize ) {
			CG_ZoneSplitBlock( block, alignedSize );
			block->free = qfalse;
			Com_Memset( block + 1, 0, alignedSize );
			return block + 1;
		}
	}

	Com_Error( ERR_DROP, "Z_Malloc: failed on allocation of %i bytes", size );
	return NULL;
#endif
}

void Z_Free( const void *ptr ) {
	if ( !ptr ) {
		return;
	}

#ifndef Q3_VM
	{
		cg_zone_block_t *block = ((cg_zone_block_t *)ptr) - 1;
		if ( block->prev ) {
			block->prev->next = block->next;
		} else {
			cg_zone_head = block->next;
		}
		if ( block->next ) {
			block->next->prev = block->prev;
		}
		free( block );
	}
#else
	{
		cg_zone_block_t *block = ((cg_zone_block_t *)ptr) - 1;
		block->free = qtrue;
		CG_ZoneCoalesce( block );
	}
#endif
}

int Z_AvailableMemory( void ) {
#ifndef Q3_VM
	return MAX_QINT;
#else
	int freeBytes;
	cg_zone_block_t *block;

	freeBytes = 0;
	for ( block = cg_zone_head; block; block = block->next ) {
		if ( block->free ) {
			freeBytes += block->size;
		}
	}

	return freeBytes;
#endif
}

void Z_Stats( zone_stats_t *stats ) {
	if ( !stats ) {
		return;
	}

	Com_Memset( stats, 0, sizeof( *stats ) );
	stats->freeBytes = Z_AvailableMemory();
}

char *Z_StrDup( const char *str ) {
	char *copy;
	int size;

	if ( !str ) {
		return NULL;
	}

	size = (int)strlen( str ) + 1;
	copy = (char *)Z_Malloc( size );
	if ( copy ) {
		Com_Memcpy( copy, str, size );
	}

	return copy;
}