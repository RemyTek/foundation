#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudDrawContext_t ctx;
} shudElementSpawnProtect_t;

void* CG_SHUDElementSpawnProtectCreate(const superhudConfig_t* config)
{
	shudElementSpawnProtect_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementSpawnProtectRoutine(void* context)
{
	shudElementSpawnProtect_t* element = (shudElementSpawnProtect_t*)context;
	qboolean show = qfalse;

	if (!cg_enableOSPHUD.integer && !cg_shud.integer)
		return;

	if (cg.snap->ps.pm_type == PM_INTERMISSION || cg.warmup)
		return;

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
	{
		/* self has spawn protection */
		if (cg.snap->ps.powerups[PW_SPAWNPROTECTION] > cg.time)
		{
			show = qtrue;
		}
		else
		{
			/* crosshair target is an enemy with spawn protection */
			trace_t trace;
			vec3_t start, end;

			VectorCopy(cg.refdef.vieworg, start);
			VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

			CG_Trace(&trace, start, vec3_origin, vec3_origin, end,
			         cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);

			if (trace.entityNum >= 0 && trace.entityNum < MAX_CLIENTS)
			{
				clientInfo_t* ci = &cgs.clientinfo[trace.entityNum];
				if (ci->infoValid && ci->team != cg.snap->ps.persistant[PERS_TEAM])
				{
					if (cg_entities[trace.entityNum].currentState.powerups & (1 << PW_SPAWNPROTECTION))
						show = qtrue;
				}
			}
		}
	}

	if (show)
	{
		element->ctx.image = cgs.media.spawnProtectionIcon;
		CG_SHUDFill(&element->config);
		CG_SHUDDrawBorder(&element->config);
		CG_SHUDDrawStretchPicCtx(&element->config, &element->ctx);
	}
}

void CG_SHUDElementSpawnProtectDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
