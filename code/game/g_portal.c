/*
===========================================================================
Threewave Portal Voting System

Implements the func_portal entity and map-voting logic driven by the
portal.cfg server configuration file.

Server cvars (set in portal.cfg):
  p_enablePortal    - 1 = portal system active, 0 = disabled
  p_voteSeconds     - seconds after first vote before winner is resolved
  p_randomPortal    - 1 = random tiebreaker among tied winners
  p_gametypeVote    - 1 = respect p_gtPortal## per room
  p_defaultGametype - gametype used when p_gametypeVote is 0 or room has no override
  p_portalMap01 .. p_portalMap64 - map name for each portal slot
  p_gtPortal01 .. p_gtPortal05  - gametype override per room of 8 portals (-1 = default)

Entity: func_portal
  "portal"  <number>   portal number 1-64; selects which p_portalMap## cvar to read
  "target"  <cvarname> informational (e.g. "p_portalMap12"); not read at runtime
  "angle"   <degrees>  button travel direction (-1 = up); drives visual animation
  "lip"     <units>    button stop point; drives visual animation
  "health"  <hp>       if set, portal can also be shot to vote (in addition to touch)
  "wait"    <seconds>  reset delay before button can be re-triggered (default 1 s)

Players vote by walking into (touching) a func_portal brush or by shooting it
(when health > 0). Each player may change their vote at any time before the
timer expires. When p_voteSeconds have elapsed from the first vote, the portal
with the most votes wins and the server loads that map.
===========================================================================
*/

#include "g_local.h"

#define MAX_PORTAL_MAPS     64

static int G_Portal_SelectMinigameForPortal( int portalNum ) {
	int desired;
	int i;

	desired = ( portalNum - 1 ) % 5;

	if ( desired >= 0 && desired < 5
		&& level.portalMinigameEnabled[desired]
		&& level.portalMinigameEnt[desired] )
		return desired;

	for ( i = 0; i < 5; i++ ) {
		if ( level.portalMinigameEnabled[i] && level.portalMinigameEnt[i] )
			return i;
	}

	return -1;
}

/*
================
G_Portal_MapForNum

Looks up the map name for portal number 1..64 by reading the corresponding
p_portalMap## cvar. Writes "" into out if not configured.
================
*/
static void G_Portal_MapForNum( int portalNum, char *out, int outSize ) {
	char cvarName[32];

	out[0] = '\0';

	if ( portalNum < 1 || portalNum > MAX_PORTAL_MAPS )
		return;

	if ( portalNum < 10 )
		Com_sprintf( cvarName, sizeof( cvarName ), "p_portalMap0%i", portalNum );
	else
		Com_sprintf( cvarName, sizeof( cvarName ), "p_portalMap%i", portalNum );

	trap_Cvar_VariableStringBuffer( cvarName, out, outSize );
}

/*
================
G_Portal_RandomPortal

Picks a random configured (and non-disabled) portal number.
Returns the selected portal number, or 0 if none are available.
================
*/
static int G_Portal_RandomPortal( void ) {
	int i, candidates[MAX_PORTAL_MAPS], count = 0;
	char mapname[MAX_QPATH];

	for ( i = 1; i <= MAX_PORTAL_MAPS; i++ ) {
		if ( level.portalDisabled[i - 1] )
			continue;
		G_Portal_MapForNum( i, mapname, sizeof( mapname ) );
		if ( mapname[0] )
			candidates[count++] = i;
	}
	return count > 0 ? candidates[rand() % count] : 0;
}

/*
================
G_Portal_GametypeForNum

Returns the gametype override for a given portal number, derived from the
p_gtPortal0# room cvars. Portals are grouped into rooms of 8:
  portals 1-8  => room 1 => p_gtPortal01
  portals 9-16 => room 2 => p_gtPortal02
  ... up to room 5 (portals 33-40); rooms 6-8 fall back to p_defaultGametype.

Returns -1 (use p_defaultGametype) if p_gametypeVote is disabled, the cvar
is not set, or the value is -1.
================
*/
static int G_Portal_GametypeForNum( int portalNum ) {
	char cvarName[32];
	char val[32];
	int  room;

	if ( !p_gametypeVote.integer )
		return -1;

	room = ((portalNum - 1) / 8) + 1;   // rooms 1-8
	if ( room < 1 || room > 5 )
		return -1;                        // only 5 gt-portal cvars defined

	Com_sprintf( cvarName, sizeof( cvarName ), "p_gtPortal0%i", room );
	trap_Cvar_VariableStringBuffer( cvarName, val, sizeof( val ) );

	return atoi( val );   // -1 means "use default", callers must handle that
}

/*
================
G_Portal_Resolve

Called when the vote timer expires. Counts votes, selects the winner
(respecting p_randomPortal for ties), and issues the map change command.
================
*/
static void G_Portal_Resolve( void ) {
	int  i, bestVotes, candidateCount, bestPortal, gametype;
	int  candidates[MAX_PORTAL_MAPS];
	char mapname[64];

	level.portalVoteTime = 0;

	// Find the highest vote count
	bestVotes = 0;
	for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
		if ( level.portalVotes[i] > bestVotes )
			bestVotes = level.portalVotes[i];
	}

	if ( bestVotes == 0 ) {
		// No votes at all
		if ( p_randomPortal.integer ) {
			// Pick a random portal that has a map configured
			candidateCount = 0;
			for ( i = 1; i <= MAX_PORTAL_MAPS; i++ ) {
				G_Portal_MapForNum( i, mapname, sizeof( mapname ) );
				if ( mapname[0] )
					candidates[candidateCount++] = i;
			}
			bestPortal = ( candidateCount > 0 )
				? candidates[rand() % candidateCount]
				: 1;
		} else {
			bestPortal = 1;
		}
	} else {
		// Collect all portals tied at bestVotes
		candidateCount = 0;
		for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
			if ( level.portalVotes[i] == bestVotes )
				candidates[candidateCount++] = i + 1;   // 1-based portal number
		}

		if ( p_randomPortal.integer && candidateCount > 1 ) {
			bestPortal = candidates[rand() % candidateCount];
		} else {
			bestPortal = candidates[0];   // lowest-numbered winner
		}
	}

	G_Portal_MapForNum( bestPortal, mapname, sizeof( mapname ) );

	if ( !mapname[0] ) {
		G_Printf( "^3Portal: winning portal %i has no map configured\n", bestPortal );
		return;
	}

	gametype = G_Portal_GametypeForNum( bestPortal );
	if ( gametype < 0 )
		gametype = p_defaultGametype.integer;

	G_Printf( "^2Portal: voting done — loading '%s' (gametype %i)\n", mapname, gametype );
	G_BroadcastServerCommand( -1, va( "print \"^2Portal vote: '%s' wins! Loading map...\n\"", mapname ) );

	// Change map: set gametype, load map, then return to portal on nextmap
	trap_SendConsoleCommand( EXEC_APPEND,
		va( "g_gametype %i; map %s; set nextmap \"vstr GotoPortal\"\n", gametype, mapname ) );
}

/*
================
G_Portal_UpdateVoteCS

Re-broadcasts the vote count ("v" key) for one portal slot.
Call after any change to level.portalVotes[].
================
*/
static void G_Portal_UpdateVoteCS( int portalNum ) {
	char info[MAX_INFO_STRING];
	char val[16];

	if ( portalNum < 1 || portalNum > MAX_PORTAL_MAPS )
		return;

	trap_GetConfigstring( CS_PORTALS + portalNum - 1, info, sizeof( info ) );
	Com_sprintf( val, sizeof( val ), "%i", level.portalVotes[portalNum - 1] );
	Info_SetValueForKey( info, "v", val );
	trap_SetConfigstring( CS_PORTALS + portalNum - 1, info );
}

/*
================
G_Portal_Vote

Records or updates a player's vote for the given portal number.
Called from Touch_Portal.
================
*/
void G_Portal_Vote( gentity_t *activator, int portalNum ) {
	int  clientNum, prev = 0;
	char mapname[64];

	if ( !p_enablePortal.integer )
		return;

	if ( portalNum < 1 || portalNum > MAX_PORTAL_MAPS )
		return;

	if ( !activator || !activator->client )
		return;

	// Verify this portal has a map configured
	G_Portal_MapForNum( portalNum, mapname, sizeof( mapname ) );
	if ( !mapname[0] ) {
		trap_SendServerCommand( activator - g_entities,
			va( "print \"^3Portal %i has no map configured.\n\"", portalNum ) );
		return;
	}

	clientNum = activator - g_entities;
	if ( clientNum < 0 || clientNum >= level.maxclients )
		return;

	// Start the countdown on the very first vote
	if ( !level.portalVoteTime ) {
		level.portalVoteTime = level.time;
		G_BroadcastServerCommand( -1,
			va( "print \"^2Portal voting started! %i seconds to vote.\n\"",
				p_voteSeconds.integer ) );
	}

	// Allow vote changes
	if ( level.portalPlayerVoted[clientNum] ) {
		prev = level.portalPlayerVote[clientNum];
		if ( prev == portalNum )
			return;   // voted for the same portal already — no-op

		// Retract previous vote
		if ( prev >= 1 && prev <= MAX_PORTAL_MAPS )
			level.portalVotes[prev - 1]--;
	}

	// Record new vote
	level.portalPlayerVoted[clientNum] = qtrue;
	level.portalPlayerVote[clientNum]  = portalNum;
	level.portalVotes[portalNum - 1]++;

	// Broadcast updated tallies for affected portals
	if ( prev >= 1 && prev <= MAX_PORTAL_MAPS )
		G_Portal_UpdateVoteCS( prev );
	G_Portal_UpdateVoteCS( portalNum );

	trap_SendServerCommand( activator - g_entities,
		va( "print \"^2You voted for: ^3%s\n\"", mapname ) );

	// Broadcast running tally
	{
		int  j, total = 0;
		char msg[512];
		char *p = msg;
		int  remaining;

		for ( j = 0; j < MAX_PORTAL_MAPS; j++ )
			total += level.portalVotes[j];

		p += Com_sprintf( msg, sizeof( msg ), "print \"^3Portal votes (%i):", total );
		for ( j = 0; j < MAX_PORTAL_MAPS; j++ ) {
			if ( level.portalVotes[j] > 0 ) {
				char pmap[64];
				G_Portal_MapForNum( j + 1, pmap, sizeof( pmap ) );
				remaining = sizeof( msg ) - (int)( p - msg );
				if ( remaining > 2 )
					p += Com_sprintf( p, remaining, " %s:%i", pmap, level.portalVotes[j] );
			}
		}
		remaining = sizeof( msg ) - (int)( p - msg );
		if ( remaining > 3 )
			Q_strcat( msg, sizeof( msg ), "\\n\"" );
		G_BroadcastServerCommand( -1, msg );
	}
}

/*
================
G_Portal_Frame

Called every server frame. Fires G_Portal_Resolve() once the vote
deadline has been reached.
================
*/
void G_Portal_Frame( void ) {
	if ( !p_enablePortal.integer )
		return;

	if ( !level.portalVoteTime )
		return;

	if ( level.time - level.portalVoteTime >= p_voteSeconds.integer * 1000 )
		G_Portal_Resolve();
}

/*
================
G_Portal_Init

Resets all portal voting state. Call from G_InitGame so each map starts clean.
================
*/
/*
================
G_Portal_RoomHasPortals

Returns qtrue if any portal in the given room (1-8) has a map configured.
Room N covers portal slots N*8-7 through N*8.
Used by SP_func_static to decide whether to remove a p_blockHall wall.
================
*/
qboolean G_Portal_RoomHasPortals( int roomNum ) {
	int i, firstPortal, lastPortal;
	char mapname[MAX_QPATH];

	firstPortal = roomNum * 8 - 7;
	lastPortal  = roomNum * 8;

	for ( i = firstPortal; i <= lastPortal && i <= MAX_PORTAL_MAPS; i++ ) {
		G_Portal_MapForNum( i, mapname, sizeof( mapname ) );
		if ( mapname[0] )
			return qtrue;
	}
	return qfalse;
}

/*
================
G_Portal_FindMinigames

Caches the five target_teleporter entities named minigame0..minigame4 from
the q3start BSP. Call after G_SpawnEntitiesFromString(). Safe to call on any
map; missing entities just leave their slot NULL.
================
*/
void G_Portal_FindMinigames( void ) {
	int  i;
	char tname[12];
	int  foundCount = 0;

	for ( i = 0; i < 5; i++ ) {
		if ( !level.portalMinigameEnabled[i] ) {
			level.portalMinigameEnt[i] = NULL;
			G_Printf( "Portal: minigame%i disabled by worldspawn\n", i );
			continue;
		}

		Com_sprintf( tname, sizeof( tname ), "minigame%i", i );
		level.portalMinigameEnt[i] = G_Find( NULL, FOFS(targetname), tname );
		if ( level.portalMinigameEnt[i] ) {
			foundCount++;
			G_Printf( "Portal: cached minigame%i -> %s\n", i,
			          level.portalMinigameEnt[i]->classname );
		} else {
			G_Printf( "Portal: minigame%i not found in map\n", i );
		}
	}

	if ( foundCount < 2 ) {
		G_Printf( "There needs to be at least two minigames hardcoded.\n" );
	}
}

void G_Portal_Init( void ) {
	char list[256], *tok;
	int  n;
	int  i;

	memset( level.portalVotes,        0, sizeof( level.portalVotes ) );
	memset( level.portalPlayerVoted,  0, sizeof( level.portalPlayerVoted ) );
	memset( level.portalPlayerVote,   0, sizeof( level.portalPlayerVote ) );
	memset( level.portalEntityMap,    0, sizeof( level.portalEntityMap ) );
	memset( level.portalDisabled,     0, sizeof( level.portalDisabled ) );
	level.portalCurrentMinigame = -1;
	level.portalVoteTime = 0;

	if ( level.portalNumEnabledMinigames <= 0 ) {
		for ( i = 0; i < 5; i++ )
			level.portalMinigameEnabled[i] = qtrue;
		level.portalNumEnabledMinigames = 5;
	}

	// Parse p_disablePortalList: comma-separated portal numbers to exclude from voting
	trap_Cvar_VariableStringBuffer( "p_disablePortalList", list, sizeof( list ) );
	tok = strtok( list, "," );
	while ( tok ) {
		n = atoi( tok );
		if ( n >= 1 && n <= MAX_PORTAL_MAPS )
			level.portalDisabled[n - 1] = qtrue;
		tok = strtok( NULL, "," );
	}

	// Broadcast portal info to clients via configstrings
	{
		int  i;
		char mapname[MAX_QPATH];
		char info[MAX_INFO_STRING];

		for ( i = 1; i <= MAX_PORTAL_MAPS; i++ ) {
			G_Portal_MapForNum( i, mapname, sizeof( mapname ) );
			info[0] = '\0';
			if ( mapname[0] ) {
				Info_SetValueForKey( info, "m", mapname );
				Info_SetValueForKey( info, "e", level.portalDisabled[i - 1] ? "0" : "1" );
			}
			trap_SetConfigstring( CS_PORTALS + i - 1, info );
		}
	}
}

/* -----------------------------------------------------------------------
   func_portal entity
   ----------------------------------------------------------------------- */

/*
================
Touch_Portal

Fires when a player walks into (or through) the func_portal brush.
Records their vote and triggers the button animation for visual feedback.
================
*/
static void Touch_Portal( gentity_t *self, gentity_t *other, trace_t *trace ) {
	int portalNum;

	if ( !other->client )
		return;

	// Portal voting only runs when the portal system is active (hub is GT_FFA)
	if ( !p_enablePortal.integer ) {
		trap_SendServerCommand( other - g_entities,
			"print \"The portal is disabled on this server.\\n\"" );
		return;
	}

	portalNum = self->count;

	if ( portalNum == 0 ) {
		// RandomPortal entity: pick a random configured portal to vote for
		if ( !p_randomPortal.integer )
			return;
		portalNum = G_Portal_RandomPortal();
		if ( portalNum == 0 )
			return;
	}

	// Teleport to mini-game on first vote only; all players share the same room.
	if ( !level.portalPlayerVoted[other - g_entities] ) {
		int        miniGame = G_Portal_SelectMinigameForPortal( portalNum );
		char       mgName[12];
		gentity_t *dest;

		if ( miniGame < 0 ) {
			G_Printf( "Portal: no valid mini-game destination available\n" );
			G_Portal_Vote( other, portalNum );
			return;
		}

		// Lock in the server-wide mini-game on the very first vote.
		if ( level.portalCurrentMinigame < 0 || !level.portalMinigameEnt[level.portalCurrentMinigame] )
			level.portalCurrentMinigame = miniGame;

		Com_sprintf( mgName, sizeof( mgName ), "minigame%i", level.portalCurrentMinigame );
		dest = G_PickTarget( mgName );
		if ( dest ) {
			TeleportPlayer( other, dest->s.origin, dest->s.angles );
		}
	}

	G_Portal_Vote( other, portalNum );
}

/*QUAKED func_portal (0 .5 .8) ?
A Threewave portal entrance.  Players walk into the brush to cast a vote
for the associated map.  When p_voteSeconds have elapsed from the first
vote the portal with the most votes wins.

Keys:
  "portal"       1-64  portal slot number; selects p_portalMap## cvar
  "RandomPortal" 1     marks this brush as the random-pick zone; no "portal" key
*/
void SP_func_portal( gentity_t *ent ) {
	int      portalNum, randomFlag;
	qboolean isValid = qfalse;
	char     mapname[MAX_QPATH];

	G_SpawnInt( "portal", "0", &portalNum );

	if ( portalNum == 0 ) {
		// RandomPortal entity: active when p_randomPortal is set and gametype is portal
		G_SpawnInt( "RandomPortal", "0", &randomFlag );
		if ( randomFlag && p_enablePortal.integer )
			isValid = qtrue;
	} else if ( portalNum >= 1 && portalNum <= MAX_PORTAL_MAPS ) {
		if ( !level.portalDisabled[portalNum - 1] ) {
			G_Portal_MapForNum( portalNum, mapname, sizeof( mapname ) );
			if ( mapname[0] ) {
				level.portalEntityMap[portalNum] = ent;
				isValid = qtrue;
			}
		}
	}

	trap_SetBrushModel( ent, ent->model );

	if ( isValid ) {
		ent->r.contents = CONTENTS_TRIGGER;
		ent->r.svFlags  = SVF_NOCLIENT;
		ent->touch      = Touch_Portal;
		ent->count      = portalNum;
	} else {
		// Not configured or disabled — leave as solid brush, no voting callback
		ent->touch = NULL;
		ent->count = 0;
	}

	trap_LinkEntity( ent );

	/* After linking the bounds are resolved — encode the brush centre into the
	   portal configstring so cgame can show the map label on the crosshair. */
	if ( isValid && portalNum >= 1 && portalNum <= MAX_PORTAL_MAPS ) {
		char   info[MAX_INFO_STRING];
		char   val[16];
		vec3_t center;

		trap_GetConfigstring( CS_PORTALS + portalNum - 1, info, sizeof( info ) );
		center[0] = ( ent->r.absmin[0] + ent->r.absmax[0] ) * 0.5f;
		center[1] = ( ent->r.absmin[1] + ent->r.absmax[1] ) * 0.5f;
		center[2] = ( ent->r.absmin[2] + ent->r.absmax[2] ) * 0.5f;
		Com_sprintf( val, sizeof( val ), "%i", (int)center[0] );
		Info_SetValueForKey( info, "px", val );
		Com_sprintf( val, sizeof( val ), "%i", (int)center[1] );
		Info_SetValueForKey( info, "py", val );
		Com_sprintf( val, sizeof( val ), "%i", (int)center[2] );
		Info_SetValueForKey( info, "pz", val );
		trap_SetConfigstring( CS_PORTALS + portalNum - 1, info );
	}
}
