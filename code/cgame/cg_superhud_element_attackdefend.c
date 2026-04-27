#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t      config;
	superhudDrawContext_t ctx;
	int                   lockedAttackingTeam; /* snapshot taken at each warmup start */
	int                   prevWarmup;          /* cg.warmup from previous frame        */
	qboolean              visible;             /* stays true once shown until round ends */
} shudElementAttackDefend_t;

void* CG_SHUDElementAttackDefendCreate(const superhudConfig_t* config)
{
	shudElementAttackDefend_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDDrawMakeContext(&element->config, &element->ctx);
	element->lockedAttackingTeam = 0;
	element->prevWarmup          = 0;
	element->visible             = qfalse;

	return element;
}

void CG_SHUDElementAttackDefendRoutine(void* context)
{
	shudElementAttackDefend_t* element = (shudElementAttackDefend_t*)context;
	int       playerTeam;
	qboolean  isAttacker;
	qhandle_t shader;
	vec4_t    color;
	int       warmup;
	int       msLeft;

	if (cgs.gametype != GT_CTFS)
		return;
	if (!cg.atdInterRound)
		return;
	if (cg.intermissionStarted)
		return;

	warmup = cg.warmup;

	/* Lock in the attacking team when a new inter-round warmup countdown begins
	   (warmup transitions from <= 0 to > 0). */
	if (warmup > 0 && element->prevWarmup <= 0)
	{
		element->lockedAttackingTeam = cgs.atdAttackingTeam;
		element->visible             = qfalse; /* reset; will re-show at 5s mark */
	}

	/* When a round goes live (warmup -> 0), keep visible for the whole round. */
	if (warmup == 0 && element->prevWarmup > 0)
		element->visible = qtrue;

	element->prevWarmup = warmup;

	if (element->lockedAttackingTeam == 0)
		return;

	/* During the warmup countdown, show only in the last 5 seconds. */
	if (warmup > 0)
	{
		msLeft = warmup - cg.time;
		if (msLeft > 5000)
			return;
		element->visible = qtrue;
	}

	if (!element->visible)
		return;

	playerTeam = cgs.clientinfo[cg.snap->ps.clientNum].team;

	if (playerTeam != TEAM_RED && playerTeam != TEAM_BLUE)
		return;

	isAttacker = (playerTeam == element->lockedAttackingTeam) ? qtrue : qfalse;

	shader = isAttacker ? cgs.media.flagAttackPOI : cgs.media.flagDefendPOI;

	/* Color represents the flag associated with the role:
	   - Attacker → enemy flag color  (blue attacks → red tint; red attacks → blue tint)
	   - Defender → own  flag color   (blue defends → blue tint; red defends → red tint) */
	if (isAttacker)
	{
		if (playerTeam == TEAM_BLUE)
			Vector4Copy(colorRed, color);
		else
			Vector4Copy(colorBlue, color);
	}
	else
	{
		if (playerTeam == TEAM_BLUE)
			Vector4Copy(colorBlue, color);
		else
			Vector4Copy(colorRed, color);
	}

	CG_SHUDFill(&element->config);
	CG_SHUDDrawBorder(&element->config);
	CG_SHUDDrawStretchPic(element->ctx.coord, element->ctx.coordPicture, color, shader);
}

void CG_SHUDElementAttackDefendDestroy(void* context)
{
	Z_Free(context);
}
