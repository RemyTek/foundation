#include "cg_local.h"

const vec3_t uniqueColorTable[UNIQUE_COLORS_TABLE_SIZE] =
{
	{ 1, 0, 0 },       /* Red */
	{ 0, 1, 0 },       /* Green */
	{ 0, 0, 1 },       /* Blue */
	{ 1, 1, 0 },       /* Yellow */
	{ 0, 1, 1 },       /* Cyan */
	{ 1, 0, 1 },       /* Magenta */
	{ 1, 0.65, 0 },    /* Orange */
	{ 0, 0.75, 1 },    /* Sky Blue */
	{ 0, 1, 0.6 },     /* Aqua Green */
	{ 1, 0.35, 0.35 }, /* Soft Red */
	{ 0.93, 0.5, 0.93 }, /* Purple */
	{ 1, 0.35, 0 },    /* Tomato */
	{ 0.35, 1, 0.35 }, /* Lime Green */
	{ 1, 1, 0.35 },    /* Light Yellow */
	{ 0, 0.75, 0.75 }, /* Teal */
	{ 1, 0.5, 0.5 },   /* Light Red */
	{ 1, 0.4, 0.75 },  /* Pink */
	{ 0.25, 0.75, 0.5 }, /* Mint Green */
	{ 0.5, 0, 0 },     /* Dark Red */
	{ 0.5, 0, 0.5 },   /* Dark Magenta */
	{ 0, 0.55, 0.55 }, /* Dark Teal */
	{ 0.3, 0, 0.5 },   /* Indigo */
	{ 0.5, 0.5, 0 },   /* Olive */
	{ 1, 1, 1 },       /* White */
};

const vec3_t uniqueColorTableLight[UNIQUE_COLORS_TABLE_SIZE] =
{
	{ 0, 1, 0 },       /* Green */
	{ 1, 1, 0 },       /* Yellow */
	{ 0, 1, 1 },       /* Cyan */
	{ 1, 0.4, 1 },     /* Soft Magenta */
	{ 1, 0.67, 0.32 }, /* Orange */
	{ 0.37, 1, 0.37 }, /* Lime Green */
	{ 0, 0.75, 1 },    /* Sky Blue */
	{ 1, 0.35, 0.35 }, /* Soft Red */
	{ 0.93, 0.5, 0.93 }, /* Purple */
	{ 1, 0.35, 0.47 }, /* Light Tomato */
	{ 0.9, 0.9, 0.5 }, /* Light Gold */
	{ 0.5, 0.9, 0.9 }, /* Light Cyan */
	{ 0.5, 0.5, 1 },   /* Light Blue */
	{ 1, 1, 0.35 },    /* Light Yellow */
	{ 0.1, 0.85, 0.85 }, /* Teal */
	{ 1, 0.6, 0.6 },   /* Light Red */
	{ 1, 0.4, 0.75 },  /* Pink */
	{ 0.25, 0.75, 0.5 }, /* Mint Green */
	{ 1, 0.8, 0.6 },   /* Peach */
	{ 0.6, 1, 0.8 },   /* Light Turquoise */
	{ 0.8, 0.6, 1 },   /* Lavender */
	{ 1, 0.6, 0.8 },   /* Light Pink */
	{ 0.6, 0.8, 1 },   /* Light Purple */
	{ 0, 1, 0.6 }      /* Aqua Green */
};

void CG_PlayerColorsFromCS(playerColors_t* colors, playerColorsOverride_t* override, const char* color1, const char* color2)
{
	int color1Len = color1 ? strlen(color1) : 0;
	int color2Len = color2 ? strlen(color2) : 0;

	if (color1Len > 0)
	{
		if (color1[0] != '0')
		{
			if (override) override->isRailColorSet = qtrue;
			CG_OSPColorFromChar(color1[0], colors->railCore);
		}

		if (color2Len > 0)
		{
			CG_OSPColorFromChar(color2[0], colors->railRings);
		}
		else
		{
			VectorCopy(colorYellow, colors->railRings);
		}
	}

	if (color1Len >= 4)
	{
		if (override) override->isModelColorSet = qtrue;
		CG_OSPColorFromChar(color1[1], colors->head);
		CG_OSPColorFromChar(color1[2], colors->torso);
		CG_OSPColorFromChar(color1[3], colors->legs);
	}

}

static void CG_PlayerColorsFromEnemyColors(playerColors_t* colors, playerColorsOverride_t* override, const char* color1)
{
	int color1Len = color1 ? strlen(color1) : 0;

	if (color1Len >= 4)
	{
		if (cg_teamRails.integer == 2 && color1[0] != '0')
		{
			if (override) override->isRailColorSet = qtrue;
			CG_OSPColorFromChar(color1[0], colors->railCore);
		}

		if (override) override->isModelColorSet = qtrue;
		CG_OSPColorFromChar(color1[1], colors->head);
		CG_OSPColorFromChar(color1[2], colors->torso);
		CG_OSPColorFromChar(color1[3], colors->legs);
	}
	else if (color1Len == 3)
	{
		if (override) override->isModelColorSet = qtrue;
		CG_OSPColorFromChar(color1[0], colors->head);
		CG_OSPColorFromChar(color1[1], colors->torso);
		CG_OSPColorFromChar(color1[2], colors->legs);
	}

}

static void CG_PlayerColorsFromTeamColors(playerColors_t* colors, playerColorsOverride_t* override, const char* color1)
{
	int color1Len = color1 ? strlen(color1) : 0;

	if (color1Len >= 4)
	{
		if (cg_teamRails.integer == 2 && color1[0] != '0')
		{
			if (override) override->isRailColorSet = qtrue;
			CG_OSPColorFromChar(color1[0], colors->railCore);
		}

		if (override) override->isModelColorSet = qtrue;
		CG_OSPColorFromChar(color1[1], colors->head);
		CG_OSPColorFromChar(color1[2], colors->torso);
		CG_OSPColorFromChar(color1[3], colors->legs);
	}
	else if (color1Len == 3)
	{
		if (override) override->isModelColorSet = qtrue;
		CG_OSPColorFromChar(color1[0], colors->head);
		CG_OSPColorFromChar(color1[1], colors->torso);
		CG_OSPColorFromChar(color1[2], colors->legs);
	}

}

void CG_PlayerColorsLoadDefault(playerColors_t* colors)
{
	VectorCopy(colorGreen, colors->head);
	VectorCopy(colorGreen, colors->torso);
	VectorCopy(colorGreen, colors->legs);

	VectorCopy(colorWhite, colors->railCore);
	VectorCopy(colorWhite, colors->railRings);
	VectorCopy(colorWhite, colors->frozen);
}


static void CG_PlayerColorsLoadOverrides(playerColors_t* colors,
        playerColorsOverride_t* override,
        vmCvar_t* modelColors,
        vmCvar_t* railColors,
        vmCvar_t* frozenColor,
        vmCvar_t* lightningColor)
{
	if (modelColors)
	{
		int len = strlen(modelColors->string);
		// one number per color format
		if (len == 3)
		{
			if (override) override->isModelColorSet = qtrue;
			CG_OSPColorFromChar(modelColors->string[0], colors->head);
			CG_OSPColorFromChar(modelColors->string[1], colors->torso);
			CG_OSPColorFromChar(modelColors->string[2], colors->legs);
		}
		// 8bit hex format: rrggbb rrggbb rrggbb
		if (len == 18)
		{
			if (override) override->isModelColorSet = qtrue;
			CG_Hex16GetColor(&modelColors->string[0], &colors->head[0]);
			CG_Hex16GetColor(&modelColors->string[2], &colors->head[1]);
			CG_Hex16GetColor(&modelColors->string[4], &colors->head[2]);

			CG_Hex16GetColor(&modelColors->string[6], &colors->torso[0]);
			CG_Hex16GetColor(&modelColors->string[8], &colors->torso[1]);
			CG_Hex16GetColor(&modelColors->string[10], &colors->torso[2]);

			CG_Hex16GetColor(&modelColors->string[12], &colors->legs[0]);
			CG_Hex16GetColor(&modelColors->string[14], &colors->legs[1]);
			CG_Hex16GetColor(&modelColors->string[16], &colors->legs[2]);
		}
	}

	if (railColors)
	{
		int len = strlen(railColors->string);
		// one number per color format
		if (len == 2)
		{
			if (override) override->isRailColorSet = qtrue;
			CG_OSPColorFromChar(railColors->string[0], colors->railCore);
			CG_OSPColorFromChar(railColors->string[1], colors->railRings);
		}
		// 8bit hex format: rrggbbrrggbb
		if (len == 12)
		{
			if (override) override->isRailColorSet = qtrue;
			CG_Hex16GetColor(&railColors->string[0], colors->railCore);
			CG_Hex16GetColor(&railColors->string[3], colors->railRings);

			CG_Hex16GetColor(&railColors->string[0], &colors->railCore[0]);
			CG_Hex16GetColor(&railColors->string[2], &colors->railCore[1]);
			CG_Hex16GetColor(&railColors->string[4], &colors->railCore[2]);

			CG_Hex16GetColor(&railColors->string[6], &colors->railRings[0]);
			CG_Hex16GetColor(&railColors->string[8], &colors->railRings[1]);
			CG_Hex16GetColor(&railColors->string[10], &colors->railRings[2]);
		}
	}

	if (frozenColor)
	{
		int len = strlen(frozenColor->string);
		// one number per color format
		if (len == 1)
		{
			if (override) override->isFrozenColorSet = qtrue;
			CG_OSPColorFromChar(frozenColor->string[0], colors->frozen);
		}

		// 8bit hex format: rrggbb
		if (len == 6)
		{
			if (override) override->isFrozenColorSet = qtrue;
			CG_Hex16GetColor(&frozenColor->string[0], &colors->frozen[0]);
			CG_Hex16GetColor(&frozenColor->string[2], &colors->frozen[1]);
			CG_Hex16GetColor(&frozenColor->string[4], &colors->frozen[2]);
		}
	}
	if (lightningColor)
	{
		int len = strlen(lightningColor->string);
		// one number per color format
		if (len == 1)
		{
			if (override) override->isLightningColorSet = qtrue;
			CG_OSPColorFromChar(lightningColor->string[0], colors->lightning);
		}

		// 8bit hex format: rrggbb
		if (len == 6)
		{
			if (override) override->isLightningColorSet = qtrue;
			CG_Hex16GetColor(&lightningColor->string[0], &colors->lightning[0]);
			CG_Hex16GetColor(&lightningColor->string[2], &colors->lightning[1]);
			CG_Hex16GetColor(&lightningColor->string[4], &colors->lightning[2]);
		}
	}
}

static void CG_RebuildOurPlayerColors(void)
{
	char color1[8];
	char color2[8];
	/* our colors allways is set */
	CG_PlayerColorsLoadDefault(&cgs.osp.myColors);

	trap_Cvar_VariableStringBuffer("color1", color1, 8);
	trap_Cvar_VariableStringBuffer("color2", color2, 8);
	CG_PlayerColorsFromCS(&cgs.osp.myColors, NULL, color1, color2);

	CG_PlayerColorsLoadOverrides(&cgs.osp.myColors,
	                             NULL,
	                             &cg_playerModelColors,
	                             &cg_playerRailColors,
	                             &cg_playerFrozenColor,
	                             NULL);
}

void CG_RebuildPlayerColors(void)
{
	/* Our colors */
	CG_RebuildOurPlayerColors();

	/* Team colors */
	/* Do not load default */
	memset(&cgs.osp.teamColorsOverride, 0, sizeof(cgs.osp.teamColorsOverride));
	CG_PlayerColorsFromTeamColors(&cgs.osp.teamColors, &cgs.osp.teamColorsOverride, cg_teamColors.string);
	CG_PlayerColorsLoadOverrides(&cgs.osp.teamColors,
	                             &cgs.osp.teamColorsOverride,
	                             &cg_teamModelColors,
	                             &cg_teamRailColors,
	                             &cg_teamFrozenColor,
	                             NULL);

	/* Enemy colors */
	/* Do not load default */
	memset(&cgs.osp.enemyColorsOverride, 0, sizeof(cgs.osp.enemyColorsOverride));
	CG_PlayerColorsFromEnemyColors(&cgs.osp.enemyColors, &cgs.osp.enemyColorsOverride, cg_enemyColors.string);
	CG_PlayerColorsLoadOverrides(&cgs.osp.enemyColors,
	                             &cgs.osp.enemyColorsOverride,
	                             &cg_enemyModelColors,
	                             &cg_enemyRailColors,
	                             &cg_enemyFrozenColor,
	                             &cg_enemyLightningColor);
}

void CG_ModelUniqueColors(int clientNum, playerColors_t* colors)
{
	if (cg_enemyModelColorsUnique.integer & 1)
	{
		VectorCopy(UNIQUE_COLOR(clientNum), colors->head);
	}
	if (cg_enemyModelColorsUnique.integer & 2)
	{
		VectorCopy(UNIQUE_COLOR(clientNum), colors->torso);
	}
	if (cg_enemyModelColorsUnique.integer & 4)
	{
		VectorCopy(UNIQUE_COLOR(clientNum), colors->legs);
	}
}


void CG_ClientInfoUpdateColors(clientInfo_t* ci, int clientNum)
{
	if (clientNum == cg.clientNum) /* our client */
	{
		CG_RebuildOurPlayerColors();
		memcpy(&ci->colors, &cgs.osp.myColors, sizeof(ci->colors));
	}
	else if (cgs.gametype >= GT_TEAM) /* team game */
	{
		const clientInfo_t* ourClient = &cgs.clientinfo[cg.clientNum];
		qboolean isEnemy = qfalse;
		const float* teamColor = CG_TeamColor(ci->rt);

		if (cg_spectPOV.integer)
		{
			team_t ourPerspectiveTeam;

			if (ourClient->rt == TEAM_SPECTATOR)
			{
				if (cg.snap->ps.pm_flags & PMF_FOLLOW && 
				    cg.snap->ps.clientNum >= 0 && 
				    cg.snap->ps.clientNum < MAX_CLIENTS)
				{
					ourPerspectiveTeam = cgs.clientinfo[cg.snap->ps.clientNum].rt;
				}
				else
				{
					ourPerspectiveTeam = TEAM_SPECTATOR;
				}
			}
			else
			{
				ourPerspectiveTeam = ourClient->rt;
			}

			if (ourPerspectiveTeam == TEAM_SPECTATOR)
			{
				/* Not following anyone - use default: blue is enemy */
				isEnemy = (ci->rt == TEAM_BLUE);
			}
			else
			{
				isEnemy = (ourPerspectiveTeam != ci->rt);
			}
		}
		else
		{
			if (ourClient->rt == TEAM_SPECTATOR)
			{
				/* if spectator, blue allways enemy */
				isEnemy = ci->rt == TEAM_BLUE;
			}
			else
			{
				isEnemy = ci->rt != ourClient->rt;
			}
		}

		if (cg_swapSkins.integer)
		{
			isEnemy = !isEnemy;
		}

		if (isEnemy)
		{
			if (cgs.osp.enemyColorsOverride.isModelColorSet)
			{
				VectorCopy(cgs.osp.enemyColors.head, ci->colors.head);
				VectorCopy(cgs.osp.enemyColors.torso, ci->colors.torso);
				VectorCopy(cgs.osp.enemyColors.legs, ci->colors.legs);
			}
			else
			{
				VectorCopy(teamColor, ci->colors.head);
				VectorCopy(teamColor, ci->colors.torso);
				VectorCopy(teamColor, ci->colors.legs);
			}

			CG_ModelUniqueColors(clientNum, &ci->colors);

			if (cgs.osp.enemyColorsOverride.isRailColorSet)
			{
				VectorCopy(cgs.osp.enemyColors.railCore, ci->colors.railCore);
				VectorCopy(cgs.osp.enemyColors.railRings, ci->colors.railRings);
			}
			if (cgs.osp.enemyColorsOverride.isFrozenColorSet)
			{
				VectorCopy(cgs.osp.enemyColors.frozen, ci->colors.frozen);
			}
		}
		else /* if teammate */
		{
			if (cgs.osp.teamColorsOverride.isModelColorSet)
			{
				VectorCopy(cgs.osp.teamColors.head, ci->colors.head);
				VectorCopy(cgs.osp.teamColors.torso, ci->colors.torso);
				VectorCopy(cgs.osp.teamColors.legs, ci->colors.legs);
			}
			else
			{
				VectorCopy(teamColor, ci->colors.head);
				VectorCopy(teamColor, ci->colors.torso);
				VectorCopy(teamColor, ci->colors.legs);
			}

			if (cgs.osp.teamColorsOverride.isRailColorSet)
			{
				VectorCopy(cgs.osp.teamColors.railCore, ci->colors.railCore);
				VectorCopy(cgs.osp.teamColors.railRings, ci->colors.railRings);
			}
			if (cgs.osp.teamColorsOverride.isFrozenColorSet)
			{
				VectorCopy(cgs.osp.teamColors.frozen, ci->colors.frozen);
			}
		}
	}
	else /* non-team game (FFA, 1vs1) */
	{
		qboolean isFollowedPlayer = qfalse;

		/* BE: Enhanced spectator perspective logic for non-team games */
		if (cg_spectPOV.integer && cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR &&
		    cg.snap->ps.pm_flags & PMF_FOLLOW &&
		    cg.snap->ps.clientNum >= 0 && cg.snap->ps.clientNum < MAX_CLIENTS &&
		    cg.snap->ps.clientNum == clientNum)
		{
			isFollowedPlayer = qtrue;
		}

		if (isFollowedPlayer)
		{
			/* Use team colors for followed player */
			if (cgs.osp.teamColorsOverride.isModelColorSet)
			{
				VectorCopy(cgs.osp.teamColors.head, ci->colors.head);
				VectorCopy(cgs.osp.teamColors.torso, ci->colors.torso);
				VectorCopy(cgs.osp.teamColors.legs, ci->colors.legs);
			}
			else
			{
				/* Use our own colors for followed player */
				VectorCopy(cgs.osp.myColors.head, ci->colors.head);
				VectorCopy(cgs.osp.myColors.torso, ci->colors.torso);
				VectorCopy(cgs.osp.myColors.legs, ci->colors.legs);
			}

			if (cgs.osp.teamColorsOverride.isRailColorSet)
			{
				VectorCopy(cgs.osp.teamColors.railCore, ci->colors.railCore);
				VectorCopy(cgs.osp.teamColors.railRings, ci->colors.railRings);
			}
			else
			{
				/* Use our own rail colors for followed player */
				VectorCopy(cgs.osp.myColors.railCore, ci->colors.railCore);
				VectorCopy(cgs.osp.myColors.railRings, ci->colors.railRings);
			}
			/* Note: frozen color not used in non-team games */
		}
		else /* enemy in FFA or 1vs1 game */
		{
			if (cgs.osp.enemyColorsOverride.isModelColorSet)
			{
				VectorCopy(cgs.osp.enemyColors.head, ci->colors.head);
				VectorCopy(cgs.osp.enemyColors.torso, ci->colors.torso);
				VectorCopy(cgs.osp.enemyColors.legs, ci->colors.legs);
			}

			CG_ModelUniqueColors(clientNum, &ci->colors);

			if (cgs.osp.enemyColorsOverride.isRailColorSet)
			{
				VectorCopy(cgs.osp.enemyColors.railCore, ci->colors.railCore);
				VectorCopy(cgs.osp.enemyColors.railRings, ci->colors.railRings);
			}
			if (cgs.osp.enemyColorsOverride.isFrozenColorSet)
			{
				VectorCopy(cgs.osp.enemyColors.frozen, ci->colors.frozen);
			}
		}
	}
}



