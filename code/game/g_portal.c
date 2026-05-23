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
#define PORTAL_TRANSITION_MS 5000

static int G_Portal_RandomPortal( void );
static void G_Portal_MapForNum( int portalNum, char *out, int outSize );
static int s_portalDisabledWarnTime[MAX_CLIENTS];
static qboolean s_portalReliableTextEnabled = qtrue;

static void G_Portal_ExpireAllSillyQuads( void ) {
	int i;

	for ( i = 0; i < level.maxclients; i++ ) {
		gentity_t *ent = &g_entities[i];
		if ( !ent->inuse || !ent->client )
			continue;
		if ( ent->client->pers.connected != CON_CONNECTED )
			continue;

		ent->client->ps.powerups[PW_QUAD] = 0;
		ent->s.powerups &= ~( 1 << PW_QUAD );
	}
}

static void G_Portal_SendClientText( int clientNum, const char *text ) {
	if ( !s_portalReliableTextEnabled )
		return;
	trap_SendServerCommand( clientNum, text );
}

static void G_Portal_BroadcastText( const char *text ) {
	if ( !s_portalReliableTextEnabled )
		return;
	G_BroadcastServerCommand( -1, text );
}

static void G_Portal_BroadcastVoteTallies( void ) {
	char msg[1024];
	char chunk[80];
	int i;
	int totalVotes = 0;

	if ( !s_portalReliableTextEnabled )
		return;

	for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
		totalVotes += level.portalVotes[i];
	}

	if ( totalVotes <= 0 )
		return;

	Com_sprintf( msg, sizeof( msg ), "print \"Portal votes (%i): ", totalVotes );

	for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
		char mapname[MAX_QPATH];
		int count = level.portalVotes[i];

		if ( count <= 0 )
			continue;

		G_Portal_MapForNum( i + 1, mapname, sizeof( mapname ) );
		if ( !mapname[0] )
			continue;

		Com_sprintf( chunk, sizeof( chunk ), "%s:%i ", mapname, count );
		if ( strlen( msg ) + strlen( chunk ) + 4 >= sizeof( msg ) )
			break;
		Q_strcat( msg, sizeof( msg ), chunk );
	}

	Q_strcat( msg, sizeof( msg ), "\\n\"" );
	G_Portal_BroadcastText( msg );
}

static void G_Portal_BeginMapTransition( const char *mapname, int gametype ) {
	int i;
	vec3_t zero = { 0, 0, 0 };

	if ( !mapname || !mapname[0] )
		return;

	Q_strncpyz( level.portalNextMap, mapname, sizeof( level.portalNextMap ) );
	level.portalNextGametype = gametype;
	level.portalMapChangeTime = level.time + PORTAL_TRANSITION_MS;
	trap_SetConfigstring( CS_PORTAL_TRANSITION, va( "%i", level.portalMapChangeTime ) );

	// Mimic Threewave election output while entering the short portal transition.
	G_Portal_ExpireAllSillyQuads();
	G_Portal_BroadcastText( va( "print \"%s has won the election!\n\"", mapname ) );
	G_Portal_BroadcastText( "print \"Map voting complete\n\"" );
	G_Portal_BroadcastText( va( "print \"Map: %s\n\"", g_mapname.string ) );
	G_ATDGlobalSound( "sound/misc/gong.wav" );

	// Return everyone to q3start's spectator/intermission camera context so the
	// transition view matches the original portal map behavior.
	level.portalCurrentMinigame = -1;
	level.portalVoteTime = 0;
	FindIntermissionPoint();

	// Push a fresh scoreboard snapshot so clients can immediately render scores.
	for ( i = 0; i < level.maxclients; i++ ) {
		gentity_t *ent = &g_entities[i];
		if ( !ent->inuse || !ent->client )
			continue;
		if ( ent->client->pers.connected != CON_CONNECTED )
			continue;

		VectorCopy( level.intermission_origin, ent->s.origin );
		VectorCopy( level.intermission_origin, ent->client->ps.origin );
		VectorCopy( zero, ent->client->ps.velocity );
		SetClientViewAngle( ent, level.intermission_angle );

		ent->client->ps.pm_type = PM_SPECTATOR;
		ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
		ent->client->ps.pm_flags &= ~PMF_FOLLOW;
		DeathmatchScoreboardMessage( ent );
	}
}

static void G_Portal_PersistDisabledPortal( int portalNum ) {
	int i;

	if ( portalNum < 1 || portalNum > MAX_PORTAL_MAPS )
		return;

	for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
		level.portalDisabled[i] = qfalse;
	}
 	level.portalDisabled[portalNum - 1] = qtrue;

	trap_Cvar_Set( "p_disablePortalList", va( "%i", portalNum ) );
	trap_Cvar_Set( "p_disablePortalTemp", va( "%i", portalNum ) );
}

static void G_Portal_MarkPortalDisabledInCS( int portalNum ) {
	int i;
	char info[MAX_INFO_STRING];
	const char *currentEnabled;
	const char *desiredEnabled;

	if ( portalNum < 0 || portalNum > MAX_PORTAL_MAPS )
		return;

	for ( i = 1; i <= MAX_PORTAL_MAPS; i++ ) {
		trap_GetConfigstring( CS_PORTALS + i - 1, info, sizeof( info ) );
		if ( !info[0] )
			continue;

		desiredEnabled = ( portalNum > 0 && i == portalNum ) ? "0" : "1";
		currentEnabled = Info_ValueForKey( info, "e" );
		if ( currentEnabled && !Q_stricmp( currentEnabled, desiredEnabled ) )
			continue;

		Info_SetValueForKey( info, "e", desiredEnabled );
		trap_SetConfigstring( CS_PORTALS + i - 1, info );
	}
}

static void G_Portal_SyncDisabledState( void ) {
	int i;
	int desired = 0;
	char rememberStr[32];
	char disabledListStr[32];
	char tempStr[32];
	qboolean needsUpdate = qfalse;

	trap_Cvar_VariableStringBuffer( "p_disablePortalTemp", rememberStr, sizeof( rememberStr ) );
	desired = atoi( rememberStr );
	if ( desired < 1 || desired > MAX_PORTAL_MAPS ) {
		desired = 0;
	}

	for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
		qboolean want = ( desired > 0 && i == desired - 1 );
		if ( level.portalDisabled[i] != want ) {
			needsUpdate = qtrue;
			break;
		}
	}

	if ( desired > 0 ) {
		trap_Cvar_VariableStringBuffer( "p_disablePortalList", disabledListStr, sizeof( disabledListStr ) );
		trap_Cvar_VariableStringBuffer( "p_disablePortalTemp", tempStr, sizeof( tempStr ) );
		if ( atoi( disabledListStr ) != desired || atoi( tempStr ) != desired )
			needsUpdate = qtrue;
	} else {
		trap_Cvar_VariableStringBuffer( "p_disablePortalList", disabledListStr, sizeof( disabledListStr ) );
		trap_Cvar_VariableStringBuffer( "p_disablePortalTemp", tempStr, sizeof( tempStr ) );
		if ( disabledListStr[0] || atoi( tempStr ) > 0 )
			needsUpdate = qtrue;
	}

	if ( !needsUpdate )
		return;

	for ( i = 0; i < MAX_PORTAL_MAPS; i++ ) {
		level.portalDisabled[i] = ( desired > 0 && i == desired - 1 );
	}

	if ( desired > 0 ) {
		trap_Cvar_Set( "p_disablePortalList", va( "%i", desired ) );
		trap_Cvar_Set( "p_disablePortalTemp", va( "%i", desired ) );
	} else {
		trap_Cvar_Set( "p_disablePortalList", "" );
		trap_Cvar_Set( "p_disablePortalTemp", "-1" );
	}

	G_Portal_MarkPortalDisabledInCS( desired );
}

qboolean G_PortalMiniGameRulesEnabled( void ) {
	return ( p_enablePortal.integer
		&& g_gametype.integer == GT_FFA
		&& level.portalCurrentMinigame >= 0
		&& level.portalCurrentMinigame < 5 );
}

qboolean G_PortalLobbyRulesEnabled( void ) {
	return ( p_enablePortal.integer
		&& g_gametype.integer == GT_FFA
		&& level.portalCurrentMinigame < 0 );
}

int G_PortalCurrentMiniGame( void ) {
	if ( !G_PortalMiniGameRulesEnabled() )
		return -1;
	return level.portalCurrentMinigame;
}

qboolean G_PortalIsWeaponAllowed( int miniGame, int weapon ) {
	switch ( miniGame ) {
		case 0:
			return weapon == WP_GAUNTLET;
		case 1:
			return weapon == WP_GAUNTLET || weapon == WP_RAILGUN || weapon == WP_SHOTGUN;
		case 2:
			return weapon == WP_RAILGUN;
		case 3:
			return weapon == WP_GAUNTLET || weapon == WP_GRENADE_LAUNCHER;
		case 4:
			return weapon == WP_GAUNTLET || weapon == WP_SHOTGUN
				|| weapon == WP_ROCKET_LAUNCHER || weapon == WP_RAILGUN;
		default:
			return qtrue;
	}
}

void G_PortalApplyMiniGameLoadout( gentity_t *ent ) {
	int i;
	int miniGame;

	if ( !ent || !ent->client )
		return;

	miniGame = G_PortalCurrentMiniGame();
	if ( miniGame < 0 )
		return;

	ent->client->ps.stats[STAT_WEAPONS] = 0;
	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		ent->client->ps.ammo[i] = 0;
	}
	ent->client->ps.ammo[WP_GRAPPLING_HOOK] = -1;
	ent->client->ps.powerups[PW_SPAWNPROTECTION] = 0;
	ent->client->ps.powerups[PW_REGEN] = 0;
	ent->client->ps.stats[STAT_ARMOR] = 0;

	switch ( miniGame ) {
		case 0:
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET );
			ent->client->ps.ammo[WP_GAUNTLET] = -1;
			ent->client->ps.weapon = WP_GAUNTLET;
			break;
		case 1:
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET ) | ( 1 << WP_RAILGUN ) | ( 1 << WP_SHOTGUN );
			ent->client->ps.ammo[WP_GAUNTLET] = -1;
			ent->client->ps.ammo[WP_RAILGUN] = AMMO_HARD_LIMIT;
			ent->client->ps.ammo[WP_SHOTGUN] = AMMO_HARD_LIMIT;
			ent->client->ps.weapon = WP_RAILGUN;
			break;
		case 2:
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_RAILGUN );
			ent->client->ps.ammo[WP_RAILGUN] = AMMO_HARD_LIMIT;
			ent->client->ps.weapon = WP_RAILGUN;
			break;
		case 3:
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET );
			ent->client->ps.ammo[WP_GAUNTLET] = -1;
			ent->client->ps.weapon = WP_GAUNTLET;
			break;
		case 4:
			ent->client->ps.stats[STAT_WEAPONS] |= ( 1 << WP_GAUNTLET ) | ( 1 << WP_SHOTGUN )
				| ( 1 << WP_ROCKET_LAUNCHER ) | ( 1 << WP_RAILGUN );
			ent->client->ps.ammo[WP_GAUNTLET] = -1;
			ent->client->ps.ammo[WP_SHOTGUN] = 50;
			ent->client->ps.ammo[WP_ROCKET_LAUNCHER] = 50;
			ent->client->ps.ammo[WP_RAILGUN] = 50;
			ent->client->ps.powerups[PW_REGEN] = 0x7fffffff;
			ent->client->ps.weapon = WP_ROCKET_LAUNCHER;
			break;
		default:
			break;
	}
}

void G_PortalApplyLobbyLoadout( gentity_t *ent ) {
	int i;

	if ( !ent || !ent->client )
		return;

	if ( !G_PortalLobbyRulesEnabled() )
		return;

	ent->client->ps.stats[STAT_WEAPONS] = ( 1 << WP_GAUNTLET );
	for ( i = 0; i < MAX_WEAPONS; i++ ) {
		ent->client->ps.ammo[i] = 0;
	}
	ent->client->ps.ammo[WP_GAUNTLET] = -1;
	ent->client->ps.ammo[WP_GRAPPLING_HOOK] = -1;
	ent->client->ps.powerups[PW_SPAWNPROTECTION] = 0;
	ent->client->ps.powerups[PW_REGEN] = 0;
	ent->client->ps.stats[STAT_ARMOR] = 0;
	ent->client->ps.weapon = WP_GAUNTLET;
}

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

qboolean G_Portal_HandleVoteTouch( gentity_t *activator, int portalNum ) {
	if ( !activator || !activator->client )
		return qfalse;

	// Portal voting only runs when the portal system is active (hub is GT_FFA)
	if ( !p_enablePortal.integer ) {
		G_Portal_SendClientText( activator - g_entities,
			"print \"The portal is disabled on this server.\\n\"" );
		return qfalse;
	}

	if ( portalNum == 0 ) {
		// RandomPortal touch: pick a fresh random configured portal every touch
		if ( !p_randomPortal.integer )
			return qfalse;
		portalNum = G_Portal_RandomPortal();
		if ( portalNum == 0 )
			return qfalse;
	}

	if ( portalNum >= 1 && portalNum <= MAX_PORTAL_MAPS && level.portalDisabled[portalNum - 1] ) {
		int clientNum = activator - g_entities;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			if ( level.time >= s_portalDisabledWarnTime[clientNum] ) {
				s_portalDisabledWarnTime[clientNum] = level.time + 1000;
				G_Portal_SendClientText( clientNum,
					"print \"This portal is disabled.\\n\"" );
			}
		}
		return qfalse;
	}

	{
		int        miniGame;
		char       mgName[12];
		gentity_t *dest;

		miniGame = level.portalCurrentMinigame;
		if ( miniGame < 0 || miniGame >= 5 || !level.portalMinigameEnt[miniGame] ) {
			miniGame = G_Portal_SelectMinigameForPortal( portalNum );
			if ( miniGame < 0 ) {
				G_Printf( "Portal: no valid mini-game destination available\n" );
				G_Portal_Vote( activator, portalNum );
				return qtrue;
			}
			level.portalCurrentMinigame = miniGame;
			G_PortalSillyQuadSetupForMiniGame( level.portalCurrentMinigame );
		}

		Com_sprintf( mgName, sizeof( mgName ), "minigame%i", level.portalCurrentMinigame );
		dest = G_PickTarget( mgName );
		if ( dest ) {
			G_PortalApplyMiniGameLoadout( activator );
			TeleportPlayer( activator, dest->s.origin, dest->s.angles );
		}
	}

	G_Portal_Vote( activator, portalNum );
	return qtrue;
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

	// Disable the winning portal for subsequent returns to the hub (same visual/logic path as disabled slots).
	G_Portal_PersistDisabledPortal( bestPortal );
	G_Portal_MarkPortalDisabledInCS( bestPortal );

	G_Printf( "^2Portal: voting done — winner '%s' (gametype %i), transition starts\n", mapname, gametype );
	G_Portal_BeginMapTransition( mapname, gametype );
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
		G_Portal_SendClientText( activator - g_entities,
			va( "print \"^3Portal %i has no map configured.\n\"", portalNum ) );
		return;
	}

	clientNum = activator - g_entities;
	if ( clientNum < 0 || clientNum >= level.maxclients )
		return;

	// Start the countdown on the very first vote
	if ( !level.portalVoteTime ) {
		level.portalVoteTime = level.time;
		G_Portal_ExpireAllSillyQuads();
		G_Portal_BroadcastText(
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
	G_Portal_BroadcastVoteTallies();

	G_Portal_SendClientText( activator - g_entities,
		va( "print \"^2You voted for: ^3%s\n\"", mapname ) );
}

/*
================
G_Portal_Frame

Called every server frame. Fires G_Portal_Resolve() once the vote
deadline has been reached.
================
*/
void G_Portal_Frame( void ) {
	int i;

	if ( !p_enablePortal.integer )
		return;

	G_Portal_SyncDisabledState();

	if ( level.portalMapChangeTime ) {
		// Hold clients in free spectator view during the short transition.
		for ( i = 0; i < level.maxclients; i++ ) {
			gentity_t *ent = &g_entities[i];
			if ( !ent->inuse || !ent->client )
				continue;
			if ( ent->client->pers.connected != CON_CONNECTED )
				continue;
			ent->client->ps.pm_type = PM_SPECTATOR;
			ent->client->ps.pm_flags &= ~PMF_SCOREBOARD;
		}

		if ( level.time >= level.portalMapChangeTime ) {
			int nextGT = level.portalNextGametype;
			char nextMap[MAX_QPATH];

			Q_strncpyz( nextMap, level.portalNextMap, sizeof( nextMap ) );
			if ( nextGT < 0 )
				nextGT = p_defaultGametype.integer;

			level.portalMapChangeTime = 0;
			level.portalNextGametype = -1;
			level.portalNextMap[0] = '\0';
			trap_SetConfigstring( CS_PORTAL_TRANSITION, "0" );

			if ( nextMap[0] ) {
				trap_SendConsoleCommand( EXEC_APPEND,
					va( "g_gametype %i; map %s; set nextmap \"vstr GotoPortal\"\n", nextGT, nextMap ) );
			}
		}

		return;
	}

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
	int  i;

	memset( level.portalVotes,        0, sizeof( level.portalVotes ) );
	memset( level.portalPlayerVoted,  0, sizeof( level.portalPlayerVoted ) );
	memset( level.portalPlayerVote,   0, sizeof( level.portalPlayerVote ) );
	memset( level.portalEntityMap,    0, sizeof( level.portalEntityMap ) );
	memset( level.portalDisabled,     0, sizeof( level.portalDisabled ) );
	memset( s_portalDisabledWarnTime, 0, sizeof( s_portalDisabledWarnTime ) );
	level.portalCurrentMinigame = -1;
	level.portalVoteTime = 0;
	level.portalMapChangeTime = 0;
	level.portalNextGametype = -1;
	level.portalNextMap[0] = '\0';
	trap_SetConfigstring( CS_PORTAL_TRANSITION, "0" );

	if ( level.portalNumEnabledMinigames <= 0 ) {
		for ( i = 0; i < 5; i++ )
			level.portalMinigameEnabled[i] = qtrue;
		level.portalNumEnabledMinigames = 5;
	}

	G_Portal_SyncDisabledState();

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
	G_Portal_HandleVoteTouch( other, self->count );
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
		// RandomPortal entity is identified by key or target name; cvar gating is done at touch-time.
		G_SpawnInt( "RandomPortal", "0", &randomFlag );
		if ( !randomFlag && ent->target && !Q_stricmp( ent->target, "p_randomPortal" ) )
			randomFlag = 1;
		if ( randomFlag )
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
