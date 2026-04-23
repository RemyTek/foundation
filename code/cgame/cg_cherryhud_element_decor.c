#include "cg_cherryhud_private.h"
#include "cg_local.h"

typedef struct
{
	cherryhudDrawContext_t drawCtx;
	cherryhudTextContext_t textCtx;
	cherryhudConfig_t config;
	char displayText[MAX_QPATH];
	char infoType[MAX_QPATH]; /* "player.score", "game.map", "weapon.railgun.accuracy" */
	int clientNum;
	int weaponType; /* WP_RAILGUN, WP_LIGHTNING, etc. */
	int flags; /* Bit flags for element state */
	
	// Информация о контейнере
	const char* containerType;         // "!scoreboard", "playersRows", "spectatorsRows"
} cherryhudDecorElement_t;

/* Bit flags for cherryhudDecorElement_t.flags */
#define CHUD_DECOR_HAS_MANUAL_TEXT    (1 << 0)
#define CHUD_DECOR_HAS_MANUAL_IMAGE   (1 << 1)
#define CHUD_DECOR_HAS_EMPTY_TEXT     (1 << 2)
#define CHUD_DECOR_HAS_EMPTY_IMAGE    (1 << 3)

/* Render mode flags */
#define CHUD_DECOR_RENDER_TEXT        (1 << 4)
#define CHUD_DECOR_RENDER_IMAGE       (1 << 5)
#define CHUD_DECOR_RENDER_DYNAMIC     (1 << 6)
#define CHUD_DECOR_BOTH_SPECIFIED     (1 << 7)

/* Content source flags - set once at creation */
#define CHUD_DECOR_CONTENT_INITIALIZED (1 << 8)
#define CHUD_DECOR_USE_AUTO_IMAGE      (1 << 9)
#define CHUD_DECOR_USE_AUTO_TEXT       (1 << 10)
#define CHUD_DECOR_USE_DYNAMIC_IMAGE   (1 << 11)

// Player type handler structure
typedef struct {
    const char* typeName;
    void (*handler)(cherryhudDecorElement_t* element);
    qboolean hasAutoIcon;  // Whether this type has an automatic icon
} cherryhudPlayerTypeHandler_t;

// Game type handler structure
typedef struct {
    const char* typeName;
    void (*handler)(cherryhudDecorElement_t* element);
} cherryhudGameTypeHandler_t;

// Weapon stat handler structure
typedef struct {
    const char* statName;
    void (*handler)(cherryhudDecorElement_t* element);
} cherryhudWeaponStatHandler_t;

// Forward declarations for type handlers
static void CG_CHUDDecorGetPlayerName(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerScore(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerPing(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerDeaths(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerKD(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerDamageGiven(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerDamageReceived(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerDamageRatio(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerId(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerTime(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerThaws(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerXid(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerSkill(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetPlayerHead(cherryhudDecorElement_t* element);

static void CG_CHUDDecorGetGameServerName(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetGameMapName(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetGameType(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetGameTimeLimit(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetGameFragLimit(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetGameCaptureLimit(cherryhudDecorElement_t* element);

static void CG_CHUDDecorGetWeaponKills(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetWeaponAccuracy(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetWeaponShots(cherryhudDecorElement_t* element);
static void CG_CHUDDecorGetWeaponIcon(cherryhudDecorElement_t* element);

static void CG_CHUDDecorGetUnknownType(cherryhudDecorElement_t* element);
static qhandle_t CG_CHUDDecorGetAutoImage(cherryhudDecorElement_t* element);
static int CG_CHUDDecorGetWeaponTypeFromString(const char* weaponName);
static void CG_CHUDDecorProcessType(cherryhudDecorElement_t* element);
static void CG_CHUDDecorProcessPlayerType(cherryhudDecorElement_t* element);
static void CG_CHUDDecorProcessGameType(cherryhudDecorElement_t* element);
static void CG_CHUDDecorProcessWeaponType(cherryhudDecorElement_t* element);
static void CG_CHUDDecorInitializeContent(cherryhudDecorElement_t* element);
static void CG_CHUDDecorApplyFormatting(cherryhudDecorElement_t* element, float value);

// Player type handler table
// Each entry defines: type name, handler function, and whether it has an automatic icon
// hasAutoIcon = qtrue: The type can display both text and icon (e.g., player.ping shows "50ms" + signal icon)
// hasAutoIcon = qfalse: The type shows only text or handles its own icon (e.g., player.skill shows only skill icon)
static const cherryhudPlayerTypeHandler_t playerTypeHandlers[] = {
    {"name", CG_CHUDDecorGetPlayerName, qfalse},           // Text only
    {"score", CG_CHUDDecorGetPlayerScore, qtrue},          // Text + icon
    {"ping", CG_CHUDDecorGetPlayerPing, qtrue},            // Text + icon
    {"deaths", CG_CHUDDecorGetPlayerDeaths, qtrue},        // Text + icon
    {"kd", CG_CHUDDecorGetPlayerKD, qtrue},                // Text + icon
    {"damageGiven", CG_CHUDDecorGetPlayerDamageGiven, qtrue}, // Text + icon
    {"damageReceived", CG_CHUDDecorGetPlayerDamageReceived, qtrue}, // Text + icon
    {"damageTaken", CG_CHUDDecorGetPlayerDamageReceived, qtrue}, // Alias for damageReceived
    {"damageRatio", CG_CHUDDecorGetPlayerDamageRatio, qfalse}, // Text only
    {"id", CG_CHUDDecorGetPlayerId, qtrue},                // Text + icon
    {"time", CG_CHUDDecorGetPlayerTime, qtrue},            // Text + icon
    {"thaws", CG_CHUDDecorGetPlayerThaws, qtrue},          // Text + icon
    {"xid", CG_CHUDDecorGetPlayerXid, qfalse},             // Text only
    {"skill", CG_CHUDDecorGetPlayerSkill, qfalse},         // Icon only (handled in handler)
    {"head", CG_CHUDDecorGetPlayerHead, qfalse}            // Special case (3D head)
};

#define PLAYER_TYPE_HANDLER_COUNT (sizeof(playerTypeHandlers) / sizeof(playerTypeHandlers[0]))

// Game type handler table
static const cherryhudGameTypeHandler_t gameTypeHandlers[] = {
    {"serverName", CG_CHUDDecorGetGameServerName},
    {"map", CG_CHUDDecorGetGameMapName},
    {"gameType", CG_CHUDDecorGetGameType},
    {"gametype", CG_CHUDDecorGetGameType}, // Alias
    {"timelimit", CG_CHUDDecorGetGameTimeLimit},
    {"timeLimit", CG_CHUDDecorGetGameTimeLimit}, // Alias
    {"fraglimit", CG_CHUDDecorGetGameFragLimit},
    {"fragLimit", CG_CHUDDecorGetGameFragLimit}, // Alias
    {"capturelimit", CG_CHUDDecorGetGameCaptureLimit},
    {"captureLimit", CG_CHUDDecorGetGameCaptureLimit} // Alias
};

#define GAME_TYPE_HANDLER_COUNT (sizeof(gameTypeHandlers) / sizeof(gameTypeHandlers[0]))

// Weapon stat handler table
static const cherryhudWeaponStatHandler_t weaponStatHandlers[] = {
    {"kills", CG_CHUDDecorGetWeaponKills},
    {"accuracy", CG_CHUDDecorGetWeaponAccuracy},
    {"shots", CG_CHUDDecorGetWeaponShots},
    {"icon", CG_CHUDDecorGetWeaponIcon}
};

#define WEAPON_STAT_HANDLER_COUNT (sizeof(weaponStatHandlers) / sizeof(weaponStatHandlers[0]))

// Helper function to find player type handler
static void (*CG_CHUDDecorFindPlayerTypeHandler(const char* typeName))(cherryhudDecorElement_t*) {
    int i;
    for (i = 0; i < PLAYER_TYPE_HANDLER_COUNT; i++) {
        if (Q_stricmp(playerTypeHandlers[i].typeName, typeName) == 0) {
            return playerTypeHandlers[i].handler;
        }
    }
    return NULL;
}

// Helper function to check if player type has auto icon
static qboolean CG_CHUDDecorPlayerTypeHasAutoIcon(const char* typeName) {
    int i;
    for (i = 0; i < PLAYER_TYPE_HANDLER_COUNT; i++) {
        if (Q_stricmp(playerTypeHandlers[i].typeName, typeName) == 0) {
            return playerTypeHandlers[i].hasAutoIcon;
        }
    }
    return qfalse;
}

// Helper function to find game type handler
static void (*CG_CHUDDecorFindGameTypeHandler(const char* typeName))(cherryhudDecorElement_t*) {
    int i;
    for (i = 0; i < GAME_TYPE_HANDLER_COUNT; i++) {
        if (Q_stricmp(gameTypeHandlers[i].typeName, typeName) == 0) {
            return gameTypeHandlers[i].handler;
        }
    }
    return NULL;
}

// Helper function to find weapon stat handler
static void (*CG_CHUDDecorFindWeaponStatHandler(const char* statName))(cherryhudDecorElement_t*) {
    int i;
    for (i = 0; i < WEAPON_STAT_HANDLER_COUNT; i++) {
        if (Q_stricmp(weaponStatHandlers[i].statName, statName) == 0) {
            return weaponStatHandlers[i].handler;
        }
    }
    return NULL;
}

void* CG_CHUDElementDecorCreate(const cherryhudConfig_t* config, const char* containerType)
{
	cherryhudDecorElement_t* element;
	
	
	element = (cherryhudDecorElement_t*)CG_CHUDCreateElementBase(config, sizeof(cherryhudDecorElement_t));
	if (!element) {
		return NULL;
	}
	
	memcpy(&element->config, config, sizeof(element->config));
	
	// Initialize new fields
	Q_strncpyz(element->displayText, "", sizeof(element->displayText));
	Q_strncpyz(element->infoType, "", sizeof(element->infoType));
	element->clientNum = (cg.snap && cg.snap->ps.clientNum >= 0) ? cg.snap->ps.clientNum : 0;
	element->weaponType = WP_GAUNTLET; // Default
	element->containerType = containerType; // Set container type
	
	// Set flags once at creation to avoid checking every frame
	element->flags = 0;
	if (config->text.isSet) {
		element->flags |= CHUD_DECOR_HAS_MANUAL_TEXT;
		if (config->text.value[0] == '\0') {
			element->flags |= CHUD_DECOR_HAS_EMPTY_TEXT;
		}
	}
	if (config->image.isSet) {
		element->flags |= CHUD_DECOR_HAS_MANUAL_IMAGE;
		if (config->image.value[0] == '\0') {
			element->flags |= CHUD_DECOR_HAS_EMPTY_IMAGE;
		}
	}
	
	// Determine render mode once at creation
	if (element->flags & CHUD_DECOR_HAS_MANUAL_TEXT && element->flags & CHUD_DECOR_HAS_MANUAL_IMAGE) {
		// Both text and image specified - render both
		element->flags |= CHUD_DECOR_BOTH_SPECIFIED;
	} else if (element->flags & CHUD_DECOR_HAS_MANUAL_TEXT) {
		// Only text specified
		element->flags |= CHUD_DECOR_RENDER_TEXT;
	} else if (element->flags & CHUD_DECOR_HAS_MANUAL_IMAGE) {
		// Only image specified
		element->flags |= CHUD_DECOR_RENDER_IMAGE;
	} else {
		// No manual content - generate based on type
		element->flags |= CHUD_DECOR_RENDER_DYNAMIC;
	}
	
	// Parse type from config
	if (config->type.isSet) {
		Q_strncpyz(element->infoType, config->type.value, sizeof(element->infoType));
	} else {
		Q_strncpyz(element->infoType, "player.name", sizeof(element->infoType)); // Default
	}
	
	// Parse weapon type if it's a weapon type
	if (Q_strncmp(element->infoType, "weapon.", 7) == 0) {
		char buffer[MAX_QPATH];
		char* weaponName;
		char* dotPos;
		
		/* Copy infoType to buffer to avoid modifying the original */
		Q_strncpyz(buffer, element->infoType, sizeof(buffer));
		weaponName = buffer + 7; /* Skip "weapon." */
		dotPos = strchr(weaponName, '.');
		
		if (dotPos) {
			*dotPos = '\0'; /* Null terminate weapon name in buffer */
			element->weaponType = CG_CHUDDecorGetWeaponTypeFromString(weaponName);
		}
	}
	
	// Initialize contexts
	CG_CHUDTextMakeContext(config, &element->textCtx);
	
	// Initialize draw context if image is set or if this might be a dynamic image type
	if (config->image.isSet || (config->type.isSet && Q_stricmp(config->type.value, "player.skill") == 0)) {
        CG_CHUDDrawMakeContext(config, &element->drawCtx);
		if (config->image.isSet) {
			element->drawCtx.image = trap_R_RegisterShader(config->image.value);
			if (!element->drawCtx.image) {
				element->drawCtx.image = cgs.media.whiteShader;
			}
		}
	}
	
	
	if (config->text.isSet) {
		Q_strncpyz(element->displayText, config->text.value, sizeof(element->displayText));
		element->textCtx.text = element->displayText;
		// CG_CHUDFillAndFrameForText((cherryhudConfig_t*)config, &element->textCtx); // Текст0контекст заполняется правильно из конфига.
	}
	
	// Initialize content source once at creation
	CG_CHUDDecorInitializeContent(element);
	
	return element;
}

void CG_CHUDElementDecorRoutine(void* context)
{
	cherryhudDecorElement_t* element;
	const cherryhudConfig_t* config;
	cherryhudLayoutBounds_t* bounds;
	int clientNum;
	
	element = (cherryhudDecorElement_t*)context;
	config = &element->config;
	
	if (!CG_CHUDValidateElementAndConfig(element, config)) {
		return;
	}
	
	bounds = CG_CHUDLayoutGetBoundsByContext(context);
	if (!CG_CHUDValidateBounds(bounds)) {
		return;
	}

	CG_CHUDSetDrawContextFromBounds(&element->drawCtx, bounds);
	
    CG_CHUDSetTextContextFromBounds(&element->textCtx, bounds);
	
	// Get client number from bounds for player types
	clientNum = bounds->clientNum;
	element->clientNum = clientNum;
	
	// Use common validation and visibility checking
	CG_CHUDRenderElementWithValidation(element, config, bounds, clientNum);
	
	// Update dynamic content
	if (element->flags & CHUD_DECOR_USE_AUTO_TEXT) {
		CG_CHUDDecorProcessType(element);
	}
	
	// Update dynamic images
	if (element->flags & CHUD_DECOR_USE_DYNAMIC_IMAGE) {
		CG_CHUDDecorProcessType(element);
	}
	
	// Draw element background and border using common utility
	CG_CHUDRenderElementBackgroundAndBorder(config, bounds);
	
	// Special handling for player.head - render 3D head
	if (Q_stricmp(element->infoType, "player.head") == 0) {
		// Render 3D head using CG_DrawHead
		vec3_t headAngles = {0, 0, 0};
		
		// Draw border if configured
		if (config->border.isSet) {
			vec4_t borderColor;
			CG_CHUDConfigPickBorderColor(config, borderColor, qfalse);
			CG_CHUDDrawBorderCentralized(bounds->x, bounds->y, bounds->width, bounds->height,
			                             config->border.sideSizes, borderColor);
		}

		CG_DrawHead(bounds->x, bounds->y, bounds->width, bounds->height, clientNum, headAngles);

	} else if (element->drawCtx.image) {
		// Render regular image
		CG_CHUDDrawStretchPicCtx(config, &element->drawCtx);
	}

	// Render text if available
	if (element->textCtx.text && element->textCtx.text[0] != '\0') {
		CG_CHUDRenderElementText(config, &element->textCtx, element->textCtx.text);
    }
	
	// CG_CHUDDrawBorder(config); // fix me later. Need to use border/background for whole ELEMENT
}

void CG_CHUDElementDecorDestroy(void* context)
{
	CG_CHUDDestroyElementBase(context);
}

void CG_CHUDElementDecorSetClientNum(void* context, int clientNum)
{
	cherryhudDecorElement_t* element;
	
	element = (cherryhudDecorElement_t*)context;
	if (!element) return;
	
	element->clientNum = clientNum;
}

// Main type processing function
static void CG_CHUDDecorProcessType(cherryhudDecorElement_t* element)
{
	if (Q_strncmp(element->infoType, "player.", 7) == 0) {
		CG_CHUDDecorProcessPlayerType(element);
	} else if (Q_strncmp(element->infoType, "game.", 5) == 0) {
		CG_CHUDDecorProcessGameType(element);
	} else if (Q_strncmp(element->infoType, "weapon.", 7) == 0) {
		CG_CHUDDecorProcessWeaponType(element);
	} else if (Q_stricmp(element->infoType, "serverName") == 0 ||
	           Q_stricmp(element->infoType, "mapName") == 0 ||
	           Q_stricmp(element->infoType, "gameType") == 0 ||
	           Q_stricmp(element->infoType, "gametype") == 0 ||
	           Q_stricmp(element->infoType, "timelimit") == 0 ||
	           Q_stricmp(element->infoType, "timeLimit") == 0 ||
	           Q_stricmp(element->infoType, "fraglimit") == 0 ||
	           Q_stricmp(element->infoType, "fragLimit") == 0 ||
	           Q_stricmp(element->infoType, "capturelimit") == 0 ||
	           Q_stricmp(element->infoType, "captureLimit") == 0) {
		// Legacy serverInfo types - convert to game.* format
		char newType[MAX_QPATH];
		Com_sprintf(newType, sizeof(newType), "game.%s", element->infoType);
		Q_strncpyz(element->infoType, newType, sizeof(element->infoType));
		CG_CHUDDecorProcessGameType(element);
	} else {
		CG_CHUDDecorGetUnknownType(element);
	}
}

// Process player types (player.score, player.name, etc.)
static void CG_CHUDDecorProcessPlayerType(cherryhudDecorElement_t* element)
{
	char* subType = element->infoType + 7; // Skip "player."
	void (*handler)(cherryhudDecorElement_t*);
	qhandle_t autoIcon;
	
	// Find handler using lookup table
	handler = CG_CHUDDecorFindPlayerTypeHandler(subType);
	if (handler) {
		// Call the handler to set text content
		handler(element);
		
		// Check if this type has an automatic icon and set it if needed
		if (CG_CHUDDecorPlayerTypeHasAutoIcon(subType)) {
			autoIcon = CG_CHUDDecorGetAutoImage(element);
			if (autoIcon) {
				// Initialize draw context if not already done
				if (!element->drawCtx.pos[0] && !element->drawCtx.pos[1]) {
					CG_CHUDDrawMakeContext(&element->config, &element->drawCtx);
				}
				element->drawCtx.image = autoIcon;
			}
		}
	} else {
		CG_CHUDDecorGetUnknownType(element);
	}
}

// Process game types (game.map, game.serverName, etc.)
static void CG_CHUDDecorProcessGameType(cherryhudDecorElement_t* element)
{
	char* subType = element->infoType + 5; // Skip "game."
	void (*handler)(cherryhudDecorElement_t*);
	
	// Find handler using lookup table
	handler = CG_CHUDDecorFindGameTypeHandler(subType);
	if (handler) {
		handler(element);
	} else {
		CG_CHUDDecorGetUnknownType(element);
	}
}

// Process weapon types (weapon.railgun.accuracy, weapon.grenade.kills, etc.)
static void CG_CHUDDecorProcessWeaponType(cherryhudDecorElement_t* element)
{
	char buffer[MAX_QPATH];
	char* subType;
	char* dotPos;
	char* weaponStat;
	void (*handler)(cherryhudDecorElement_t*);

	/* Copy infoType to buffer to avoid modifying the original */
	Q_strncpyz(buffer, element->infoType, sizeof(buffer));
	subType = buffer + 7; /* Skip "weapon." */
	dotPos = strchr(subType, '.');

	if (!dotPos) {
		CG_CHUDDecorGetUnknownType(element);
		return;
	}
	
	*dotPos = '\0'; /* Null terminate weapon name in buffer */
	weaponStat = dotPos + 1; /* Get the stat part */
	
	// Find handler using lookup table
	handler = CG_CHUDDecorFindWeaponStatHandler(weaponStat);
	if (handler) {
		handler(element);
	} else {
		CG_CHUDDecorGetUnknownType(element);
	}
}

// Common utility functions to reduce duplication
static score_t* CG_CHUDDecorFindPlayerScore(int clientNum) {
	int i;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cg.scores[i].client == clientNum) {
			return &cg.scores[i];
		}
	}
	return NULL;
}

static void CG_CHUDDecorSetTextFromString(cherryhudDecorElement_t* element, const char* text) {
	Q_strncpyz(element->displayText, text, sizeof(element->displayText));
	element->textCtx.text = element->displayText;
}

static void CG_CHUDDecorSetTextFromClientInfo(cherryhudDecorElement_t* element, const char* field) {
	clientInfo_t* ci = &cgs.clientinfo[element->clientNum];
	const char* value = NULL;
	
	if (Q_stricmp(field, "name") == 0) {
		value = ci->name;
	} else if (Q_stricmp(field, "xid") == 0) {
		value = (const char*)ci->xidStr;
	}
	
	CG_CHUDDecorSetTextFromString(element, value ? value : "Unknown");
}

// Player type handlers
static void CG_CHUDDecorGetPlayerName(cherryhudDecorElement_t* element) {
	CG_CHUDDecorSetTextFromClientInfo(element, "name");
}

static void CG_CHUDDecorGetPlayerScore(cherryhudDecorElement_t* element) {
	score_t* score = CG_CHUDDecorFindPlayerScore(element->clientNum);
	int playerScore = 0;
	
	if (score) {
		playerScore = score->score;
	} else {
		playerScore = cgs.be.statsAll[element->clientNum].kills;
	}
	CG_CHUDDecorApplyFormatting(element, (float)playerScore);
}

static void CG_CHUDDecorGetPlayerPing(cherryhudDecorElement_t* element) {
	score_t* score = CG_CHUDDecorFindPlayerScore(element->clientNum);
	int ping = 0;
	
	if (score && score->ping != -1) {
		ping = score->ping;
	}
	CG_CHUDDecorApplyFormatting(element, (float)ping);
}

// Helper function for simple stats formatting
static void CG_CHUDDecorGetPlayerStat(cherryhudDecorElement_t* element, float value) {
	CG_CHUDDecorApplyFormatting(element, value);
}

// Helper function for score-based stats
static void CG_CHUDDecorGetPlayerScoreStat(cherryhudDecorElement_t* element, const char* field) {
	score_t* score = CG_CHUDDecorFindPlayerScore(element->clientNum);
	int value = 0;
	
	if (score) {
		if (Q_stricmp(field, "time") == 0) {
			value = score->time;
		} else if (Q_stricmp(field, "thaws") == 0) {
			value = score->scoreFlags;
		}
	}
	
	CG_CHUDDecorApplyFormatting(element, (float)value);
}

static void CG_CHUDDecorGetPlayerDeaths(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerStat(element, (float)cgs.be.statsAll[element->clientNum].deaths);
}

static void CG_CHUDDecorGetPlayerKD(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerStat(element, cgs.be.statsAll[element->clientNum].kdratio);
}

static void CG_CHUDDecorGetPlayerDamageGiven(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerStat(element, (float)cgs.be.statsAll[element->clientNum].dmgGiven);
}

static void CG_CHUDDecorGetPlayerDamageReceived(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerStat(element, (float)cgs.be.statsAll[element->clientNum].dmgReceived);
}

static void CG_CHUDDecorGetPlayerDamageRatio(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerStat(element, cgs.be.statsAll[element->clientNum].damageRatio);
}

static void CG_CHUDDecorGetPlayerId(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerStat(element, (float)element->clientNum);
}

static void CG_CHUDDecorGetPlayerTime(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetPlayerScoreStat(element, "time");
}

static void CG_CHUDDecorGetPlayerThaws(cherryhudDecorElement_t* element) {
	// Only show thaws in freeze mode
	if (!CG_OSPIsGameTypeFreeze()) {
		CG_CHUDDecorGetPlayerStat(element, 0.0f);
		return;
	}
	
	CG_CHUDDecorGetPlayerScoreStat(element, "thaws");
}

static void CG_CHUDDecorGetPlayerXid(cherryhudDecorElement_t* element) {
	CG_CHUDDecorSetTextFromClientInfo(element, "xid");
}

static void CG_CHUDDecorGetPlayerSkill(cherryhudDecorElement_t* element) {
	newStatsInfo_t* st = &cgs.be.statsAll[element->clientNum];
	qhandle_t skillShader = 0;
	
	// Initialize draw context if not already done
	if (!element->drawCtx.pos[0] && !element->drawCtx.pos[1]) {
		CG_CHUDDrawMakeContext(&element->config, &element->drawCtx);
	}
	
	// Check if this is a bot - use bot skill directly
	if (cgs.clientinfo[element->clientNum].botSkill > 0 && cgs.clientinfo[element->clientNum].botSkill <= 5) {
		skillShader = cgs.media.botSkillShaders[cgs.clientinfo[element->clientNum].botSkill - 1];
	}
	// Check if player stats are valid and has meaningful damage
	else if (st->infoValid && st->dmgGiven > 0) {
		// Use the pre-calculated damageRatio from statsAll
		float efficiency = st->damageRatio;
		
		// Determine skill level based on efficiency and damage thresholds
		if (efficiency >= 2.0f && st->dmgGiven > 2000) {
			skillShader = cgs.media.botSkillShaders[5]; // Highest skill (skill 6)
		} else if (efficiency >= 1.6f && st->dmgGiven > 1000) {
			skillShader = cgs.media.botSkillShaders[4]; // High skill (skill 5)
		} else if (efficiency >= 1.2f) {
			skillShader = cgs.media.botSkillShaders[3]; // Good skill (skill 4)
		} else if (efficiency >= 0.8f) {
			skillShader = cgs.media.botSkillShaders[2]; // Average skill (skill 3)
		} else if (efficiency >= 0.4f) {
			skillShader = cgs.media.botSkillShaders[1]; // Low skill (skill 2)
		} else {
			skillShader = cgs.media.botSkillShaders[0]; // Very low skill (skill 1)
		}
	} else {
		// No valid stats or no damage - show skill 1
		skillShader = cgs.media.botSkillShaders[0];
	}
	
	// Set the skill icon
	element->drawCtx.image = skillShader;
	// Clear text for icon display
	element->textCtx.text = NULL;
}

static void CG_CHUDDecorGetPlayerHead(cherryhudDecorElement_t* element) {
	// For player.head, we don't need to set text content - it's handled in rendering
	element->textCtx.text = "";
}

// Helper function for server info fields
static void CG_CHUDDecorGetServerInfoField(cherryhudDecorElement_t* element, const char* key, const char* defaultValue) {
	const char* info = CG_ConfigString(CS_SERVERINFO);
	char* value = Info_ValueForKey(info, key);
	CG_CHUDDecorSetTextFromString(element, value ? value : defaultValue);
}

// Game type handlers
static void CG_CHUDDecorGetGameServerName(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetServerInfoField(element, "sv_hostname", "Unknown Server");
}

static void CG_CHUDDecorGetGameMapName(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetServerInfoField(element, "mapname", "Unknown Map");
}

static void CG_CHUDDecorGetGameType(cherryhudDecorElement_t* element) {
	const char* gametypeNames[] = {
		"Free for all",
		"Tournament", 
		"Single Player",
		"Team Deathmatch",
		"Capture The flag",
		"Clan Arena",
		"Freeze Tag",
		"Freeze tag CTF",
		"Unknown"
	};
	
	int gt;
	short isFreeze;
	
	gt = cgs.gametype;
	isFreeze = cgs.osp.gameTypeFreeze;
	
	if (gt >= 0 && gt < 8) {
		if (isFreeze && gt == GT_TEAM) {
			Q_strncpyz(element->displayText, gametypeNames[6], sizeof(element->displayText));
		} else if (isFreeze && gt == GT_CTF) {
			Q_strncpyz(element->displayText, gametypeNames[7], sizeof(element->displayText));
		} else {
			Q_strncpyz(element->displayText, gametypeNames[gt], sizeof(element->displayText));
		}
	} else {
		Q_strncpyz(element->displayText, gametypeNames[8], sizeof(element->displayText));
	}
	element->textCtx.text = element->displayText;
}

static void CG_CHUDDecorGetGameTimeLimit(cherryhudDecorElement_t* element) {
	CG_CHUDDecorApplyFormatting(element, (float)cgs.timelimit);
}

static void CG_CHUDDecorGetGameFragLimit(cherryhudDecorElement_t* element) {
	short isFreeze;
	int gt;
	int limit;
	
	isFreeze = cgs.osp.gameTypeFreeze;
	gt = cgs.gametype;
	limit = (isFreeze || gt == GT_CTF) ? cgs.capturelimit : cgs.fraglimit;
	CG_CHUDDecorApplyFormatting(element, (float)limit);
}

static void CG_CHUDDecorGetGameCaptureLimit(cherryhudDecorElement_t* element) {
	short isFreeze;
	int gt;
	int limit;
	
	isFreeze = cgs.osp.gameTypeFreeze;
	gt = cgs.gametype;
	limit = (isFreeze || gt == GT_CTF) ? cgs.capturelimit : cgs.fraglimit;
	CG_CHUDDecorApplyFormatting(element, (float)limit);
}

// Helper function for weapon stats
static void CG_CHUDDecorGetWeaponStat(cherryhudDecorElement_t* element, const char* statType) {
	newStatsInfo_t* stats = &cgs.be.statsAll[element->clientNum];
	float value = 0.0f;
	
	if (Q_stricmp(statType, "kills") == 0) {
		value = (float)stats->stats[element->weaponType].kills;
	} else if (Q_stricmp(statType, "accuracy") == 0) {
		value = stats->stats[element->weaponType].accuracy;
	} else if (Q_stricmp(statType, "shots") == 0) {
		value = (float)stats->stats[element->weaponType].shots;
	}
	
	CG_CHUDDecorApplyFormatting(element, value);
}

// Weapon type handlers
static void CG_CHUDDecorGetWeaponKills(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetWeaponStat(element, "kills");
}

static void CG_CHUDDecorGetWeaponAccuracy(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetWeaponStat(element, "accuracy");
}

static void CG_CHUDDecorGetWeaponShots(cherryhudDecorElement_t* element) {
	CG_CHUDDecorGetWeaponStat(element, "shots");
}

static void CG_CHUDDecorGetWeaponIcon(cherryhudDecorElement_t* element) {
	// For icon type, we'll use the weapon icon
	qhandle_t icon = cg_weapons[element->weaponType].weaponIcon;
	if (icon) {
		element->drawCtx.image = icon;
		// Clear text for icon display
		element->textCtx.text = NULL;
	} else {
		Q_strncpyz(element->displayText, "?", sizeof(element->displayText));
		element->textCtx.text = element->displayText;
	}
}

static void CG_CHUDDecorGetUnknownType(cherryhudDecorElement_t* element) {
	const cherryhudConfig_t* config = &element->config;
	if (config->text.isSet) {
		Q_strncpyz(element->displayText, config->text.value, sizeof(element->displayText));
	} else {
		Q_strncpyz(element->displayText, "Unknown", sizeof(element->displayText));
	}
	element->textCtx.text = element->displayText;
}

// Helper function to get weapon type from string
static int CG_CHUDDecorGetWeaponTypeFromString(const char* weaponName) {
	if (Q_stricmp(weaponName, "gauntlet") == 0) return WP_GAUNTLET;
	if (Q_stricmp(weaponName, "machinegun") == 0) return WP_MACHINEGUN;
	if (Q_stricmp(weaponName, "shotgun") == 0) return WP_SHOTGUN;
	if (Q_stricmp(weaponName, "grenade") == 0) return WP_GRENADE_LAUNCHER;
	if (Q_stricmp(weaponName, "rocket") == 0) return WP_ROCKET_LAUNCHER;
	if (Q_stricmp(weaponName, "lightning") == 0) return WP_LIGHTNING;
	if (Q_stricmp(weaponName, "railgun") == 0) return WP_RAILGUN;
	if (Q_stricmp(weaponName, "plasma") == 0) return WP_PLASMAGUN;
	if (Q_stricmp(weaponName, "bfg") == 0) return WP_BFG;
	
	return WP_GAUNTLET; // Default fallback
}

// Get automatic image based on infoType
static qhandle_t CG_CHUDDecorGetAutoImage(cherryhudDecorElement_t* element) {
    char buffer[MAX_QPATH];
    char* token;
    char* nextToken;
    int weaponIndex;
    
    // Copy infoType to avoid modifying original
    Q_strncpyz(buffer, element->infoType, sizeof(buffer));
    
    // Parse weapon type - для любого weapon.weaponname.* показываем иконку оружия
    if (Q_strncmp(buffer, "weapon.", 7) == 0) {
        token = buffer + 7; // Skip "weapon."
        nextToken = strchr(token, '.');
        if (nextToken) {
            *nextToken = '\0'; // Null-terminate weapon name
        }
        
        // Get weapon index
        weaponIndex = CG_CHUDDecorGetWeaponTypeFromString(token);
        if (weaponIndex >= 0 && weaponIndex < WP_NUM_WEAPONS) {
            qhandle_t icon = cg_weapons[weaponIndex].weaponIcon;
            if (icon) {
                return icon;
            }
        }
    }
    
    // Parse other types - привязываем к player.* для единообразия
    if (Q_stricmp(buffer, "player.ping") == 0) {
        return cgs.media.scoreboardBESignal;
    }
    if (Q_stricmp(buffer, "player.ping.nosignal") == 0) {
        return cgs.media.scoreboardBENoSignal;
    }
    if (Q_stricmp(buffer, "player.score") == 0) {
        return cgs.media.scoreboardBEScore;
    }
    if (Q_stricmp(buffer, "player.deaths") == 0) {
        return cgs.media.obituariesSkull;
    }
    if (Q_stricmp(buffer, "player.kdratio") == 0) {
        return cgs.media.scoreboardBELeaderboard;
    }
    if (Q_stricmp(buffer, "player.time") == 0) {
        return cgs.media.scoreboardBEClock;
    }
    if (Q_stricmp(buffer, "player.id") == 0) {
        return cgs.media.scoreboardBEScore;
    }
    if (Q_stricmp(buffer, "player.thaws") == 0) {
        return cgs.media.scoreboardBEScore;
    }
    if (Q_stricmp(buffer, "player.ready") == 0) {
        return cgs.media.scoreboardBEReady;
    }
    if (Q_stricmp(buffer, "player.spectator") == 0) {
        return cgs.media.scoreboardBEEye;
    }
    if (Q_stricmp(buffer, "player.damageGiven") == 0) {
        return cgs.media.arrowUp;
    }
    if (Q_stricmp(buffer, "player.damageReceived") == 0) {
        return cgs.media.arrowDown;
    }
    
    // No matching image found
    return 0;
}

// Initialize content source once at creation - determines what content to use
static void CG_CHUDDecorInitializeContent(cherryhudDecorElement_t* element)
{
    qhandle_t autoImage;
    
    // Mark content as initialized
    element->flags |= CHUD_DECOR_CONTENT_INITIALIZED;
    
    // Determine content source based on render mode
    if (element->flags & CHUD_DECOR_BOTH_SPECIFIED) {
        // Both text and image specified - render both
        if (element->flags & CHUD_DECOR_HAS_EMPTY_TEXT) {
            // Empty text specified - generate content based on type instead
            element->flags |= CHUD_DECOR_USE_AUTO_TEXT;
            CG_CHUDDecorProcessType(element);
        }
        if (element->flags & CHUD_DECOR_HAS_EMPTY_IMAGE) {
            // Empty image specified - generate automatic image based on type
            autoImage = CG_CHUDDecorGetAutoImage(element);
            if (autoImage) {
                element->drawCtx.image = autoImage;
            }
        }
    } else if (element->flags & CHUD_DECOR_RENDER_DYNAMIC) {
        // No manual content - generate content based on type
        // Check if this is a dynamic image type
        if (Q_stricmp(element->infoType, "player.skill") == 0) {
            element->flags |= CHUD_DECOR_USE_DYNAMIC_IMAGE;
            CG_CHUDDecorProcessType(element);
        } else {
            element->drawCtx.image = 0; // Clear image
            element->flags |= CHUD_DECOR_USE_AUTO_TEXT;
            CG_CHUDDecorProcessType(element);
        }
    } else if (element->flags & CHUD_DECOR_RENDER_TEXT) {
        // Text mode - clear image, handle empty text
        element->drawCtx.image = 0; // Clear image
        if (element->flags & CHUD_DECOR_HAS_EMPTY_TEXT) {
            // Empty text specified - generate content based on type instead
            element->flags |= CHUD_DECOR_USE_AUTO_TEXT;
            CG_CHUDDecorProcessType(element);
        }
    } else if (element->flags & CHUD_DECOR_RENDER_IMAGE) {
        // Image mode - clear text, handle empty image
        element->textCtx.text = NULL; // Clear text
        if (element->flags & CHUD_DECOR_HAS_EMPTY_IMAGE) {
            // Empty image specified - generate automatic image based on type
            autoImage = CG_CHUDDecorGetAutoImage(element);
            if (autoImage) {
                element->drawCtx.image = autoImage;
            }
        }
    }
}

// Apply formatting to numeric values if textExt.format is set
static void CG_CHUDDecorApplyFormatting(cherryhudDecorElement_t* element, float value)
{
    const cherryhudConfig_t* config;
    const char* infoType;
    qboolean isFloatType;
    
    config = &element->config;
    infoType = element->infoType;
    isFloatType = qfalse;
    
    // Check if this is a float type that should use .2f by default
    if (infoType) {
        if (Q_stricmp(infoType, "player.kd") == 0 ||
            Q_stricmp(infoType, "player.damageRatio") == 0) {
            isFloatType = qtrue;
        } else if (strstr(infoType, "accuracy")) {
            // Special case for accuracy - use percentage format by default
            if (config->textExt.format.isSet && config->textExt.format.value[0] != '\0') {
                // Use custom format if specified
                CG_CHUDFormatNumber(value, config->textExt.format.value, element->displayText, sizeof(element->displayText));
            } else {
                // Default accuracy format: 2 decimal places + %
                Com_sprintf(element->displayText, sizeof(element->displayText), "%.1f%%", value);
            }
            element->textCtx.text = element->displayText;
            return;
        }
    }
    
    // Check if maxValue is set and value exceeds it
    if (config->maxValue.isSet && value > config->maxValue.value) {
        // Value exceeds maxValue, show maxValue + "+"
        if (value == (int)value && !isFloatType) {
            // Integer value
            Com_sprintf(element->displayText, sizeof(element->displayText), "%d+", (int)config->maxValue.value);
        } else {
            // Float value
            if (isFloatType) {
                Com_sprintf(element->displayText, sizeof(element->displayText), "%.1f+", config->maxValue.value);
            } else {
                Com_sprintf(element->displayText, sizeof(element->displayText), "%.0f+", config->maxValue.value);
            }
        }
        element->textCtx.text = element->displayText;
        return;
    }
    
    // Check if textExt.format is set
    if (config->textExt.format.isSet && config->textExt.format.value[0] != '\0') {
        // Apply formatting
        CG_CHUDFormatNumber(value, config->textExt.format.value, element->displayText, sizeof(element->displayText));
        element->textCtx.text = element->displayText;
    } else {
        // Default formatting based on value type
        if (value == (int)value && !isFloatType) {
            // Integer value (and not a special float type)
            Com_sprintf(element->displayText, sizeof(element->displayText), "%d", (int)value);
        } else {
            // Float value - use 2 decimal places for special float types, 1 for others
            if (isFloatType) {
                Com_sprintf(element->displayText, sizeof(element->displayText), "%.1f", value);
            } else {
                Com_sprintf(element->displayText, sizeof(element->displayText), "%.0f", value);
            }
        }
        element->textCtx.text = element->displayText;
    }
}
