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
// g_combat.c

#include "g_local.h"

static gitem_t *G_FindItemByClassname( const char *classname ) {
	int i;

	if ( !classname || !classname[0] )
		return NULL;

	for ( i = 1; i < bg_numItems; i++ ) {
		if ( bg_itemlist[i].classname && !Q_stricmp( bg_itemlist[i].classname, classname ) )
			return &bg_itemlist[i];
	}

	return NULL;
}

static void G_PortalRandomizeCoinVelocity( gentity_t *drop ) {
	vec3_t dir;
	float speed;

	if ( !drop )
		return;

	// Random outward burst in XY with a consistent upward kick.
	dir[0] = crandom();
	dir[1] = crandom();
	dir[2] = 0.0f;

	if ( dir[0] == 0.0f && dir[1] == 0.0f )
		dir[0] = 1.0f;

	VectorNormalize( dir );
	speed = 175.0f + random() * 225.0f;

	drop->s.pos.trDelta[0] = dir[0] * speed;
	drop->s.pos.trDelta[1] = dir[1] * speed;
	drop->s.pos.trDelta[2] = 170.0f + random() * 170.0f;
}

static void G_PortalSpillCoins( gentity_t *ent, int points ) {
	gitem_t *coinBig;
	gitem_t *coinMedium;
	gitem_t *coinSmall;
	gentity_t *drop;
	int bigCount;
	int mediumCount;
	int smallCount;
	int i;
	float angle;

	if ( !ent || points <= 0 )
		return;

	coinBig = G_FindItemByClassname( "item_coin_big" );
	coinMedium = G_FindItemByClassname( "item_coin_medium" );
	coinSmall = G_FindItemByClassname( "item_coin_small" );
	if ( !coinBig || !coinMedium || !coinSmall )
		return;

	bigCount = points / 50;
	points %= 50;
	mediumCount = points / 10;
	smallCount = points % 10;

	angle = (float)( rand() % 360 );
	for ( i = 0; i < bigCount; i++ ) {
		drop = Drop_Item( ent, coinBig, angle );
		if ( drop ) {
			G_PortalRandomizeCoinVelocity( drop );
			drop->r.ownerNum = ent->s.number;
			drop->dropTime = level.time + 1000;
		}
		angle = (float)( rand() % 360 );
	}
	for ( i = 0; i < mediumCount; i++ ) {
		drop = Drop_Item( ent, coinMedium, angle );
		if ( drop ) {
			G_PortalRandomizeCoinVelocity( drop );
			drop->r.ownerNum = ent->s.number;
			drop->dropTime = level.time + 1000;
		}
		angle = (float)( rand() % 360 );
	}
	for ( i = 0; i < smallCount; i++ ) {
		drop = Drop_Item( ent, coinSmall, angle );
		if ( drop ) {
			G_PortalRandomizeCoinVelocity( drop );
			drop->r.ownerNum = ent->s.number;
			drop->dropTime = level.time + 1000;
		}
		angle = (float)( rand() % 360 );
	}
}


/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( level.warmupTime ) {
		return;
	}
	// no frag scoring between round end and the next round going live (GT_CTFS)
	if ( g_gametype.integer == GT_CTFS &&
	     level.atdRoundNumber != level.atdRoundNumberStarted ) {
		return;
	}
	// show score plum
	ScorePlum(ent, origin, score);
	//
	ent->client->ps.persistant[PERS_SCORE] += score;
	if ( g_gametype.integer == GT_TEAM ) {
		AddTeamScore( origin, ent->client->ps.persistant[PERS_TEAM], score );
	}
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
// void TossClientItems( gentity_t *self ) {
// 	gitem_t		*item;
// 	int			weapon;
// 	float		angle;
// 	int			i;
// 	gentity_t	*drop;

// 	// drop the weapon if not a gauntlet or machinegun
// 	weapon = self->s.weapon;

// 	// make a special check to see if they are changing to a new
// 	// weapon that isn't the mg or gauntlet.  Without this, a client
// 	// can pick up a weapon, be killed, and not drop the weapon because
// 	// their weapon change hasn't completed yet and they are still holding the MG.
// 	if ( weapon == WP_MACHINEGUN || weapon == WP_GRAPPLING_HOOK ) {
// 		if ( self->client->ps.weaponstate == WEAPON_DROPPING ) {
// 			weapon = self->client->pers.cmd.weapon;
// 		}
// 		if ( !( self->client->ps.stats[STAT_WEAPONS] & ( 1 << weapon ) ) ) {
// 			weapon = WP_NONE;
// 		}
// 	}

// 	if ( weapon > WP_MACHINEGUN && weapon != WP_GRAPPLING_HOOK &&
// 		self->client->ps.ammo[ weapon ] && !g_instagib.integer ) {
// 		// find the item type for this weapon
// 		item = BG_FindItemForWeapon( weapon );

// 		// spawn the item
// 		drop = Drop_Item( self, item, 0 );

// 		// for pickup prediction
// 		drop->s.time2 = item->quantity;
// 	}

// 	// drop all the powerups if not in teamplay
// 	if ( g_gametype.integer != GT_TEAM ) {
// 		angle = 45;
// 		for ( i = 1 ; i < PW_NUM_POWERUPS ; i++ ) {
// 			if ( self->client->ps.powerups[ i ] > level.time ) {
// 				item = BG_FindItemForPowerup( i );
// 				if ( !item ) {
// 					continue;
// 				}
// 				drop = Drop_Item( self, item, angle );
// 				// decide how many seconds it has left
// 				drop->count = ( self->client->ps.powerups[ i ] - level.time ) / 1000;
// 				if ( drop->count < 1 ) {
// 					drop->count = 1;
// 				}
// 				// for pickup prediction
// 				drop->s.time2 = drop->count;
// 				angle += 45;
// 			}
// 		}
// 	}
// }

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems(gentity_t *self) {
    gitem_t *item;
    float angle;
    int i;
    gentity_t *drop;

    /* GT_CTFS: no weapons or holdables on the map.  Drop any carried flag in
       place so it stays on the field (Team_DroppedFlagThink returns early for
       GT_CTFS, so the flag is never auto-returned).  LaunchItem also calls
       Team_CheckDroppedItem which updates CS_FLAGSTATUS to FLAG_DROPPED so
       the POI tracking reflects the dropped position. */
    if ( g_gametype.integer == GT_CTFS ) {
        int      flagPw   = 0;
        gitem_t *flagItem = NULL;
        if ( self->client->ps.powerups[PW_REDFLAG] > level.time ) {
            flagPw = PW_REDFLAG;
        } else if ( self->client->ps.powerups[PW_BLUEFLAG] > level.time ) {
            flagPw = PW_BLUEFLAG;
        }
        if ( flagPw ) {
            flagItem = BG_FindItemForPowerup( flagPw );
            if ( flagItem ) {
                Drop_Item( self, flagItem, 0 );
                self->client->ps.powerups[flagPw] = 0;
            }
        }
        return;
    }

    // Drop the weapon if enabled
    if (g_itemDrop.integer & 2) { // qlone - conditional weapon toss
        int weapon = self->s.weapon;

        // Special check for weapon change in progress
        if (weapon == WP_MACHINEGUN || weapon == WP_GRAPPLING_HOOK) {
            if (self->client->ps.weaponstate == WEAPON_DROPPING) {
                weapon = self->client->pers.cmd.weapon;
            }
            if (!(self->client->ps.stats[STAT_WEAPONS] & (1 << weapon))) {
                weapon = WP_NONE;
            }
        }

        // Drop the weapon if it's valid and has ammo
        if (weapon > WP_MACHINEGUN && weapon != WP_GRAPPLING_HOOK && self->client->ps.ammo[weapon]) {
            // Find the item type for this weapon
            item = BG_FindItemForWeapon(weapon);

            // Spawn the dropped weapon
            drop = Drop_Item(self, item, 0);

            // Set the ammo count for the dropped weapon
            drop->ammoCount = self->client->ps.ammo[weapon];

            // Remove the weapon and its ammo from the player
            self->client->ps.stats[STAT_WEAPONS] &= ~(1 << weapon);
            self->client->ps.ammo[weapon] = 0;
        }
    }

    // Drop all holdable items
    for (i = 1; i < HI_NUM_HOLDABLE; i++) {
        if (i == HI_KAMIKAZE) continue;
        if (bg_itemlist[self->client->ps.stats[STAT_HOLDABLE_ITEM]].giTag == i) {
            item = BG_FindItemForHoldable(i);
            if (!item) break;
            drop = Drop_Item(self, item, 45);
            break;
        }
    }

    // Drop all powerups if enabled
    angle = 45;
    for (i = 1; i < PW_NUM_POWERUPS; i++) {
        if (g_itemDrop.integer & 32) {
            if (self->client->ps.powerups[i] > level.time) {
                item = BG_FindItemForPowerup(i);
                if (!item) {
                    continue;
                }
                drop = Drop_Item(self, item, angle);

                // Set the remaining time for the powerup
                drop->count = (self->client->ps.powerups[i] - level.time) / 1000;
                if (drop->count < 1) {
                    drop->count = 1;
                }

                // For pickup prediction
                drop->s.time2 = drop->count;
                angle += 45;
            }
        } else {
            self->client->ps.powerups[i] = 0;
        }
    }
}


#ifdef MISSIONPACK
/*
=================
TossClientCubes
=================
*/
extern gentity_t	*neutralObelisk;

void TossClientCubes( gentity_t *self ) {
	gitem_t		*item;
	gentity_t	*drop;
	vec3_t		velocity;
	vec3_t		angles;
	vec3_t		origin;

	self->client->ps.generic1 = 0;

	// this should never happen but we should never
	// get the server to crash due to skull being spawned in
	if (!G_EntitiesFree()) {
		return;
	}

	if( self->client->sess.sessionTeam == TEAM_RED ) {
		item = BG_FindItem( "Red Cube" );
	}
	else {
		item = BG_FindItem( "Blue Cube" );
	}

	angles[YAW] = (float)(level.time % 360);
	angles[PITCH] = 0;	// always forward
	angles[ROLL] = 0;

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;

	if( neutralObelisk ) {
		VectorCopy( neutralObelisk->s.pos.trBase, origin );
		origin[2] += 44;
	} else {
		VectorClear( origin ) ;
	}

	drop = LaunchItem( item, origin, velocity );

	drop->nextthink = level.time + g_cubeTimeout.integer * 1000;
	drop->think = G_FreeEntity;
	drop->spawnflags = self->client->sess.sessionTeam;
}


/*
=================
TossClientPersistantPowerups
=================
*/
void TossClientPersistantPowerups( gentity_t *ent ) {
	gentity_t	*powerup;

	if( !ent->client ) {
		return;
	}

	if( !ent->client->persistantPowerup ) {
		return;
	}

	powerup = ent->client->persistantPowerup;

	powerup->r.svFlags &= ~SVF_NOCLIENT;
	powerup->s.eFlags &= ~EF_NODRAW;
	powerup->r.contents = CONTENTS_TRIGGER;
	trap_LinkEntity( powerup );

	ent->client->ps.stats[STAT_PERSISTANT_POWERUP] = 0;
	ent->client->persistantPowerup = NULL;
}
#endif


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
#ifdef MISSIONPACK
	gentity_t *ent;
	int i;

	//if this entity still has kamikaze
	if (self->s.eFlags & EF_KAMIKAZE) {
		// check if there is a kamikaze timer around for this owner
		for (i = 0; i < level.num_entities; i++) {
			ent = &g_entities[i];
			if (!ent->inuse)
				continue;
			if (ent->activator != self)
				continue;
			if (strcmp(ent->classname, "kamikaze timer"))
				continue;
			G_FreeEntity(ent);
			break;
		}
	}
#endif

	G_AddEvent( self, EV_GIB_PLAYER, killer );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}
	if ( !g_blood.integer ) {
		self->health = GIB_HEALTH+1;
		return;
	}

	GibEntity( self, 0 );
}


// these are just for logging, the client prints its own messages
char	*modNames[] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_GAUNTLET",
	"MOD_MACHINEGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_PLASMA",
	"MOD_PLASMA_SPLASH",
	"MOD_RAILGUN",
	"MOD_LIGHTNING",
	"MOD_BFG",
	"MOD_BFG_SPLASH",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
#ifdef MISSIONPACK
	"MOD_NAIL",
	"MOD_CHAINGUN",
	"MOD_PROXIMITY_MINE",
	"MOD_KAMIKAZE",
	"MOD_JUICED",
#endif
	"MOD_GRAPPLE"
};

#ifdef MISSIONPACK
/*
==================
Kamikaze_DeathActivate
==================
*/
void Kamikaze_DeathActivate( gentity_t *ent ) {
	G_StartKamikaze(ent);
	G_FreeEntity(ent);
}

/*
==================
Kamikaze_DeathTimer
==================
*/
void Kamikaze_DeathTimer( gentity_t *self ) {
	gentity_t *ent;

	ent = G_Spawn();
	ent->classname = "kamikaze timer";
	VectorCopy(self->s.pos.trBase, ent->s.pos.trBase);
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->think = Kamikaze_DeathActivate;
	ent->nextthink = level.time + 5 * 1000;

	ent->activator = self;
}

#endif

/*
==================
CheckAlmostCapture
==================
*/
void CheckAlmostCapture( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if this player was carrying a flag
	if ( self->client->ps.powerups[PW_REDFLAG] ||
		self->client->ps.powerups[PW_BLUEFLAG] ||
		self->client->ps.powerups[PW_NEUTRALFLAG] ) {
		// get the goal flag this player should have been going for
		if ( g_gametype.integer == GT_CTF ) {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_blueflag";
			}
			else {
				classname = "team_CTF_redflag";
			}
		}
		else {
			if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
				classname = "team_CTF_redflag";
			}
			else {
				classname = "team_CTF_blueflag";
			}
		}
		ent = NULL;
		do
		{
			ent = G_Find(ent, FOFS(classname), classname);
		} while (ent && (ent->flags & FL_DROPPED_ITEM));
		// if we found the destination flag and it's not picked up
		if (ent && !(ent->r.svFlags & SVF_NOCLIENT) ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

/*
==================
CheckAlmostScored
==================
*/
void CheckAlmostScored( gentity_t *self, gentity_t *attacker ) {
	gentity_t	*ent;
	vec3_t		dir;
	char		*classname;

	// if the player was carrying cubes
	if ( self->client->ps.generic1 ) {
		if ( self->client->sess.sessionTeam == TEAM_BLUE ) {
			classname = "team_redobelisk";
		}
		else {
			classname = "team_blueobelisk";
		}
		ent = G_Find(NULL, FOFS(classname), classname);
		// if we found the destination obelisk
		if ( ent ) {
			// if the player was *very* close
			VectorSubtract( self->client->ps.origin, ent->s.origin, dir );
			if ( VectorLength(dir) < 200 ) {
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				if ( attacker->client ) {
					attacker->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_HOLYSHIT;
				}
			}
		}
	}
}

void G_GenericDeathCleanup( gentity_t *self ) {
	if (self->client && self->client->hook) {
		Weapon_HookFree(self->client->hook);
	}

#ifdef MISSIONPACK
	if ((self->client->ps.eFlags & EF_TICKING) && self->activator) {
		self->client->ps.eFlags &= ~EF_TICKING;
		self->activator->think = G_FreeEntity;
		self->activator->nextthink = level.time;
	}
#endif
}

/*
==================
G_LastAliveOnTeam

Returns the client number of the last living player on the given team
if exactly one is left, or -1 if zero or more than one are alive.
Used by GT_CTFS elimination logic and the last-standing sound cue.
==================
*/
int G_LastAliveOnTeam( team_t team ) {
	int        i;
	int        aliveCount = 0;
	int        lastAlive  = -1;
	gclient_t *cl;

	for ( i = 0; i < level.maxclients; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected != CON_CONNECTED ) continue;
		if ( cl->sess.sessionTeam != team ) continue;
		if ( cl->atdDeadSpecTeam != TEAM_FREE ) continue;
		if ( g_entities[i].health <= 0 ) continue;
		aliveCount++;
		lastAlive = i;
	}
	return ( aliveCount == 1 ) ? lastAlive : -1;
}

/*
==================
G_CheckLastTeamStanding

Plays the "last standing" sound cue when exactly one teammate is still
alive after a player dies in GT_CTFS.
==================
*/
static void G_CheckLastTeamStanding( gentity_t *self ) {
	int        i;
	int        lastAlive;
	team_t     myTeam;
	gclient_t *cl;

	if ( g_gametype.integer != GT_CTFS ) return;
	if ( level.warmupTime != 0 ) return;
	if ( level.atdRoundNumber != level.atdRoundNumberStarted ) return;

	if ( self->client->atdDeadSpecTeam != TEAM_FREE ) {
		myTeam = self->client->atdDeadSpecTeam;
	} else {
		myTeam = self->client->sess.sessionTeam;
		if ( myTeam == TEAM_SPECTATOR || myTeam == TEAM_FREE ) return;
	}

	lastAlive = G_LastAliveOnTeam( myTeam );
	if ( lastAlive < 0 ) return;

	G_ATDClientSound( lastAlive, "sound/vo_evil/last_standing.wav" );

	for ( i = 0; i < level.maxclients; i++ ) {
		if ( i == lastAlive ) continue;
		cl = &level.clients[i];
		if ( cl->pers.connected != CON_CONNECTED ) continue;
		if ( cl->sess.spectatorState != SPECTATOR_FOLLOW ) continue;
		if ( cl->sess.spectatorClient != lastAlive ) continue;
		G_ATDClientSound( i, "sound/vo_evil/last_standing.wav" );
	}
}

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	qboolean	miniGameOneScoring;
	qboolean	miniGameFourNoFragScoring;
	int			fallPusherNum;
	char		*killerName, *obit;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	//unlag the client
	G_UnTimeShiftClient( self );

	// check for an almost capture
	CheckAlmostCapture( self, attacker );
	// check for a player that almost brought in cubes
	CheckAlmostScored( self, attacker );

	if (self->client && self->client->hook) {
		Weapon_HookFree(self->client->hook);
	}
#ifdef MISSIONPACK
	if ((self->client->ps.eFlags & EF_TICKING) && self->activator) {
		self->client->ps.eFlags &= ~EF_TICKING;
		self->activator->think = G_FreeEntity;
		self->activator->nextthink = level.time;
	}
#endif
	G_GenericDeathCleanup(self); // removes hook and prox mines stuck to victim

	self->client->ps.pm_type = PM_DEAD;
	fallPusherNum = -1;

	/*
	 * Platform mini-games can kill via world hazards (e.g. trigger_hurt pit)
	 * rather than strict falling damage, so attribute recent platform shots
	 * across environment death mods.
	 */
	if ( self->client
		&& p_enablePortal.integer && g_gametype.integer == GT_FFA
		&& G_PortalMiniGameRulesEnabled()
		&& ( meansOfDeath == MOD_FALLING
			|| meansOfDeath == MOD_TRIGGER_HURT
			|| meansOfDeath == MOD_LAVA
			|| meansOfDeath == MOD_SLIME
			|| meansOfDeath == MOD_WATER
			|| meansOfDeath == MOD_CRUSH ) ) {
		int pusherNum = self->client->portalStepPusherClient;
		if ( self->client->portalStepPusherTime > 0
			&& level.time - self->client->portalStepPusherTime <= 7000
			&& pusherNum >= 0 && pusherNum < level.maxclients
			&& pusherNum != self->s.number
			&& g_entities[pusherNum].inuse
			&& g_entities[pusherNum].client
			&& g_entities[pusherNum].client->pers.connected == CON_CONNECTED
			&& !OnSameTeam( self, &g_entities[pusherNum] ) ) {
			fallPusherNum = pusherNum;
			attacker = &g_entities[pusherNum];
			inflictor = attacker;
		}
	}

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( (unsigned)meansOfDeath >= ARRAY_LEN( modNames ) ) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

	G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n",
		killer, self->s.number, meansOfDeath, killerName,
		self->client->pers.netname, obit );

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self - g_entities;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	self->enemy = attacker;
	if ( self->client ) {
		self->client->portalStepPusherClient = -1;
		self->client->portalStepPusherTime = 0;
	}

	self->client->ps.persistant[PERS_KILLED]++;
	miniGameOneScoring = ( G_PortalCurrentMiniGame() == 1 );
	miniGameFourNoFragScoring = ( G_PortalCurrentMiniGame() == 4 );

	if (attacker && attacker->client) {
		attacker->client->lastkilled_client = self->s.number;

		if ( attacker == self || OnSameTeam (self, attacker ) ) {
			if ( !miniGameOneScoring && !miniGameFourNoFragScoring ) {
				AddScore( attacker, self->r.currentOrigin, -1 );
			}
		} else {
			if ( miniGameOneScoring ) {
				if ( attacker->client->ps.powerups[PW_QUAD] > level.time ) {
					AddScore( attacker, self->r.currentOrigin, 1 );
				}
			} else if ( !miniGameFourNoFragScoring ) {
				AddScore( attacker, self->r.currentOrigin, 1 );
			}

			if( meansOfDeath == MOD_GAUNTLET ) {

				// play humiliation on player
				attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
				attacker->client->ps.eFlags |= EF_AWARD_GAUNTLET;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;

				// also play humiliation on target
				self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
			}

			// check for two kills in a short amount of time
			// if this is close enough to the last kill, give a reward sound
			if ( level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME ) {
				// play excellent on player
				attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;

				// add the sprite over the player's head
				attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP );
				attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
				attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
			}
			attacker->client->lastKillTime = level.time;

		}
	} else {
		if ( !miniGameOneScoring && !miniGameFourNoFragScoring && fallPusherNum < 0 ) {
			AddScore( self, self->r.currentOrigin, -1 );
		}
	}

	// Add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE) {
#ifdef MISSIONPACK
		if ( self->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
			Team_ReturnFlag( TEAM_FREE );
			self->client->ps.powerups[PW_NEUTRALFLAG] = 0;
		} else
#endif
		if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
			self->client->ps.powerups[PW_REDFLAG] = 0;
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
			self->client->ps.powerups[PW_BLUEFLAG] = 0;
		}
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP )) {
		TossClientItems( self );
	}
	else {
		if ( self->client->ps.powerups[PW_NEUTRALFLAG] ) {		// only happens in One Flag CTF
			Team_ReturnFlag( TEAM_FREE );
		}
		else if ( self->client->ps.powerups[PW_REDFLAG] ) {		// only happens in standard CTF
			Team_ReturnFlag( TEAM_RED );
		}
		else if ( self->client->ps.powerups[PW_BLUEFLAG] ) {	// only happens in standard CTF
			Team_ReturnFlag( TEAM_BLUE );
		}
	}
#ifdef MISSIONPACK
	TossClientPersistantPowerups( self );
	if( g_gametype.integer == GT_HARVESTER ) {
		TossClientCubes( self );
	}
#endif

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	self->r.maxs[2] = -8;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 1700;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// never gib in a nodrop
	if ( (self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity( self, killer );
	} else {
		// normal death
		static int i;

		switch ( i ) {
		case 0:
			anim = BOTH_DEATH1;
			break;
		case 1:
			anim = BOTH_DEATH2;
			break;
		case 2:
		default:
			anim = BOTH_DEATH3;
			break;
		}

		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

		self->client->ps.legsAnim =
			( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
		self->client->ps.torsoAnim =
			( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

		G_AddEvent( self, EV_DEATH1 + i, killer );

		// the body can still be gibbed
		self->die = body_die;

		// globally cycle through the different death animations
		i = ( i + 1 ) % 3;

#ifdef MISSIONPACK
		if (self->s.eFlags & EF_KAMIKAZE) {
			Kamikaze_DeathTimer( self );
		}
#endif
	}

	/* GT_CTFS: move dead human players into free-spectate for the round.
	   G_CheckATDRound will restore them to their team before the next round. */
	if ( g_gametype.integer == GT_CTFS &&
	     level.warmupTime == 0 &&
	     level.atdRoundNumber == level.atdRoundNumberStarted &&
	     !( self->r.svFlags & SVF_BOT ) ) {
		team_t origTeam = self->client->sess.sessionTeam;
		self->client->sess.sessionTeam    = TEAM_SPECTATOR;
		self->client->sess.spectatorState = SPECTATOR_FREE;
		self->client->atdDeadSpecTeam     = origTeam;
		G_ATDCycleTeammateFollow( self );
	}
	G_CheckLastTeamStanding( self );

	trap_LinkEntity (self);

}


/*
================
CheckArmor
================
*/
int CheckArmor (gentity_t *ent, int damage, int dflags)
{
	gclient_t	*client;
	int			save;
	int			count;
	float		type; // CPM

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	// armor
	count = client->ps.stats[STAT_ARMOR];
	save = ceil( damage * ARMOR_PROTECTION );
	if (save >= count)
		save = count;

	if (!save)
		return 0;

	client->ps.stats[STAT_ARMOR] -= save;

	return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

#ifdef MISSIONPACK
/*
================
G_InvulnerabilityEffect
================
*/
int G_InvulnerabilityEffect( gentity_t *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir ) {
	gentity_t	*impact;
	vec3_t		intersections[2], vec;
	int			n;

	if ( !targ->client ) {
		return qfalse;
	}
	VectorCopy(dir, vec);
	VectorInverse(vec);
	// sphere model radius = 42 units
	n = RaySphereIntersections( targ->client->ps.origin, 42, point, vec, intersections);
	if (n > 0) {
		impact = G_TempEntity( targ->client->ps.origin, EV_INVUL_IMPACT );
		VectorSubtract(intersections[0], targ->client->ps.origin, vec);
		vectoangles(vec, impact->s.angles);
		impact->s.angles[0] += 90;
		if (impact->s.angles[0] > 360)
			impact->s.angles[0] -= 360;
		if ( impactpoint ) {
			VectorCopy( intersections[0], impactpoint );
		}
		if ( bouncedir ) {
			VectorCopy( vec, bouncedir );
			VectorNormalize( bouncedir );
		}
		return qtrue;
	}
	else {
		return qfalse;
	}
}
#endif
/*
============
G_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	gclient_t	*client;
	int			take;
	int			asave;
	int			knockback;
	int			max;
	int			preDamageHealth;
#ifdef MISSIONPACK
	vec3_t		bouncedir, impactpoint;
#endif

	if (!targ->takedamage) {
		return;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}
	/* GT_CTFS: damage is allowed during inter-round warmup; scoring is
	   suppressed separately in AddScore. */
#ifdef MISSIONPACK
	if ( targ->client && mod != MOD_JUICED) {
		if ( targ->client->invulnerabilityTime > level.time) {
			if ( dir && point ) {
				G_InvulnerabilityEffect( targ, dir, point, impactpoint, bouncedir );
			}
			return;
		}
	}
#endif
	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	/* Portal map-voting room: no damage at all while selecting maps. */
	if ( G_PortalLobbyRulesEnabled() && targ->client ) {
		return;
	}

	if ( p_enablePortal.integer && g_gametype.integer == GT_FFA && targ->client && attacker->client && targ != attacker ) {
		/* Mini-game 2 (steps): rail can only break platforms, never damage players. */
		if ( level.portalCurrentMinigame == 2 && mod == MOD_RAILGUN ) {
			return;
		}
	}

	/* spawn protection: immune to damage (except telefrag), except in portal contexts */
	if ( !G_PortalLobbyRulesEnabled() && !G_PortalMiniGameRulesEnabled() ) {
		if ( targ && targ->client && targ->client->ps.powerups[PW_SPAWNPROTECTION] > level.time && mod != MOD_TELEFRAG ) {
			return;
		}
		if ( attacker && attacker->client && attacker->client->ps.powerups[PW_SPAWNPROTECTION] > level.time && mod != MOD_TELEFRAG ) {
			return;
		}
	}

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		if ( p_enablePortal.integer && g_gametype.integer == GT_FFA
			&& G_PortalMiniGameRulesEnabled() && mod == MOD_RAILGUN
			&& attacker && attacker->client ) {
			int i;
			for ( i = 0; i < level.maxclients; i++ ) {
				gentity_t *victim = &g_entities[i];
				qboolean onHitPlatform;
				if ( !victim->inuse || !victim->client || victim == attacker ) {
					continue;
				}
				if ( victim->client->pers.connected != CON_CONNECTED ) {
					continue;
				}

				onHitPlatform = ( victim->client->ps.groundEntityNum == targ->s.number
					|| victim->s.groundEntityNum == targ->s.number );
				if ( !onHitPlatform ) {
					float px = victim->r.currentOrigin[0];
					float py = victim->r.currentOrigin[1];
					float pz = victim->r.currentOrigin[2];
					onHitPlatform = ( px >= ( targ->r.absmin[0] - 20.0f )
						&& px <= ( targ->r.absmax[0] + 20.0f )
						&& py >= ( targ->r.absmin[1] - 20.0f )
						&& py <= ( targ->r.absmax[1] + 20.0f )
						&& pz >= ( targ->r.absmax[2] - 40.0f )
						&& pz <= ( targ->r.absmax[2] + 72.0f ) );
				}
				if ( !onHitPlatform ) {
					continue;
				}
				victim->client->portalStepPusherClient = attacker->s.number;
				victim->client->portalStepPusherTime = level.time;
			}
		}
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			targ->use( targ, inflictor, attacker );
		}
		return;
	}
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_OBELISK && CheckObeliskAttack( targ, attacker ) ) {
		return;
	}
#endif
	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	if ( attacker->client && attacker != targ ) {
		max = attacker->client->ps.stats[STAT_MAX_HEALTH];
#ifdef MISSIONPACK
		if( bg_itemlist[attacker->client->ps.stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD ) {
			max /= 2;
		}
#endif
		damage = damage * max / 100;
	}

	client = targ->client;

	if ( client ) {
		if ( client->noclip ) {
			return;
		}
	}

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

	knockback = damage;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	}
	if ( dflags & DAMAGE_NO_KNOCKBACK ) {
		knockback = 0;
	}

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) {
		vec3_t	kvel;
		float	mass;

		mass = 200;

		if (targ != attacker) {
			if (g_instagib.integer) {
				switch (mod) {
					case MOD_GAUNTLET:
					case MOD_RAILGUN:
						damage = INFINITE;
				}
			}
		}

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
#ifdef MISSIONPACK
		if ( mod != MOD_JUICED && targ != attacker && !(dflags & DAMAGE_NO_TEAM_PROTECTION) && OnSameTeam (targ, attacker)  ) {
#else
		if ( targ != attacker && OnSameTeam (targ, attacker)  ) {
#endif
			if ( !g_friendlyFire.integer ) {
				return;
			}
		}
#ifdef MISSIONPACK
		if (mod == MOD_PROXIMITY_MINE) {
			if (inflictor && inflictor->parent && OnSameTeam(targ, inflictor->parent)) {
				return;
			}
			if (targ == attacker) {
				return;
			}
		}
#endif

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( client && client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( ( dflags & DAMAGE_RADIUS ) || ( mod == MOD_FALLING ) ) {
			return;
		}
		damage *= 0.5;
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker) {
		damage *= 0.5;
	}

	if ( damage < 1 ) {
		damage = 1;
	}
	take = damage;
	preDamageHealth = targ->health;

	//qlone - self damages
	if ( targ == attacker && g_noSelfDamage.integer )
		asave = 0;
	else {
	//qlone - self damages
		// save some from armor
		asave = CheckArmor( targ, take, dflags );
		take -= asave;
	} //qlone - self damages

	if ( g_debugDamage.integer ) {
		G_Printf( "%i: client:%i health:%i damage:%i armor:%i\n", level.time, targ->s.number,
			targ->health, take, asave );
	}

	if ( take > 0 && targ->client && preDamageHealth > 0
		&& p_enablePortal.integer && g_gametype.integer == GT_FFA
		&& level.portalCurrentMinigame == 4 ) {
		int score = targ->client->ps.persistant[PERS_SCORE];
		if ( score > 0 ) {
			int loss;
			if ( take >= preDamageHealth ) {
				loss = score;
			} else {
				loss = ( score * take + preDamageHealth - 1 ) / preDamageHealth;
			}
			if ( loss > score )
				loss = score;
			if ( loss > 0 ) {
				AddScore( targ, targ->r.currentOrigin, -loss );
				G_PortalSpillCoins( targ, loss );
			}
		}
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if ( attacker->client && client && targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_GENERAL) {
#ifdef MISSIONPACK
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->ps.persistant[PERS_HITS]--;
		} else {
			attacker->client->ps.persistant[PERS_HITS]++;
		}
		attacker->client->ps.persistant[PERS_ATTACKEE_ARMOR] = (targ->health<<8)|(client->ps.stats[STAT_ARMOR]);
#else
		// we may hit multiple targets from different teams
		// so usual PERS_HITS increments/decrements could result in ZERO delta
		if ( OnSameTeam( targ, attacker ) ) {
			attacker->client->damage.team++;
		} else {
			attacker->client->damage.enemy++;
			// accumulate damage during server frame
			attacker->client->damage.amount += take + asave;
		}
#endif
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) {
		if ( attacker ) { // FIXME: always true?
			client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		client->damage_armor += asave;
		client->damage_blood += take;
		client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, client->damage_from );
			client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, client->damage_from );
			client->damage_fromWorld = qtrue;
		}
	}

	// See if it's the player hurting the emeny flag carrier
#ifdef MISSIONPACK
	if( g_gametype.integer == GT_CTF || g_gametype.integer == GT_1FCTF ) {
#else
	if( g_gametype.integer == GT_CTF) {
#endif
		Team_CheckHurtCarrier(targ, attacker);
	}

	if (targ->client) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	//qlone - self damages
	if ( targ == attacker && g_noSelfDamage.integer )
		return;
	//qlone - self damages

	// do the damage
	if (take) {
		targ->health = targ->health - take;
		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}

		if ( targ->health <= 0 ) {
			if ( client )
				targ->flags |= FL_NO_KNOCKBACK;

			if (targ->health < -999)
				targ->health = -999;

			targ->enemy = attacker;
			targ->die (targ, inflictor, attacker, take, mod);
			return;
		} else if ( targ->pain ) {
			targ->pain (targ, attacker, take);
		}
	}

}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage( gentity_t *targ, vec3_t origin )
{
	//we check if the attacker can damage the target, return qtrue if yes, qfalse if no
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;
	vec3_t				size;

	// use the midpoint of the bounds instead of the origin, because bmodels may have their origin 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale( midpoint, 0.5, dest );

	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	VectorSubtract( targ->r.absmax, targ->r.absmin, size );

	// top quad

	// - +
	// - -
	VectorCopy( targ->r.absmax, dest );
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	// + -
	// - -
	dest[0] -= size[0];
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 )
		return qtrue;

	// - -
	// + -
	dest[1] -= size[1];
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 )
		return qtrue;

	// - -
	// - +
	dest[0] += size[0];
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 )
		return qtrue;

	// bottom quad

	// - -
	// + -
	VectorCopy( targ->r.absmin, dest );
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	// - -
	// - +
	dest[0] += size[0];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	// - +
	// - -
	dest[1] += size[1];
	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	if (tr.fraction == 1.0)
		return qtrue;

	// + -
	// - -
	dest[0] -= size[0];
	trap_Trace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID );
	if ( tr.fraction == 1.0 )
		return qtrue;

	return qfalse;
}


/*
============
G_RadiusDamage
============
*/
qboolean G_RadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, float radius,
					 gentity_t *ignore, int mod) {
	float		points, dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	qboolean	hitClient = qfalse;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) ) {
			if( LogAccuracyHit( ent, attacker ) ) {
				hitClient = qtrue;
			}
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS, mod);
		}
	}

	return hitClient;
}
