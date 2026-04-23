#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

#define SHUD_STYLE_PERCENTAGE       (1 << 0)
#define SHUD_STYLE_COLORIZED   (1 << 1)


typedef enum
{
	SHUD_ELEMENT_PLAYER_STATS_DG,
	SHUD_ELEMENT_PLAYER_STATS_DR,
	SHUD_ELEMENT_PLAYER_STATS_DG_ICON,
	SHUD_ELEMENT_PLAYER_STATS_DR_ICON,
	SHUD_ELEMENT_PLAYER_STATS_DAMAGE_RATIO
} shudElementPlayerStatsType_t;

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t textCtx;
	superhudDrawContext_t drawCtx;
	shudElementPlayerStatsType_t type;
} shudElementPlayerStats_t;

void* CG_SHUDElementPlayerStatsCreate(const superhudConfig_t* config, shudElementPlayerStatsType_t type)
{
	shudElementPlayerStats_t* element;
	SHUD_ELEMENT_INIT(element, config);
	element->type = type;
	if (type == SHUD_ELEMENT_PLAYER_STATS_DG ||
	        type == SHUD_ELEMENT_PLAYER_STATS_DR ||
	        type == SHUD_ELEMENT_PLAYER_STATS_DAMAGE_RATIO)

	{
		CG_SHUDTextMakeContext(&element->config, &element->textCtx);
		CG_SHUDFillAndFrameForText(&element->config, &element->textCtx);
	}
	else
	{
		CG_SHUDDrawMakeContext(&element->config, &element->drawCtx);
	}
	return element;
}

void* CG_SHUDElementCreatePlayerStatsDG(const superhudConfig_t* config)
{
	return CG_SHUDElementPlayerStatsCreate(config, SHUD_ELEMENT_PLAYER_STATS_DG);
}
void* CG_SHUDElementCreatePlayerStatsDR(const superhudConfig_t* config)
{
	return CG_SHUDElementPlayerStatsCreate(config, SHUD_ELEMENT_PLAYER_STATS_DR);
}
void* CG_SHUDElementCreatePlayerStatsDGIcon(const superhudConfig_t* config)
{
	return CG_SHUDElementPlayerStatsCreate(config, SHUD_ELEMENT_PLAYER_STATS_DG_ICON);
}
void* CG_SHUDElementCreatePlayerStatsDRIcon(const superhudConfig_t* config)
{
	return CG_SHUDElementPlayerStatsCreate(config, SHUD_ELEMENT_PLAYER_STATS_DR_ICON);
}
void* CG_SHUDElementCreatePlayerStatsDamageRatio(const superhudConfig_t* config)
{
	return CG_SHUDElementPlayerStatsCreate(config, SHUD_ELEMENT_PLAYER_STATS_DAMAGE_RATIO);
}

static void CG_SHUDStyleDamageRatioColor(vec4_t color, const superhudConfig_t* config, float ratio)
{
	if (config->style.isSet && (config->style.value & SHUD_STYLE_COLORIZED))
	{
		const float thresholds[] = { 0.6f, 1.0f, 1.6f, 2.5f };

		const vec4_t colors[] =
		{
			{1.0f, 0.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f, 0.0f, 1.0f},
			{0.0f, 1.0f, 1.0f, 1.0f},
			{1.0f, 0.0f, 1.0f, 1.0f}
		};

		if (ratio < thresholds[0])
		{
			Vector4Copy(colors[0], color); // red
		}
		else if (ratio < thresholds[1])
		{
			Vector4Copy(colors[1], color); // yellow
		}
		else if (ratio < thresholds[2])
		{
			Vector4Copy(colors[2], color); // green
		}
		else if (ratio < thresholds[3])
		{
			Vector4Copy(colors[3], color); // cyan
		}
		else
		{
			Vector4Copy(colors[4], color); // magenta
		}
	}
	else
	{
		Vector4Copy(config->color.value.rgba, color);
	}
}

void CG_SHUDElementPlayerStatsRoutine(void* context)
{
	shudElementPlayerStats_t* element = (shudElementPlayerStats_t*)context;
	newStatsInfo_t* ws = &cgs.be.newStats;
	float damageRatio = ws->damageRatio;
	int dmgGiven = statsInfo[OSP_STATS_DMG_GIVEN];
	int dmgReceived = statsInfo[OSP_STATS_DMG_RCVD];

	char buffer[64];
	vec4_t ratioColor;

	static int lastUpdateTime = 0;
	const int updateInterval = 2000;
	static int lastClientNum = -1;
	int currentClientNum = (cg.snap ? cg.snap->ps.clientNum : -1);
	qboolean clientChanged = (currentClientNum != lastClientNum);

	static qboolean wasSpectator = qfalse;
	qboolean isSpectator = CG_IsSpectatorOnScreen();
	qboolean becameSpectator = (isSpectator && !wasSpectator);

	if (CG_OSPIsStatsHidden(qtrue, qtrue))
		return;

	// update statsInfo if
	if ((cg.time - cg.damageTime <= 100 ||              // damage recieved
	        cg.time - cgs.osp.lastHitTime <= 100 ||         // damage dealt
	        clientChanged || becameSpectator) &&            // client changed or free float
	        (cg.time - lastUpdateTime >= updateInterval))   // 2 sec interval
	{
		lastUpdateTime = cg.time;
		trap_SendClientCommand("getstatsinfo");
		lastClientNum = currentClientNum;
	}

	wasSpectator = isSpectator;

	switch (element->type)
	{
		case SHUD_ELEMENT_PLAYER_STATS_DG:
			if (dmgGiven <= 0 && !SHUD_CHECK_SHOW_EMPTY(element)) return;

			Com_sprintf(buffer, sizeof(buffer), "%d", dmgGiven);
			element->textCtx.text = buffer;
			CG_SHUDTextPrint(&element->config, &element->textCtx);
			return;

		case SHUD_ELEMENT_PLAYER_STATS_DR:
			if (dmgReceived <= 0 && !SHUD_CHECK_SHOW_EMPTY(element)) return;
			Com_sprintf(buffer, sizeof(buffer), "%d", dmgReceived);
			element->textCtx.text = buffer;
			CG_SHUDTextPrint(&element->config, &element->textCtx);
			return;
		case SHUD_ELEMENT_PLAYER_STATS_DAMAGE_RATIO:
		{
			if (damageRatio <= 0.0f)
			{
				if (!SHUD_CHECK_SHOW_EMPTY(element))
				{
					return;
				}
				if (element->config.style.isSet && (element->config.style.value & SHUD_STYLE_PERCENTAGE))
					Q_strncpyz(buffer, "0%", sizeof(buffer));
				else
				{
					Q_strncpyz(buffer, "0.00", sizeof(buffer));
				}
			}
			else
			{
				if (element->config.style.isSet && (element->config.style.value & SHUD_STYLE_PERCENTAGE))
				{
					Com_sprintf(buffer, sizeof(buffer), "%.0f%%", damageRatio * 100.0f);
				}
				else
				{
					Com_sprintf(buffer, sizeof(buffer), "%.2f", damageRatio);
				}
			}

			CG_SHUDStyleDamageRatioColor(ratioColor, &element->config, damageRatio);
			Vector4Copy(ratioColor, element->textCtx.color);

			element->textCtx.text = buffer;
			CG_SHUDTextPrintNew(&element->config, &element->textCtx, qfalse);
			return;
		}
		case SHUD_ELEMENT_PLAYER_STATS_DG_ICON:
			if (dmgGiven <= 0 && !!SHUD_CHECK_SHOW_EMPTY(element)) return;
			element->drawCtx.image = cgs.media.arrowUp;
			CG_SHUDDrawBorder(&element->config);
			CG_SHUDFill(&element->config);
			CG_SHUDDrawStretchPicCtx(&element->config, &element->drawCtx);
			return;

		case SHUD_ELEMENT_PLAYER_STATS_DR_ICON:
			if (dmgReceived <= 0 && !SHUD_CHECK_SHOW_EMPTY(element)) return;
			element->drawCtx.image = cgs.media.arrowDown;
			CG_SHUDDrawBorder(&element->config);
			CG_SHUDFill(&element->config);
			CG_SHUDDrawStretchPicCtx(&element->config, &element->drawCtx);
			return;

		default:
			return;
	}
}

void CG_SHUDElementPlayerStatsDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
