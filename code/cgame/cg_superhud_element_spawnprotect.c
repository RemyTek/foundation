#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	int timePrev;
	superhudTextContext_t ctx;
} shudElementSpawnProtect_t;

void* CG_SHUDElementSpawnProtectCreate(const superhudConfig_t* config)
{
	shudElementSpawnProtect_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	CG_SHUDFillAndFrameForText(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementSpawnProtectRoutine(void* context)
{
	shudElementSpawnProtect_t* element = (shudElementSpawnProtect_t*)context;

	element->ctx.text = NULL;

	if (!cg_enableOSPHUD.integer && !cg_shud.integer)
	{
		return;
	}

	if (cg.snap->ps.pm_type == PM_INTERMISSION || cg.warmup)
	{
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
	{
		trace_t trace;
		vec3_t start, end;

		VectorCopy(cg.refdef.vieworg, start);
		VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

		CG_Trace(&trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);

		if (trace.entityNum >= 0 && trace.entityNum < MAX_CLIENTS)
		{
			clientInfo_t* ci = &cgs.clientinfo[trace.entityNum];
			if (ci->infoValid && ci->team != cg.snap->ps.persistant[PERS_TEAM])
			{
				if (cg_entities[trace.entityNum].currentState.eFlags & EF_SPAWNPROTECTION)
				{
					element->ctx.text = "SPAWN PROTECTION";
				}
			}
		}

		if (cg.snap->ps.eFlags & EF_SPAWNPROTECTION)
		{
			element->ctx.text = "SPAWN PROTECTION";
		}
	}

	if (element->ctx.text)
	{
		CG_SHUDTextPrint(&element->config, &element->ctx);
	}
}

void CG_SHUDElementSpawnProtectDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
