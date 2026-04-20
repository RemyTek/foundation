#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
} shudElementWarnLast_t;

void* CG_SHUDElementWarnLastCreate(const superhudConfig_t* config)
{
	shudElementWarnLast_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	CG_SHUDFillAndFrameForText(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementWarnLastRoutine(void* context)
{
	shudElementWarnLast_t* element = (shudElementWarnLast_t*)context;
	static qboolean soundPlayed = qfalse;
	static qboolean lastPlayerState = qfalse;

	qboolean showLast = (cg.lastPlayerWarning && cg.snap->ps.pm_type != PM_INTERMISSION);

	if (showLast != lastPlayerState)
	{
		lastPlayerState = showLast;
		if (lastPlayerState)
		{
			element->ctx.text = "LAST";
			if (!soundPlayed && cg_lastSound.integer)
			{
				trap_S_StartLocalSound(cgs.media.lastStandingSound, CHAN_ANNOUNCER);
				soundPlayed = qtrue;
			}
		}
		else
		{
			soundPlayed = qfalse;
			element->ctx.text = NULL;
		}
	}

	if (element->ctx.text)
	{
		CG_SHUDTextPrint(&element->config, &element->ctx);
	}
}

void CG_SHUDElementWarnLastDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
