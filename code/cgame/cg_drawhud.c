// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "../game/bg_promode.h" // CPM

/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
#ifndef MISSIONPACK
static void CG_HDrawField (int x, int y, int width, int value, int superhud) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
	//	value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
	//	value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		//value = value > 999 ? 999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + CHAR_WIDTH*(width - l);

	if ( hud[superhud].textalign == 2 ) {
		//align left
		x = hud[superhud].rect[0];
	} else if ( hud[superhud].textalign == 1 ) {
		//align center
		//FIXME?:Centering bug?
		x = hud[superhud].rect[0] + ( (hud[superhud].rect[2] * .5) - ((l * hud[superhud].fontsize[0]) * .5) );
	} else {
		//align right
		x = (hud[superhud].rect[0] + hud[superhud].rect[2]) - (l * hud[superhud].fontsize[0]);
	}

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		CG_DrawPic( x, y, hud[superhud].fontsize[0], hud[superhud].fontsize[1], cgs.media.numberShaders[frame] );
		x += hud[superhud].fontsize[0];
		
		ptr++;
		l--;
	}
}
#endif // MISSIONPACK

/*
================
CG_DrawStatusBarHead

================
*/
#ifndef MISSIONPACK

static void CG_HDrawStatusBarHead( float x, int superhud ) {
	vec3_t		angles;
	playerState_t	*ps;
	float		size, stretch;
	float		frac;

	ps = &cg.snap->ps;

	VectorClear( angles );

	if ( cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME ) {
		frac = (float)(cg.time - cg.damageTime ) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * ( 1.5 - frac * 0.5 );

		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
		cg.headEndPitch = 5 * cos( crandom()*M_PI );

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	} else {
		if ( cg.time >= cg.headEndTime ) {
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;
			
			if (ps->stats[STAT_HEALTH] > 0) {
			cg.headEndYaw = 180 + 20 * cos( crandom()*M_PI );
			cg.headEndPitch = 5 * cos( crandom()*M_PI );
			}
		}

		size = ICON_SIZE * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if ( cg.headStartTime > cg.time ) {
		cg.headStartTime = cg.time;
	}

	frac = ( cg.time - cg.headStartTime ) / (float)( cg.headEndTime - cg.headStartTime );
	frac = frac * frac * ( 3 - 2 * frac );
	angles[YAW] = cg.headStartYaw + ( cg.headEndYaw - cg.headStartYaw ) * frac;
	angles[PITCH] = cg.headStartPitch + ( cg.headEndPitch - cg.headStartPitch ) * frac;

	CG_DrawHead( x, hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], 
				cg.snap->ps.clientNum, angles );
}
#endif // MISSIONPACK


/*
=================
CG_DrawStatusBars
This is a fun piece of code =)
--Enjoy. -Evolution/XZero450
=================
*/
void CG_DrawStatusBars ( int superhud ) {
	int		value1, value2;
	int		max = 200;
	float	x, y, w, h;
	vec4_t	color;
	centity_t	*cent;
	playerState_t	*ps;

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;
	value2 = 0;
	x = hud[superhud].rect[0];
	y = hud[superhud].rect[1];
	color[0] = 0.90f ;
	color[1] = 0.65f;
	color[2] = 0.0f;
	color[3] = 1.0f;

	switch ( hud[superhud].element ) {
		case StatusBar_AmmoBar:
			ps->ammo[cent->currentState.weapon] > 200 ? (value1 = 200):(value1 = ps->ammo[cent->currentState.weapon]);
			if ( g_pro_mode.integer ) {
				if ( cent->currentState.weapon == WP_SHOTGUN ) {
					max = 25;
				} else if ( cent->currentState.weapon == WP_GRENADE_LAUNCHER ) {
					max = 25;
				} else if ( cent->currentState.weapon == WP_ROCKET_LAUNCHER ) {
					max = 25;
				} else if ( cent->currentState.weapon == WP_LIGHTNING ) {
					max = 150;
				} else if ( cent->currentState.weapon == WP_RAILGUN ) {
					max = 25;
				}
			}
			break;
		case StatusBar_HealthBar:
			value1 = ps->stats[STAT_HEALTH];
			max = 200;
			if ( value1 > 100 ) {
				color[0] = color[1] = color[2] = 1.0f;
			} else if ( value1 <= 25 && value1 > 0 ) {
				color[0] = 0.85f;
				color[1] = color[2] = 0.0f;
			}
			break;
		case StatusBar_ArmorBar:
			max = 200;
			value1 = ps->stats[STAT_ARMOR];
			if ( value1 > 100 ) {
				color[0] = color[1] = color[2] = 1.0f;
			}
			break;
	}

	color[3] = 0.75f;

	if ( value1 <= 0 ) {
		w = 0;
	} else {
		if ( hud[superhud].doublebar ) {
			if ( value1 > (max * .5) ) {//Figure out value2
				value2 = ( value1 - (max * .5) );
				value1 = max * .5;
			}

			if ( hud[superhud].textstyle == 1 ) {//Vertical bars
				w = (hud[superhud].rect[2] - 4) * .5;//Width
				h = (hud[superhud].rect[3] / (max * .5) ) * value1;//Height
			} else {
				h = (hud[superhud].rect[3] - 4) * .5;
				w = (hud[superhud].rect[2] / (max * .5) ) * value1;
			}
		} else {
			if ( hud[superhud].textstyle == 1 ) {//vertical bars
				w = hud[superhud].rect[2];//Width
				h = ( (float)hud[superhud].rect[3] / (float)max ) * value1;//Height
			} else {
				//Figure out width
				w = ( (float)hud[superhud].rect[2] / (float)max ) * value1;
				h = hud[superhud].rect[3];
			}
		}
	}
	if ( hud[superhud].textstyle == 1 ) {
		//Vertical bars
		if ( hud[superhud].textalign == 2 ) {//align top
			y = hud[superhud].rect[1];
		} else if ( hud[superhud].textalign == 1 ) {//align center
			y = ((hud[superhud].rect[1] + hud[superhud].rect[3]) - h) * .5;
		} else {//align bottom
			y = (hud[superhud].rect[1] + hud[superhud].rect[3]) - h;
		}
	} else {
		//Horizontal bars
		if ( hud[superhud].textalign == 2 ) {//align left
			x = hud[superhud].rect[0];
		} else if ( hud[superhud].textalign == 1 ) {//align center
			x = ((hud[superhud].rect[0] + hud[superhud].rect[2]) - w) * .5;
		} else {//align right
			x = (hud[superhud].rect[0] + hud[superhud].rect[2]) - w;
		}
	}

	//Draw bar
	//CG_Printf("%3.3f, %3.3f, %3.3f, %3.3f -- %3.3f, %3.3f, %3.3f, %3.3f", x,y,w,h ,color[0],color[1],color[2],color[3]);
	CG_FillRect(x, y, w, h, color);

	//Draw another bar.
	if ( value2 ) {
		if ( hud[superhud].textstyle == 1 ) {
			//Vertical bars
			x += (w + 4);
			h = (hud[superhud].rect[3] / (max * .5)) * value2;
			//Align
			if ( hud[superhud].textalign == 2 ) {//align top
				y = hud[superhud].rect[1];
			} else if ( hud[superhud].textalign == 1 ) {//align center
				y = ((hud[superhud].rect[1] + hud[superhud].rect[3]) - h) * .5;
			} else {//align bottom
				y = (hud[superhud].rect[1] + hud[superhud].rect[3]) - h;
			}
		} else {
			y += (h + 4); //Gap between bars
			//Figure out width
			w = (hud[superhud].rect[2] / (max * .5)) * value2;
			//Align
			if ( hud[superhud].textalign == 2 ) {//align left
				x = hud[superhud].rect[0];
			} else if ( hud[superhud].textalign == 1 ) {//align center
				x = ((hud[superhud].rect[0] + hud[superhud].rect[2]) - w) * .5;
			} else {//align right
				x = (hud[superhud].rect[0] + hud[superhud].rect[2]) - w;
			}	
		}
		CG_FillRect(x, y, w, h, color);
	}
}

/*
================
CG_DrawStatusBar

================
*/

static void CG_HDrawStatusBar( int superhud ) {
	int			color;
	centity_t	*cent;
	playerState_t	*ps;
	int			value;
	vec4_t		hcolor;
	vec3_t		angles;
	vec3_t		origin;
#ifdef MISSIONPACK
	qhandle_t	handle;
#endif
	static float colors[4][4] = { 
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1.0f, 0.69f, 0.0f, 1.0f },    // normal
		{ 1.0f, 0.2f, 0.2f, 1.0f },     // low health
		{ 0.5f, 0.5f, 0.5f, 1.0f },     // weapon firing
		{ 1.0f, 1.0f, 1.0f, 1.0f } };   // health > 100

	// draw the team background
	//CG_DrawTeamBackground( 0, 420, 640, 60, 0.33f, cg.snap->ps.persistant[PERS_TEAM] );

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear( angles );

	// draw any 3D icons first, so the changes back to 2D are minimized
	if ( cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel ) {
		if ( hud[superhud].element == StatusBar_AmmoIcon ) {
			origin[0] = 70;
			origin[1] = 0;
			origin[2] = 0;
			angles[YAW] = 90 + hud[superhud].angles[3] * sin( cg.time / 1000.0 );
			
			if ( !hud[superhud].model ) {
				qhandle_t	icon;

				icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
				if ( icon ) {
					CG_DrawPic( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], icon );
				}
			} else {
				CG_Draw3DModel( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3],
						   cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles );
			}
			return;
		}
	}
	
	if ( hud[superhud].element == StatusBar_HealthIcon ) {
		
		//memcpy(origin, hud[superhud].offset, sizeof(origin));
			
		angles[0] = hud[superhud].angles[0];
		angles[1] = hud[superhud].angles[1];
		angles[2] = hud[superhud].angles[2];
		angles[YAW] = 90 + 20 * sin( cg.time / 1000.0 );
			
		CG_HDrawStatusBarHead( hud[superhud].rect[0], superhud );
		return;
	}

	
	if ( ps->stats[ STAT_ARMOR ] ) {
		qhandle_t model = cgs.media.armorModel; // CPM

		// CPM: Fix RA shader
		if ( g_pro_mode.integer ) {
			if ( ps->stats[ STAT_ARMORTYPE ] == 2 ) {
				model = cgs.media.armorModelRA;
			}
		}

		if ( hud[superhud].element == StatusBar_ArmorIcon ) {

			memcpy(origin, hud[superhud].offset, sizeof(origin));
			
			origin[0] = 90;
			origin[1] = 0;
			origin[2] = -10;
			angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;

			if ( !hud[superhud].model ) {
				qhandle_t icon;

				// CPM: Armor icon
				if ( g_pro_mode.integer && ps->stats[STAT_ARMORTYPE] == 2 ) {
					icon = cgs.media.armorIconRA;
				} else {
					icon = cgs.media.armorIcon;
				}
				// !CPM

				CG_DrawPic( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], icon );
			} else {
				CG_Draw3DModel( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3],
						   model, 0, origin, angles );
			}
			return;
		}
	}
#ifdef MISSIONPACK
	if( cgs.gametype == GT_HARVESTER ) {
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
		if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			handle = cgs.media.redCubeModel;
		} else {
			handle = cgs.media.blueCubeModel;
		}
		CG_Draw3DModel( 640 - (TEXT_ICON_SPACE + ICON_SIZE), 416, ICON_SIZE, ICON_SIZE, handle, 0, origin, angles );
	}
#endif
	//
	// ammo
	//
	
	if ( cent->currentState.weapon ) {
		value = ps->ammo[cent->currentState.weapon];
		if ( hud[superhud].element == StatusBar_AmmoCount && value < 0 ) {
			return;
		}
		if ( value > -1 ) {
			if ( cg.predictedPlayerState.weaponstate == WEAPON_FIRING
				&& cg.predictedPlayerState.weaponTime > 100 ) {
				// draw as dark grey when reloading
				color = 2;	// dark grey
			} else {
				if ( value >= 0 ) {
					color = 0;	// green
				} else {
					color = 1;	// red
				}
			}
				
			trap_R_SetColor( colors[color] );	
			if ( hud[superhud].element == StatusBar_AmmoCount) {
				if ( value > 0 ) {
					CG_HDrawField ( hud[superhud].rect[0], hud[superhud].rect[1], 3, value, superhud);
				}
				return;
			}
			trap_R_SetColor( NULL );	

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
				qhandle_t	icon;

				icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
				if ( icon ) {
					CG_DrawPic( CHAR_WIDTH*3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, icon );
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if ( value > 100 ) {
		trap_R_SetColor( colors[3] );		// white
	} else if (value > 25) {
		trap_R_SetColor( colors[0] );	// green
	} else if (value > 0) {
		color = (cg.time >> 8) & 1;	// flash
		trap_R_SetColor( colors[color] );
	} else {
		trap_R_SetColor( colors[1] );	// red
	}

	// stretch the health up when taking damage
	if ( hud[superhud].element == StatusBar_HealthCount) {
		CG_HDrawField ( hud[superhud].rect[0], hud[superhud].rect[1], 4, value, superhud);
		return;
	}
	CG_ColorForHealth( hcolor );
	trap_R_SetColor( hcolor );
	trap_R_SetColor( NULL );

	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0 ) {
		// CPM: Armor icon
		qhandle_t icon = cgs.media.armorIcon;

		if (g_pro_mode.integer && ps->stats[STAT_ARMORTYPE] == 2)
			icon = cgs.media.armorIconRA;
		// !CPM

		if ( value > 100 ) {
			trap_R_SetColor( colors[3] );
		} else {
			trap_R_SetColor( colors[0] );
		}	
		if ( hud[superhud].element == StatusBar_ArmorCount ) {
			CG_HDrawField ( hud[superhud].rect[0], hud[superhud].rect[1], 3, value, superhud);
			return;
		}
		trap_R_SetColor( NULL );

		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
			CG_DrawPic( 370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, icon ); // CPM
		}
	}

#ifdef MISSIONPACK
	//
	// cubes
	//
	if( cgs.gametype == GT_HARVESTER ) {
		value = ps->generic1;
		if( value > 99 ) {
			value = 99;
		}
		trap_R_SetColor( colors[0] );
		CG_DrawField (640 - (CHAR_WIDTH*2 + TEXT_ICON_SPACE + ICON_SIZE), 432, 2, value, 0);
		trap_R_SetColor( NULL );
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if ( !cg_draw3dIcons.integer && cg_drawIcons.integer ) {
			if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
				handle = cgs.media.redCubeIcon;
			} else {
				handle = cgs.media.blueCubeIcon;
			}
			CG_DrawPic( 640 - (TEXT_ICON_SPACE + ICON_SIZE), 432, ICON_SIZE, ICON_SIZE, handle );
		}
	}
#endif
}

/*
================
CG_DrawAttacker

================
*/
static void CG_HDrawAttacker( int superhud ) {
	int			t;
	float		size;
	vec3_t		angles;
//	const char	*info;
//	const char	*name;
	int			clientNum;

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if ( !cg.attackerTime ) {
		return;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) {
		return;
	}

	t = cg.time - cg.attackerTime;
	if ( t > ATTACKER_HEAD_TIME ) {
		cg.attackerTime = 0;
		return;
	}

	if ( hud[superhud].element == AttackerIcon ) {
		size = ICON_SIZE * 1.25;

		angles[PITCH] = hud[superhud].angles[PITCH];
		angles[YAW] = hud[superhud].angles[YAW];
		angles[ROLL] = hud[superhud].angles[ROLL];
		CG_DrawHead( (float)(hud[superhud].rect[0] - size), (float)hud[superhud].rect[1], size, size, clientNum, angles );
	}

	if ( hud[superhud].element == AttackerName ) {
		int w = 0;

		//info = CG_ConfigString( CS_PLAYERS + clientNum );
		//name = Info_ValueForKey(  info, "n" );

		w = CG_DrawStrlen( cgs.clientinfo[ clientNum ].name ) * BIGCHAR_WIDTH;
		CG_DrawStringExt( 0, 0, cgs.clientinfo[ clientNum ].name, colorWhite, qfalse, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
	}

}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static void CG_HDrawFPS( int superhud ) {
	char		*s;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va( "%ifps", fps );

		//CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
		//CG_Printf("FPS -- %i, %i, %i, %i\n", hud[(int)y].rect[0], hud[(int)y].rect[1], hud[(int)y].fontsize[0], hud[(int)y].fontsize[1]);
		if ( hud[superhud].fill ) {
			CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
		}
		CG_DrawStringExt( 0, 0, s, colorWhite, qfalse, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
	}
}

/*
=================
CG_DrawTimer
=================
*/
static void CG_HDrawTimer( int superhud ) {
	char		*s;
	int			mins, seconds;//, tens;
	int			msec;

	if ( (cg_drawTimer.integer == 2 || (!cg_drawTimer.integer && superhud) ) && cgs.timelimit > 0 ) {
		//Figure out how much time left and subtract a second so we're at 0 when timelimit is hit.
		msec = (((cgs.levelStartTime + (cgs.timelimit * 60000)) - cg.time));
	} else {
		msec = cg.time - cgs.levelStartTime;
	}

	if ( cgs.timelimit > 0 && cg.time >= (cgs.levelStartTime + (cgs.timelimit * 60000)) ) {
		//do we need -1000?
		//Prevent negative numbers when we go OVER the timelimit
	//	msec = seconds = tens = mins = 0;
		msec = seconds = mins = 0;
	} else {
		seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		//tens = seconds / 10;
		//seconds -= tens * 10;
	}

	//s = va( "%i:%i%i", mins, tens, seconds );
	s = va( "%i:%02i", mins, seconds );

	//CG_DrawBigString( 635 - w, y + 2, s, 1.0F);
	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}

	CG_DrawStringExt( 0, 0, s, colorWhite, qfalse, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
}

/*
=================
CG_DrawSuperTeamOverlay
rewritten to only draw at a time
=================
*/

void CG_DrawSuperTeamOverlay( int superhud, int i ) {
	int x, w, h, xx;
	int j, k, len;//i, j, k, len;
	const char *p;
	vec4_t		hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	gitem_t	*item;
	int y;//, count;

	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
		return ; // Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;

	if ( (numSortedTeamPlayers - 1) < i ) {
		return;
	}

	ci = cgs.clientinfo + sortedTeamPlayers[i];
	if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
		plyrs++;
		len = CG_DrawStrlen(ci->name);
		if (len > pwidth) {
			pwidth = len;
		}
	}

	if ( !plyrs ) {
		return;
	}

	//if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
	pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for ( k = 1; k < MAX_LOCATIONS; k++ ) {
		p = CG_ConfigString(CS_LOCATIONS + k);
		if (p && *p) {
			len = CG_DrawStrlen(p);
			if (len > lwidth) {
				lwidth = len;
			}
		}
	}

	if ( lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH ) {
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;
	}


	//Set width
	if ( g_gamemode.integer != 1 ) {
		w = (pwidth + lwidth + 4 + 7) * hud[superhud].fontsize[0];
	} else {
		w = (pwidth + lwidth + 3) * hud[superhud].fontsize[0];
	}
	//Set x
	if ( hud[superhud].textalign == 2 ) {
		//align left
		x = hud[superhud].rect[0];
	} else if ( hud[superhud].textalign == 1 ) {
		//align center
		x = ((hud[superhud].rect[0] + hud[superhud].rect[2]) - w) * .5;
	} else {
		//align right
		x = (hud[superhud].rect[0] + hud[superhud].rect[2]) - w;
	}
	//Set y
	y = hud[superhud].rect[1];
	//Set height
	h = plyrs * hud[superhud].fontsize[1];
	
	
	memcpy(hcolor, hud[superhud].bgcolor, sizeof(hcolor) );
	CG_FillRect( (float)x, (float)y, (float)hud[superhud].rect[2], (float)hud[superhud].rect[3], hud[superhud].bgcolor );
	trap_R_SetColor( NULL );

	ci = cgs.clientinfo + sortedTeamPlayers[i];
	if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

		hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

		xx = x + hud[superhud].fontsize[0];

		CG_DrawStringExt( xx, y, ci->name, hcolor, qfalse, qfalse, hud[superhud].fontsize[0], hud[superhud].fontsize[1], TEAM_OVERLAY_MAXNAME_WIDTH, 0, 0);

		if (lwidth) {
			p = CG_ConfigString(CS_LOCATIONS + ci->location);
			if (!p || !*p) {
				p = "unknown";
			}
			len = CG_DrawStrlen(p);
			if (len > lwidth) {
				len = lwidth;
			}

			//xx = x + hud[superhud].fontsize[0] * 2 + hud[superhud].fontsize[0] * pwidth;
			xx = x + hud[superhud].fontsize[0] * 2 + hud[superhud].fontsize[0] * TEAM_OVERLAY_MAXNAME_WIDTH;
			CG_DrawStringExt( xx, y, p, hcolor, qfalse, qfalse, hud[superhud].fontsize[0], hud[superhud].fontsize[1], TEAM_OVERLAY_MAXLOCATION_WIDTH, 0, 0);
		}

		if ( g_gamemode.integer != 1 ) {
			CG_GetColorForHealth( ci->health, ci->armor, hcolor );

			Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);

			xx = x + hud[superhud].fontsize[0] * 3 + hud[superhud].fontsize[0] * pwidth + hud[superhud].fontsize[0] * lwidth;

			CG_DrawStringExt( xx, y, st, hcolor, qfalse, qfalse, hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
		}

		// draw weapon icon
		if ( g_gamemode.integer != 1 ) {
			xx += hud[superhud].fontsize[0] * 3;

			if ( cg_weapons[ci->curWeapon].weaponIcon ) {
				CG_DrawPic( xx, y, hud[superhud].fontsize[0], hud[superhud].fontsize[1], cg_weapons[ci->curWeapon].weaponIcon );
			} else {
				CG_DrawPic( xx, y, hud[superhud].fontsize[0], hud[superhud].fontsize[1], cgs.media.deferShader );
			}
		}

		// Draw powerup icons
		xx = x + w - hud[superhud].fontsize[0];

		for (j = 0; j <= PW_NUM_POWERUPS; j++) {
//freeze
			if ( cgs.gametype == GT_FREEZE ) {
				if ( Q_Isfreeze( ci - cgs.clientinfo ) ) {
					CG_DrawPic( xx, y, hud[superhud].fontsize[0], hud[superhud].fontsize[1], cgs.media.noammoShader );
					break;
				}
			}
//freeze
			if (ci->powerups & (1 << j)) {
				item = BG_FindItemForPowerup( j );
				if (item) {
					/* This is sloppy but it sets the color of the flag icons for cg_adjustTeamColors */
					 if ( item->giType == IT_TEAM && cgs.gametype == GT_CTF && cg_adjustTeamColors.integer ) {
						if ( cgs.clientinfo[cg.clientNum].team == TEAM_RED ) {
							if ( item->giTag == PW_REDFLAG ) {
								trap_R_SetColor( g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ] );
							} else {
								trap_R_SetColor( g_color_table[ ColorIndex(cg_EnemyColors.string[cg_adjustTeamColors.integer - 1]) ] );
							}
						} else {
							if ( item->giTag == PW_BLUEFLAG ) {
								trap_R_SetColor( g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ] );
							} else {
								trap_R_SetColor( g_color_table[ ColorIndex(cg_EnemyColors.string[cg_adjustTeamColors.integer - 1]) ] );
							}
						}
					}
					 if ( cg_adjustTeamColors.integer ) {
						CG_DrawPic( xx, y, hud[superhud].fontsize[0], hud[superhud].fontsize[1], trap_R_RegisterShader( "icons/iconf_fla1" ) );
					 } else {
						CG_DrawPic( xx, y, hud[superhud].fontsize[0], hud[superhud].fontsize[1], trap_R_RegisterShader( item->icon ) );
					 }
					xx += hud[superhud].fontsize[0];
				}
			}
		}

		y += hud[superhud].fontsize[1];
	}
	return;
//#endif
}

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
//#ifndef MISSIONPACK
static void CG_HDrawScores( int superhud ) {
	const char	*s;
	int			s1, s2/*, score*/;
	//int			x, w;
	int			v;
	vec4_t		color;
	//float		y, y1;
	gitem_t		*item;

	
	s1 = cgs.scores1;
	s2 = cgs.scores2;

	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1],  hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}

	switch ( hud[superhud].element ) {
		case Score_OWN:
			if ( cgs.gametype >= GT_TEAM ) {
				if ( cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE ) {
					s = va( "%2i", s2 );//Blue
				} else {
					s = va( "%2i", s1 );//Red
				}
				CG_DrawStringExt(0, 0, s, colorWhite, qtrue, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
			} else if ( s1 != SCORE_NOT_PRESENT ) { //Need a score present
				if ( cgs.clientinfo[ cg.clientNum ].team == TEAM_SPECTATOR ) {
					s = va( "%2i", s1 ); //highest score
				} else if ( cg.snap->ps.persistant[PERS_SCORE] == s1 ) {
					s = va( "%2i", s1 ); //OWN score
				} else {
					s = va( "%2i", s2 ); //NME score
				}
				CG_DrawStringExt(0, 0, s, colorWhite, qtrue, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
			}
			break;
		case Score_NME:
			if ( cgs.gametype >= GT_TEAM ) {
				if ( cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE ) {
					s = va( "%2i", s1 );//Red
				} else {
					s = va( "%2i", s2 );//Blue
				}
				CG_DrawStringExt(0, 0, s, colorWhite, qtrue, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
			} else  if ( s1 != SCORE_NOT_PRESENT ) { //Need a score present
				if ( cgs.clientinfo[ cg.clientNum ].team == TEAM_SPECTATOR ) {
					s = va( "%2i", s2 ); //second highest score
				} else if ( cg.snap->ps.persistant[PERS_SCORE] == s2 ) {
					s = va( "%2i", s1 ); //NME score
				} else {
					s = va( "%2i", s2 ); //OWN score
				}
				CG_DrawStringExt(0, 0, s, colorWhite, qtrue, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
			}
			break;
		case Score_Limit:
			if ( cgs.gametype >= GT_CTF /*|| g_gamemode.integer >= 4*/ ) {
				v = cgs.capturelimit;
			} else {
				v = cgs.fraglimit;
			}
			if ( v ) {
				s = va( "%2i", v );
				memcpy(color, hud[superhud].bgcolor, sizeof(color) );
				CG_DrawStringExt(0, 0, s, colorWhite, qtrue, qfalse, hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
			}
			break;
		case FlagStatus_OWN:
			if ( cgs.gametype == GT_CTF ) {
				int flag = (cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE? PW_BLUEFLAG : PW_REDFLAG);
				int team = (cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE? TEAM_BLUE : TEAM_RED);
				item = BG_FindItemForPowerup( flag );

				trap_R_SetColor( NULL );//FIXME: This shouldn't be here
				trap_R_SetColor( hud[superhud].color );
				if( cgs.flags[team-1] >= 0 && cgs.flags[team-1] <= 3 ) {
					CG_DrawPic( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3],
						cgs.media.whiteFlagShader[cgs.flags[team-1]] );
				}
				trap_R_SetColor( NULL );
			}
			break;
		case FlagStatus_NME:
			if ( cgs.gametype == GT_CTF ) {
				int flag = (cgs.clientinfo[ cg.clientNum ].team != TEAM_BLUE? PW_BLUEFLAG : PW_REDFLAG);
				int team = (cgs.clientinfo[ cg.clientNum ].team != TEAM_BLUE? TEAM_BLUE : TEAM_RED);
				item = BG_FindItemForPowerup( flag );

				trap_R_SetColor( NULL );//FIXME: This shouldn't be here
				trap_R_SetColor( hud[superhud].color );
				if( cgs.flags[team-1] >= 0 && cgs.flags[team-1] <= 3 ) {
					CG_DrawPic( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3],
						cgs.media.whiteFlagShader[cgs.flags[team-1]] );
				}
				trap_R_SetColor( NULL );
			}
			break;
	}
}
//#endif // MISSIONPACK

/*
================
CG_DrawSuperPowerups
================
*/
#ifndef MISSIONPACK
static void CG_DrawSuperPowerups( int superhud ) {
	int		sorted[MAX_POWERUPS];
	int		sortedTime[MAX_POWERUPS];
	int		i, j, k;
	int		active;
	playerState_t	*ps;
	int		t;
	gitem_t	*item;
	int		x;
	int		color;
	float	size;
	float	f;
	static float colors[2][4] = { 
    { 0.2f, 1.0f, 0.2f, 1.0f } , 
    { 1.0f, 0.2f, 0.2f, 1.0f } 
  };

	ps = &cg.snap->ps;

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// sort the list by time remaining
	active = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( !ps->powerups[ i ] ) {
			continue;
		}
		t = ps->powerups[ i ] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if ( (t < 0 || t > 999000) && !superhud) {//Can draw flags in the superhud
			continue;
		}

		// insert into the list
		for ( j = 0 ; j < active ; j++ ) {
			if ( sortedTime[j] >= t ) {
				for ( k = active - 1 ; k >= j ; k-- ) {
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	//x = 640 - ICON_SIZE - CHAR_WIDTH * 2;
	//Set x
	if ( hud[superhud].element == PowerUp1_Icon ||
		 hud[superhud].element == PowerUp2_Icon ||
		 hud[superhud].element == PowerUp3_Icon ||
		 hud[superhud].element == PowerUp4_Icon ) {
		if ( hud[superhud].textalign == 2 ) {
			//align left
			x = hud[superhud].rect[0];
		} else if ( hud[superhud].textalign == 1 ) {
			//align center
			x = ((hud[superhud].rect[0] + hud[superhud].rect[2]) - hud[superhud].fontsize[0]) * .5;
		} else {
			//align right
			x = (hud[superhud].rect[0] + hud[superhud].rect[2]) - hud[superhud].fontsize[0];
		}
	}
	//for ( i = 0 ; i < active ; i++ ) {
	if ( (hud[superhud].element == PowerUp1_Icon || hud[superhud].element == PowerUp1_Time) && 0 < active ) {
		i = 0;
	} else if ( (hud[superhud].element == PowerUp2_Icon || hud[superhud].element == PowerUp2_Time) && 1 < active ) {
		i = 1;
	} else if ( (hud[superhud].element == PowerUp3_Icon || hud[superhud].element == PowerUp3_Time) && 2 < active ) {
		i = 2;
	} else if ( (hud[superhud].element == PowerUp4_Icon || hud[superhud].element == PowerUp4_Time) && 3 < active ) {
		i = 3;
	} else {
		return;
	}
	
	item = BG_FindItemForPowerup( sorted[i] );

    if (item) {

		  color = 1;

//		  y -= ICON_SIZE;

		  
		trap_R_SetColor( hud[superhud].color );
		//Don't draw time for flags.
		if ( hud[superhud].element == PowerUp1_Time ||
			 hud[superhud].element == PowerUp2_Time ||
			 hud[superhud].element == PowerUp3_Time ||
			 hud[superhud].element == PowerUp4_Time ) {
				 char	time[24];
				 
				 t = ps->powerups[ i ] - cg.time;
				 if ( !(t < 0 || t > 999000) ) {
					 return;
				 }
				Com_sprintf(time, sizeof(time), "%i", sortedTime[ i ] / 1000 );
				if ( item->giType != IT_TEAM ) {
					CG_DrawStringExt( hud[superhud].rect[0], hud[superhud].rect[1], time, hud[superhud].color, qfalse, qfalse, hud[superhud].fontsize[0], hud[superhud].fontsize[1], 3, 0, superhud );
				}
				return;
			}
		 // CG_DrawField( x, y, 2, sortedTime[ i ] / 1000 );
		  

		  t = ps->powerups[ sorted[i] ];
		  if ( t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME ) {
			  trap_R_SetColor( NULL );
		  } else {
			  vec4_t	modulate;

			  f = (float)( t - cg.time ) / POWERUP_BLINK_TIME;
			  f -= (int)f;
			  modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
			  trap_R_SetColor( modulate );
		  }

		  if ( cg.powerupActive == sorted[i] && 
			  cg.time - cg.powerupTime < PULSE_TIME ) {
			  f = 1.0 - ( ( (float)cg.time - cg.powerupTime ) / PULSE_TIME );
			  size = ICON_SIZE * ( 1.0 + ( PULSE_SCALE - 1.0 ) * f );
		  } else {
			  size = ICON_SIZE;
		  }

		  CG_DrawPic( hud[superhud].rect[0], hud[superhud].rect[1], 
			  hud[superhud].rect[2], hud[superhud].rect[3], trap_R_RegisterShader( item->icon ) );
    }
	trap_R_SetColor( NULL );

	return;
}
#endif // MISSIONPACK

/*
===================
CG_DrawPickupItem
===================
*/
#ifndef MISSIONPACK
static void CG_HDrawPickupItem( int superhud ) {
	int		value;
	float	*fadeColor;
	float	*fontFade;//Doesn't remove text...
	int pos_x = 0, w = 0;//, pos_y = 0;

	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	value = cg.itemPickup;
	if ( !value ) {
		return;
	}


	fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
	if ( fadeColor ) {
		CG_RegisterItemVisuals( value );
		if ( cgs.gametype == GT_CTF && bg_itemlist[value].giType == IT_TEAM && cg.itemPickupFlag == PW_BLUEFLAG ) {
			//CG_Printf("DrawPickup: %i -- %i -- set blue\n", bg_itemlist[value].giType, bg_itemlist[value].giTag );
			if ( cg_adjustTeamColors.integer && cgs.clientinfo[ cg.clientNum ].team != TEAM_BLUE && cgs.clientinfo[ cg.clientNum ].team != TEAM_SPECTATOR && cgs.clientinfo[ cg.clientNum ].team != TEAM_SPECTATOR && strlen(cg_EnemyColors.string) >= cg_adjustTeamColors.integer ) {
				fadeColor[0] = g_color_table[ ColorIndex( cg_EnemyColors.string[cg_adjustTeamColors.integer - 1] ) ][0];
				fadeColor[1] = g_color_table[ ColorIndex( cg_EnemyColors.string[cg_adjustTeamColors.integer - 1] ) ][1];
				fadeColor[2] = g_color_table[ ColorIndex( cg_EnemyColors.string[cg_adjustTeamColors.integer - 1] ) ][2];
			} else if ( cg_adjustTeamColors.integer && cg_forceColors.integer && cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE && strlen(colors.string) >= cg_adjustTeamColors.integer ) {
				fadeColor[0] = g_color_table[ ColorIndex( colors.string[cg_adjustTeamColors.integer - 1] ) ][0];
				fadeColor[1] = g_color_table[ ColorIndex( colors.string[cg_adjustTeamColors.integer - 1] ) ][1];
				fadeColor[2] = g_color_table[ ColorIndex( colors.string[cg_adjustTeamColors.integer - 1] ) ][2];
			} else if ( cg_adjustTeamColors.integer && cgs.clientinfo[ cg.clientNum ].team == TEAM_BLUE && strlen(cg_TeamColors.string) >= cg_adjustTeamColors.integer ) {
				fadeColor[0] = g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ][0];
				fadeColor[1] = g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ][1];
				fadeColor[2] = g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ][2];
			}
		} else if ( cgs.gametype == GT_CTF && bg_itemlist[value].giType == IT_TEAM && cg.itemPickupFlag == PW_REDFLAG ) {
			//CG_Printf("DrawPickup: %i -- %i -- set red\n", bg_itemlist[value].giType, bg_itemlist[value].giTag );
			if ( cg_adjustTeamColors.integer && cgs.clientinfo[ cg.clientNum ].team != TEAM_RED && cgs.clientinfo[ cg.clientNum ].team != TEAM_SPECTATOR && cgs.clientinfo[ cg.clientNum ].team != TEAM_SPECTATOR && strlen(cg_EnemyColors.string) >= cg_adjustTeamColors.integer ) {
				fadeColor[0] = g_color_table[ ColorIndex( cg_EnemyColors.string[cg_adjustTeamColors.integer - 1] ) ][0];
				fadeColor[1] = g_color_table[ ColorIndex( cg_EnemyColors.string[cg_adjustTeamColors.integer - 1] ) ][1];
				fadeColor[2] = g_color_table[ ColorIndex( cg_EnemyColors.string[cg_adjustTeamColors.integer - 1] ) ][2];
			} else if ( cg_adjustTeamColors.integer && cg_forceColors.integer && cgs.clientinfo[ cg.clientNum ].team == TEAM_RED && strlen(colors.string) >= cg_adjustTeamColors.integer ) {
				fadeColor[0] = g_color_table[ ColorIndex( colors.string[cg_adjustTeamColors.integer - 1] ) ][0];
				fadeColor[1] = g_color_table[ ColorIndex( colors.string[cg_adjustTeamColors.integer - 1] ) ][1];
				fadeColor[2] = g_color_table[ ColorIndex( colors.string[cg_adjustTeamColors.integer - 1] ) ][2];
			} else if ( cg_adjustTeamColors.integer && cgs.clientinfo[ cg.clientNum ].team == TEAM_RED && strlen(cg_TeamColors.string) >= cg_adjustTeamColors.integer ) {
				fadeColor[0] = g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ][0];
				fadeColor[1] = g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ][1];
				fadeColor[2] = g_color_table[ ColorIndex( cg_TeamColors.string[cg_adjustTeamColors.integer - 1] ) ][2];
			}
		} else {
			fadeColor[0] = hud[superhud].fade[0];
			fadeColor[1] = hud[superhud].fade[1];
			fadeColor[2] = hud[superhud].fade[2];
		}
		trap_R_SetColor( fadeColor );
		if ( cg_adjustTeamColors.integer && bg_itemlist[value].giType == IT_TEAM ) {
			CG_DrawPic( (float)hud[superhud].rect[0], (float)hud[superhud].rect[1], ICON_SIZE, ICON_SIZE, trap_R_RegisterShader("icons/iconf_fla1") );
		} else {
			CG_DrawPic( (float)hud[superhud].rect[0], (float)hud[superhud].rect[1], ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
		}
	}


	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}

	fontFade = CG_FadeColor( cg.itemPickupTime, 3000 );

	if ( fontFade ) {
		fontFade[0] = hud[superhud].color[0];
		fontFade[1] = hud[superhud].color[1];
		fontFade[2] = hud[superhud].color[2];
		CG_DrawStringExt(pos_x, hud[superhud].rect[1], bg_itemlist[ value ].pickup_name, fontFade, qfalse, qfalse,
						hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
	}

	trap_R_SetColor( NULL );
	
	return;
}
#endif // MISSIONPACK


//===========================================================================================

/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_HDrawLagometer( int superhud ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
#ifdef MISSIONPACK
	x = 640 - 48;
	y = 480 - 144;
#else
	if ( hud[superhud].element == NetGraph ) {
		x = hud[superhud].rect[0];
		y = hud[superhud].rect[1];
	} else {
		x = 640 - 48;
		y = 480 - 48;
	}
#endif

	trap_R_SetColor( NULL );

	if (  hud[superhud].element == NetGraphPing ) {
			int pos_x = 0, w = 0;//, pos_y = 0;
			char	s[6];

			Com_sprintf(s, 5, "%i", cg.snap->ping);
			w = CG_DrawStrlen( s ) * hud[superhud].fontsize[0];

			if ( hud[superhud].fill ) {
				CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
			}

			//we need to "align"
			CG_DrawStringExt( pos_x, hud[superhud].rect[1], s, hud[superhud].color, qfalse, qfalse,
								hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
	}

	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}

	if ( hud[superhud].image ) {
		CG_DrawPic( hud[superhud].rect[0], hud[superhud].rect[1], 48, 48, hud[superhud].image );
	}

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		//CG_DrawBigString( ax, ay, "snc", 1.0 );
		CG_DrawStringExt(ax, ay, "snc", colorBlue, qfalse, qfalse,
								BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, 0 );
	}

	CG_DrawDisconnect();
}

/*
================================================================================

CROSSHAIR

================================================================================
*/

/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_HDrawCrosshairNames( int superhud ) {
	float		*color;
	char		*name;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}
	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color ) {
		trap_R_SetColor( NULL );
		return;
	}

	if ( (cg_drawCrosshairNames.integer > 1 && cgs.gametype < GT_TEAM) || (cg_drawCrosshairNames.integer == 2 && cgs.clientinfo[ cg.crosshairClientNum ].team == cgs.clientinfo[ cg.clientNum ].team && cgs.gametype >= GT_TEAM) ||
		 (cg_drawCrosshairNames.integer == 3 && cgs.clientinfo[ cg.crosshairClientNum ].team != cgs.clientinfo[ cg.clientNum ].team && cgs.gametype >= GT_TEAM) ) {
			return;
	}

	name = cgs.clientinfo[ cg.crosshairClientNum ].name;

	//CG_DrawBigString( 320 - w / 2, 170, name, color[3] * 0.5f );
	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}

	CG_DrawStringExt( 0, 0, name, NULL, qfalse, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );

	trap_R_SetColor( NULL );
}


//==============================================================================
/*
=================
CG_DrawChat
=================
*/
void CG_DrawChat ( int superhud, int shChatNum ) {
	//This is kinda fun.
	//
	if ( chat[shChatNum].time ) {
		//CG_Printf("Chat: %s - %i\n", hud[superhud].name, shChatNum);
		if ( cg.time < (chat[shChatNum].time + hud[superhud].time) ) {
			CG_DrawStringExt(0, 0, chat[shChatNum].msg, colorWhite, qfalse, qfalse, 0, 0, MAX_SAY_TEXT, 0, superhud);
		} else {
			chat[shChatNum].time = 0;
		}
	}
}

/*
=================
CG_DrawSpectator
=================
*/
static void CG_HDrawSpectator( int superhud) {
	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}
	CG_DrawStringExt(320 - 9 * 8, 440, "SPECTATOR", colorWhite, qfalse, qfalse,
								BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );
}

/*
=================
CG_DrawVote
=================
*/
static void CG_HDrawVote( int superhud ) {
	char	*s;
	int		sec;

	if ( !cgs.voteTime ) {
		return;
	}

	// play a talk beep whenever it is modified
	/*if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}*/

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
#ifdef MISSIONPACK
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString( 0, 58, s, 1.0F );
	s = "or press ESC then click Vote";
	CG_DrawSmallString( 0, 58 + SMALLCHAR_HEIGHT + 2, s, 1.0F );
#else
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo );

	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	} 
	CG_DrawStringExt( 0, 58, s, colorWhite, qtrue, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0, 0, superhud );

#endif
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_HDrawTeamVote( int superhud ) {
	char	*s;
	int		sec, cs_offset;

	if ( cgs.clientinfo->team == TEAM_RED )
		cs_offset = 0;
	else if ( cgs.clientinfo->team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !cgs.teamVoteTime[cs_offset] ) {
		return;
	}

	// play a talk beep whenever it is modified
	/*if ( cgs.teamVoteModified[cs_offset] ) {
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}*/

	sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[cs_offset] ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
							cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}
	CG_DrawStringExt( 0, 90, s, colorWhite, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0, 0, superhud );
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_HDrawFollow( int superhud ) {
//	float		x;
	vec4_t		color;
	const char	*name;
	char	s[512];

	if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;


	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;
	Com_sprintf(s, sizeof(s), "Following %s", name);

	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}
	CG_DrawStringExt( 0, hud[superhud].rect[1], s, hud[superhud].color, qfalse, qfalse,
								hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_HDrawAmmoWarning( int superhud ) {
	const char	*s;
	int			w;

	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if ( !cg.lowAmmoWarning ) {
		return;
	}

	if ( cg.lowAmmoWarning == 2 ) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}
	//CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
	CG_DrawStringExt( 320 - w / 2, 64, s, colorWhite, qfalse, qfalse, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0, 0, superhud );

}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_HDrawWarmup( int superhud ) {
	int			w;
	int			sec;
	int			i;
	float scale;
	clientInfo_t	*ci1, *ci2;
	int			cw;
	const char	*s;

	//FIXME: Optimize this.. Conform to CPMA..
	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	if ( sec < 0 ) {
		s = "Waiting for players";
		if ( !hud[superhud].element == WarmupInfo ) {
			if ( hud[superhud].fill ) {
				CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
			}

			CG_DrawStringExt( 0, hud[superhud].rect[1], s, hud[superhud].color, qfalse, qtrue,
								hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
			cg.warmupCount = 0;
			return;
		}
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE ) {
				if ( !ci1 ) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if ( ci1 && ci2 ) {
			s = va( "%s "S_COLOR_WHITE"vs %s", ci1->name, ci2->name );
			w = CG_DrawStrlen( s );
			if ( w > 640 / GIANT_WIDTH ) {
				cw = 640 / w;
			} else {
				cw = GIANT_WIDTH;
			}
			CG_DrawStringExt( 320 - w * cw/2, 20,s, colorWhite, qfalse, qtrue, cw, (int)(cw * 1.5f), 0, 0, superhud );
		}
	} else {
		if ( hud[superhud].element == GameType ) {
			if ( cgs.gametype == GT_FFA ) {
				s = "Free For All";
			} else if ( cgs.gametype == GT_TEAM ) {
				s = "Team Deathmatch";
			} else if ( cgs.gametype == GT_CTF ) {
				s = "Capture the Flag";
			} else {
				s = "";
			}

			if ( hud[superhud].fill ) {
				CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
			}

			CG_DrawStringExt( 0, hud[superhud].rect[1], s, hud[superhud].color, qfalse, qtrue,
								hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
		}
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
			trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
			trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
			trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}
	scale = 0.45f;
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		scale = 0.54f;
		break;
	case 1:
		cw = 24;
		scale = 0.51f;
		break;
	case 2:
		cw = 20;
		scale = 0.48f;
		break;
	default:
		cw = 16;
		scale = 0.45f;
		break;
	}
	if ( hud[superhud].element == WarmupInfo ) {
		if ( hud[superhud].fill ) {
			CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
		}

		CG_DrawStringExt( 0, hud[superhud].rect[1], s, hud[superhud].color, qfalse, qtrue,
							hud[superhud].fontsize[0], hud[superhud].fontsize[1], 0, 0, superhud );
	}
}

/*
=================
CG_DrawSpeedometer
Spike's Speedometer
=================
*/
void CG_HDrawSpeedometer(int superhud) {
	float speed;
	char *str;

	speed = 1.0f / Q_rsqrt(cg.snap->ps.velocity[0] * cg.snap->ps.velocity[0] +
					cg.snap->ps.velocity[1] * cg.snap->ps.velocity[1]);

	if(speed > max_speed)
		max_speed = speed;

	str = va("%.3fups", speed);


	if ( hud[superhud].fill ) {
		CG_FillRect( hud[superhud].rect[0], hud[superhud].rect[1], hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor );
	}

	CG_DrawStringExt(0, 0, str, 0, qfalse, qfalse, 12, SMALLCHAR_HEIGHT, 0, 0, superhud);
}

/*
=================
CG_DrawWeaponSideBar
=================
*/
#define STACK_VERTICAL_UP 1
#define STACK_HORIZONTAL_RIGHT 2
void CG_HDrawWeaponSideBar(int superhud ) {
	int		i;
	int		totalWeapons = 0, /*numWepsPerRow,*/ remainingWeps = 0;
//	vec4_t	vbgColor, vfontColor, vs_bgColor, vs_fontColor;
	int		x, y, iconx, icony, textx, texty;
//	int		numWeapons, totalHSpace, totalVSpace;
//	int		numRows;//, numCols; //May not need these
	char	ammo[6];
	qboolean firstRun = qtrue;

	//We need to find a startx and a starty to be static until we exit the function
	//They are for the Revo hax, because stacking up needs to precalc the starty and
	//right-to-left has to precalc stratx also..

	if ( hud[superhud].textalign == 1 ) {
		//X is the center point, not the starting point
		//Figure out how many weapons we can fit in a row before wrapping, if there is any
		int	curSpace = 0;

		for ( i = WP_NONE; i < WP_NUM_WEAPONS; i++ ) {
			//How many weapons do we have?
			if ( cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) || (hud[superhud].fill && cg.snap->ps.ammo[i]) ) {
				totalWeapons++;

				//FIXME: We can center align vertically in Revo via: STACK_VERTICAL_UP

				//Figure out the current amount of space and subtract the trailing 'buffer'
				curSpace = (totalWeapons * (hud[superhud].rect[2] + 4) - 4);
				if ( !((int)hud[superhud].angles[3] & STACK_HORIZONTAL_RIGHT) ) {
					//Normal left to right
					if ( curSpace * .5 + hud[superhud].rect[0] > SCREEN_WIDTH ) {
						//We're as far over as we can go, figure out the starting point and exit the loop
						x = hud[superhud].rect[0] - (((totalWeapons - 1) * (hud[superhud].rect[2] + 4) - 4) * .5);
						break;
					} else if ( (int)hud[superhud].angles[0] && curSpace * .5 + hud[superhud].rect[0] > (hud[superhud].rect[0] + (int)hud[superhud].angles[0])) {
						//We have to check against screen edge anyway...
						//Check against a forced wrap, figure out the starting point and exit the loop
						x = hud[superhud].rect[0] - (((totalWeapons - 1) * (hud[superhud].rect[2] + 4) - 4) * .5);
						break;
					}
				} else {
					//Revo special right to left
					if ( curSpace * .5 + hud[superhud].rect[0] > SCREEN_WIDTH ) {
						//We're as far over as we can go, figure out the starting point and exit the loop
						x = hud[superhud].rect[0] + (curSpace - hud[superhud].rect[2] - 4) * .5;
						break;
					} else if ( (int)hud[superhud].angles[0] && curSpace * .5 + hud[superhud].rect[0] > (hud[superhud].rect[0] + (int)hud[superhud].angles[0])) {
						//We have to check against screen edge anyway...
						//Check against a forced wrap, figure out the starting point and exit the loop
						x = hud[superhud].rect[0] + (curSpace - hud[superhud].rect[2] - 4) * .5;
						break;
					}

				}
			}
		}
	} else {
		//Not center aligning -- they go: top to bottom, left to right
		//if ( !((int)hud[superhud].angles[3] & STACK_HORIZONTAL_RIGHT) ) {
			//Normal left to right
			x = hud[superhud].rect[0];
		//} else {
			//Revo special right to left
			//FIXME: This will require some precalculation like center alignment
		//}
	}

	y = hud[superhud].rect[1];

	//CG_Printf("DBG:x:%i y:%i iconx:%i icony:%i textx:%i texty:%i\n", x,y,iconx,icony,textx,texty );

	for ( i = WP_GAUNTLET; i < WP_NUM_WEAPONS; i++ ) {
		if ( cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << i ) || (hud[superhud].fill && cg.snap->ps.ammo[i]) ) {

			/**********
			* Drawing *
			**********/
			//Background
			if ( i != cg.weaponSelect ) {
				//Not the weapon selected or holding
				CG_FillRect(x, y, hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].bgcolor);
			} else {
				//The weapon selected or holding
				CG_FillRect(x, y, hud[superhud].rect[2], hud[superhud].rect[3], hud[superhud].color);
			}

			if ( hud[superhud].textalign == 0 ) {
				iconx = x + hud[superhud].rect[2] - hud[superhud].fontsize[0];
				textx = iconx - ( CG_DrawStrlen(ammo) * hud[superhud].fontsize[0] );
			} else {
				iconx = x;
				textx = iconx + hud[superhud].fontsize[0] + 2;
			}
			
			icony = texty = y;
			//Weapon icon
			CG_DrawPic( iconx, icony, hud[superhud].fontsize[0], hud[superhud].fontsize[1], cg_weapons[i].weaponIcon );
			
			//no ammo icon
			if ( !cg.snap->ps.ammo[ i ] ) {
				CG_DrawPic( iconx, icony, hud[superhud].fontsize[0], hud[superhud].fontsize[1], cgs.media.noammoShader );
			}

			//ammo text
			if ( cg.snap->ps.ammo[i] < 0 ) {
				//Gauntlet would print -1.. do this as a sanity check.
				Com_sprintf( ammo, 5, "");
			} else if ( cg.snap->ps.ammo[i] >= 999 ) {
				//Instagib would print 169, no need for it anyway.
				Com_sprintf(ammo, 5, "999");
			} else {
				Com_sprintf(ammo, 5, "%i", cg.snap->ps.ammo[i]);
			}
			CG_DrawStringExt( textx, texty, ammo, colorWhite, qfalse, qtrue, hud[superhud].fontsize[0], hud[superhud].fontsize[1], 3, 0, superhud);

			//Calculate position for the next weapon
			if ( hud[superhud].textalign == 1 ) {
				int j;
				int curSpace = 0, weapons = 0;
				for ( j = i; j < WP_NUM_WEAPONS; j++ ) {
					//How many weapons do we have?
					if ( cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << j ) || (hud[superhud].fill && cg.snap->ps.ammo[j]) ) {
						//FIXME: We can center align vertically in Revo via: STACK_VERTICAL_UP
						weapons++;

						//Figure out the current amount of space and subtract the trailing 'buffer'
						curSpace = (weapons * (hud[superhud].rect[2] + 4) - 4);
						if ( !((int)hud[superhud].angles[3] & STACK_HORIZONTAL_RIGHT) ) {
							//Normal left to right
							if ( curSpace * .5 + hud[superhud].rect[0] > SCREEN_WIDTH ) {
								//We're as far over as we can go, figure out the starting point and exit the loop
								x = hud[superhud].rect[0] - (((weapons - 1) * (hud[superhud].rect[2] + 4) - 4) * .5);
								break;
							} else if ( (int)hud[superhud].angles[0] && curSpace * .5 + hud[superhud].rect[0] > (hud[superhud].rect[0] + (int)hud[superhud].angles[0])) {
								//We have to check against screen edge anyway...
								//Check against a forced wrap, figure out the starting point and exit the loop
								x = hud[superhud].rect[0] - (((weapons - 1) * (hud[superhud].rect[2] + 4) - 4) * .5);
								break;
							}
						} else {
							//Revo special right to left
							if ( curSpace * .5 + hud[superhud].rect[0] > SCREEN_WIDTH ) {
								//We're as far over as we can go, figure out the starting point and exit the loop
								x = hud[superhud].rect[0] + (curSpace - hud[superhud].rect[2] - 4) * .5;
								break;
							} else if ( (int)hud[superhud].angles[0] && curSpace * .5 + hud[superhud].rect[0] > (hud[superhud].rect[0] + (int)hud[superhud].angles[0])) {
								//We have to check against screen edge anyway...
								//Check against a forced wrap, figure out the starting point and exit the loop
								x = hud[superhud].rect[0] + (curSpace - hud[superhud].rect[2] - 4) * .5;
								break;
							}
						}
					}
				}
			} else {
				//FIXME? This doesn't wrap too early, does it?
				//CG_Printf("%i\n",(int)hud[superhud].angles[3]);
				if ( !((int)hud[superhud].angles[3] & STACK_VERTICAL_UP) ) {
					int	yy = y + hud[superhud].rect[3] + 4;

					if ( yy + hud[superhud].rect[3] > SCREEN_HEIGHT || ( (int)hud[superhud].angles[1] &&
						 yy + hud[superhud].rect[3] > hud[superhud].rect[1] + (int)hud[superhud].angles[1]) ) {
							//We have to wrap
							y = hud[superhud].rect[1];
							if ( !((int)hud[superhud].angles[3] & STACK_HORIZONTAL_RIGHT) ) {
								x += hud[superhud].rect[2] + 4;
							} else {
								x -= hud[superhud].rect[2] - 4;
							}
					} else {
						y = yy;
					}
				} else {
					//Revo stack vertically up
					int	yy = y - hud[superhud].rect[3] - 4;
					//CG_Printf("Stack up yy: %i\n", yy);

					if ( yy - hud[superhud].rect[3] < 0 || ( (int)hud[superhud].angles[1] &&
						 yy - hud[superhud].rect[3] < hud[superhud].rect[1]) ) {
							//We have to wrap
							y = hud[superhud].rect[1];
							if ( !((int)hud[superhud].angles[3] & STACK_HORIZONTAL_RIGHT) ) {
								x += hud[superhud].rect[2] + 4;
							} else {
								x -= hud[superhud].rect[2] - 4;
							}
					} else {
						y = yy;
					}
				}
			}

		}
	}
}
#undef STACK_VERTICAL_UP
#undef STACK_HORIZONTAL_RIGHT 


/*
=================
CG_DrawHud2D
Doesn't work with TeamArena -- will crash it.
=================
*/
void CG_DrawHud2D( void ) {
	int i;

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		DrawMOTDBox();
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		CG_DrawCrosshair();
	} else {
		//CG_DrawStatusBar();

		CG_DrawCrosshair();

		{//Evo's viewKeyPress...
			qboolean draw_keys;
			int		x, y, size;
			char	keyColor[24];
			char	fontColor[24];
			char	*sptr;
			char	token[24];
					
			sptr = ch_viewKeyPress.string;

			CG_ExtractToken(&sptr, token);
			draw_keys = atoi(token);

			if ( draw_keys ) {
				CG_ExtractToken(&sptr, token);
				x = atoi(token);

				CG_ExtractToken(&sptr, token);
				y = atoi(token);

				CG_ExtractToken(&sptr, token);
				Com_sprintf(keyColor, sizeof(keyColor), token);

				CG_ExtractToken(&sptr, token);
				Com_sprintf(fontColor, sizeof(fontColor), token);

				CG_ExtractToken(&sptr, token);
				size = atoi(token);

				CG_DrawKeyPress(draw_keys, x, y, keyColor, fontColor, size);
			}
		}

		CG_DrawHoldableItem();

		CG_DrawReward();

		if ( cgs.gametype >= GT_TEAM ) {
			CG_DrawTeamInfo();
		}
	}

	if ( cg_drawSnapshot.integer ) {
		CG_DrawSnapshot( 0 );
	}
	if ( cg_drawClock.integer ) {
		//This should be done in the hud with Clock {}
		CG_DrawClock( 60 );
	}


	// don't draw center string if scoreboard is up
	//Score_Weapon();
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if ( !cg.scoreBoardShowing) {
		CG_DrawCenterString();
	}

	for ( i = 1; i < (hud_count + 1); i++ ) {

		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ) {

			switch ( hud[i].element ) {
				case SpecMessage:
					//Should be done
					CG_HDrawSpectator(i);
					break;
				case TargetName:
					//TargetSatus hasn't been started, we don't have a TargetStatus yet anyway..
					CG_HDrawCrosshairNames(i);
					break;
			}
		} else {
			switch ( hud[i].element ) {
				case PostDecorate:
				case PreDecorate:
					if ( hud[i].fill ) {
						//Fill background
						CG_FillRect( hud[i].rect[0], hud[i].rect[1], hud[i].rect[2], hud[i].rect[3], CG_FigureHUDColor(i) );
					}
					if ( strlen(hud[i].text) ) {				
					//Draw text
						CG_DrawStringExt(0, 0, hud[i].text, colorWhite, qfalse, qfalse, 0, 0, 0, 0, i);
					}
					break;
				case FollowMessage:
					CG_HDrawFollow(i);
					break;
				case GameType:
				case WarmupInfo:
					CG_HDrawWarmup(i);
					break;
				case VoteMessageWorld:
					CG_HDrawVote(i);
					CG_HDrawTeamVote(i);
					break;
				case StatusBar_ArmorCount:
				case StatusBar_ArmorIcon:
				case StatusBar_AmmoCount:
				case StatusBar_AmmoIcon:
				case StatusBar_HealthCount:
				case StatusBar_HealthIcon:
					CG_HDrawStatusBar(i);
					break;
				case StatusBar_ArmorBar:
				case StatusBar_AmmoBar:
				case StatusBar_HealthBar:
					CG_DrawStatusBars( i );
					break;
				case PlayerSpeed:
					//This may want to end up referencing the cvar to enable max speed
					CG_HDrawSpeedometer(i);
					break;
				case WeaponList:
					//This isn't as pretty as cpma, it also doesn't wrap
					CG_HDrawWeaponSideBar(i);
					break;
			}

			// don't draw any status if dead or the scoreboard is being explicitly shown
			if ( !cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0 ) {
				switch (hud[i].element) {
					case AmmoMessage:
						CG_HDrawAmmoWarning(i);
						break;
					case TargetName:
						//Should be done, minus TargetStatus
						CG_HDrawCrosshairNames(i);
						break;
				}
			}
	    
			
		}

		switch (hud[i].element) {
			case NetGraph:
			case NetGraphPing:
				CG_HDrawLagometer(i);
				break;
			case Team1:
				CG_DrawSuperTeamOverlay( i, 0 );
				break;
			case Team2:
				CG_DrawSuperTeamOverlay( i, 1 );
				break;
			case Team3:
				CG_DrawSuperTeamOverlay( i, 2 );
				break;
			case Team4:
				CG_DrawSuperTeamOverlay( i, 3 );
				break;
			case Team5:
				CG_DrawSuperTeamOverlay( i, 4 );
				break;
			case Team6:
				CG_DrawSuperTeamOverlay( i, 5 );
				break;
			case Team7:
				CG_DrawSuperTeamOverlay( i, 6 );
				break;
			case Team8:
				CG_DrawSuperTeamOverlay( i, 7 );
				break;
			case Chat1:
				CG_DrawChat( i, 0 );
				break;
			case Chat2:
				CG_DrawChat( i, 1 );
				break;
			case Chat3:
				CG_DrawChat( i, 2 );
				break;
			case Chat4:
				CG_DrawChat( i, 3 );
				break;
			case Chat5:
				CG_DrawChat( i, 4 );
				break;
			case Chat6:
				CG_DrawChat( i, 5 );
				break;
			case Chat7:
				CG_DrawChat( i, 6 );
				break;
			case Chat8:
				CG_DrawChat( i, 7 );
				break;
			case FPS:
				CG_HDrawFPS( i );
				break;
			case GameTime:
				CG_HDrawTimer( i );
				break;
			case AttackerIcon:
			case AttackerName:
				CG_HDrawAttacker( i );
				break;
			case FragMessage:
				if ( cg.fragMessageTime + hud[i].time >= cg.time ) {
					if ( hud[i].fill ) {
						CG_FillRect(hud[i].rect[0], hud[i].rect[1], hud[i].rect[2], hud[i].rect[3], hud[i].bgcolor);
					}
					CG_DrawStringExt(0, 0, cg.fragMessageChar, colorWhite, qfalse, qfalse, 0, 0, 0, 0, i);
				}
				break;
			case RankMessage:
				if ( cg.rankMessageTime + hud[i].time >= cg.time ) {
					if ( hud[i].fill ) {
						CG_FillRect(hud[i].rect[0], hud[i].rect[1], hud[i].rect[2], hud[i].rect[3], hud[i].bgcolor);
					}
					CG_DrawStringExt(0, 0, cg.rankMessageChar, colorWhite, qfalse, qfalse, 0, 0, 0, 0, i);
				}
				break;
			case FlagStatus_NME:
			case FlagStatus_OWN:
			case Score_NME:
			case Score_OWN:
			case Score_Limit:
				 //TeamCounts aren't here.
				 //Should be done
				CG_HDrawScores( i );
				break;
			case PowerUp1_Icon:
			case PowerUp2_Icon:
			case PowerUp3_Icon:
			case PowerUp4_Icon:
			case PowerUp1_Time:
			case PowerUp2_Time:
			case PowerUp3_Time:
			case PowerUp4_Time:
				CG_DrawSuperPowerups( i );
				break;
			case ItemPickup:
			case ItemPickupIcon:
				//FIXME: This function doesn't handle 2 items.
				CG_HDrawPickupItem( i );
		}
		//if ( !Q_stricmp(hud[i].name, "Clock") ) {
		//	CG_DrawClock( 0 );
		//}
	}
	DrawMOTDBox();
}

