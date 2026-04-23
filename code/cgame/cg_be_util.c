#include "cg_local.h"
#include "../qcommon/qcommon.h"



qboolean CG_BE_Timer(int msec)
{
	static int lastTime = 0;
	int currentTime = cg.time;

	int diff = currentTime - lastTime;
	if (diff < 0)
	{
		lastTime = currentTime;
		return qfalse;
	}
	if (diff >= msec)
	{
		lastTime = currentTime;
		return qtrue;
	}
	return qfalse;
}

void CG_UpdateBeFeatures(void)
{

	qboolean changed = qfalse;
	int i = 0;
	int beFlags = 0;
	clientInfo_t* ci = &cgs.clientinfo[cg.clientNum];
	clientInfo_t* otherCi = &cgs.clientinfo[i];

	qboolean isFbSkin = qfalse;

	for (i = 0; i < MAX_CLIENTS; i++) {
		if (i == cg.clientNum) continue; // Skip self
			if (otherCi->infoValid && Q_stricmp(otherCi->skinName, "fb") == 0) {
				isFbSkin = qtrue;
				break;
		}
	}

	if (!BE_ENABLED)
	{
		beFlags = 0;
		changed = qtrue;
		CG_RebuildPlayerColors();
		CG_UpdateAllClientsInfo();
	}
	else
	{
		if (cg_friendsWallhack.integer)
		{
			beFlags |= CG_BE_TEAM_FOE_WH;
			changed = qtrue;
		}
		if (cg_altGrenades.integer == 2)
		{
			beFlags |= CG_BE_ALT_GRENADES;
			changed = qtrue;
		}
		if (cg_drawOutline.integer)
		{
			beFlags |= CG_BE_OUTLINE;
			changed = qtrue;
		}
		if (cgs.customModelSound)
		{
			beFlags |= CG_BE_MODELSOUND;
			changed = qtrue;
		}
		if (cg_markTeam.integer >= 0)
		{
			beFlags |= CG_BE_MARK_TEAMMATE;
			changed = qtrue;
		}
		if (cg_teamIndicator.integer)
		{
			beFlags |= CG_BE_TEAM_INDICATOR;
			changed = qtrue;
		}
		if (cg_altShadow.integer)
		{
			beFlags |= CG_BE_ALT_SHADOW;
			changed = qtrue;
		}
		if (cg_altBlood.integer)
		{
			beFlags |= CG_BE_ALT_BLOOD;
			changed = qtrue;
		}
		if (cg_enemyLightningColor.integer || cg_enemyLightningColor.string)
		{
			beFlags |= CG_BE_ENEMYLIGHTNING;
			changed = qtrue;
		}
		if (isFbSkin || cg_drawBrightWeapons.integer)
		{
			beFlags |= CG_BE_FULLBRIGHT;
			changed = qtrue;
		}
	}

	if (changed)
	{
		trap_Cvar_Set("be_features", va("%d", beFlags));
	}
}

void CG_PrintDisabledFeatures(qboolean request)
{
	int value = cgs.be.disableFeatures;
	if (value == 0)
	{
		if (request)
		{
		CG_Printf("   [^9OSP2-BE^7] All features enabled.\n");
		}
	}
	else if (value == 4095) // all bits set
	{
		CG_Printf("   [^9OSP2-BE^7] All features disabled.\n");
	}
	else
	{
		char buffer[1024] = "   [^9OSP2-BE^7] Disabled features: ";
		qboolean first = qtrue;
		if (value & CG_BE_TEAM_FOE_WH) { Q_strcat(buffer, sizeof(buffer), first ? "Team wh" : ", team wh"); first = qfalse; }
		if (value & CG_BE_MODELSOUND) { Q_strcat(buffer, sizeof(buffer), first ? "Model sound" : ", model sound"); first = qfalse; }
		if (value & CG_BE_ALT_GRENADES) { Q_strcat(buffer, sizeof(buffer), first ? "Alt grenades" : ", alt grenades"); first = qfalse; }
		if (value & CG_BE_ENEMYLIGHTNING) { Q_strcat(buffer, sizeof(buffer), first ? "Enemy lightning" : ", enemy lightning"); first = qfalse; }
		if (value & CG_BE_MARK_TEAMMATE) { Q_strcat(buffer, sizeof(buffer), first ? "Mark teammate" : ", mark teammate"); first = qfalse; }
		if (value & CG_BE_ALT_SHADOW) { Q_strcat(buffer, sizeof(buffer), first ? "Alt shadow" : ", alt shadow"); first = qfalse; }
		if (value & CG_BE_ALT_BLOOD) { Q_strcat(buffer, sizeof(buffer), first ? "Alt blood" : ", alt blood"); first = qfalse; }
		if (value & CG_BE_OUTLINE) { Q_strcat(buffer, sizeof(buffer), first ? "Outline" : ", outline"); first = qfalse; }
		if (value & CG_BE_TEAM_INDICATOR) { Q_strcat(buffer, sizeof(buffer), first ? "team indicator" : ", team indicator"); first = qfalse; }
		if (value & CG_BE_DAMAGEINFO) { Q_strcat(buffer, sizeof(buffer), first ? "Damage count" : ", damage count"); first = qfalse; }
		if (value & CG_BE_FULLBRIGHT) { Q_strcat(buffer, sizeof(buffer), first ? "Fullbright" : ", fullbright"); first = qfalse; }
		CG_Printf("%s\n", buffer);
	}
}

void CG_BEParseXStatsToStatsAll(void) {
	int i;
	int index = 1;
	int xstats_condition;
	int client_id;
	int hits_value, atts_value, kills_value, deaths_value;
	int pickUps, drops;
	float accuracy;
	int armor_taken, health_taken, megahealth, green_armor, red_armor, yellow_armor;
	int damage_given, damage_rcvd;
	int totalKills, totalDeaths;
	newStatsInfo_t* ws;

	client_id = atoi(CG_Argv(index++));
	xstats_condition = atoi(CG_Argv(index++));

	if (client_id < 0 || client_id >= MAX_CLIENTS)
	{
		return;
	}

	ws = &cgs.be.statsAll[client_id];

	for (i = 1; i < WP_NUM_WEAPONS; ++i)
	{
		if ((xstats_condition & (1 << i)) != 0)
		{
			hits_value = atoi(CG_Argv(index++));
			atts_value = atoi(CG_Argv(index++));
			kills_value = atoi(CG_Argv(index++));
			deaths_value = atoi(CG_Argv(index++));

			pickUps = atts_value >> cgs.osp.stats_shift;
			drops   = hits_value >> cgs.osp.stats_shift;

			ws->stats[i].hits = hits_value & cgs.osp.stats_mask;
			ws->stats[i].shots = atts_value & cgs.osp.stats_mask;
			ws->stats[i].kills = kills_value;
			ws->stats[i].deaths = deaths_value;
			ws->stats[i].pickUps = pickUps;
			ws->stats[i].drops = drops;
			ws->stats[i].accuracy = (ws->stats[i].shots > 0) ? ((float)ws->stats[i].hits / ws->stats[i].shots) * 100.0f : 0.0f;
		}
		else
		{
			ws->stats[i].hits = 0;
			ws->stats[i].shots = 0;
			ws->stats[i].kills = 0;
			ws->stats[i].deaths = 0;
			ws->stats[i].pickUps = 0;
			ws->stats[i].drops = 0;
			ws->stats[i].accuracy = 0.0f;
		}
	}

	armor_taken   = atoi(CG_Argv(index++));
	health_taken  = atoi(CG_Argv(index++));
	damage_given  = atoi(CG_Argv(index++));
	damage_rcvd   = atoi(CG_Argv(index++));
	megahealth    = atoi(CG_Argv(index++));
	green_armor   = atoi(CG_Argv(index++));
	red_armor     = atoi(CG_Argv(index++));
	yellow_armor  = atoi(CG_Argv(index++));

	ws->armor      = armor_taken;
	ws->health     = health_taken;
	ws->megahealth = megahealth;
	ws->ga         = green_armor;
	ws->ra         = red_armor;
	ws->ya         = yellow_armor;
	ws->dmgGiven   = damage_given;
	ws->dmgReceived= damage_rcvd;

	ws->damageRatio = (damage_given > 0 || damage_rcvd > 0) ?
	                  (float)damage_given / (damage_rcvd > 0 ? damage_rcvd : 1) : 0.0f;

	totalKills = 0;
	totalDeaths = 0;
	for (i = 1; i < WP_NUM_WEAPONS; ++i)
	{
		totalKills += ws->stats[i].kills;
		totalDeaths += ws->stats[i].deaths;
	}
	ws->kills = totalKills;
	ws->deaths = totalDeaths;
	ws->kdratio = (totalKills > 0 && totalDeaths == 0) ? (float)totalKills :
	              (totalDeaths > 0) ? (float)totalKills / totalDeaths : 0.0f;

	ws->efficiency = (totalKills + totalDeaths > 0) ?
	                 (100.0f * (float)totalKills / (totalKills + totalDeaths)) : 0.0f;
	if (ws->efficiency < 0.0f)
		ws->efficiency = 0.0f;

	ws->infoValid = qtrue;
}

void cg_printsa_f(void)
{
    int i;
    newStatsInfo_t *ws;

    CG_Printf("^5StatsAll:\n");
    for (i = 0; i < MAX_CLIENTS; ++i) {
        ws = &cgs.be.statsAll[i];
        if (!ws->infoValid)
            continue;

        CG_Printf("^7[%2d] ^3%-16s ^7K/D:^2%3d^7/^1%3d ^7Eff:^2%5.1f%% ^7Dmg:^2%i^7/^1%i ^7Ratio:^2%.2f\n",
            i,
            cgs.clientinfo[i].name,
            ws->kills,
            ws->deaths,
            ws->efficiency,
            ws->dmgGiven,
            ws->dmgReceived,
            ws->damageRatio
		);
    }
}

void CG_ParseCvarTwoColorsSimple(const vmCvar_t* cvar, vec4_t out1, vec4_t out2)
{
    char buffer[MAX_CVAR_VALUE_STRING];
    char* token1, *token2;

    if (!cvar || !cvar->string) return;
    if (cvar->string[0] == '\0') return;

    Q_strncpyz(buffer, cvar->string, sizeof(buffer));
    token1 = Q_strtok(buffer, " \t");
    token2 = Q_strtok(NULL, " \t");

    if (!token1 || !token2) return;

    CG_ParseColorStr(token1, out1);
    CG_ParseColorStr(token2, out2);
}

qboolean BE_IsDamageInfoAllowed(void)
{
	// Allow damage info if either BE_FEATURE_ENABLED or OSP_CUSTOM_CLIENT_2 allows it
	return (CG_BE_FEATURE_ENABLED(CG_BE_DAMAGEINFO) || OSP_CUSTOM_CLIENT_2_IS_DMG_INFO_ALLOWED());
}
