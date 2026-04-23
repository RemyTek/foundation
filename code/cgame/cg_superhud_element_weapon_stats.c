#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef enum
{
	SHUD_ELEMENT_WEAPON_STATS_TEXT,
	SHUD_ELEMENT_WEAPON_STATS_ICON
} shudElementWeaponStatsType_t;

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t textCtx;
	superhudDrawContext_t drawCtx;
	shudElementWeaponStatsType_t type;
	int weaponIndex;
} shudElementWeaponStats_t;

// === Универсальная функция создания ===
void* CG_SHUDElementWeaponStatsCreate(const superhudConfig_t* config, shudElementWeaponStatsType_t type, int weaponIndex)
{
	shudElementWeaponStats_t* element;

	SHUD_ELEMENT_INIT(element, config);
	element->type = type;
	element->weaponIndex = weaponIndex;

	if (type == SHUD_ELEMENT_WEAPON_STATS_TEXT)
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

// Text
void* CG_SHUDElementCreateCurrentWeapon(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, 0);
}
void* CG_SHUDElementWeaponStatsCreateMG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_MACHINEGUN);
}
void* CG_SHUDElementWeaponStatsCreateSG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_SHOTGUN);
}
void* CG_SHUDElementWeaponStatsCreateGL(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_GRENADE_LAUNCHER);
}
void* CG_SHUDElementWeaponStatsCreateRL(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_ROCKET_LAUNCHER);
}
void* CG_SHUDElementWeaponStatsCreateLG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_LIGHTNING);
}
void* CG_SHUDElementWeaponStatsCreateRG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_RAILGUN);
}
void* CG_SHUDElementWeaponStatsCreatePG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_TEXT, WP_PLASMAGUN);
}

// Icons
void* CG_SHUDElementIconCreateCurrentWeapon(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, 0);
}
void* CG_SHUDElementIconCreateMG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_MACHINEGUN);
}
void* CG_SHUDElementIconCreateSG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_SHOTGUN);
}
void* CG_SHUDElementIconCreateGL(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_GRENADE_LAUNCHER);
}
void* CG_SHUDElementIconCreateRL(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_ROCKET_LAUNCHER);
}
void* CG_SHUDElementIconCreateLG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_LIGHTNING);
}
void* CG_SHUDElementIconCreateRG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_RAILGUN);
}
void* CG_SHUDElementIconCreatePG(const superhudConfig_t* config)
{
	return CG_SHUDElementWeaponStatsCreate(config, SHUD_ELEMENT_WEAPON_STATS_ICON, WP_PLASMAGUN);
}

void CG_SHUDElementWeaponStatsRoutine(void* context)
{
	shudElementWeaponStats_t* element = (shudElementWeaponStats_t*)context;
	newStatsInfo_t* cs = &cgs.be.newStats;
	playerState_t* ps = &cg.snap->ps;

	int weapon = (element->weaponIndex == 0) ? ps->weapon : element->weaponIndex;
	int weaponIndex = weapon - WP_GAUNTLET;
	qboolean trackAccuracy = (cg_shud_currentWeapons.integer & (1 << weaponIndex)) != 0;

	int updateInterval = (weapon == WP_RAILGUN) ? 1500 : 1000;
	static int lastUpdateTime = 0;
	static int lastHits = 0;
	static qboolean wasSpectator = qfalse;
	static int lastClientNum = -1;
	qboolean isSpectator = CG_IsSpectatorOnScreen();
	qboolean becameSpectator = (isSpectator && !wasSpectator);
	qboolean clientChanged = (cg.snap ? cg.snap->ps.clientNum : -1) != lastClientNum;

	char textBuffer[MAX_QPATH];

	lastClientNum = cg.snap ? cg.snap->ps.clientNum : -1;

	// update statsInfo if
	if (trackAccuracy &&
	        (ps->weaponstate == WEAPON_FIRING || // firing
	         ps->persistant[PERS_HITS] != lastHits || // hit
	         becameSpectator || clientChanged) && // client changed or free float
	        (cg.time - lastUpdateTime >= updateInterval)) // 1 or 1.5 sec
	{
		lastUpdateTime = cg.time;
		lastHits = ps->persistant[PERS_HITS];
		trap_SendClientCommand("getstatsinfo");
	}

	wasSpectator = isSpectator;

	if (element->weaponIndex == 0)
	{
		if (trackAccuracy && cs->stats[weapon].shots > 0)
		{
			cs->lastTrackedWeapon = weapon;
		}

		if (cs->lastTrackedWeapon < 0)
		{
			if (!SHUD_CHECK_SHOW_EMPTY(element))
				return;
			weapon = ps->weapon;
		}
		else
		{
			weapon = cs->lastTrackedWeapon;
		}
	}

	// If no shots fired, show 0% accuracy or empty icon (based on config)
	if (cs->stats[weapon].shots <= 0)
	{
		if (!SHUD_CHECK_SHOW_EMPTY(element))
			return;

		if (element->type == SHUD_ELEMENT_WEAPON_STATS_TEXT)
		{
			if (element->config.style.value == 1)
				Q_strncpyz(textBuffer, "0.0%", sizeof(textBuffer));
			else
				Q_strncpyz(textBuffer, "0%", sizeof(textBuffer));
			element->textCtx.text = textBuffer;
			CG_SHUDTextPrint(&element->config, &element->textCtx);
		}
		else if (element->type == SHUD_ELEMENT_WEAPON_STATS_ICON)
		{
			element->drawCtx.image = cg_weapons[weapon].weaponIcon;
			CG_SHUDDrawBorder(&element->config);
			CG_SHUDFill(&element->config);
			CG_SHUDDrawStretchPicCtx(&element->config, &element->drawCtx);
		}

		return;
	}

	if (element->type == SHUD_ELEMENT_WEAPON_STATS_TEXT)
	{
		if (element->config.style.value == 1)
			Com_sprintf(textBuffer, sizeof(textBuffer), "%.1f%%", cs->stats[weapon].accuracy);
		else
			Com_sprintf(textBuffer, sizeof(textBuffer), "%.0f%%", cs->stats[weapon].accuracy);

		element->textCtx.text = textBuffer;
		CG_SHUDFillAndFrameForText(&element->config, &element->textCtx);
		CG_SHUDTextPrint(&element->config, &element->textCtx);
	}
	else if (element->type == SHUD_ELEMENT_WEAPON_STATS_ICON)
	{
		element->drawCtx.image = cg_weapons[weapon].weaponIcon;
		CG_SHUDDrawBorder(&element->config);
		CG_SHUDFill(&element->config);
		CG_SHUDDrawStretchPicCtx(&element->config, &element->drawCtx);
	}
}


void CG_SHUDElementWeaponStatsDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
