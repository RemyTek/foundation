#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	float x;
	float y;
} shudElementRoundScoreboard;

void* CG_SHUDElementRoundScoreboardCreate(const superhudConfig_t* config)
{
	shudElementRoundScoreboard* element;

	SHUD_ELEMENT_INIT(element, config);

	if (config->rect.isSet)
	{
		element->x = config->rect.value[0];
		element->y = config->rect.value[1];
	}
	else
	{
		element->x = 64.0f;
		element->y = 97.0f;
	}

	return element;
}

void CG_SHUDElementRoundScoreboardRoutine(void* context)
{
	shudElementRoundScoreboard* element = (shudElementRoundScoreboard*)context;

	if ( cgs.gametype != GT_CTFS )
		return;
	if ( cg.warmup <= 0 )
		return;
	if ( cgs.atdCompletedRounds <= 0 )
		return;
	if ( !cgs.atdRoundStartTime && !cgs.atdRoundRespawned )
		return;
	if ( cg.intermissionStarted )
		return;

	CG_DrawATDRoundScores( element->x, element->y, 1.0f );
}

void CG_SHUDElementRoundScoreboardDestroy(void* context)
{
	Z_Free(context);
}
