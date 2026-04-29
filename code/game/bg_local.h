/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// bg_local.h -- local definitions for the bg (both games) files

#define	MIN_WALK_NORMAL	0.7f		// can't walk on very steep slopes

#define	STEPSIZE		18

#define	JUMP_VELOCITY	270

#define	TIMER_LAND		130
#define	TIMER_GESTURE	(34*66+50)

#define	OVERCLIP		1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct {
	vec3_t		forward, right, up;
	float		frametime;

	int			msec;

	qboolean	walking;
	qboolean	groundPlane;
	trace_t		groundTrace;
	qboolean	ladder; // tells us when the player is on a ladder

	qboolean	sliding;    // crouchslide active
	int			slideTime;  // crouchslide timer (ms)

	float		impactSpeed;

	vec3_t		previous_origin;
	vec3_t		previous_velocity;
	int			previous_waterlevel;
} pml_t;

extern	pmove_t		*pm;
extern	pml_t		pml;

// movement parameters
extern	float	pm_stopspeed;
extern	float	pm_duckScale;
extern	float	pm_swimScale;
extern	float	pm_wadeScale;

extern	float	pm_accelerate;
extern	float	pm_airaccelerate;
extern	float	pm_wateraccelerate;
extern	float	pm_flyaccelerate;

extern	float	pm_friction;
extern	float	pm_waterfriction;
extern	float	pm_flightfriction;

extern	int		c_pmove;

// OSP2-BE mode variables (defined in bg_slidemove.c)
extern int modeMaxAmmoShotgun;
extern int modeGrenadeTime;
extern int modeMaxAmmoGrenade;
extern int modeMaxAmmoRocket;
extern int modeMaxAmmoRail;
extern int modeBeginWeaponChangeTime;
extern int modeFinishWeaponChangeTime;
extern int modePMNoAmmoTime;
extern int pm_armorPromode;
extern int modeHitLevelSounds;
extern int modePickupDistance;
extern int modePromodePhysKoeff;
extern float modePromode_pm_airaccelerate_1;
extern int modePredictionKoeff2;
extern float modePromode_pm_airaccelerate_2;
extern float modeWishspeedLimit;
extern int modePredictionKoeff1;
extern float modeSwimScale1;
extern float modeSwimScale2;
extern float modeShotgunKoeff;
extern int modeShotgunNumberOfPellets;
extern int modeUnknown2;
extern int modeUnknown3;
extern int modeUnknown4;
extern int modeShotgunPromode;

void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_ClipVelocityOSP( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_AddTouchEnt( int entityNum );
void PM_AddEvent( int newEvent );

qboolean	PM_SlideMove( qboolean gravity );
void		PM_StepSlideMove( qboolean gravity );
