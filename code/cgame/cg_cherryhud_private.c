#include "cg_cherryhud_private.h"

#include "../qcommon/qcommon.h"

static cherryhudGlobalContext_t cherryhudGlobalContext;

// Global storage for prefixed configs
#define MAX_PREFIXED_CONFIGS 32
static cherryhudConfig_t g_prefixedConfigs[MAX_PREFIXED_CONFIGS];
static int g_prefixedConfigCount;


// Global element index counters
static int g_playerRowElementIndex = 0;
static int g_spectatorRowElementIndex = 0;
static int g_titleRowElementIndex = 0;

// External table references (now managed by TableManager)



#define MAX_CONFIG_ENTRIES 256
#define CONFIG_HASH_SIZE 64
#define MAX_PLAYERINFO_ELEMENTS 2048

typedef enum {
    CONFIG_TYPE_PLAYER_ROW_TEMPLATE,
    CONFIG_TYPE_SPECTATOR_ROW_TEMPLATE, 
    CONFIG_TYPE_TITLE_ROW_TEMPLATE,
    CONFIG_TYPE_PLAYER_INFO_TEMPLATE,
    CONFIG_TYPE_PLAYER_ROW_ELEMENT,
    CONFIG_TYPE_SPECTATOR_ROW_ELEMENT,
    CONFIG_TYPE_TITLE_ROW_ELEMENT,
    CONFIG_TYPE_TITLE_BLOCK,
    CONFIG_TYPE_SCOREBOARD,
    CONFIG_TYPE_DEFAULT
} configType_t;

typedef struct configEntry_s {
    char key[MAX_QPATH];
    configType_t type;
    cherryhudConfig_t config;
    int index;
    qboolean isSet;
    struct configEntry_s* next;
} configEntry_t;

static configEntry_t* g_configHashTable[CONFIG_HASH_SIZE];
static configEntry_t g_configEntries[MAX_CONFIG_ENTRIES];
static int g_configEntryCount = 0;

static cherryhudConfig_t playerRowTemplate;
static cherryhudConfig_t spectatorRowTemplate;
static cherryhudConfig_t titleRowTemplate;
static qboolean playerRowTemplateSet = qfalse;
static qboolean spectatorRowTemplateSet = qfalse;
static qboolean titleRowTemplateSet = qfalse;

static cherryhudConfig_t titleRowElements[MAX_TITLE_ELEMENTS];
static int titleRowElementCount = 0;
static cherryhudConfig_t playerInfoTemplate[MAX_PLAYERINFO_ELEMENTS];
static int playerInfoTemplateCount = 0;
static cherryhudTitleBlock_t titleBlocks[MAX_TITLE_BLOCKS];
static int titleBlockCount = 0;

float g_playerRowHeight = 20.0f; // Default value

// Configuration storage for deferred element creation
static cherryhudConfigStorage_t g_configStorage;
static int g_titleElementIndex = 0;

// ServerInfo element types are now handled in cg_cherryhud_element_serverinfo.c

// Function to check if a string is a valid serverInfo type
// This function is now implemented in cg_cherryhud_element_serverinfo.c

// Unified element type definitions
static const cherryhudElementTypeInfo_t g_unifiedElementTypes[] = {
    // Container types (with handlers)
    { "!scoreboard", CHERRYHUD_TYPE_CONTAINER, qtrue, CG_CHUDHandleScoreboardConfig, NULL, NULL, NULL, CHERRYHUD_CONTEXT_MAIN_CONTAINER },
    { "playersRows", CHERRYHUD_TYPE_CONTAINER, qtrue, CG_CHUDHandlePlayersRows, NULL, NULL, NULL, CHERRYHUD_CONTEXT_PLAYER_ROW },
    { "spectatorsRows", CHERRYHUD_TYPE_CONTAINER, qtrue, CG_CHUDHandleSpectatorsRows, NULL, NULL, NULL, CHERRYHUD_CONTEXT_SPECTATOR_ROW },
    
    // Config types (with handlers)
    { "!default", CHERRYHUD_TYPE_CONFIG, qtrue, CG_CHUDHandleDefaultConfig, NULL, NULL, NULL, 0 },
    
    // Block types (with handlers) - templates
    // bonusFlag = context + template_flag
    { "playerRow", CHERRYHUD_TYPE_BLOCK, qtrue, CG_CHUDHandlePlayerRow, NULL, NULL, NULL, CHERRYHUD_CONTEXT_PLAYER_ROW | CHERRYHUD_FLAG_TEMPLATE },
    { "spectatorRow", CHERRYHUD_TYPE_BLOCK, qtrue, CG_CHUDHandleSpectatorRow, NULL, NULL, NULL, CHERRYHUD_CONTEXT_SPECTATOR_ROW | CHERRYHUD_FLAG_TEMPLATE },
    { "title", CHERRYHUD_TYPE_BLOCK, qtrue, CG_CHUDHandleTitle, NULL, NULL, NULL, CHERRYHUD_CONTEXT_TITLE },
    // { "template", CHERRYHUD_TYPE_BLOCK, qfalse, NULL, NULL, NULL, NULL, 0 },
    
    // Element types (with create functions) - unified decor element with aliases
    { "decor", CHERRYHUD_TYPE_ELEMENT, qfalse, NULL, CG_CHUDElementDecorCreate, CG_CHUDElementDecorRoutine, CG_CHUDElementDecorDestroy, CHERRYHUD_FLAG_AUTO_DETECT },
    { "print", CHERRYHUD_TYPE_ELEMENT, qfalse, NULL, CG_CHUDElementDecorCreate, CG_CHUDElementDecorRoutine, CG_CHUDElementDecorDestroy, CHERRYHUD_FLAG_AUTO_DETECT },
    { "text", CHERRYHUD_TYPE_ELEMENT, qfalse, NULL, CG_CHUDElementDecorCreate, CG_CHUDElementDecorRoutine, CG_CHUDElementDecorDestroy, CHERRYHUD_FLAG_AUTO_DETECT },
    { "draw", CHERRYHUD_TYPE_ELEMENT, qfalse, NULL, CG_CHUDElementDecorCreate, CG_CHUDElementDecorRoutine, CG_CHUDElementDecorDestroy, CHERRYHUD_FLAG_AUTO_DETECT },
    
    
    // End marker
    { NULL, CHERRYHUD_TYPE_ELEMENT, qfalse, NULL, NULL, NULL, NULL, 0 }
};

// Element handler system
#define MAX_ELEMENT_HANDLERS 32
static cherryhudElementHandler_t g_elementHandlers[MAX_ELEMENT_HANDLERS];
static int g_elementHandlerCount = 0;

// Handler registration table
typedef struct {
    const char* name;
    cherryhudHandlerType_t type;
    void (*handler)(const char* elementName, cherryhudConfig_t* config);
} cherryhudHandlerRegistration_t;

// Macro for easier handler registration
#define HANDLER_REG(name, type, func) { name, type, func }


static void CG_CHUDConfigSystemInit(void) {
    int i;
    
    for (i = 0; i < CONFIG_HASH_SIZE; i++) {
        g_configHashTable[i] = NULL;
    }
    
    memset(g_configEntries, 0, sizeof(g_configEntries));
    g_configEntryCount = 0;
}

static int CG_CHUDConfigHash(const char* key) {
    unsigned long hash = 0;
    int c;
    
    while ((c = *key++)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    
    return hash % CONFIG_HASH_SIZE;
}

static configEntry_t* CG_CHUDConfigFind(const char* key, configType_t type) {
    int hash;
    configEntry_t* entry;
    
    if (!key) return NULL;
    
    hash = CG_CHUDConfigHash(key);
    entry = g_configHashTable[hash];
    
    while (entry) {
        if (entry->type == type && Q_stricmp(entry->key, key) == 0) {
            return entry;
        }
        entry = entry->next;
    }
    
    return NULL;
}

static configEntry_t* CG_CHUDConfigSet(const char* key, configType_t type, const cherryhudConfig_t* config, int index) {
    configEntry_t* entry;
    int hash;
    
    if (!key || !config) {
        return NULL;
    }
    
    if (g_configEntryCount >= MAX_CONFIG_ENTRIES) {
        int clearCount = MAX_CONFIG_ENTRIES / 2;
        int i;
        // CRITICAL: Config entries array overflow - clear old entries to prevent memory leak
        
        // Clear half of the old entries to make space

        for (i = 0; i < clearCount; i++) {
            g_configEntries[i] = g_configEntries[clearCount + i];
        }
        g_configEntryCount = clearCount;
        
        // Rebuild hash table
        for (i = 0; i < CONFIG_HASH_SIZE; i++) {
            g_configHashTable[i] = NULL;
        }
        for (i = 0; i < g_configEntryCount; i++) {
            int hash = CG_CHUDConfigHash(g_configEntries[i].key);
            g_configEntries[i].next = g_configHashTable[hash];
            g_configHashTable[hash] = &g_configEntries[i];
        }
    }
    
    // Try to find existing entry
    entry = CG_CHUDConfigFind(key, type);
    if (entry) {
        // Update existing entry
        memcpy(&entry->config, config, sizeof(cherryhudConfig_t));
        entry->index = index;
        entry->isSet = qtrue;
        return entry;
    }
    
    // Create new entry
    entry = &g_configEntries[g_configEntryCount];
    Q_strncpyz(entry->key, key, sizeof(entry->key));
    entry->type = type;
    memcpy(&entry->config, config, sizeof(cherryhudConfig_t));
    entry->index = index;
    entry->isSet = qtrue;
    entry->next = NULL;
    
    // Add to hash table
    hash = CG_CHUDConfigHash(key);
    entry->next = g_configHashTable[hash];
    g_configHashTable[hash] = entry;
    
    g_configEntryCount++;
    return entry;
}

static const configEntry_t* CG_CHUDConfigGet(const char* key, configType_t type) {
    return CG_CHUDConfigFind(key, type);
}

static int CG_CHUDConfigGetAllOfType(configType_t type, configEntry_t** entries, int maxEntries) {
    int count = 0;
    int i;
    configEntry_t* entry;
    
    for (i = 0; i < g_configEntryCount && count < maxEntries; i++) {
        entry = &g_configEntries[i];
        if (entry->isSet && entry->type == type) {
            entries[count] = entry;
            count++;
        }
    }
    
    return count;
}



cherryhudGlobalContext_t* CG_CHUDGetContext(void)
{
    return &cherryhudGlobalContext;
}

void CG_CHUDSetPlayerRowTemplate(const cherryhudConfig_t* config) {
    if (config) {
        CG_CHUDConfigSet("playerRow", CONFIG_TYPE_PLAYER_ROW_TEMPLATE, config, 0);
        
        memcpy(&playerRowTemplate, config, sizeof(cherryhudConfig_t));
        playerRowTemplateSet = qtrue;
        
        
        if (config->size.isSet) {
            g_playerRowHeight = config->size.value[1];
        }
    } else {
    }
}

const cherryhudConfig_t* CG_CHUDGetPlayerRowTemplate(void) {
    return playerRowTemplateSet ? &playerRowTemplate : NULL;
}

void CG_CHUDSetPlayerInfoTemplate(const cherryhudConfig_t* config) {
    int i;
    const char* newType;
    char key[MAX_QPATH];
    
    if (!config) {
        return;
    }
    
    newType = config->type.isSet ? config->type.value : "unknown";
    
    // Create unique key for this template type
    Com_sprintf(key, sizeof(key), "playerInfo_%s", newType);
    
    CG_CHUDConfigSet(key, CONFIG_TYPE_PLAYER_INFO_TEMPLATE, config, 0);
    
    for (i = 0; i < playerInfoTemplateCount; i++) {
        const char* existingType = playerInfoTemplate[i].type.isSet ? playerInfoTemplate[i].type.value : "unknown";
        if (Q_stricmp(newType, existingType) == 0) {
            return;
        }
    }
    
    if (playerInfoTemplateCount < MAX_PLAYERINFO_ELEMENTS) {
        memcpy(&playerInfoTemplate[playerInfoTemplateCount], config, sizeof(cherryhudConfig_t));
        playerInfoTemplateCount++;
    } else {
        // CRITICAL: Player info templates array overflow - clear old entries to prevent memory leak
        int clearCount = MAX_PLAYERINFO_ELEMENTS / 2;
        int i;
        
        // Clear half of the old entries to make space

        for (i = 0; i < clearCount; i++) {
            playerInfoTemplate[i] = playerInfoTemplate[clearCount + i];
        }
        playerInfoTemplateCount = clearCount;
        
        // Add the new template
        memcpy(&playerInfoTemplate[playerInfoTemplateCount], config, sizeof(cherryhudConfig_t));
        playerInfoTemplateCount++;
    }
}

void CG_CHUDSetSpectatorRowTemplate(const cherryhudConfig_t* config) {
    if (!config) {
        return;
    }
    
    CG_CHUDConfigSet("spectatorRow", CONFIG_TYPE_SPECTATOR_ROW_TEMPLATE, config, 0);
    
    memcpy(&spectatorRowTemplate, config, sizeof(cherryhudConfig_t));
    spectatorRowTemplateSet = qtrue;
}

const cherryhudConfig_t* CG_CHUDGetSpectatorRowTemplate(void) {
    return spectatorRowTemplateSet ? &spectatorRowTemplate : NULL;
}

void CG_CHUDSetTitleRowTemplate(const cherryhudConfig_t* config) {
    if (!config) {
        return;
    }
    
    
    // Copy config and apply width inheritance
    memcpy(&titleRowTemplate, config, sizeof(cherryhudConfig_t));
    
    // Width inheritance: if title width is 0, inherit from scoreboard
    if (titleRowTemplate.size.isSet && titleRowTemplate.size.value[0] == 0.0f) {
        const cherryhudConfig_t* scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig && scoreboardConfig->size.isSet && scoreboardConfig->size.value[0] > 0.0f) {
            titleRowTemplate.size.value[0] = scoreboardConfig->size.value[0];
        }
    }
    
    titleRowTemplateSet = qtrue;
}

const cherryhudConfig_t* CG_CHUDGetTitleRowTemplate(void) {
    return titleRowTemplateSet ? &titleRowTemplate : NULL;
}

void CG_CHUDAddTitleRowElement(const cherryhudConfig_t* config) {
    char key[MAX_QPATH];
    
    if (!config) {
        return;
    }
    
    Com_sprintf(key, sizeof(key), "titleRow_element_%d", g_titleRowElementIndex++);
    
    CG_CHUDConfigSet(key, CONFIG_TYPE_TITLE_ROW_ELEMENT, config, g_titleRowElementIndex - 1);
    
    if (titleRowElementCount < MAX_TITLE_ELEMENTS) {
        memcpy(&titleRowElements[titleRowElementCount], config, sizeof(cherryhudConfig_t));
        titleRowElementCount++;
    } else {
        int clearCount = MAX_TITLE_ELEMENTS / 2;
        int i;
        // CRITICAL: Title row elements array overflow - clear old entries to prevent memory leak
        
        // Clear half of the old entries to make space

        for (i = 0; i < clearCount; i++) {
            titleRowElements[i] = titleRowElements[clearCount + i];
        }
        titleRowElementCount = clearCount;
        
        // Add the new element
        memcpy(&titleRowElements[titleRowElementCount], config, sizeof(cherryhudConfig_t));
        titleRowElementCount++;
    }
}

const cherryhudConfig_t* CG_CHUDGetTitleRowElement(int index) {
    if (index >= 0 && index < titleRowElementCount) {
        return &titleRowElements[index];
    }
    return NULL;
}

int CG_CHUDGetTitleRowElementCount(void) {
    return titleRowElementCount;
}

void CG_CHUDSetScoreboardConfig(const cherryhudConfig_t* config) {
    cherryhudGlobalContext_t* context;
    
    if (!config) {
        return;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    
    memcpy(&context->config.scoreboard, config, sizeof(cherryhudConfig_t));
    context->config.scoreboardLoaded = qtrue;
    
    // Apply width inheritance to already loaded templates
    if (config->size.isSet && config->size.value[0] > 0.0f) {
        // Apply to title template if it has width 0
        if (titleRowTemplateSet && titleRowTemplate.size.isSet && titleRowTemplate.size.value[0] == 0.0f) {
            titleRowTemplate.size.value[0] = config->size.value[0];
        }
        
        if (playerRowTemplateSet && playerRowTemplate.size.isSet && playerRowTemplate.size.value[0] == 0.0f) {
            playerRowTemplate.size.value[0] = config->size.value[0];
        }
        
        if (spectatorRowTemplateSet && spectatorRowTemplate.size.isSet && spectatorRowTemplate.size.value[0] == 0.0f) {
            spectatorRowTemplate.size.value[0] = config->size.value[0];
        }
    }
}

void CG_CHUDSetDefaultConfig(const cherryhudConfig_t* config) {
    cherryhudGlobalContext_t* context;
    
    if (!config) {
        return;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    
    memcpy(&context->config.defaultConfig, config, sizeof(cherryhudConfig_t));
    context->config.defaultLoaded = qtrue;
}

void CG_CHUDSetCompactConfig(const cherryhudConfig_t* config) {
    cherryhudGlobalContext_t* context;
    
    if (!config) {
        return;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    memcpy(&context->config.compactConfig, config, sizeof(cherryhudConfig_t));
    context->config.compactLoaded = qtrue;
}

void CG_CHUDSetDoubleConfig(const cherryhudConfig_t* config) {
    cherryhudGlobalContext_t* context;
    
    if (!config) {
        return;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    memcpy(&context->config.doubleConfig, config, sizeof(cherryhudConfig_t));
    context->config.doubleLoaded = qtrue;
}

const cherryhudConfig_t* CG_CHUDGetScoreboardConfig(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    if (!context || !context->config.scoreboardLoaded) {
        return NULL;
    }
    
    return &context->config.scoreboard;
}

const cherryhudConfig_t* CG_CHUDGetDefaultConfig(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    if (!context || !context->config.defaultLoaded) {
        return NULL;
    }
    
    return &context->config.defaultConfig;
}

const cherryhudConfig_t* CG_CHUDGetCompactConfig(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    if (!context || !context->config.compactLoaded) {
        return NULL;
    }
    
    return &context->config.compactConfig;
}

const cherryhudConfig_t* CG_CHUDGetDoubleConfig(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    if (!context || !context->config.doubleLoaded) {
        return NULL;
    }
    
    return &context->config.doubleConfig;
}

qboolean CG_CHUDIsScoreboardConfigLoaded(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    return context ? context->config.scoreboardLoaded : qfalse;
}

qboolean CG_CHUDIsDefaultConfigLoaded(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    return context ? context->config.defaultLoaded : qfalse;
}

qboolean CG_CHUDIsCompactConfigLoaded(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    return context ? context->config.compactLoaded : qfalse;
}

qboolean CG_CHUDIsDoubleConfigLoaded(void) {
    cherryhudGlobalContext_t* context;
    
    context = CG_CHUDGetContext();
    return context ? context->config.doubleLoaded : qfalse;
}

// Prefixed mode config management
cherryhudConfig_t* CG_CHUDGetPrefixedConfig(const cherryhudConfig_t* baseConfig, const char* prefix) {
    cherryhudConfig_t* mutableConfig;
    int i;
    
    if (!baseConfig || !prefix) {
        return NULL;
    }
    
    // Cast to mutable for modification
    mutableConfig = (cherryhudConfig_t*)baseConfig;
    
    // Find existing prefixed config
    for (i = 0; i < mutableConfig->prefixed.prefixedCount; i++) {
        if (Q_stricmp(mutableConfig->prefixed.prefixed[i].prefix, prefix) == 0) {
            return &g_prefixedConfigs[mutableConfig->prefixed.prefixed[i].configIndex];
        }
    }
    
    // Create new prefixed config if we have space
    if (mutableConfig->prefixed.prefixedCount < MAX_PREFIXED_MODES && g_prefixedConfigCount < MAX_PREFIXED_CONFIGS) {
        i = mutableConfig->prefixed.prefixedCount;
        Q_strncpyz(mutableConfig->prefixed.prefixed[i].prefix, prefix, sizeof(mutableConfig->prefixed.prefixed[i].prefix));
        mutableConfig->prefixed.prefixed[i].configIndex = g_prefixedConfigCount;
        mutableConfig->prefixed.prefixed[i].loaded = qtrue;
        mutableConfig->prefixed.prefixedCount++;
        g_prefixedConfigCount++;
        
        // Initialize the new config
        memset(&g_prefixedConfigs[mutableConfig->prefixed.prefixed[i].configIndex], 0, sizeof(cherryhudConfig_t));
        
        return &g_prefixedConfigs[mutableConfig->prefixed.prefixed[i].configIndex];
    }
    
    return NULL;
}

qboolean CG_CHUDIsModeActive(const cherryhudConfig_t* config, const char* mode, float currentHeight) {
    cherryhudExtendedType_t expectedType;
    
    if (!config || !mode) {
        return qfalse;
    }
    
    // Convert mode string to enum
    if (Q_stricmp(mode, "double") == 0) {
        expectedType = CHERRYHUD_EXTENDED_DOUBLE;
    } else if (Q_stricmp(mode, "compact") == 0) {
        expectedType = CHERRYHUD_EXTENDED_COMPACT;
    } else {
        return qfalse; // Unknown mode
    }
    
    // Check if extendedType is set to the specified mode
    if (!config->extendedType.isSet) {
        return qfalse;
    }
    
    if (config->extendedType.value != expectedType) {
        return qfalse;
    }
    
    // Check if current height meets the threshold
    if (!config->size.isSet) {
        return qfalse;
    }
    
    return currentHeight >= config->size.value[1];
}

// New function to automatically determine active mode based on height
const char* CG_CHUDGetActiveMode(const cherryhudConfig_t* config, float currentHeight) {
    if (!config || !config->extendedType.isSet) {
        return NULL;
    }
    
    // Check if current height meets the threshold for the configured mode
    if (config->size.isSet && currentHeight >= config->size.value[1]) {
        if (config->extendedType.value == CHERRYHUD_EXTENDED_DOUBLE) {
            return "double";
        } else if (config->extendedType.value == CHERRYHUD_EXTENDED_COMPACT) {
            return "compact";
        }
    }
    
    return NULL; // No active mode
}

// New function to calculate base height (without mode) for mode determination
float CG_CHUDGetBaseHeightForMode(const char* containerType) {
    const cherryhudScoreboardOrder_t* scoreboardOrder;
    const cherryhudConfig_t* playerRowTemplate;
    const cherryhudConfig_t* spectatorRowTemplate;
    int playerCount, spectatorCount;
    float baseRowHeight, baseRowSpacing;
    float baseHeight;
    int i;
    
    if (!containerType) {
        return 0.0f;
    }
    
    scoreboardOrder = CG_CHUDScoreboardOrderGet();
    if (!scoreboardOrder) {
        return 0.0f;
    }
    
    // Get player counts
    playerCount = CG_CHUDGetPlayerCount();
    spectatorCount = CG_CHUDGetSpectatorCount();
    
    // Get base row height and spacing (without mode)
    baseRowHeight = g_playerRowHeight; // Default
    baseRowSpacing = 0.0f; // Default
    
    if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS)) == 0) {
        playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
        if (playerRowTemplate && playerRowTemplate->size.isSet) {
            baseRowHeight = playerRowTemplate->size.value[1];
        }
        if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
            baseRowSpacing = playerRowTemplate->rowSpacing.value;
        }
        baseHeight = (playerCount * baseRowHeight) + ((playerCount - 1) * baseRowSpacing);
    } else if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) == 0) {
        spectatorRowTemplate = CG_CHUDGetSpectatorRowTemplate();
        playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
        if (spectatorRowTemplate && spectatorRowTemplate->size.isSet) {
            baseRowHeight = spectatorRowTemplate->size.value[1];
        } else if (playerRowTemplate && playerRowTemplate->size.isSet) {
            baseRowHeight = playerRowTemplate->size.value[1];
        }
        if (spectatorRowTemplate && spectatorRowTemplate->rowSpacing.isSet) {
            baseRowSpacing = spectatorRowTemplate->rowSpacing.value;
        } else if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
            baseRowSpacing = playerRowTemplate->rowSpacing.value;
        }
        baseHeight = (spectatorCount * baseRowHeight) + ((spectatorCount - 1) * baseRowSpacing);
    } else {
        baseHeight = 0.0f;
    }
    
    
    return baseHeight;
}

// New function to get active mode from container for child elements
const char* CG_CHUDGetContainerActiveMode(const char* containerType, float currentHeight) {
    const cherryhudScoreboardOrder_t* scoreboardOrder;
    const cherryhudConfig_t* scoreboardConfig;
    float baseHeight;
    int i;
    
    if (!containerType) {
        return NULL;
    }
    
    scoreboardOrder = CG_CHUDScoreboardOrderGet();
    if (!scoreboardOrder) {
        return NULL;
    }
    
    // Calculate base height (without mode) for mode determination
    baseHeight = CG_CHUDGetBaseHeightForMode(containerType);
    
    // First, check if scoreboard has extendedtype that should be inherited
    scoreboardConfig = CG_CHUDGetScoreboardConfig();
    if (scoreboardConfig && scoreboardConfig->extendedType.isSet) {
        // Check if current height meets the threshold for the configured mode
        if (scoreboardConfig->size.isSet && baseHeight >= scoreboardConfig->size.value[1]) {
            if (scoreboardConfig->extendedType.value == CHERRYHUD_EXTENDED_DOUBLE) {
                return "double";
            } else if (scoreboardConfig->extendedType.value == CHERRYHUD_EXTENDED_COMPACT) {
                return "compact";
            }
        }
    }
    
    // Find the container config
    for (i = 0; i < scoreboardOrder->count; i++) {
        if ((Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS)) == 0 && 
             scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS) ||
            (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) == 0 && 
             scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) {
            
            // Get active mode from container using BASE height (not current height)
            return CG_CHUDGetActiveMode(&scoreboardOrder->blocks[i].config, baseHeight);
        }
    }
    
    return NULL;
}

void CG_CHUDGetActivePos(const cherryhudConfig_t* config, float currentHeight, vec4_t result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config) {
        Vector4Set(result, 0, 0, 0, 0);
        return;
    }
    
    // Check if we have an active mode based on height
    mode = CG_CHUDGetActiveMode(config, currentHeight);
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig && prefixedConfig->pos.isSet) {
            Vector2Copy(prefixedConfig->pos.value, result);
            return;
        }
    }
    
    // Return default pos
    if (config->pos.isSet) {
        Vector2Copy(config->pos.value, result);
    } else {
        Vector4Set(result, 0, 0, 0, 0);
    }
}

void CG_CHUDGetActiveSize(const cherryhudConfig_t* config, float currentHeight, vec4_t result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config) {
        Vector4Set(result, 0, 0, 0, 0);
        return;
    }
    
    // Check if we have an active mode based on height
    mode = CG_CHUDGetActiveMode(config, currentHeight);
    
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig) {
        }
        if (prefixedConfig && prefixedConfig->size.isSet) {
            Vector2Copy(prefixedConfig->size.value, result);
            return;
        } else {
        }
    }
    
    // Return default size
    if (config->size.isSet) {
        Vector2Copy(config->size.value, result);
    } else {
        Vector4Set(result, 0, 0, 0, 0);
    }
}

// New function to get active size using container's mode
void CG_CHUDGetActiveSizeFromContainer(const cherryhudConfig_t* config, const char* containerType, float currentHeight, vec4_t result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config) {
        Vector4Set(result, 0, 0, 0, 0);
        return;
    }
    
    // Get active mode from container instead of config itself
    mode = CG_CHUDGetContainerActiveMode(containerType, currentHeight);
    
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig && prefixedConfig->size.isSet) {
            Vector2Copy(prefixedConfig->size.value, result);
            return;
        } else {
        }
    }
    
    // Return default size
    if (config->size.isSet) {
        Vector2Copy(config->size.value, result);
    } else {
        Vector4Set(result, 0, 0, 0, 0);
    }
}

// New function to get active position using container's mode
void CG_CHUDGetActivePosFromContainer(const cherryhudConfig_t* config, const char* containerType, float currentHeight, vec4_t result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config) {
        Vector4Set(result, 0, 0, 0, 0);
        return;
    }
    
    // Get active mode from container instead of config itself
    mode = CG_CHUDGetContainerActiveMode(containerType, currentHeight);
    
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig && prefixedConfig->pos.isSet) {
            Vector2Copy(prefixedConfig->pos.value, result);
            return;
        }
    }
    
    // Return default pos
    if (config->pos.isSet) {
        Vector2Copy(config->pos.value, result);
    } else {
        Vector4Set(result, 0, 0, 0, 0);
    }
}

void CG_CHUDGetActiveColor(const cherryhudConfig_t* config, float currentHeight, cherryhudColor_t* result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config || !result) {
        if (result) {
            result->type = CHERRYHUD_COLOR_RGBA;
            result->rgba[0] = 1.0f;
            result->rgba[1] = 1.0f;
            result->rgba[2] = 1.0f;
            result->rgba[3] = 1.0f;
            result->isSet = qtrue;
        }
        return;
    }
    
    // Check if we have an active mode based on height
    mode = CG_CHUDGetActiveMode(config, currentHeight);
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig && prefixedConfig->color.isSet) {
            *result = prefixedConfig->color.value;
            return;
        }
    }
    
    // Return default color
    if (config->color.isSet) {
        *result = config->color.value;
    } else {
        result->type = CHERRYHUD_COLOR_RGBA;
        result->rgba[0] = 1.0f;
        result->rgba[1] = 1.0f;
        result->rgba[2] = 1.0f;
        result->rgba[3] = 1.0f;
        result->isSet = qtrue;
    }
}

void CG_CHUDGetActiveColor2(const cherryhudConfig_t* config, float currentHeight, cherryhudColor_t* result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config || !result) {
        if (result) {
            result->type = CHERRYHUD_COLOR_RGBA;
            result->rgba[0] = 1.0f;
            result->rgba[1] = 1.0f;
            result->rgba[2] = 1.0f;
            result->rgba[3] = 1.0f;
            result->isSet = qtrue;
        }
        return;
    }
    
    // Check if we have an active mode based on height
    mode = CG_CHUDGetActiveMode(config, currentHeight);
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig && prefixedConfig->color2.isSet) {
            *result = prefixedConfig->color2.value;
            return;
        }
    }
    
    // Return default color2
    if (config->color2.isSet) {
        *result = config->color2.value;
    } else {
        result->type = CHERRYHUD_COLOR_RGBA;
        result->rgba[0] = 1.0f;
        result->rgba[1] = 1.0f;
        result->rgba[2] = 1.0f;
        result->rgba[3] = 1.0f;
        result->isSet = qtrue;
    }
}

void CG_CHUDGetActiveBgColor(const cherryhudConfig_t* config, float currentHeight, float* result) {
    cherryhudConfig_t* prefixedConfig;
    const char* mode;
    
    if (!config || !result) {
        if (result) {
            result[0] = 1.0f;
            result[1] = 1.0f;
            result[2] = 1.0f;
            result[3] = 1.0f;
        }
        return;
    }
    
    // Check if we have an active mode based on height
    mode = CG_CHUDGetActiveMode(config, currentHeight);
    if (mode) {
        prefixedConfig = CG_CHUDGetPrefixedConfig(config, mode);
        if (prefixedConfig && prefixedConfig->background.color.isSet) {
            CG_CHUDConfigPickBgColor(prefixedConfig, result, qfalse);
            return;
        }
    }
    
    // Return default background color
    CG_CHUDConfigPickBgColor(config, result, qfalse);
}

void CG_CHUDSetConfigByType(const char* type, const cherryhudConfig_t* config) {
    cherryhudGlobalContext_t* context;
    int i;
    
    if (!type || !config) {
        return;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    for (i = 0; i < context->config.configCount; i++) {
        if (Q_stricmp(context->config.configs[i].type, type) == 0) {
            memcpy(&context->config.configs[i].config, config, sizeof(cherryhudConfig_t));
            context->config.configs[i].loaded = qtrue;
            return;
        }
    }
    
    if (context->config.configCount < MAX_CONFIG_TYPES) {
        Q_strncpyz(context->config.configs[context->config.configCount].type, type, MAX_QPATH);
        memcpy(&context->config.configs[context->config.configCount].config, config, sizeof(cherryhudConfig_t));
        context->config.configs[context->config.configCount].loaded = qtrue;
        context->config.configCount++;
    }
}

const cherryhudConfig_t* CG_CHUDGetConfigByType(const char* type) {
    cherryhudGlobalContext_t* context;
    int i;
    
    if (!type) {
        return NULL;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return NULL;
    }
    
    for (i = 0; i < context->config.configCount; i++) {
        if (Q_stricmp(context->config.configs[i].type, type) == 0 && context->config.configs[i].loaded) {
            return &context->config.configs[i].config;
        }
    }
    
    return NULL;
}

qboolean CG_CHUDIsConfigLoadedByType(const char* type) {
    cherryhudGlobalContext_t* context;
    int i;
    
    if (!type) {
        return qfalse;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return qfalse;
    }
    
    for (i = 0; i < context->config.configCount; i++) {
        if (Q_stricmp(context->config.configs[i].type, type) == 0) {
            return context->config.configs[i].loaded;
        }
    }
    
    return qfalse;
}

void CG_CHUDClearConfig(cherryhudConfig_t* config) {
    if (!config) return;
    
    // Clear all config fields to prevent memory leaks
    memset(config, 0, sizeof(cherryhudConfig_t));
}

void CG_CHUDClearConfigByType(const char* type) {
    cherryhudGlobalContext_t* context;
    int i;
    
    if (!type) {
        return;
    }
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    // Find and clear config by type
    for (i = 0; i < context->config.configCount; i++) {
        if (Q_stricmp(context->config.configs[i].type, type) == 0) {
            context->config.configs[i].loaded = qfalse;
            return;
        }
    }
}

void CG_CHUDClearAllConfigs(void) {
    cherryhudGlobalContext_t* context;
    int i;
    
    context = CG_CHUDGetContext();
    if (!context) {
        return;
    }
    
    for (i = 0; i < context->config.configCount; i++) {
        context->config.configs[i].loaded = qfalse;
        // Clear the config structure to prevent stale data
        memset(&context->config.configs[i].config, 0, sizeof(cherryhudConfig_t));
    }
    context->config.configCount = 0;
    
    context->config.scoreboardLoaded = qfalse;
    context->config.defaultLoaded = qfalse;
    context->config.compactLoaded = qfalse;
    context->config.doubleLoaded = qfalse;
    
    // Clear the main config structures
    memset(&context->config.scoreboard, 0, sizeof(cherryhudConfig_t));
    memset(&context->config.defaultConfig, 0, sizeof(cherryhudConfig_t));
    memset(&context->config.compactConfig, 0, sizeof(cherryhudConfig_t));
    memset(&context->config.doubleConfig, 0, sizeof(cherryhudConfig_t));
    
    // Reset prefixed configs
    g_prefixedConfigCount = 0;
    memset(g_prefixedConfigs, 0, sizeof(g_prefixedConfigs));
}

void CG_CHUDLoadDefaultTemplates(void) {
    cherryhudConfig_t defaultPlayerRow;
    cherryhudConfig_t defaultSpectatorRow;
    cherryhudConfig_t defaultTitleRow;
    cherryhudConfig_t defaultElement;

    
    // Create default playerRow template
    memset(&defaultPlayerRow, 0, sizeof(defaultPlayerRow));
    defaultPlayerRow.size.value[0] = 400.0f;
    defaultPlayerRow.size.value[1] = 20.0f;
    defaultPlayerRow.size.isSet = qtrue;
    defaultPlayerRow.pos.value[0] = 0.0f;
    defaultPlayerRow.pos.value[1] = 0.0f;
    defaultPlayerRow.pos.isSet = qtrue;
    defaultPlayerRow.rowSpacing.value = 2.0f;
    defaultPlayerRow.rowSpacing.isSet = qtrue;
    CG_CHUDSetPlayerRowTemplate(&defaultPlayerRow);
    
    // Create default spectatorRow template
    memset(&defaultSpectatorRow, 0, sizeof(defaultSpectatorRow));
    defaultSpectatorRow.size.value[0] = 400.0f;
    defaultSpectatorRow.size.value[1] = 20.0f;
    defaultSpectatorRow.size.isSet = qtrue;
    defaultSpectatorRow.pos.value[0] = 0.0f;
    defaultSpectatorRow.pos.value[1] = 0.0f;
    defaultSpectatorRow.pos.isSet = qtrue;
    defaultSpectatorRow.rowSpacing.value = 2.0f;
    defaultSpectatorRow.rowSpacing.isSet = qtrue;
    CG_CHUDSetSpectatorRowTemplate(&defaultSpectatorRow);
    
    // Create default titleRow template
    memset(&defaultTitleRow, 0, sizeof(defaultTitleRow));
    defaultTitleRow.size.value[0] = 400.0f;
    defaultTitleRow.size.value[1] = 20.0f;
    defaultTitleRow.size.isSet = qtrue;
    defaultTitleRow.pos.value[0] = 0.0f;
    defaultTitleRow.pos.value[1] = 0.0f;
    defaultTitleRow.pos.isSet = qtrue;
    defaultTitleRow.rowSpacing.value = 2.0f;
    defaultTitleRow.rowSpacing.isSet = qtrue;
    CG_CHUDSetTitleRowTemplate(&defaultTitleRow);
    
    // Create default elements for player rows
    memset(&defaultElement, 0, sizeof(defaultElement));
    defaultElement.size.value[0] = 100.0f;
    defaultElement.size.value[1] = 20.0f;
    defaultElement.size.isSet = qtrue;
    defaultElement.pos.value[0] = 0.0f;
    defaultElement.pos.value[1] = 0.0f;
    defaultElement.pos.isSet = qtrue;
    Q_strncpyz(defaultElement.type.value, "decor", sizeof(defaultElement.type.value));
    defaultElement.type.isSet = qtrue;
    
    // Note: Default player row elements are only added if no user elements are defined
    // This is handled in CG_CHUDTableCreateRow when templateCount == 0
    
}

void CG_CHUDResetSystem(void) {
    int i;
    
    // idk why but this is need to be
    CG_CHUDClearTitleBlocks();
    
    // Clear all configurations
    CG_CHUDClearAllConfigs();
    
    // Clear all templates
    CG_CHUDClearPlayerInfoTemplates();
    
    // // Clear title blocks
    // CG_CHUDClearTitleBlocks();
    
    // Clear configuration storage
    CG_CHUDConfigStorageClear();
    
    // Register element handlers (moved from removed Dictionary system)
    CG_CHUDRegisterElementHandlers();
    
    // Shutdown and reset table system first
    CG_CHUDTableShutdown();
    
    // Destroy all windows and their elements to prevent memory leaks
    CG_CHUDDestroyAllWindows();
    
    // Shutdown layout system
    CG_CHUDLayoutShutdown();
    
    // CRITICAL: Reset parser static variables to prevent memory leaks
    CG_CHUDLexerParserResetStatics();
    
    // CRITICAL: Reset element counter to prevent memory overflow
    // This resets the static elementCount variable in CG_CHUDCreateElementFromConfig
    // We need to access it through a function call since it's static
    CG_CHUDResetElementCounter();
    
    // CRITICAL: Force memory cleanup to prevent fragmentation
    // This helps reduce memory fragmentation by forcing garbage collection
    if (Z_AvailableMemory() < 100000) { // If less than 100KB available
        // Force a more aggressive cleanup by clearing all caches
        CG_CHUDClearAllConfigs();
        CG_CHUDClearPlayerInfoTemplates();
        CG_CHUDClearTitleBlocks();
        CG_CHUDConfigStorageClear();
    }
    
    // Reset static initialization flags
    // Note: This requires access to internal static variables
    // We'll handle this in the table init function
    
}

const cherryhudConfig_t* CG_CHUDGetPlayerInfoTemplate(int index) {
    if (index >= 0 && index < playerInfoTemplateCount) {
        return &playerInfoTemplate[index];
    }
    return NULL;
}

int CG_CHUDGetPlayerInfoTemplateCount(void) {
    return playerInfoTemplateCount;
}

void CG_CHUDAddPlayerRowElement(const cherryhudConfig_t* config) {
    char key[MAX_QPATH];
    cherryhudConfig_t modifiedConfig;
    
    if (!config) {
        return;
    }
    
    Com_sprintf(key, sizeof(key), "playerRow_element_%d", g_playerRowElementIndex++);
    
    
    // Check if this should be a weaponInfo element
    if (config->weapon.isSet) {
        // Copy the config and modify it to be a weaponInfo element
        memcpy(&modifiedConfig, config, sizeof(cherryhudConfig_t));
        
        // Set the type to weaponInfo
        Q_strncpyz(modifiedConfig.type.value, "weaponInfo", sizeof(modifiedConfig.type.value));
        modifiedConfig.type.isSet = qtrue;
        
        CG_CHUDConfigSet(key, CONFIG_TYPE_PLAYER_ROW_ELEMENT, &modifiedConfig, g_playerRowElementIndex - 1);
    } else {
        CG_CHUDConfigSet(key, CONFIG_TYPE_PLAYER_ROW_ELEMENT, config, g_playerRowElementIndex - 1);
    }
    
}

void CG_CHUDAddSpectatorRowElement(const cherryhudConfig_t* config) {
    char key[MAX_QPATH];
    
    if (!config) {
        return;
    }
    
    Com_sprintf(key, sizeof(key), "spectatorRow_element_%d", g_spectatorRowElementIndex++);
    
    CG_CHUDConfigSet(key, CONFIG_TYPE_SPECTATOR_ROW_ELEMENT, config, g_spectatorRowElementIndex - 1);
}

const cherryhudConfig_t* CG_CHUDGetPlayerRowElement(int index) {
    configEntry_t* entries[32];
    int count;
    
    count = CG_CHUDConfigGetAllOfType(CONFIG_TYPE_PLAYER_ROW_ELEMENT, entries, 32);
    
    
    if (index >= 0 && index < count) {
        return &entries[index]->config;
    }
    return NULL;
}

const cherryhudConfig_t* CG_CHUDGetSpectatorRowElement(int index) {
    configEntry_t* entries[32];
    int count;
    
    count = CG_CHUDConfigGetAllOfType(CONFIG_TYPE_SPECTATOR_ROW_ELEMENT, entries, 32);
    
    if (index >= 0 && index < count) {
        return &entries[index]->config;
    }
    return NULL;
}

int CG_CHUDGetPlayerRowElementCount(void) {
    configEntry_t* entries[32];
    int count;
    
    count = CG_CHUDConfigGetAllOfType(CONFIG_TYPE_PLAYER_ROW_ELEMENT, entries, 32);
    return count;
}

int CG_CHUDGetSpectatorRowElementCount(void) {
    configEntry_t* entries[32];
    return CG_CHUDConfigGetAllOfType(CONFIG_TYPE_SPECTATOR_ROW_ELEMENT, entries, 32);
}

void CG_CHUDClearPlayerInfoTemplates(void) {
    int i;
    
    CG_CHUDConfigSystemInit();
    
    // Clear all player info templates
    for (i = 0; i < playerInfoTemplateCount; i++) {
        memset(&playerInfoTemplate[i], 0, sizeof(cherryhudConfig_t));
    }
    
    playerRowTemplateSet = qfalse;
    spectatorRowTemplateSet = qfalse;
    titleRowTemplateSet = qfalse;
    playerInfoTemplateCount = 0;
    titleRowElementCount = 0;
    titleBlockCount = 0;
    
    // Reset element index counters
    CG_CHUDResetElementIndexes();
    
}


void CG_CHUDResetElementIndexes(void) {
    // Reset global element index counters
    
    g_playerRowElementIndex = 0;
    g_spectatorRowElementIndex = 0;
    g_titleRowElementIndex = 0;
    
}



// Unused function removed - it was part of old parsing system

// Title block management functions
void CG_CHUDCreateTitleBlock(const cherryhudConfig_t* config) {
    cherryhudTitleBlock_t* block;
    const cherryhudConfig_t* scoreboardConfig;
    
    if (!config || titleBlockCount >= MAX_TITLE_BLOCKS) {
        return;
    }
    
    // Create new title block
    block = &titleBlocks[titleBlockCount];
    memset(block, 0, sizeof(cherryhudTitleBlock_t));
    
    // Copy config first
    memcpy(&block->config, config, sizeof(cherryhudConfig_t));
    

    // Apply defaults for properties that are still not set FIRST
    CG_CHUDConfigDefaultsCheck(&block->config);

    // Apply inheritance from scoreboard config AFTER applying defaults
    // This ensures that title blocks can inherit size and other properties from scoreboard
    // But NOT position - title blocks should have their own relative positioning
    {
        const cherryhudConfig_t* scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig) {
            // Do NOT inherit position - title blocks should have their own relative positioning
            // Position will be calculated during rendering based on scoreboard order
            // Clear any inherited position to prevent scoreboard position inheritance
            block->config.pos.isSet = qfalse;

            // Inherit size if not set in title block
            // But for title blocks, we should use their own size from config
            // Only inherit if the title block has no size specified at all
            if (!block->config.size.isSet && scoreboardConfig->size.isSet) {
                // For title blocks, don't inherit size from scoreboard
                // They should have their own size specified in config
                // block->config.size = scoreboardConfig->size;
            }

            // Inherit other properties as needed
            if (!block->config.color.isSet && scoreboardConfig->color.isSet) {
                block->config.color = scoreboardConfig->color;
            }
            if (!block->config.font.isSet && scoreboardConfig->font.isSet) {
                block->config.font = scoreboardConfig->font;
            }
            if (!block->config.textExt.size.isSet && scoreboardConfig->textExt.size.isSet) {
                block->config.textExt.size = scoreboardConfig->textExt.size;
            }
            if (!block->config.textExt.color.isSet && scoreboardConfig->textExt.color.isSet) {
                block->config.textExt.color = scoreboardConfig->textExt.color;
            }
            if (!block->config.textExt.color2.isSet && scoreboardConfig->textExt.color2.isSet) {
                block->config.textExt.color2 = scoreboardConfig->textExt.color2;
            }
        }
    }
    

    
    // Width inheritance: if title width is 0, inherit from scoreboard
    if (block->config.size.isSet && block->config.size.value[0] == 0.0f) {
        scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig && scoreboardConfig->size.isSet && scoreboardConfig->size.value[0] > 0.0f) {
            block->config.size.value[0] = scoreboardConfig->size.value[0];
        }
    }
    
    // Height should come only from config, no inheritance
    
    block->elementCount = 0;
    
    // Initialize container association
    block->containerType[0] = '\0';
    block->isContainerTitle = qfalse;
    
    titleBlockCount++;
    
}

void CG_CHUDCreateTitleBlockForContainer(const cherryhudConfig_t* config, const char* containerType) {
    cherryhudTitleBlock_t* block;
    const cherryhudConfig_t* scoreboardConfig;
    
    if (!config) {
        return;
    }
    
    if (titleBlockCount >= MAX_TITLE_BLOCKS) {
        int clearCount = MAX_TITLE_BLOCKS / 2;
        int i;
        // CRITICAL: Title blocks array overflow - clear old entries to prevent memory leak
        
        // Clear half of the old entries to make space

        for (i = 0; i < clearCount; i++) {
            titleBlocks[i] = titleBlocks[clearCount + i];
        }
        titleBlockCount = clearCount;
    }
    
    // Create new title block
    block = &titleBlocks[titleBlockCount];
    memset(block, 0, sizeof(cherryhudTitleBlock_t));
    
    
    // Copy config first
    memcpy(&block->config, config, sizeof(cherryhudConfig_t));
    
    // Apply defaults for properties that are still not set FIRST
    CG_CHUDConfigDefaultsCheck(&block->config);
    
    // Apply inheritance from scoreboard config AFTER applying defaults
    {
        const cherryhudConfig_t* scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig) {
            // Do NOT inherit position - title blocks should have their own relative positioning
            block->config.pos.isSet = qfalse;

            // Inherit other properties as needed
            if (!block->config.color.isSet && scoreboardConfig->color.isSet) {
                block->config.color = scoreboardConfig->color;
            }
            if (!block->config.font.isSet && scoreboardConfig->font.isSet) {
                block->config.font = scoreboardConfig->font;
            }
            if (!block->config.textExt.size.isSet && scoreboardConfig->textExt.size.isSet) {
                block->config.textExt.size = scoreboardConfig->textExt.size;
            }
            if (!block->config.textExt.color.isSet && scoreboardConfig->textExt.color.isSet) {
                block->config.textExt.color = scoreboardConfig->textExt.color;
            }
            if (!block->config.textExt.color2.isSet && scoreboardConfig->textExt.color2.isSet) {
                block->config.textExt.color2 = scoreboardConfig->textExt.color2;
            }
        }
    }
    
    // Width inheritance: if title width is 0, inherit from scoreboard
    if (block->config.size.isSet && block->config.size.value[0] == 0.0f) {
        scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig && scoreboardConfig->size.isSet && scoreboardConfig->size.value[0] > 0.0f) {
            block->config.size.value[0] = scoreboardConfig->size.value[0];
        }
    }
    
    block->elementCount = 0;
    
    // Set container association
    if (containerType) {
        Q_strncpyz(block->containerType, containerType, sizeof(block->containerType));
        block->isContainerTitle = qtrue;
    } else {
        block->containerType[0] = '\0';
        block->isContainerTitle = qfalse;
    }
    
    titleBlockCount++;
    
}

int CG_CHUDGetTitleBlockCount(void) {
    return titleBlockCount;
}

qboolean CG_CHUDTitleBlockHasContainer(const cherryhudTitleBlock_t* titleBlock) {
    if (!titleBlock) return qfalse;
    return titleBlock->isContainerTitle && titleBlock->containerType[0] != '\0';
}

qboolean CG_CHUDContainerHasContent(const char* containerType) {
    cherryhudTable_t* table;
    int playerCount;
    int spectatorCount;
    const char* tableType;
    const char* playersContainerName;
    const char* spectatorsContainerName;
    
    if (!containerType) return qfalse;
    
    playersContainerName = CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS);
    spectatorsContainerName = CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS);
    
    // Use old table management functions
    if (Q_stricmp(containerType, playersContainerName) == 0) {
        // Check players count using old API
        playerCount = CG_CHUDGetPlayerCount();
        return (playerCount > 0);
    } else if (Q_stricmp(containerType, spectatorsContainerName) == 0) {
        // Check spectators count using old API
        spectatorCount = CG_CHUDGetSpectatorCount();
        return (spectatorCount > 0);
    }
    
    // Generic fallback - check count for any container type
    playerCount = CG_CHUDGetPlayerCount();
    return (playerCount > 0);
}

const cherryhudTitleBlock_t* CG_CHUDGetTitleBlock(int index) {
    if (index >= 0 && index < titleBlockCount) {
        return &titleBlocks[index];
    }
    return NULL;
}


void CG_CHUDUpdateTitleBlockConfig(int blockIndex, const cherryhudConfig_t* config) {
    cherryhudTitleBlock_t* block;
    
    if (!config || blockIndex < 0 || blockIndex >= titleBlockCount) {
        return;
    }
    
    block = &titleBlocks[blockIndex];
    
    // Update the title block configuration
    memcpy(&block->config, config, sizeof(cherryhudConfig_t));
    
}

void CG_CHUDAddTitleBlockElement(int blockIndex, const cherryhudConfig_t* config) {
    cherryhudTitleBlock_t* block;
    cherryhudElement_t* newElement;
    cherryhudConfig_t elementConfig;
    
    if (!config || blockIndex < 0 || blockIndex >= titleBlockCount) {
        return;
    }
    
    block = &titleBlocks[blockIndex];
    if (block->elementCount >= MAX_TITLE_ELEMENTS) {
        int clearCount = MAX_TITLE_ELEMENTS / 2;
        int i;
        // CRITICAL: Title block elements array overflow - clear old entries to prevent memory leak
        
        // Clear half of the old entries to make space

        for (i = 0; i < clearCount; i++) {
            if (block->elements[i]) {
                CG_CHUDDestroyElement(block->elements[i]);
                block->elements[i] = NULL;
            }
            block->elements[i] = block->elements[clearCount + i];
            block->elements[clearCount + i] = NULL;
        }
        block->elementCount = clearCount;
    }
    
    // Copy config and set default positioning for title elements
    memcpy(&elementConfig, config, sizeof(cherryhudConfig_t));
    
    // Set default positioning for title elements if not set
    if (!elementConfig.pos.isSet) {
        // Calculate X position based on element index (create columns)
        float elementWidth = elementConfig.size.isSet ? elementConfig.size.value[0] : 100.0f;
        float elementSpacing = 2.0f; // Default spacing between elements
        
        // Calculate X position with spacing
        elementConfig.pos.value[0] = block->elementCount * (elementWidth + elementSpacing);
        elementConfig.pos.value[1] = 2.0f; // Small margin from top edge
        elementConfig.pos.isSet = qtrue;
        
    } else {
    }
    
    // Create the element from configuration
    newElement = CG_CHUDCreateElementFromConfig(&elementConfig, -1, NULL);
    if (newElement) {
        block->elements[block->elementCount] = newElement;
        block->elementCount++;
    } else {
    }
}

// Configuration storage management functions
void CG_CHUDConfigStorageInit(void) {
    memset(&g_configStorage, 0, sizeof(g_configStorage));
    g_configStorage.isParsed = qfalse;
}

void CG_CHUDResetTitleElementIndex(void) {
    g_titleElementIndex = 0;
}

void CG_CHUDResetTitleBlockCount(void) {
    titleBlockCount = 0;
}

void CG_CHUDConfigStorageClear(void) {
    int i;
    
    
    // Clear all element configs to prevent stale references
    for (i = 0; i < g_configStorage.elementCount; i++) {
        memset(&g_configStorage.elements[i], 0, sizeof(cherryhudElementConfig_t));
    }
    
    memset(&g_configStorage, 0, sizeof(g_configStorage));
    g_configStorage.isParsed = qfalse;
    
    // Reset static variables used in CG_CHUDConfigStorageProcessElements
    // This is necessary to ensure proper element distribution on config reload
    CG_CHUDResetTitleElementIndex();
}

void CG_CHUDConfigStorageAddElement(const char* elementName, const cherryhudConfig_t* config, int context) {
    if (g_configStorage.elementCount < (MAX_TITLE_ELEMENTS * 4)) {
        cherryhudElementConfig_t* elementConfig = &g_configStorage.elements[g_configStorage.elementCount];
        
        Q_strncpyz(elementConfig->elementName, elementName, sizeof(elementConfig->elementName));
        memcpy(&elementConfig->config, config, sizeof(cherryhudConfig_t));
        elementConfig->context = context;
        elementConfig->order = g_configStorage.elementCount;
        
        g_configStorage.elementCount++;
    } else {
        int clearCount = (MAX_TITLE_ELEMENTS * 4) / 2;
        int i;
        cherryhudElementConfig_t* elementConfig;
        // CRITICAL: Config storage array overflow - clear old entries to prevent memory leak
        
        // Clear half of the old entries to make space

        for (i = 0; i < clearCount; i++) {
            g_configStorage.elements[i] = g_configStorage.elements[clearCount + i];
        }
        g_configStorage.elementCount = clearCount;
        
        // Add the new element
        elementConfig = &g_configStorage.elements[g_configStorage.elementCount];
        Q_strncpyz(elementConfig->elementName, elementName, sizeof(elementConfig->elementName));
        memcpy(&elementConfig->config, config, sizeof(cherryhudConfig_t));
        elementConfig->context = context;
        elementConfig->order = g_configStorage.elementCount;
        g_configStorage.elementCount++;
    }
}

cherryhudConfigStorage_t* CG_CHUDGetConfigStorage(void) {
    return &g_configStorage;
}

void CG_CHUDConfigStorageProcessElements(void) {
    int i;
    const cherryhudElementTypeInfo_t* typeInfo;
    
    
    for (i = 0; i < g_configStorage.elementCount; i++) {
        cherryhudElementConfig_t* elementConfig = &g_configStorage.elements[i];
        
        // Determine element type for better processing
        typeInfo = CG_CHUDGetElementTypeInfo(elementConfig->elementName);
        if (!typeInfo) {
            // Try auto-detection if element name not found
            typeInfo = CG_CHUDAutoDetectElementType(&elementConfig->config);
        }
        
        // Process based on context and element type
        if (elementConfig->context == CHERRYHUD_CONTEXT_PLAYER_ROW) {
            // Player row context
            CG_CHUDAddPlayerRowElement(&elementConfig->config);
        } else if (elementConfig->context == CHERRYHUD_CONTEXT_SPECTATOR_ROW) {
            // Spectator row context
            CG_CHUDAddSpectatorRowElement(&elementConfig->config);
        } else if (elementConfig->context == CHERRYHUD_CONTEXT_TITLE) {
            // Title context - elements are now added directly during parsing
            // This should not happen anymore since we changed the parsing logic
        } else {
            // Default context - use player info template for all elements
            CG_CHUDSetPlayerInfoTemplate(&elementConfig->config);
        }
    }
    
    g_configStorage.isParsed = qtrue;
}

// Universal function to get element type info
const cherryhudElementTypeInfo_t* CG_CHUDGetElementTypeInfo(const char* elementName) {
    int i;
    
    if (!elementName) return NULL;
    
    for (i = 0; g_unifiedElementTypes[i].name; i++) {
        if (Q_stricmp(elementName, g_unifiedElementTypes[i].name) == 0) {
            return &g_unifiedElementTypes[i];
        }
    }
    
    return NULL;
}

// Function to get element type info by bonus flag

// Function to auto-detect element type based on config
const cherryhudElementTypeInfo_t* CG_CHUDAutoDetectElementType(const cherryhudConfig_t* config) {
    const cherryhudElementTypeInfo_t* typeInfo;
    int i;
    
    if (!config) return NULL;
    
    
    /* First, check if type is explicitly specified */
    if (config->type.isSet) {
        typeInfo = CG_CHUDGetElementTypeInfo(config->type.value);
        if (typeInfo && typeInfo->create && typeInfo->routine && typeInfo->destroy) {
            return typeInfo;
        }
        /* If specified type is invalid, continue with auto-detection */
    }
    
    /* Auto-detect based on content - single pass through all elements */
    for (i = 0; g_unifiedElementTypes[i].name; i++) {
        typeInfo = &g_unifiedElementTypes[i];
        
        /* Skip non-element types */
        if (typeInfo->category != CHERRYHUD_TYPE_ELEMENT) {
            continue;
        }
        
        /* Skip if missing required functions */
        if (!typeInfo->create || !typeInfo->routine || !typeInfo->destroy) {
            continue;
        }
        
        /* Check for elements with AUTO_DETECT flag */
        if (typeInfo->bonusFlag & CHERRYHUD_FLAG_AUTO_DETECT) {
            return typeInfo;
        }
    }
    
    /* Final fallback - return NULL to indicate no suitable type found */
    return NULL;
}

// Element type management functions
qboolean CG_CHUDIsElementType(const char* elementName) {
    const cherryhudElementTypeInfo_t* info = CG_CHUDGetElementTypeInfo(elementName);
    return (info && info->category == CHERRYHUD_TYPE_ELEMENT) ? qtrue : qfalse;
}

qboolean CG_CHUDIsContainerType(const char* elementName) {
    const cherryhudElementTypeInfo_t* info = CG_CHUDGetElementTypeInfo(elementName);
    return (info && info->category == CHERRYHUD_TYPE_CONTAINER) ? qtrue : qfalse;
}

qboolean CG_CHUDIsConfigType(const char* elementName) {
    const cherryhudElementTypeInfo_t* info = CG_CHUDGetElementTypeInfo(elementName);
    if (info && info->category == CHERRYHUD_TYPE_CONFIG) {
        return qtrue;
    }
    
    // Check for dynamic config types (starting with !) but exclude registered ones
    if (elementName && elementName[0] == '!') {
        // Exclude !scoreboard and !default which are registered with specific types
        if (Q_stricmp(elementName, "!scoreboard") == 0 || Q_stricmp(elementName, "!default") == 0) {
            return qfalse;
        }
        return qtrue;
    }
    
    return qfalse;
}

qboolean CG_CHUDIsBlockType(const char* elementName) {
    const cherryhudElementTypeInfo_t* info = CG_CHUDGetElementTypeInfo(elementName);
    return (info && info->category == CHERRYHUD_TYPE_BLOCK) ? qtrue : qfalse;
}



// Helper function to map cherryhudBlockType_t enum to context value
static int CG_CHUDBlockTypeToContext(cherryhudBlockType_t type) {
    switch (type) {
        case CHERRYHUD_BLOCK_TYPE_TITLE:
            return CHERRYHUD_CONTEXT_TITLE;
        case CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS:
            return CHERRYHUD_CONTEXT_PLAYER_ROW;
        case CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS:
            return CHERRYHUD_CONTEXT_SPECTATOR_ROW;
        default:
            return 0;
    }
}

// Helper functions for working with bonus flags
qboolean CG_CHUDIsTemplateBlock(int bonusFlag) {
    return (bonusFlag & CHERRYHUD_FLAG_TEMPLATE) != 0;
}


// Helper function to get container name from block type
const char* CG_CHUDGetContainerName(cherryhudBlockType_t type) {
    int i;
    int contextValue;
    
    // Convert enum to context value
    contextValue = CG_CHUDBlockTypeToContext(type);
    
    // Search for container (not template) with matching context
    for (i = 0; g_unifiedElementTypes[i].name; i++) {
        if (g_unifiedElementTypes[i].category == CHERRYHUD_TYPE_CONTAINER &&
            (g_unifiedElementTypes[i].bonusFlag & (CHERRYHUD_CONTEXT_PLAYER_ROW | CHERRYHUD_CONTEXT_SPECTATOR_ROW | CHERRYHUD_CONTEXT_TITLE)) == contextValue) {
            return g_unifiedElementTypes[i].name;
        }
    }
    
    // If not found, return unknown
    return "unknown";
}


// Helper function to build arrays from unified types
static void CG_CHUDBuildTypeArray(cherryhudElementTypeCategory_t category, const char** array, int maxSize) {
    int i, j;
    
    j = 0;
    for (i = 0; g_unifiedElementTypes[i].name && j < maxSize - 1; i++) {
        if (g_unifiedElementTypes[i].category == category) {
            array[j] = g_unifiedElementTypes[i].name;
            j++;
        }
    }
    array[j] = NULL;
}



// Element handler functions
void CG_CHUDHandlePlayerRow(const char* elementName, cherryhudConfig_t* config) {
    CG_CHUDSetPlayerRowTemplate(config);
}

void CG_CHUDHandleSpectatorRow(const char* elementName, cherryhudConfig_t* config) {
    CG_CHUDSetSpectatorRowTemplate(config);
}

void CG_CHUDHandleTitle(const char* elementName, cherryhudConfig_t* config) {
    // Title block already created in parser
}

void CG_CHUDHandlePlayersRows(const char* elementName, cherryhudConfig_t* config) {
    // Create separate table for players only
    cherryhudTable_t* playersTable = CG_CHUDTableCreate(config);
    if (playersTable) {
        // Set table type to players only
        Q_strncpyz(playersTable->tableType, "players", sizeof(playersTable->tableType));
        // Store in TableManager
        {
            cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
            if (manager) {
                manager->playersTable = playersTable;
            }
        }
    }
    CG_CHUDScoreboardOrderAdd(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS, config);
}

void CG_CHUDHandleSpectatorsRows(const char* elementName, cherryhudConfig_t* config) {
    // Create separate table for spectators only
    cherryhudTable_t* spectatorsTable = CG_CHUDTableCreate(config);
    if (spectatorsTable) {
        // Set table type to spectators only
        Q_strncpyz(spectatorsTable->tableType, "spectators", sizeof(spectatorsTable->tableType));
        // Store in TableManager
        {
            cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
            if (manager) {
                manager->spectatorsTable = spectatorsTable;
            }
        }
    } else {
    }
    CG_CHUDScoreboardOrderAdd(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS, config);
}

void CG_CHUDHandleScoreboardConfig(const char* elementName, cherryhudConfig_t* config) {
    CG_CHUDSetScoreboardConfig(config);
    CG_CHUDSetConfigByType("scoreboard", config);
    g_scoreboardBlockDepth++;
}

void CG_CHUDHandleTableConfig(const char* elementName, cherryhudConfig_t* config) {
    // For !table, we need to extract the table name from elementName
    // elementName will be like "!table" or "!table.mytable"
    char tableName[MAX_QPATH];
    char* dotPos;
    
    // Default table name is "default"
    Q_strncpyz(tableName, "default", sizeof(tableName));
    
    // Check if there's a dot in the element name (e.g., "!table.mytable")
    dotPos = strchr(elementName, '.');
    if (dotPos) {
        // Extract the table name after the dot
        Q_strncpyz(tableName, dotPos + 1, sizeof(tableName));
    }
    
    // Set the config for this specific table
    CG_CHUDSetConfigByType(tableName, config);
    
    // Initialize table by type if it doesn't exist
    CG_CHUDTableInitByType(tableName);
    
    g_scoreboardBlockDepth++;
}

void CG_CHUDHandleDefaultConfig(const char* elementName, cherryhudConfig_t* config) {
    // SuperHUD-style behavior: !default sets current default for all following elements
    // !default{} (empty) clears the current default
    
    // Check if this is an empty default (no properties set)
    qboolean isEmptyDefault = qtrue;
    
    // Check if any properties are set in the config
    if (config->pos.isSet || config->size.isSet || config->color.isSet || 
        config->color2.isSet || config->text.isSet || config->font.isSet ||
        config->textExt.size.isSet || config->textExt.alignH.isSet || 
        config->textExt.alignV.isSet || config->visflags.isSet || 
        config->hideflags.isSet || config->type.isSet || config->time.isSet ||
        config->hlsize.isSet || config->hlcolor.isSet || config->textOffset.isSet ||
        config->fade.isSet || config->fadedelay.isSet || config->fill.isSet ||
        config->image.isSet || config->imagetc.isSet || config->itTeam.isSet ||
        config->margins.isSet || config->model.isSet || config->monospace.isSet ||
        config->offset.isSet || config->rowSpacing.isSet || config->style.isSet ||
        config->doublebar.isSet || config->direction.isSet || config->alignH.isSet ||
        config->alignV.isSet || config->angles.isSet || config->background.isSet ||
        config->border.isSet || config->hlcolor.isSet || config->maxWidth.isSet ||
        config->textBgColor.isSet || config->textBorder.isSet || config->textBorderColor.isSet ||
        config->imageBgColor.isSet || config->imageBorder.isSet || config->imageBorderColor.isSet ||
        config->weapon.isSet || config->extendedType.isSet || config->tableSpacing.isSet ||
        config->textExt.pos.isSet || config->textExt.color.isSet || config->textExt.color2.isSet ||
        config->textExt.background.isSet || config->textExt.border.isSet || 
        config->textExt.bordercolor.isSet || config->textExt.shadow.isSet ||
        config->textExt.shadowColor.isSet || config->textExt.font.isSet ||
        config->textExt.format.isSet || config->maxValue.isSet) {
        isEmptyDefault = qfalse;
    }
    
    if (isEmptyDefault) {
        // Empty !default{} - clear current default
        CG_CHUDSetCurrentDefaultConfig(NULL);
    } else {
        // Non-empty !default - set as current default
        CG_CHUDSetCurrentDefaultConfig(config);
    }
    
    // Also store in global default for backward compatibility
    CG_CHUDSetDefaultConfig(config);
}

void CG_CHUDHandleCustomConfig(const char* elementName, cherryhudConfig_t* config) {
    const char* type = elementName + 1; // Skip '!' character
    CG_CHUDSetConfigByType(type, config);
}

// Register all element handlers
void CG_CHUDRegisterElementHandlers(void) {
    int i;
    
    g_elementHandlerCount = 0;
    
    // Register handlers from unified element types
    for (i = 0; g_unifiedElementTypes[i].name && g_elementHandlerCount < MAX_ELEMENT_HANDLERS; i++) {
        if (g_unifiedElementTypes[i].isHandler && g_unifiedElementTypes[i].handler) {
            g_elementHandlers[g_elementHandlerCount].name = g_unifiedElementTypes[i].name;
            
            // Determine handler type based on category
            switch (g_unifiedElementTypes[i].category) {
                case CHERRYHUD_TYPE_BLOCK:
                    g_elementHandlers[g_elementHandlerCount].type = CHERRYHUD_HANDLER_CONTAINER;
                    break;
                case CHERRYHUD_TYPE_CONTAINER:
                    g_elementHandlers[g_elementHandlerCount].type = CHERRYHUD_HANDLER_CONTAINER;
                    break;
                case CHERRYHUD_TYPE_CONFIG:
                    g_elementHandlers[g_elementHandlerCount].type = CHERRYHUD_HANDLER_CONFIG;
                    break;
                default:
                    g_elementHandlers[g_elementHandlerCount].type = CHERRYHUD_HANDLER_ELEMENT;
                    break;
            }
            
            g_elementHandlers[g_elementHandlerCount].handler = g_unifiedElementTypes[i].handler;
            g_elementHandlerCount++;
        }
    }
}

cherryhudElementHandler_t* CG_CHUDFindElementHandler(const char* elementName) {
    int i;
    
    if (!elementName) return NULL;
    
    // First, check registered handlers (including !scoreboard, !default, etc.)
    for (i = 0; i < g_elementHandlerCount; i++) {
        if (Q_stricmp(g_elementHandlers[i].name, elementName) == 0) {
            return &g_elementHandlers[i];
        }
    }
    
    // Only if no registered handler found, check for dynamic config types (starting with !)
    // This excludes !scoreboard and !default which are already registered
    if (elementName[0] == '!') {
        static cherryhudElementHandler_t customHandler;
        // Create a temporary handler for custom config types
        
        customHandler.name = elementName;
        customHandler.type = CHERRYHUD_HANDLER_CONFIG;
        customHandler.handler = CG_CHUDHandleCustomConfig;
        return &customHandler;
    }
    
    return NULL;
}

void CG_CHUDHandleElement(const char* elementName, cherryhudConfig_t* config) {
    cherryhudElementHandler_t* handler;
    
    if (!elementName || !config) return;
    
    handler = CG_CHUDFindElementHandler(elementName);
    if (handler && handler->handler) {
        handler->handler(elementName, config);
    }
}




