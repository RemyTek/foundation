#include "cg_superhud.h"
#include "cg_local.h"

int global_handicap; // unused?
//	void         (*onChanged)(cvarTable_t *cvart);


void CG_ParseCvarTwoStrings(const cvarTable_t* cvart, char* out1, char* out2, int outSize)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1, *token2;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string) return;
	if (cvart->vmCvar->string[0] == '\0') return;

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));
	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");

	if (!token1 || !token2) return;

	Q_strncpyz(out1, token1, outSize);
	Q_strncpyz(out2, token2, outSize);
}

void CG_ParseCvarTwoInts(const cvarTable_t* cvart, int* out1, int* out2)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1, *token2;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string) return;
	if (cvart->vmCvar->string[0] == '\0') return;

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));
	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");

	if (!token1 || !token2) return;

	*out1 = atoi(token1);
	*out2 = atoi(token2);
}

void CG_ParseCvarTwoFloats(const cvarTable_t* cvart, float* out1, float* out2)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1, *token2;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string) return;
	if (cvart->vmCvar->string[0] == '\0') return;

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));
	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");

	if (!token1 || !token2) return;

	*out1 = (float)atof(token1);
	*out2 = (float)atof(token2);
}

static void CG_ParseCvarTwoColors(const cvarTable_t* cvart, vec4_t out1, vec4_t out2)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1, *token2;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string) return;
	if (cvart->vmCvar->string[0] == '\0') return;

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));
	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");

	if (!token1 || !token2) return;

	CG_ParseColorStr(token1, out1);
	CG_ParseColorStr(token2, out2);
}

void CG_LocalEventCvarChanged_cg_drawTeamOverlay(cvarTable_t* cvart)
{
	if ((cg_drawTeamOverlay.integer > 0) ||
	        ((!(cgs.osp.server_mode & OSP_SERVER_MODE_PROMODE)) &&
	         (ch_TeamCrosshairHealth.integer > 0 || ch_crosshairTeamInfo.integer > 0)))
	{
		trap_Cvar_Set("teamoverlay", "1");
	}
	else
	{
		trap_Cvar_Set("teamoverlay", "0");
	}
}

void CG_LocalEventCvarChanged_cl_maxpackets(cvarTable_t* cvart)
{
	if (cgs.osp.serverConfigMinimumMaxpackets)
	{
		if (cl_maxpackets.integer < cgs.osp.serverConfigMinimumMaxpackets)
		{
			CG_Printf("^3Server-imposed minimum maxpackets set to %d.  Adjusting...\n", cgs.osp.serverConfigMinimumMaxpackets);
			trap_Cvar_Set("cl_maxpackets", va("%d", cgs.osp.serverConfigMinimumMaxpackets));
		}
	}

	if (cgs.osp.serverConfigMaximumMaxpackets)
	{
		if (cl_maxpackets.integer > cgs.osp.serverConfigMaximumMaxpackets)
		{
			CG_Printf("^3Server-imposed maximum maxpackets set to %d.  Adjusting...\n", cgs.osp.serverConfigMaximumMaxpackets);
			trap_Cvar_Set("cl_maxpackets", va("%d", cgs.osp.serverConfigMaximumMaxpackets));
		}
	}

	if (cgs.osp.nextCvarsUpdateTime == 0)
	{
		cgs.osp.nextCvarsUpdateTime = (cg.time + 1200);
	}
}

void CG_LocalEventCvarChanged_cl_timenudge(cvarTable_t* cvart)
{
	if (cgs.osp.serverConfigMinimumTimenudge)
	{
		if (cgs.osp.serverConfigMinimumTimenudge > cl_timenudge.integer)
		{
			CG_Printf("^3Server-imposed minimum timenudge set to %d.  Adjusting...\n", cgs.osp.serverConfigMinimumTimenudge);
			trap_Cvar_Set("cl_timenudge", va("%d", cgs.osp.serverConfigMinimumTimenudge));
		}
		if (cgs.osp.serverConfigMaximumTimenudge)
		{
			if (cgs.osp.serverConfigMaximumTimenudge < cl_timenudge.integer)
			{
				CG_Printf("^3Server-imposed maximum timenudge set to %d.  Adjusting...\n", cgs.osp.serverConfigMaximumTimenudge);
				trap_Cvar_Set("cl_timenudge", va("%d", cgs.osp.serverConfigMaximumTimenudge));
			}
		}
		if (cgs.osp.nextCvarsUpdateTime == 0)
		{
			cgs.osp.nextCvarsUpdateTime = (cg.time + 1200);
		}
	}
}

void CG_LocalEventCvarChanged_ch_recordMessage(cvarTable_t* cvart)
{
	if (cg.snap)
	{
		trap_Cvar_Set("ui_recordSPDemo", ch_recordMessage.integer <= 0 ? "0" : "1");
	}
}

void CG_LocalEventCvarChanged_cg_followpowerup(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_followkiller(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_followviewcam(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_autoAction(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_snaps(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}
void CG_LocalEventCvarChanged_cg_useScreenShotJPEG(cvarTable_t* cvart)
{
	if (cgs.osp.nextCvarsUpdateTime)
	{
		cgs.osp.nextCvarsUpdateTime = cg.time + 1200;
	}
}

void CG_LocalEventCvarChanged_r_lodCurveError(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_subdivisions(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_znear(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_cg_trueLightning(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_shownormals(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_r_showtris(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
void CG_LocalEventCvarChanged_cg_shadows(cvarTable_t* cvart)
{
	CG_OSPCvarsRestrictValues();
}
// void CG_LocalEventCvarChanged_r_fullbright(cvarTable_t* cvart)
// {
//  CG_OSPCvarsRestrictValues();
// }


void CG_LocalEventCvarChanged_cg_swapSkins(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyModel(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamModel(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}


void CG_LocalEventCvarChanged_handicap(cvarTable_t* cvart)
{
	if (!cg.warmup)
	{
		if (cgs.gametype != GT_FFA && cgs.gametype != GT_SINGLE_PLAYER && handicap.integer < 5)
		{
			global_handicap = 100;
			trap_Cvar_Set("handicap", va("%d", global_handicap));
		}
	}
	global_handicap = handicap.integer;
}

void CG_LocalEventCvarChanged_s_ambient(cvarTable_t* cvart)
{
	trap_SendConsoleCommand("vid_restart;\n");
}

void CG_LocalEventCvarChanged_pmove_fixed(cvarTable_t* cvart)
{
	if (cgs.osp.allow_pmove == 0 && pmove_fixed.integer)
	{
		CG_Printf("^3Enhanced client sampling has been disabled on this server.\n");
		trap_Cvar_Set("pmove_fixed", "0");
	}
}

void CG_LocalEventCvarChanged_cg_hitSounds(cvarTable_t* cvart)
{
	if (!(BE_IsDamageInfoAllowed()) && cg_hitSounds.integer)
	{
		CG_Printf("^3Damage info has been disabled on this server.\n");
		// trap_Cvar_Set("cg_hitSounds", "0");
	}
}

void CG_LocalEventCvarChanged_cg_playerModelColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_playerRailColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_playerFrozenColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamModelColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamRailColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_teamFrozenColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyModelColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyRailColors(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_enemyFrozenColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_fragSound(cvarTable_t* cvart)
{
	/* touch fragsound to reload it */
	(void)CG_GetFragSound();
}

void CG_LocalEventCvarChanged_ch_file(cvarTable_t* cvart)
{
	if (cvart->vmCvar->string[0])
	{
		CG_SHUDLoadConfig();
	}
}

void CG_LocalEventCvarChanged_cg_shud(cvarTable_t* cvart)
{
	if (cvart->vmCvar->integer && ch_file.string[0])
	{
		CG_SHUDLoadConfig();
	}
}

static void CG_LocalEventCvarParseColor(cvarTable_t* cvart, vec4_t color)
{
	if (!CG_ParseColorStr(cvart->vmCvar->string, color))
	{
		Com_Printf("^1Color is wrong, default value used\n");
		CG_ParseColorStr(cvart->defaultString, color);
	}
}

void CG_LocalEventCvarChanged_ch_crosshairColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.color);
}

void CG_LocalEventCvarChanged_ch_crosshairActionColor(cvarTable_t* cvart)
{
	if (Q_stricmp(cvart->cvarName, "ch_crosshairActionColor") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.actionColor);
	}
	if (Q_stricmp(cvart->cvarName, "ch_crosshairActionColorLow") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.actionColorLow);
	}
	if (Q_stricmp(cvart->cvarName, "ch_crosshairActionColorMid") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.actionColorMid);
	}
	if (Q_stricmp(cvart->cvarName, "ch_crosshairActionColorHigh") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.actionColorHigh);
	}
}

void CG_LocalEventCvarChanged_ch_crosshairDecorColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.decorColor);
}

void CG_LocalEventCvarChanged_ch_crosshairDecorActionColor(cvarTable_t* cvart)
{
	if (Q_stricmp(cvart->cvarName, "ch_crosshairDecorActionColor") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.decorActionColor);
	}
	if (Q_stricmp(cvart->cvarName, "ch_crosshairDecorActionColorLow") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.decorActionColorLow);
	}
	if (Q_stricmp(cvart->cvarName, "ch_crosshairDecorActionColorMid") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.decorActionColorMid);
	}
	if (Q_stricmp(cvart->cvarName, "ch_crosshairDecorActionColorHigh") == 0)
	{
		CG_LocalEventCvarParseColor(cvart, cgs.osp.crosshair.decorActionColorHigh);
	}
}


void CG_LocalEventCvarChanged_ch_crosshairDecorOpaque(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0, 1, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_ch_crosshairOpaque(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0, 1, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_ch_crosshairActionScale(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0.1, 10, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_ch_crosshairDecorActionScale(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0.1, 10, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_ch_crosshairActionTime(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(50, 1000, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_ch_crosshairDecorActionTime(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(50, 1000, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_cg_damageIndicatorOpaque(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0, 1, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_cg_dlightGauntlet(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_GAUNTLET);
}

void CG_LocalEventCvarChanged_cg_dlightMG(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_MACHINEGUN);
}

void CG_LocalEventCvarChanged_cg_dlightSG(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_SHOTGUN);
}

void CG_LocalEventCvarChanged_cg_dlightGL(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_GRENADE_LAUNCHER);
}

void CG_LocalEventCvarChanged_cg_dlightRL(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_ROCKET_LAUNCHER);
}

void CG_LocalEventCvarChanged_cg_dlightLG(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_LIGHTNING);
}

void CG_LocalEventCvarChanged_cg_dlightRG(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_RAILGUN);
}

void CG_LocalEventCvarChanged_cg_dlightPG(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_PLASMAGUN);
}

void CG_LocalEventCvarChanged_cg_dlightBFG(cvarTable_t* cvart)
{
	CG_UpdateWeaponDlightColor(WP_BFG);
}

void CG_LocalEventCvarChanged_cg_conObituaries(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0, 1, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_cg_enemyLightningColor(cvarTable_t* cvart)
{
	CG_RebuildPlayerColors();
	CG_UpdateAllClientsInfo();
}

void CG_LocalEventCvarChanged_cg_drawHitBox(cvarTable_t* cvart)
{
	if (cgs.osp.serverConfigXHitBox != 1 && cg_drawHitBox.integer)
	{
		if (!cg.demoPlayback)
		{
			CG_Printf("^3HitBox is disabled on this server.\n");
		}
	}
}

void CG_LocalEventCvarChanged_cg_hitBoxColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.hitBoxColor);
}

void CG_LocalEventCvarChanged_cg_damageFrameSize(cvarTable_t* cvart)
{
	if (cg_damageDrawFrame.integer == 1)
	{
		cvart->vmCvar->value = Com_Clamp(2, 512, cvart->vmCvar->value);
	}
	else
		cvart->vmCvar->value = Com_Clamp(1, 512, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_cg_damageFrameOpaque(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0.1f, 1.0f, cvart->vmCvar->value);

}
void CG_LocalEventCvarChanged_cg_damageDrawFrame(cvarTable_t* cvart)
{
	CG_CvarTouch("cg_damageFrameSize");
}

void CG_LocalEventCvarChanged_cg_enemyOutlineColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.enemyOutlineColor);
}

void CG_LocalEventCvarChanged_cg_teamOutlineColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.teamOutlineColor);
}

void CG_LocalEventCvarChanged_cg_altBloodColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.altBloodColor);
}

void CG_LocalEventCvarChanged_cg_railRingsRadius(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0.05f, 16, cvart->vmCvar->value);
}

void CG_LocalEventCvarChanged_cg_railRingsRotation(cvarTable_t* cvart)
{
	cvart->vmCvar->integer = Com_Clamp(0, 2, cvart->vmCvar->integer);
}

void CG_LocalEventCvarChanged_cg_railRingsSpacing(cvarTable_t* cvart)
{
	cvart->vmCvar->integer = Com_Clamp(4, 128, cvart->vmCvar->integer);
}

void CG_LocalEventCvarChanged_cg_railRingsSize(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0.05f, 4, cvart->vmCvar->value);
}
void CG_LocalEventCvarChanged_cg_altShadowColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.altShadowColor);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorAdjust(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0.5f, 2, cvart->vmCvar->value);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.playerIndicatorColor);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorOpaque(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0, 1, cvart->vmCvar->value);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorBgColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.playerIndicatorBgColor);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorBgOpaque(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(0, 1, cvart->vmCvar->value);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorOffset(cvarTable_t* cvart)
{
	cvart->vmCvar->value = Com_Clamp(-16, 32, cvart->vmCvar->value);
}
void CG_LocalEventCvarChanged_cg_teamIndicatorMaxLength(cvarTable_t* cvart)
{
	cvart->vmCvar->integer = Com_Clamp(1, MAX_QPATH, cvart->vmCvar->integer);
}
void CG_LocalEventCvarChanged_cg_scoreboardFont(cvarTable_t* cvart)
{
	if (!CG_FontAvailable(cvart->vmCvar->integer))
	{
		trap_Cvar_Set("cg_scoreboardFont", 0);
	}
	// CG_ScoreboardContext_Init();
}
void CG_LocalEventCvarChanged_cg_teamIndicatorFont(cvarTable_t* cvart)
{
	if (!CG_FontAvailable(cvart->vmCvar->integer))
	{
		trap_Cvar_Set("cg_teamIndicatorFont", 0);
	}
}
void CG_LocalEventCvarChanged_cg_centerMessagesFont(cvarTable_t* cvart)
{
	if (!CG_FontAvailable(cvart->vmCvar->integer))
	{
		trap_Cvar_Set("cg_centerMessagesFont", 0);
	}
}
void CG_LocalEventCvarChanged_cg_altGrenadesColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.altGrenadesColor);
}
void CG_LocalEventCvarChanged_cg_enemyGrenadesColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.enemyGrenadesColor);
}
void CG_LocalEventCvarChanged_cg_healthColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.healthColor);
}
void CG_LocalEventCvarChanged_cg_healthLowColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.healthLowColor);
}
void CG_LocalEventCvarChanged_cg_healthMidColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.healthMidColor);
}
void CG_LocalEventCvarChanged_cg_redTeamColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.redTeamColor);
	CG_LocalEventCvarParseColor(cvart, scoreboard_rtColor);

}
void CG_LocalEventCvarChanged_cg_blueTeamColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.blueTeamColor);
	CG_LocalEventCvarParseColor(cvart, scoreboard_btColor);
}
void CG_LocalEventCvarChanged_cg_accuracyFont(cvarTable_t* cvart)
{
	if (!CG_FontAvailable(cvart->vmCvar->integer))
	{
		trap_Cvar_Set("cg_accuracyFont", 0);
	}
}

void CG_LocalEventCvarChanged_cg_markTeam(cvarTable_t* cvart)
{
	int clientNum;
	const char* str = cvart->vmCvar->string;

	if (CG_BE_FEATURE_ENABLED(CG_BE_MARK_TEAMMATE) && cg_markTeam.integer != 1)
		{
		memset(cgs.be.markedTeam, qfalse, sizeof(cgs.be.markedTeam));

		if (Q_stricmp(str, "-1") == 0)
		{
			return;
		}

		while (*str)
		{
			while (*str == ' ')
			{
				str++;
			}

			if (!*str)
				break;

			clientNum = atoi(str);

			if (clientNum >= 0 && clientNum < MAX_CLIENTS)
			{
				cgs.be.markedTeam[clientNum] = qtrue;
			}
			else
			{
				CG_Printf("cg_markTeam: invalid clientNum %d\n", clientNum);
			}

			while (*str && *str != ' ')
			{
				str++;
			}
		}
	}
	else
	{
		trap_Cvar_Set("cg_markTeam", "-1");
		CG_Printf("^2cg_markTeam^7 ^1disabled^7 on this server!\n");
	}
}

void CG_LocalEventCvarChanged_cg_markTeamColor(cvarTable_t* cvart)
{
	CG_LocalEventCvarParseColor(cvart, cgs.be.markedTeamColor);
}

void CG_LocalEventCvarChanged_cg_customSound(cvarTable_t* cvart)
{
	if (CG_BE_FEATURE_ENABLED(CG_BE_MODELSOUND))
	{
		CG_LoadForcedSounds();
	}
	else
	{
		CG_Printf("^7Cusom sound ^1disabled ^7on this server!");
	}
}

void CG_LocalEventCvarChanged_cg_scoreboardRtColors(cvarTable_t* cvart)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1;
	char* token2;
	char* token3;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string)
	{
		CG_Printf("^1Invalid cvar or string pointer\n");
		return;
	}

	if (cvart->vmCvar->string[0] == '\0')
	{
		customScoreboardColorIsSet_red = 0;
		return;
	}

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));

	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");
	token3 = Q_strtok(NULL, " \t");

	if (!token1 || !token2)
	{
		CG_Printf("^1Invalid value: expected at least two color values (e.g. \"red blue\")\n");
		customScoreboardColorIsSet_red = 0;
		return;
	}

	CG_ParseColorStr(token1, scoreboard_rtColorHeader);
	CG_ParseColorStr(token2, scoreboard_rtColorBody);
	customScoreboardColorIsSet_red = 1;
	if (token3)
	{
		CG_ParseColorStr(token3, scoreboard_rtColorTitle);
		customScoreboardColorIsSet_red |= 2;
	}
}

void CG_LocalEventCvarChanged_cg_scoreboardBtColors(cvarTable_t* cvart)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1;
	char* token2;
	char* token3;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string)
	{
		CG_Printf("^1Invalid cvar or string pointer\n");
		return;
	}

	if (cvart->vmCvar->string[0] == '\0')
	{
		customScoreboardColorIsSet_blue = 0;
		return;
	}

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));

	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");
	token3 = Q_strtok(NULL, " \t");

	if (!token1 || !token2)
	{
		CG_Printf("^1Invalid value: expected at least two color values (e.g. \"1 3\" or \"990033 green\")\n");
		customScoreboardColorIsSet_blue = 0;
		return;
	}

	CG_ParseColorStr(token1, scoreboard_btColorHeader);
	CG_ParseColorStr(token2, scoreboard_btColorBody);
	customScoreboardColorIsSet_blue = 1;
	if (token3)
	{
		CG_ParseColorStr(token3, scoreboard_btColorTitle);
		customScoreboardColorIsSet_blue |= 2;
	}
}

void CG_LocalEventCvarChanged_cg_scoreboardSpecColor(cvarTable_t* cvart)
{
	char buffer[MAX_CVAR_VALUE_STRING];
	char* token1;
	char* token2;

	if (!cvart || !cvart->vmCvar || !cvart->vmCvar->string)
	{
		CG_Printf("^1Invalid cvar or string pointer\n");
		customScoreboardColorIsSet_spec = 0;
		return;
	}

	if (cvart->vmCvar->string[0] == '\0')
	{
		customScoreboardColorIsSet_spec = 0;
		return;
	}

	Q_strncpyz(buffer, cvart->vmCvar->string, sizeof(buffer));
	token1 = Q_strtok(buffer, " \t");
	token2 = Q_strtok(NULL, " \t");

	if (!token1)
	{
		customScoreboardColorIsSet_spec = 0;
		return;
	}

	CG_ParseColorStr(token1, scoreboard_specColor);
	customScoreboardColorIsSet_spec = 1;
	if (token2)
	{
		CG_ParseColorStr(token2, scoreboard_specColorTitle);
		customScoreboardColorIsSet_spec |= 2;
	}
}


void CG_LocalEventCvarChanged_cg_bestats_font(cvarTable_t* cvart)
{
	if (!CG_FontAvailable(cvart->vmCvar->integer))
	{
		trap_Cvar_Set("cg_bestats_font", 0);
	}
}

void CG_LocalEventCvarChanged_cg_bestats_pos(cvarTable_t* cvart)
{
	CG_ParseCvarTwoFloats(cvart, &cgs.be.settings.x, &cgs.be.settings.y);
}

void CG_LocalEventCvarChanged_cg_bestats_textSize(cvarTable_t* cvart)
{
	CG_ParseCvarTwoFloats(cvart, &cgs.be.settings.textSize[0], &cgs.be.settings.textSize[1]);
}
void CG_LocalEventCvarChanged_cg_bestats_bgColor(cvarTable_t* cvart)
{
	if (!cvart->vmCvar->string[0])
	{
		cgs.be.settings.bgColorIsSet = qfalse;
	}
	else
	{
		CG_LocalEventCvarParseColor(cvart, cgs.be.settings.bgColor);
		cgs.be.settings.bgColorIsSet = qtrue;
	}
}

void CG_LocalEventBeFeaturesChanged(cvarTable_t* cvart)
{
	CG_UpdateBeFeatures();
}

void CG_LocalEventCvarChanged_cg_scoreboardTextSize(cvarTable_t* cvart)
{
	CG_ParseCvarTwoFloats(cvart, &cgs.be.sbSettings.textSize[0], &cgs.be.sbSettings.textSize[1]);
	// CG_ScoreboardContext_Init();
}
void CG_LocalEventCvarChanged_cg_scoreboardScale(cvarTable_t* cvart)
{
	// CG_ScoreboardContext_Init();
}
void CG_LocalEventCvarChanged_chud_file(cvarTable_t* cvart)
{
	// Initialize CherryHUD parser first
	CG_CHUDLoadConfig();
}
void CG_LocalEventCvarChanged_cg_chud(cvarTable_t* cvart)
{
	// Initialize CherryHUD parser first
	CG_CHUDLoadConfig();
}
