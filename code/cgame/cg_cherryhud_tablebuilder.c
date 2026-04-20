#include "cg_cherryhud_private.h"
#include "cg_local.h"
#include "../qcommon/q_shared.h"  // For vec2_t and VectorCopy
#include "../qcommon/qcommon.h" 

// ============================================================================
// GLOBAL VARIABLES AND DATA STRUCTURES
// ============================================================================

// Global TableManager instance (replaces all global state)
static cherryhudTableManager_t* g_tableManager = NULL;

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

// Player data collection and sorting
static void CG_CHUDTableSortPlayers(cherryhudPlayerData_t* players, int numPlayers);
static int CG_CHUDTableComparePlayers(const void* a, const void* b);
static void CG_CHUDTableCollectPlayers(cherryhudPlayerData_t* players, int* numPlayers);
static void CG_CHUDTableCollectPlayersByType(cherryhudPlayerData_t* players, int* numPlayers, const char* tableType);

// Table row management
static void CG_CHUDTableCreateRow(cherryhudTable_t* table, int index, const cherryhudPlayerData_t* playerData);
static void CG_CHUDTableCreateRowElements(cherryhudTable_t* table, cherryhudTableRow_t* row, const cherryhudConfig_t* playerRowTemplate);
static void CG_CHUDTableDestroyRow(cherryhudTableRow_t* row);
static void CG_CHUDTableUpdateRow(cherryhudTable_t* table, int index, const cherryhudPlayerData_t* playerData);

// Table rendering helpers
static void CG_CHUDTableRenderBorder(cherryhudTable_t* table);

// Configuration helpers
static void CG_CHUDSetDefaultTableProperties(cherryhudTable_t* table);
static void CG_CHUDGetConfigWithFallback(cherryhudConfig_t* config, const char* type);

qboolean CG_CHUDTableRenderBackgroundAndBorder(cherryhudTable_t* table);
qboolean CG_CHUDTableRowRenderBackgroundAndBorder(cherryhudTable_t* table, cherryhudTableRow_t* row);

// ============================================================================
// TABLE MANAGER - REPLACES GLOBAL STATE
// ============================================================================

// Create TableManager instance
cherryhudTableManager_t* CG_CHUDTableManagerCreate(void) {
    cherryhudTableManager_t* manager;
    
    manager = (cherryhudTableManager_t*)Z_Malloc(sizeof(cherryhudTableManager_t));
    OSP_MEMORY_CHECK(manager);
    if (!manager) {
        return NULL;
    }
    
    memset(manager, 0, sizeof(cherryhudTableManager_t));
    
    // Initialize state
    manager->tableDirty = qtrue;
    manager->lastPlayerHash = 0;
    manager->tableTypeCount = 0;
    manager->blockOrderCounter = 0;
    
    // Allocate scoreboard order
    manager->scoreboardOrder = (cherryhudScoreboardOrder_t*)Z_Malloc(sizeof(cherryhudScoreboardOrder_t));
    OSP_MEMORY_CHECK(manager->scoreboardOrder);
    if (manager->scoreboardOrder) {
        memset(manager->scoreboardOrder, 0, sizeof(cherryhudScoreboardOrder_t));
    }
    
    return manager;
}

// Destroy TableManager instance
void CG_CHUDTableManagerDestroy(cherryhudTableManager_t* manager) {
    int i;
    
    if (!manager) return;
    
    // Destroy all tables
    if (manager->globalTable) {
        CG_CHUDTableDestroy(manager->globalTable);
        manager->globalTable = NULL;
    }
    if (manager->playersTable) {
        CG_CHUDTableDestroy(manager->playersTable);
        manager->playersTable = NULL;
    }
    if (manager->spectatorsTable) {
        CG_CHUDTableDestroy(manager->spectatorsTable);
        manager->spectatorsTable = NULL;
    }
    
    // Clear all table types
    for (i = 0; i < manager->tableTypeCount; i++) {
        if (manager->tablesByType[i].table) {
            CG_CHUDTableDestroy(manager->tablesByType[i].table);
            manager->tablesByType[i].table = NULL;
        }
        manager->tablesByType[i].initialized = qfalse;
    }
    manager->tableTypeCount = 0;
    
    // Free scoreboard order
    if (manager->scoreboardOrder) {
        Z_Free(manager->scoreboardOrder);
        manager->scoreboardOrder = NULL;
    }
    
    Z_Free(manager);
}

// Get global TableManager instance
cherryhudTableManager_t* CG_CHUDGetTableManager(void) {
    if (!g_tableManager) {
        g_tableManager = CG_CHUDTableManagerCreate();
    }
    return g_tableManager;
}

// ============================================================================
// SERVICE IMPLEMENTATIONS - SEPARATION OF CONCERNS
// ============================================================================

// Data Service Implementation
static void CG_CHUDDataServiceCollectData(cherryhudPlayerData_t* players, int* numPlayers, const char* tableType) {
    CG_CHUDTableCollectPlayerData(players, numPlayers, tableType);
}

static void CG_CHUDDataServiceSortData(cherryhudPlayerData_t* players, int numPlayers) {
    CG_CHUDTableSortPlayers(players, numPlayers);
}

static int CG_CHUDDataServiceCalculateHash(const cherryhudPlayerData_t* players, int numPlayers) {
    int hash = 0;
    int i;
    
    for (i = 0; i < numPlayers; i++) {
        hash ^= (players[i].clientNum << 16) ^ players[i].score ^ (players[i].ping & 0xFFFF);
    }
    
    return hash;
}

cherryhudDataService_t* CG_CHUDDataServiceCreate(void) {
    cherryhudDataService_t* service;
    
    service = (cherryhudDataService_t*)Z_Malloc(sizeof(cherryhudDataService_t));
    OSP_MEMORY_CHECK(service);
    if (!service) {
        return NULL;
    }
    
    service->collectData = CG_CHUDDataServiceCollectData;
    service->sortData = CG_CHUDDataServiceSortData;
    service->calculateHash = CG_CHUDDataServiceCalculateHash;
    
    return service;
}

void CG_CHUDDataServiceDestroy(cherryhudDataService_t* service) {
    if (service) {
        Z_Free(service);
    }
}

// Memory Service Implementation
static void CG_CHUDMemoryServiceManageMemory(cherryhudTable_t* table, int requiredRows) {
    CG_CHUDTableManageRows(table, requiredRows);
}

static void CG_CHUDMemoryServiceUpdateRows(cherryhudTable_t* table, const cherryhudPlayerData_t* players, int numPlayers) {
    CG_CHUDTableUpdateAllRows(table, players, numPlayers);
}

static void CG_CHUDMemoryServiceFinalizeTable(cherryhudTable_t* table) {
    CG_CHUDTableFinalize(table);
}

cherryhudMemoryService_t* CG_CHUDMemoryServiceCreate(void) {
    cherryhudMemoryService_t* service;
    
    service = (cherryhudMemoryService_t*)Z_Malloc(sizeof(cherryhudMemoryService_t));
    OSP_MEMORY_CHECK(service);
    if (!service) {
        return NULL;
    }
    
    service->manageMemory = CG_CHUDMemoryServiceManageMemory;
    service->updateRows = CG_CHUDMemoryServiceUpdateRows;
    service->finalizeTable = CG_CHUDMemoryServiceFinalizeTable;
    
    return service;
}

void CG_CHUDMemoryServiceDestroy(cherryhudMemoryService_t* service) {
    if (service) {
        Z_Free(service);
    }
}

// Validation Service Implementation
static void CG_CHUDValidationServiceUpdateProperties(cherryhudTable_t* table) {
    CG_CHUDTableUpdateProperties(table);
}

static qboolean CG_CHUDValidationServiceNeedsRebuild(cherryhudTable_t* table) {
    return CG_CHUDTableNeedsRebuild(table);
}

static qboolean CG_CHUDValidationServiceDataChanged(cherryhudTableManager_t* manager, cherryhudTable_t* table, int playerHash, int requiredRows) {
    return CG_CHUDTableDataChanged(manager, table, playerHash, requiredRows);
}

cherryhudValidationService_t* CG_CHUDValidationServiceCreate(void) {
    cherryhudValidationService_t* service;
    
    service = (cherryhudValidationService_t*)Z_Malloc(sizeof(cherryhudValidationService_t));
    OSP_MEMORY_CHECK(service);
    if (!service) {
        return NULL;
    }
    
    service->updateProperties = CG_CHUDValidationServiceUpdateProperties;
    service->needsRebuild = CG_CHUDValidationServiceNeedsRebuild;
    service->dataChanged = CG_CHUDValidationServiceDataChanged;
    
    return service;
}

void CG_CHUDValidationServiceDestroy(cherryhudValidationService_t* service) {
    if (service) {
        Z_Free(service);
    }
}

// ============================================================================
// TABLE CREATION AND DESTRUCTION
// ============================================================================

cherryhudTable_t* CG_CHUDTableCreate(const cherryhudConfig_t* config) {
    cherryhudTable_t* table;
    
    // Allocate and initialize table
    table = (cherryhudTable_t*)Z_Malloc(sizeof(cherryhudTable_t));
    OSP_MEMORY_CHECK(table);
    if (!table) {
        return NULL;
    }
    memset(table, 0, sizeof(cherryhudTable_t));
    
    // Copy config if provided
    if (config) {
        memcpy(&table->config, config, sizeof(cherryhudConfig_t));
    }
    
    // Initialize table context
    CG_CHUDTableContextInit(&table->context);
    CG_CHUDTableContextUpdateFromConfig(&table->context, &table->config);
    
    // Initialize table properties
    table->maxRows = 0;
    table->rows = NULL;
    table->numRows = 0;
    table->playerRowsStart = 0;
    
    // Apply defaults and return
    CG_CHUDSetDefaultTableProperties(table);
    CG_CHUDConfigDefaultsCheck(&table->config);
    
    return table;
}

void CG_CHUDTableDestroy(cherryhudTable_t* table) {
    int i;
    
    if (!table) return;
    
    // Destroy all rows
    if (table->rows && table->numRows > 0) {
        for (i = 0; i < table->numRows; i++) {
            CG_CHUDTableDestroyRow(&table->rows[i]);
        }
        Z_Free(table->rows);
    }
    
    Z_Free(table);
}

// ============================================================================
// PLAYER DATA COLLECTION AND SORTING
// ============================================================================

void CG_CHUDTableCollectPlayerData(cherryhudPlayerData_t* players, int* numPlayers, const char* tableType) {
    int i, k;
    clientInfo_t* ci;
    cherryhudPlayerData_t* player;
    qboolean isSpectator;
    qboolean wantSpectators;
    
    *numPlayers = 0;
    wantSpectators = (tableType && Q_stricmp(tableType, "spectators") == 0);
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        ci = &cgs.clientinfo[i];
        
        // Skip invalid clients
        if (!ci->infoValid || !ci->name || !ci->name[0]) continue;
        
        // Determine if this client is a spectator
        if (ci->team == TEAM_SPECTATOR) {
            isSpectator = qtrue;
        } else if (cgs.gametype <= GT_SINGLE_PLAYER) {
            isSpectator = qfalse;
        } else if (cgs.gametype >= GT_TEAM) {
            isSpectator = (ci->team != TEAM_RED && ci->team != TEAM_BLUE);
        } else {
            isSpectator = qtrue;
        }
        
        // Filter based on table type if specified
        if (tableType) {
            if (wantSpectators && !isSpectator) continue;
            if (!wantSpectators && isSpectator) continue;
        }
        
        player = &players[*numPlayers];
        player->clientNum = i;
        player->clientInfo = ci;
        player->team = ci->team;
        player->isSpectator = isSpectator;
        
        // Find score data
        player->scoreData = NULL;
        for (k = 0; k < MAX_CLIENTS; k++) {
            if (cg.scores[k].client == i) {
                player->scoreData = &cg.scores[k];
                player->score = cg.scores[k].score;
                player->ping = cg.scores[k].ping;
                player->connected = (cg.scores[k].ping != -1);
                break;
            }
        }
        
        // Fallback to stats if no score data
        if (!player->scoreData) {
            if (cgs.be.statsAll) {
                player->score = cgs.be.statsAll[i].kills;
                player->ping = 0;
                player->connected = qtrue;
            } else {
                player->score = 0;
                player->ping = 0;
                player->connected = qfalse;
            }
        }
        
        (*numPlayers)++;
    }
}

static void CG_CHUDTableCollectPlayers(cherryhudPlayerData_t* players, int* numPlayers) {
    CG_CHUDTableCollectPlayerData(players, numPlayers, NULL);
}

static void CG_CHUDTableCollectPlayersByType(cherryhudPlayerData_t* players, int* numPlayers, const char* tableType) {
    CG_CHUDTableCollectPlayerData(players, numPlayers, tableType);
}

static int CG_CHUDTableComparePlayers(const void* a, const void* b) {
    const cherryhudPlayerData_t* pa = (const cherryhudPlayerData_t*)a;
    const cherryhudPlayerData_t* pb = (const cherryhudPlayerData_t*)b;
    
    // Spectators go to the end
    if (pa->isSpectator && !pb->isSpectator) return 1;
    if (!pa->isSpectator && pb->isSpectator) return -1;
    
    // Sort by score (descending)
    return pb->score - pa->score;
}

static void CG_CHUDTableSortPlayers(cherryhudPlayerData_t* players, int numPlayers) {
    qsort(players, numPlayers, sizeof(cherryhudPlayerData_t), CG_CHUDTableComparePlayers);
}

// ============================================================================
// TABLE ROW MANAGEMENT
// ============================================================================

static void CG_CHUDTableCreateRow(cherryhudTable_t* table, int index, const cherryhudPlayerData_t* playerData) {
    cherryhudTableRow_t* row;
    const cherryhudConfig_t* playerRowTemplate;
    const cherryhudConfig_t* playerInfoTemplate;
    cherryhudConfig_t elementConfig;
    cherryhudElement_t* element;
    cherryhudElement_t* last;
    cherryhudLayoutContext_t layoutContext;
    cherryhudLayoutProperties_t layoutProps;
    cherryhudLayoutBounds_t* finalBounds;
    int i, templateCount;
    
    if (!table || !table->rows || index < 0 || index >= table->maxRows || !playerData) {
        return;
    }
    
    row = &table->rows[index];
    if (!row) return;
    
    // Clear existing elements first only if row was already populated
    if (row->elements) {
        CG_CHUDTableDestroyRow(row);
    }
    
    // Set row type
    row->type = playerData->isSpectator ? CHERRYHUD_ROW_TYPE_SPECTATOR : CHERRYHUD_ROW_TYPE_PLAYER;
    
    // Copy player data
    memcpy(&row->playerData, playerData, sizeof(cherryhudPlayerData_t));
    
    // Set row properties
    row->rowIndex = index;
    // Initialize yPosition - will be recalculated in CG_CHUDTableUpdatePosition if needed
    row->yPosition = table->context.baseY + (index * (table->context.rowHeight + table->context.rowSpacing));
    
    // Initialize elements list
    row->elements = NULL;
    
    // Get appropriate row template based on row type
    if (row->type == CHERRYHUD_ROW_TYPE_SPECTATOR) {
        playerRowTemplate = (cherryhudConfig_t*)CG_CHUDGetSpectatorRowTemplate();
        if (!playerRowTemplate) {
            // Fallback to playerRow template if spectatorRow not set
            playerRowTemplate = (cherryhudConfig_t*)CG_CHUDGetPlayerRowTemplate();
        }
    } else {
        playerRowTemplate = (cherryhudConfig_t*)CG_CHUDGetPlayerRowTemplate();
    }
    
    if (!playerRowTemplate) {
        return;
    }
    
    // Create elements for this row
    CG_CHUDTableCreateRowElements(table, row, playerRowTemplate);
}

// Get template count based on row type
static int CG_CHUDTableGetTemplateCount(cherryhudTableRow_t* row) {
    if (row->type == CHERRYHUD_ROW_TYPE_PLAYER) {
        return CG_CHUDGetPlayerRowElementCount();
    } else if (row->type == CHERRYHUD_ROW_TYPE_SPECTATOR) {
        return CG_CHUDGetSpectatorRowElementCount();
    }
    return 0;
}

// Get template config based on row type and index
static const cherryhudConfig_t* CG_CHUDTableGetTemplate(cherryhudTableRow_t* row, int index) {
    if (row->type == CHERRYHUD_ROW_TYPE_PLAYER) {
        return CG_CHUDGetPlayerRowElement(index);
    } else if (row->type == CHERRYHUD_ROW_TYPE_SPECTATOR) {
        return CG_CHUDGetSpectatorRowElement(index);
    }
    return NULL;
}

// Get modified template config with container's active parameters applied
static void CG_CHUDTableGetModifiedTemplate(cherryhudConfig_t* result, cherryhudTableRow_t* row, int index, const cherryhudConfig_t* containerConfig, float containerHeight) {
    const cherryhudConfig_t* originalTemplate;
    const char* activeMode;
    cherryhudConfig_t* prefixedConfig;
    
    if (!result || !row) return;
    
    // Get original template
    originalTemplate = CG_CHUDTableGetTemplate(row, index);
    if (!originalTemplate) {
        memset(result, 0, sizeof(cherryhudConfig_t));
        return;
    }
    
    // Copy original template
    memcpy(result, originalTemplate, sizeof(cherryhudConfig_t));
    
    // Show original size
    
    // Apply container's active parameters if available
    if (containerConfig) {
        activeMode = CG_CHUDGetActiveMode(containerConfig, containerHeight);
        
        if (activeMode) {
            // Each template should use its own prefixed config for the active mode
            prefixedConfig = CG_CHUDGetPrefixedConfig(result, activeMode);
            
            if (prefixedConfig) {
                if (prefixedConfig->size.isSet) {
                    result->size.value[0] = prefixedConfig->size.value[0];
                    result->size.value[1] = prefixedConfig->size.value[1];
                    result->size.isSet = qtrue;
                } else {
                }
            } else {
            }
        }
    }
}

// Prepare element configuration from template
static void CG_CHUDTablePrepareElementConfig(cherryhudConfig_t* elementConfig, const cherryhudConfig_t* template, int elementIndex, const cherryhudConfig_t* containerConfig, float containerHeight) {
    const cherryhudConfig_t* defaultConfig;
    vec4_t activePos;
    vec4_t activeSize;
    const char* activeMode;
    
    if (!elementConfig || !template) return;
    
    // Initialize element config to clean state
    memset(elementConfig, 0, sizeof(cherryhudConfig_t));
    
    // Copy template config
    memcpy(elementConfig, template, sizeof(cherryhudConfig_t));
    
    // Apply active parameters from container if available (only if not already applied to template)
    if (containerConfig) {
        activeMode = CG_CHUDGetActiveMode(containerConfig, containerHeight);
        
        if (activeMode) {
            // Apply active position if available
            CG_CHUDGetActivePos(containerConfig, containerHeight, activePos);
            if (activePos[0] != 0.0f || activePos[1] != 0.0f || activePos[2] != 0.0f || activePos[3] != 0.0f) {
                // Don't override element position with container position
                // Container position is handled at table level
            }
            
            // Apply active size if available
            CG_CHUDGetActiveSize(containerConfig, containerHeight, activeSize);
            
            if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || activeSize[2] != 0.0f || activeSize[3] != 0.0f) {
                // Apply container's active size to element if element size is not set
                if (!elementConfig->size.isSet) {
                    elementConfig->size.value[0] = activeSize[0];
                    elementConfig->size.value[1] = activeSize[1];
                    elementConfig->size.isSet = qtrue;
                } else {
                }
            }
        }
    } else {
    }
    
    // Apply default configuration values from !default if available
    defaultConfig = CG_CHUDGetDefaultConfig();
    if (defaultConfig) {
        // Inherit from default config for any unset properties
        CG_CHUDInheritConfig(elementConfig, defaultConfig);
    }
    
    // Set hardcoded default values for any still unset properties
    CG_CHUDSetDefaultElementProperties(elementConfig);
    
    // Position elements relative to table and row - layout system will handle final positioning
    if (!elementConfig->pos.isSet) {
        // If pos is not set, set it based on table position and row
        // Calculate X position based on element index (create columns)
        float elementWidth = elementConfig->size.isSet ? elementConfig->size.value[0] : 100.0f;
        float elementSpacing = 2.0f; // Default spacing between elements
        
        // Calculate X position with spacing
        elementConfig->pos.value[0] = elementIndex * (elementWidth + elementSpacing); // Relative to table
        elementConfig->pos.value[1] = 0.0f; // Relative to row
        elementConfig->pos.isSet = qtrue;
    }
}

// Setup layout context for element
static void CG_CHUDTableSetupLayoutContext(cherryhudLayoutContext_t* layoutContext, cherryhudTable_t* table, cherryhudTableRow_t* row) {
    if (!layoutContext || !table || !row) return;
    
    layoutContext->parentX = table->context.baseX;
    layoutContext->parentY = row->yPosition; // row->yPosition already includes rowSpacing
    layoutContext->parentWidth = table->context.width;
    layoutContext->parentHeight = table->context.rowHeight;
    layoutContext->availableWidth = table->context.width;
    layoutContext->availableHeight = table->context.rowHeight;
    layoutContext->currentTime = cg.time;
    layoutContext->forceRecalc = qtrue; // Force calculation for new elements
    layoutContext->clientNum = row->playerData.clientNum; // Pass clientNum to layout system
}

// Create and configure a single element
static cherryhudElement_t* CG_CHUDTableCreateSingleElement(cherryhudTable_t* table, cherryhudTableRow_t* row, const cherryhudConfig_t* template, int elementIndex) {
    cherryhudConfig_t elementConfig;
    cherryhudElement_t* element;
    cherryhudLayoutContext_t layoutContext;
    cherryhudLayoutProperties_t layoutProps;
    const char* containerName;
    if (!table || !row || !template) return NULL;
    
    // Prepare element configuration (template already has container parameters applied)
    CG_CHUDTablePrepareElementConfig(&elementConfig, template, elementIndex, NULL, 0.0f);
    
    // Create the element using unified factory with proper container name

    if (Q_stricmp(table->tableType, "players") == 0) {
        containerName = CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS);
    } else if (Q_stricmp(table->tableType, "spectators") == 0) {
        containerName = CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS);
    } else {
        containerName = table->tableType; // fallback to table type
    }
    element = CG_CHUDCreateElementFromConfig(&elementConfig, row->playerData.clientNum, containerName);
    if (!element) {
        return NULL;
    }
    
    // Setup layout context
    CG_CHUDTableSetupLayoutContext(&layoutContext, table, row);
    
    // Set layout properties for table elements
    CG_CHUDLayoutSetDefaultProperties(&layoutProps);
    layoutProps.mode = CHERRYHUD_LAYOUT_MODE_TABLE;
    CG_CHUDLayoutSetProperties(element, &layoutProps);
    
    // Calculate layout
    CG_CHUDLayoutCalculateElement(element, &layoutContext);
    
    return element;
}

// Add element to row's element list (internal helper)
static void CG_CHUDTableAddElementToList(cherryhudTableRow_t* row, cherryhudElement_t* element) {
    cherryhudElement_t* last;
    
    if (!row || !element) return;
    
    // Add to row
    if (row->elements == NULL) {
        row->elements = element;
    } else {
        last = row->elements;
        while (last->next) last = last->next;
        last->next = element;
    }
}

// Main function to create all elements for a row
static void CG_CHUDTableCreateRowElements(cherryhudTable_t* table, cherryhudTableRow_t* row, const cherryhudConfig_t* playerRowTemplate) {
    cherryhudConfig_t modifiedTemplate;
    cherryhudElement_t* element;
    const cherryhudScoreboardOrder_t* scoreboardOrder;
    const cherryhudConfig_t* containerConfig;
    float containerHeight;
    int i, templateCount;
    
    if (!table || !row) return;
    
    // Get container config from scoreboard order
    containerConfig = NULL;
    containerHeight = 0.0f;
    scoreboardOrder = CG_CHUDScoreboardOrderGet();
    if (scoreboardOrder) {
        for (i = 0; i < scoreboardOrder->count; i++) {
            if ((Q_stricmp(table->tableType, "players") == 0 && scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS) ||
                (Q_stricmp(table->tableType, "spectators") == 0 && scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) {
                containerConfig = &scoreboardOrder->blocks[i].config;
                containerHeight = CG_CHUDTableCalculateHeight(table);
                break;
            }
        }
    }
    
    // Get template count based on row type
    templateCount = CG_CHUDTableGetTemplateCount(row);
    if (templateCount == 0) {
        return;
    }
    
    // Create elements for each template
    for (i = 0; i < templateCount; i++) {
        // Get modified template with container's active parameters
        CG_CHUDTableGetModifiedTemplate(&modifiedTemplate, row, i, containerConfig, containerHeight);
        
        // Create and configure the element
        element = CG_CHUDTableCreateSingleElement(table, row, &modifiedTemplate, i);
        if (!element) {
            continue;
        }
        
        // Add element to row
        CG_CHUDTableAddElementToList(row, element);
    }
}

static void CG_CHUDTableDestroyRow(cherryhudTableRow_t* row) {
    cherryhudElement_t* element;
    cherryhudElement_t* next;
    
    if (!row) return;
    
    
    // Destroy all elements in this row
    element = row->elements;
    while (element) {
        next = element->next;
        
        
        // Remove element from layout system
        CG_CHUDLayoutRemoveElement((void*)element);
        
        if (element && element->element.destroy) {
            element->element.destroy(element->element.context);
        }
        if (element) {
            Z_Free(element);
        }
        element = next;
    }
    
    row->elements = NULL;
    
    // Clear row data
    memset(&row->playerData, 0, sizeof(cherryhudPlayerData_t));
    row->type = CHERRYHUD_ROW_TYPE_PLAYER;
    row->rowIndex = 0;
    row->yPosition = 0.0f;
}

// ============================================================================
// TABLE SIZE AND HEIGHT CALCULATIONS
// ============================================================================

float CG_CHUDTableCalculateHeight(cherryhudTable_t* table) {
    float rowHeight;
    float rowSpacing;
    int numRows;
    
    if (!table) return 0.0f;
    
    // Use actual table rows, not player count
    numRows = table->numRows;
    if (numRows == 0) return 0.0f;
    
    // Use actual table row height and spacing
    rowHeight = table->context.rowHeight;
    rowSpacing = table->context.rowSpacing;
    
    // Calculate total height based on actual table rows
    return (numRows * rowHeight) + ((numRows - 1) * rowSpacing);
}

float CG_CHUDContainerCalculateHeight(const char* containerType) {
    int playerCount;
    int spectatorCount;
    float rowHeight;
    float rowSpacing;
    float totalHeight;
    const cherryhudConfig_t* playerRowTemplate;
    const cherryhudConfig_t* spectatorRowTemplate;
    // Get player counts using our helper functions
    playerCount = CG_CHUDGetPlayerCount();
    spectatorCount = CG_CHUDGetSpectatorCount();
    
    // Get appropriate row height based on container type
    if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS)) == 0) {
        // For players, use playerRow template height
        playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
        if (playerRowTemplate) {
            vec4_t activeSize;
            float currentHeight = 400.0f; // Default height for calculation
            CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, containerType, currentHeight, activeSize);
            
            if (activeSize[1] != 0.0f) {
                rowHeight = activeSize[1];
            } else if (playerRowTemplate->size.isSet) {
                rowHeight = playerRowTemplate->size.value[1];
            } else {
                rowHeight = g_playerRowHeight; // Use global row height
            }
        } else {
            rowHeight = g_playerRowHeight; // Use global row height
        }
    } else if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) == 0) {
        // For spectators, use spectatorRow template height, fallback to playerRow
        spectatorRowTemplate = CG_CHUDGetSpectatorRowTemplate();
        playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
        if (spectatorRowTemplate) {
            vec4_t activeSize;
            float currentHeight = 400.0f; // Default height for calculation
            CG_CHUDGetActiveSizeFromContainer(spectatorRowTemplate, containerType, currentHeight, activeSize);
            
            if (activeSize[1] != 0.0f) {
                rowHeight = activeSize[1];
            } else if (spectatorRowTemplate->size.isSet) {
                rowHeight = spectatorRowTemplate->size.value[1];
            } else if (playerRowTemplate) {
                vec4_t playerActiveSize;
                float currentHeight = 400.0f; // Default height for calculation
                CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, containerType, currentHeight, playerActiveSize);
                
                if (playerActiveSize[1] != 0.0f) {
                    rowHeight = playerActiveSize[1];
                } else if (playerRowTemplate->size.isSet) {
                    rowHeight = playerRowTemplate->size.value[1];
                } else {
                    rowHeight = g_playerRowHeight; // Use global row height
                }
            } else {
                rowHeight = g_playerRowHeight; // Use global row height
            }
        } else if (playerRowTemplate) {
            vec4_t activeSize;
            float currentHeight = 400.0f; // Default height for calculation
            CG_CHUDGetActiveSize(playerRowTemplate, currentHeight, activeSize);
            
            if (activeSize[1] != 0.0f) {
                rowHeight = activeSize[1];
            } else if (playerRowTemplate->size.isSet) {
                rowHeight = playerRowTemplate->size.value[1];
            } else {
                rowHeight = g_playerRowHeight; // Use global row height
            }
        } else {
            rowHeight = g_playerRowHeight; // Use global row height
        }
    } else {
        // Default to playerRow template height
        playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
        if (playerRowTemplate) {
            vec4_t activeSize;
            float currentHeight = 400.0f; // Default height for calculation
            CG_CHUDGetActiveSize(playerRowTemplate, currentHeight, activeSize);
            
            if (activeSize[1] != 0.0f) {
                rowHeight = activeSize[1];
            } else if (playerRowTemplate->size.isSet) {
                rowHeight = playerRowTemplate->size.value[1];
            } else {
                rowHeight = g_playerRowHeight; // Use global row height
            }
        } else {
            rowHeight = g_playerRowHeight; // Use global row height
        }
    }
    
    // Get row spacing from templates
    rowSpacing = 0.0f; // Default row spacing
    if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS)) == 0) {
        if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
            rowSpacing = playerRowTemplate->rowSpacing.value;
        }
    } else if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) == 0) {
        if (spectatorRowTemplate && spectatorRowTemplate->rowSpacing.isSet) {
            rowSpacing = spectatorRowTemplate->rowSpacing.value;
        } else if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
            rowSpacing = playerRowTemplate->rowSpacing.value;
        }
    } else {
        if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
            rowSpacing = playerRowTemplate->rowSpacing.value;
        }
    }
    
    // Calculate height based on container type
    if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS)) == 0) {
        if (playerCount == 0) {
            totalHeight = 0.0f;
        } else {
            totalHeight = (playerCount * rowHeight) + ((playerCount - 1) * rowSpacing);
        }
    } else if (Q_stricmp(containerType, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) == 0) {
        if (spectatorCount == 0) {
            totalHeight = 0.0f;
        } else {
            totalHeight = (spectatorCount * rowHeight) + ((spectatorCount - 1) * rowSpacing);
        }
    } else {
        // Default to total height calculation
        totalHeight = (playerCount + spectatorCount) * rowHeight + ((playerCount + spectatorCount - 1) * rowSpacing);
    }
    
    return totalHeight;
}

float CG_CHUDScoreboardCalculateHeight(void) {
    const cherryhudScoreboardOrder_t* order;
    float totalHeight;
    float defaultSpacing;
    float titleHeight;
    float playersHeight;
    float spectatorsHeight;
    int i;
    int playerCount;
    int spectatorCount;
    float rowHeight;
    float rowSpacing;
    
    // Initialize
    totalHeight = 0.0f;
    defaultSpacing = 2.0f; // Default spacing between blocks
    
    // Get the order of blocks from config
    order = CG_CHUDScoreboardOrderGet();
    if (!order || order->count == 0) {
        return 0.0f;
    }
    
    
    // Get player counts and row properties
    playerCount = CG_CHUDGetPlayerCount();
    spectatorCount = CG_CHUDGetSpectatorCount();
    rowHeight = 20.0f; // Default row height
    rowSpacing = 0.0f; // Default row spacing
    
    // Calculate height for each block type
    for (i = 0; i < order->count; i++) {
        const cherryhudScoreboardBlock_t* block = &order->blocks[i];
        float blockSpacing;
        float heightBefore = totalHeight;
        
        // Use rowSpacing from block config if set, otherwise use default spacing
        blockSpacing = block->config.rowSpacing.isSet ? block->config.rowSpacing.value : defaultSpacing;
        
        
        switch (block->type) {
            case CHERRYHUD_BLOCK_TYPE_TITLE: {
                // Check if this title block should be rendered (not hidden due to empty container)
                qboolean shouldRender = qtrue;
                if (block->titleBlockIndex >= 0) {
                    const cherryhudTitleBlock_t* titleBlock = CG_CHUDGetTitleBlock(block->titleBlockIndex);
                    if (titleBlock && CG_CHUDTitleBlockHasContainer(titleBlock)) {
                        shouldRender = CG_CHUDContainerHasContent(titleBlock->containerType);
                    }
                }
                
                if (shouldRender) {
                    titleHeight = block->config.size.isSet ? block->config.size.value[1] : 12.0f;
                    totalHeight += titleHeight;
                    if (i < order->count - 1) totalHeight += blockSpacing;
                } else {
                }
                break;
            }
            
            case CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS: {
                if (playerCount > 0) {
                    // Get actual row height and spacing from playerRow template
                    const cherryhudConfig_t* playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
                    if (playerRowTemplate) {
                        vec4_t activeSize;
                        float currentHeight = 400.0f; // Default height for calculation
                        CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS), currentHeight, activeSize);
                        
                        if (activeSize[1] != 0.0f) {
                            rowHeight = activeSize[1];
                        } else if (playerRowTemplate->size.isSet) {
                            rowHeight = playerRowTemplate->size.value[1];
                        }
                    }
                    if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
                        rowSpacing = playerRowTemplate->rowSpacing.value;
                    }
                    playersHeight = (playerCount * rowHeight) + ((playerCount - 1) * rowSpacing);
                    totalHeight += playersHeight;
                    if (i < order->count - 1) totalHeight += blockSpacing;
                }
                break;
            }
            
            case CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS: {
                if (spectatorCount > 0) {
                    // Get actual row height and spacing from spectatorRow template, fallback to playerRow
                    const cherryhudConfig_t* spectatorRowTemplate = CG_CHUDGetSpectatorRowTemplate();
                    const cherryhudConfig_t* playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
                    if (spectatorRowTemplate) {
                        vec4_t activeSize;
                        float currentHeight = 400.0f; // Default height for calculation
                        CG_CHUDGetActiveSizeFromContainer(spectatorRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS), currentHeight, activeSize);
                        
                        if (activeSize[1] != 0.0f) {
                            rowHeight = activeSize[1];
                        } else if (spectatorRowTemplate->size.isSet) {
                            rowHeight = spectatorRowTemplate->size.value[1];
                        } else if (playerRowTemplate) {
                            vec4_t playerActiveSize;
                            float currentHeight = 400.0f; // Default height for calculation
                            CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS), currentHeight, playerActiveSize);
                            
                            if (playerActiveSize[1] != 0.0f) {
                                rowHeight = playerActiveSize[1];
                            } else if (playerRowTemplate->size.isSet) {
                                rowHeight = playerRowTemplate->size.value[1];
                            }
                        }
                    } else if (playerRowTemplate) {
                        vec4_t activeSize;
                        float currentHeight = 400.0f; // Default height for calculation
                        CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS), currentHeight, activeSize);
                        
                        if (activeSize[1] != 0.0f) {
                            rowHeight = activeSize[1];
                        } else if (playerRowTemplate->size.isSet) {
                            rowHeight = playerRowTemplate->size.value[1];
                        }
                    }
                    if (spectatorRowTemplate && spectatorRowTemplate->rowSpacing.isSet) {
                        rowSpacing = spectatorRowTemplate->rowSpacing.value;
                    } else if (playerRowTemplate && playerRowTemplate->rowSpacing.isSet) {
                        rowSpacing = playerRowTemplate->rowSpacing.value;
                    }
                    spectatorsHeight = (spectatorCount * rowHeight) + ((spectatorCount - 1) * rowSpacing);
                    totalHeight += spectatorsHeight;
                    if (i < order->count - 1) totalHeight += blockSpacing;
                }
                break;
            }
        }
        
    }
    
    return totalHeight;
}

// ============================================================================
// TABLE BUILDING AND UPDATING
// ============================================================================

// ============================================================================
// TABLE CONTEXT MANAGEMENT
// ============================================================================

// Initialize table context with default values
void CG_CHUDTableContextInit(cherryhudTableContext_t* context) {
    if (!context) return;
    
    context->baseX = 0.0f;
    context->baseY = 0.0f;
    context->width = 640.0f;
    context->height = 200.0f;
    context->rowHeight = 20.0f;
    context->rowSpacing = 0.0f;
    context->lastBuildTime = 0;
    context->needsRecalc = qtrue;
    context->isDirty = qtrue;
}

// Update context from config (one-way: config -> context)
void CG_CHUDTableContextUpdateFromConfig(cherryhudTableContext_t* context, const cherryhudConfig_t* config) {
    vec4_t activePos;
    vec4_t activeSize;
    float currentHeight;
    
    if (!context || !config) return;
    
    // Calculate current height for mode detection
    currentHeight = context->height > 0.0f ? context->height : 200.0f;
    
    // Get active values based on mode
    CG_CHUDGetActivePos(config, currentHeight, activePos);
    CG_CHUDGetActiveSize(config, currentHeight, activeSize);
    
    // Update position
    if (activePos[0] != 0.0f || activePos[1] != 0.0f || config->pos.isSet) {
        context->baseX = activePos[0];
        context->baseY = activePos[1];
    }
    
    // Update size
    if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || config->size.isSet) {
        context->width = activeSize[0];
        context->height = activeSize[1];
    }
    
    // Update row properties
    if (config->rowSpacing.isSet) {
        context->rowSpacing = config->rowSpacing.value;
    }
}

// Set position in context
void CG_CHUDTableContextSetPosition(cherryhudTableContext_t* context, float x, float y) {
    if (!context) return;
    
    context->baseX = x;
    context->baseY = y;
    context->isDirty = qtrue;
}

// Set size in context
void CG_CHUDTableContextSetSize(cherryhudTableContext_t* context, float width, float height) {
    if (!context) return;
    
    context->width = width;
    context->height = height;
    context->isDirty = qtrue;
}

// Set row properties in context
void CG_CHUDTableContextSetRowProperties(cherryhudTableContext_t* context, float rowHeight, float rowSpacing) {
    if (!context) return;
    
    context->rowHeight = rowHeight;
    context->rowSpacing = rowSpacing;
    context->isDirty = qtrue;
}

// Mark context as dirty (needs recalculation)
void CG_CHUDTableContextMarkDirty(cherryhudTableContext_t* context) {
    if (!context) return;
    
    context->isDirty = qtrue;
    context->needsRecalc = qtrue;
}

// Mark context as clean (no recalculation needed)
void CG_CHUDTableContextMarkClean(cherryhudTableContext_t* context) {
    if (!context) return;
    
    context->isDirty = qfalse;
    context->needsRecalc = qfalse;
}

// Check if context needs recalculation
qboolean CG_CHUDTableContextNeedsRecalc(cherryhudTableContext_t* context) {
    if (!context) return qfalse;
    
    return context->needsRecalc || context->isDirty;
}

// ============================================================================
// TABLE BUILDING - REFACTORED INTO SMALLER FUNCTIONS
// ============================================================================

// Check if table needs rebuilding based on timing and changes
qboolean CG_CHUDTableNeedsRebuild(cherryhudTable_t* table) {
    int currentTime;
    
    if (!table) return qfalse;
    
    currentTime = cg.time;
    
    // Only rebuild every 100ms to avoid performance issues
    if (currentTime - table->context.lastBuildTime < 100) {
        return qfalse;
    }
    
    table->context.lastBuildTime = currentTime;
    return qtrue;
}

// Update table properties from templates
void CG_CHUDTableUpdateProperties(cherryhudTable_t* table) {
    const cherryhudConfig_t* playerRowTemplate;
    const cherryhudConfig_t* spectatorRowTemplate;
    vec4_t activeSize;
    float currentHeight;
    qboolean rowSpacingChanged = qfalse;
    qboolean rowHeightChanged = qfalse;
    int i;
    cherryhudTableRow_t* row;
    const cherryhudConfig_t* activeTemplate = NULL;
    if (!table) return;
    
    // Get appropriate template based on table type
    playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
    spectatorRowTemplate = CG_CHUDGetSpectatorRowTemplate();
    
    // Choose the right template based on table type
    if (Q_stricmp(table->tableType, "spectators") == 0) {
        // For spectators, use spectatorRowTemplate if it exists, otherwise fallback to playerRowTemplate
        if (spectatorRowTemplate) {
            activeTemplate = spectatorRowTemplate;
        } else if (playerRowTemplate) {
            activeTemplate = playerRowTemplate;
        }
    } else if (playerRowTemplate) {
        // For players, use playerRowTemplate
        activeTemplate = playerRowTemplate;
    }
    
    if (activeTemplate) {
        float newRowHeight;
        // rowSpacing is now managed only by templates (playerRow, spectatorRow)
        if (activeTemplate->rowSpacing.isSet) {
            if (table->context.rowSpacing != activeTemplate->rowSpacing.value) {
                rowSpacingChanged = qtrue;
            }
            table->context.rowSpacing = activeTemplate->rowSpacing.value;
        }
        
        // Check if we need to use active size (for compact/double modes)
        // We need to get the current height to determine if compact mode is active
        currentHeight = table->context.height;
        CG_CHUDGetActiveSizeFromContainer(activeTemplate, table->tableType, currentHeight, activeSize);
        
        newRowHeight = table->context.rowHeight;
        if (activeSize[1] != 0.0f) {
            // Use active size (compact.size or double.size)
            newRowHeight = activeSize[1];
        } else if (activeTemplate->size.isSet) {
            // Use base size
            newRowHeight = activeTemplate->size.value[1];
        }
        
        if (table->context.rowHeight != newRowHeight) {
            rowHeightChanged = qtrue;
        }
        table->context.rowHeight = newRowHeight;
    }
    
    // If rowSpacing or rowHeight changed, recalculate all row positions
    if ((rowSpacingChanged || rowHeightChanged) && table->rows && table->numRows > 0) {
        for (i = 0; i < table->numRows; i++) {
            row = &table->rows[i];
            if (row) {
                // Recalculate row position
                row->yPosition = table->context.baseY + (i * (table->context.rowHeight + table->context.rowSpacing));
                
                // Force layout recalculation for all elements in this row
                if (row->elements) {
                    cherryhudElement_t* element = row->elements;
                    while (element) {
                        // Remove from layout cache to force recalculation
                        CG_CHUDLayoutRemoveElement((void*)element);
                        element = element->next;
                    }
                }
            }
        }
        
        // Force layout system recalculation for this table
        CG_CHUDLayoutForceRecalcForTable(table);
    }
}

// Collect and prepare player data
static void CG_CHUDTablePrepareData(cherryhudTable_t* table, cherryhudPlayerData_t* players, int* numPlayers, int* playerHash) {
    int i;
    
    if (!table || !players || !numPlayers || !playerHash) return;
    
    // Collect players based on table type
    if (table->tableType[0] != '\0') {
        CG_CHUDTableCollectPlayersByType(players, numPlayers, table->tableType);
    } else {
        CG_CHUDTableCollectPlayers(players, numPlayers);
    }
    
    // Compute simple hash for change detection
    *playerHash = 0;
    for (i = 0; i < *numPlayers; i++) {
        *playerHash ^= (players[i].clientNum << 16) ^ players[i].score ^ (players[i].ping & 0xFFFF);
    }
}

// Check if table data has changed significantly
qboolean CG_CHUDTableDataChanged(cherryhudTableManager_t* manager, cherryhudTable_t* table, int playerHash, int requiredRows) {
    if (!manager) return qtrue;
    
    // Skip rebuild if nothing changed, not dirty, and row count matches
    if (playerHash == manager->lastPlayerHash && !manager->tableDirty && requiredRows == table->numRows) {
        return qfalse;
    }
    
    manager->lastPlayerHash = playerHash;
    manager->tableDirty = qfalse;
    return qtrue;
}

// Manage table row memory allocation
void CG_CHUDTableManageRows(cherryhudTable_t* table, int requiredRows) {
    cherryhudTableRow_t* newRows;
    int i;
    
    if (!table) return;
    
    // Realloc if requiredRows differs from maxRows
    if (requiredRows != table->maxRows) {
        // Destroy existing rows only if table was already populated
        if (table->rows && table->numRows > 0) {
            for (i = 0; i < table->numRows; i++) {
                CG_CHUDTableDestroyRow(&table->rows[i]);
            }
            Z_Free(table->rows);
            table->rows = NULL;
        }
        
        if (requiredRows > 0) {
            newRows = (cherryhudTableRow_t*)Z_Malloc(sizeof(cherryhudTableRow_t) * requiredRows);
            OSP_MEMORY_CHECK(newRows);
            if (!newRows) {
                return;
            }
            
            // Initialize new rows
            for (i = 0; i < requiredRows; i++) {
                memset(&newRows[i], 0, sizeof(cherryhudTableRow_t));
                newRows[i].rowIndex = i;
                newRows[i].elements = NULL;
            }
            table->rows = newRows;
        }
        table->maxRows = requiredRows;
    }
}

// Update all table rows with new data
void CG_CHUDTableUpdateAllRows(cherryhudTable_t* table, const cherryhudPlayerData_t* players, int numPlayers) {
    int i;
    
    if (!table || !players) return;
    
    // Update rows: Use UpdateRow for reuse
    for (i = 0; i < numPlayers; i++) {
        CG_CHUDTableUpdateRow(table, i, &players[i]);
    }
    
    table->numRows = numPlayers;
}

// Finalize table after building (height calculation, layout)
void CG_CHUDTableFinalize(cherryhudTable_t* table) {
    float dynamicHeight;
    
    if (!table) return;
    
    // Calculate and update dynamic table height in context (not config!)
    dynamicHeight = CG_CHUDTableCalculateHeight(table);
    table->context.height = dynamicHeight;
    
    // Mark context as clean after successful build
    CG_CHUDTableContextMarkClean(&table->context);
    
    // Force layout recalculation for all elements in the table
    CG_CHUDLayoutRecalculate();
}

// Main table build function - now much cleaner!
// Helper function to create and validate services
static qboolean CG_CHUDTableBuildCreateServices(cherryhudDataService_t** dataService, 
                                               cherryhudMemoryService_t** memoryService, 
                                               cherryhudValidationService_t** validationService) {
    *dataService = CG_CHUDDataServiceCreate();
    *memoryService = CG_CHUDMemoryServiceCreate();
    *validationService = CG_CHUDValidationServiceCreate();
    
    if (!*dataService || !*memoryService || !*validationService) {
        // Cleanup on error
        if (*dataService) CG_CHUDDataServiceDestroy(*dataService);
        if (*memoryService) CG_CHUDMemoryServiceDestroy(*memoryService);
        if (*validationService) CG_CHUDValidationServiceDestroy(*validationService);
        return qfalse;
    }
    return qtrue;
}

// Helper function to cleanup services
static void CG_CHUDTableBuildCleanupServices(cherryhudDataService_t* dataService, 
                                            cherryhudMemoryService_t* memoryService, 
                                            cherryhudValidationService_t* validationService) {
    if (dataService) CG_CHUDDataServiceDestroy(dataService);
    if (memoryService) CG_CHUDMemoryServiceDestroy(memoryService);
    if (validationService) CG_CHUDValidationServiceDestroy(validationService);
}

// Helper function to prepare table data
static qboolean CG_CHUDTableBuildPrepareData(cherryhudTable_t* table, 
                                            cherryhudDataService_t* dataService,
                                            cherryhudPlayerData_t* players, 
                                            int* numPlayers, 
                                            int* playerHash, 
                                            int* requiredRows) {
    // Collect and prepare player data
    dataService->collectData(players, numPlayers, table->tableType);
    *playerHash = dataService->calculateHash(players, *numPlayers);
    
    // Calculate required rows
    *requiredRows = *numPlayers;  // Currently just players/spectators; expand if there are other rows
    
    return qtrue;
}

// Helper function to rebuild table if needed
static void CG_CHUDTableBuildRebuildTable(cherryhudTable_t* table, 
                                         cherryhudDataService_t* dataService,
                                         cherryhudMemoryService_t* memoryService,
                                         cherryhudPlayerData_t* players, 
                                         int numPlayers, 
                                         int requiredRows) {
    // Sort players
    dataService->sortData(players, numPlayers);
    
    // Manage row memory allocation
    memoryService->manageMemory(table, requiredRows);
    
    // Update all rows with new data
    memoryService->updateRows(table, players, numPlayers);
    
    // Finalize table (height, layout)
    memoryService->finalizeTable(table);
}

void CG_CHUDTableBuild(cherryhudTableManager_t* manager, cherryhudTable_t* table) {
    cherryhudPlayerData_t players[MAX_CLIENTS];
    int numPlayers;
    int playerHash;
    int requiredRows;
    
    // Create services
    cherryhudDataService_t* dataService;
    cherryhudMemoryService_t* memoryService;
    cherryhudValidationService_t* validationService;
    
    if (!table || !manager) return;
    
    // Create and validate services
    if (!CG_CHUDTableBuildCreateServices(&dataService, &memoryService, &validationService)) {
        return;
    }
    
    // Step 1: Check if rebuild is needed
    if (!validationService->needsRebuild(table)) {
        CG_CHUDTableBuildCleanupServices(dataService, memoryService, validationService);
        return;
    }
    
    // Step 2: Update table properties from templates
    validationService->updateProperties(table);
    
    // Step 3: Prepare data
    if (!CG_CHUDTableBuildPrepareData(table, dataService, players, &numPlayers, &playerHash, &requiredRows)) {
        CG_CHUDTableBuildCleanupServices(dataService, memoryService, validationService);
        return;
    }
    
    // Step 4: Check if data has changed
    if (!validationService->dataChanged(manager, table, playerHash, requiredRows)) {
        CG_CHUDTableBuildCleanupServices(dataService, memoryService, validationService);
        return;
    }
    
    // Step 5: Rebuild table
    CG_CHUDTableBuildRebuildTable(table, dataService, memoryService, players, numPlayers, requiredRows);
    
    // Cleanup services
    CG_CHUDTableBuildCleanupServices(dataService, memoryService, validationService);
}

static void CG_CHUDTableUpdateRow(cherryhudTable_t* table, int index, const cherryhudPlayerData_t* playerData) {
    cherryhudTableRow_t* row;
    qboolean needsRecreate;
    float newYPosition;
    const cherryhudScoreboardOrder_t* scoreboardOrder;
    const cherryhudConfig_t* containerConfig;
    int i;
    
    if (!table || !table->rows || index < 0 || index >= table->maxRows || !playerData) return;
    
    row = &table->rows[index];
    if (!row) return;
    
    // Check if container has extendedType - if so, always recreate elements
    needsRecreate = qtrue;
    scoreboardOrder = CG_CHUDScoreboardOrderGet();
    if (scoreboardOrder) {
        for (i = 0; i < scoreboardOrder->count; i++) {
            if ((Q_stricmp(table->tableType, "players") == 0 && scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS) ||
                (Q_stricmp(table->tableType, "spectators") == 0 && scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) {
                containerConfig = &scoreboardOrder->blocks[i].config;
                if (containerConfig->extendedType.isSet) {
                    // Container has extendedType, always recreate to apply active parameters
                    needsRecreate = qtrue;
                    break;
                }
            }
        }
    }
    
    // If no extendedType, check if we can reuse (same client, same type)
    if (!needsRecreate) {
        if (row->playerData.clientNum == playerData->clientNum &&
            row->type == (playerData->isSpectator ? CHERRYHUD_ROW_TYPE_SPECTATOR : CHERRYHUD_ROW_TYPE_PLAYER)) {
            // Same player - check if data changed significantly
            if (row->playerData.score == playerData->score &&
                row->playerData.ping == playerData->ping &&
                row->playerData.team == playerData->team) {
                needsRecreate = qfalse;  // No major changes, skip recreate
            }
        }
    }
    
    if (needsRecreate) {
        // Full recreate if needed
        CG_CHUDTableCreateRow(table, index, playerData);
    } else {
        // Update in-place: Copy new data and update elements
        memcpy(&row->playerData, playerData, sizeof(cherryhudPlayerData_t));
        
        // Update yPosition (in case spacing changed) - use same calculation as CG_CHUDTableUpdatePosition
        newYPosition = table->context.baseY + (index * (table->context.rowHeight + table->context.rowSpacing));
        if (row->yPosition != newYPosition) {
            row->yPosition = newYPosition;
            
            // Force layout recalculation for all elements in this row
            if (row->elements) {
                cherryhudElement_t* element = row->elements;
                while (element) {
                    // Remove from layout cache to force recalculation with new position
                    CG_CHUDLayoutRemoveElement((void*)element);
                    element = element->next;
                }
            }
        }
    }
}

// ============================================================================
// TABLE RENDERING
// ============================================================================

void CG_CHUDTableRender(cherryhudTable_t* table) {
    int i;
    cherryhudElement_t* element;
    cherryhudTableRow_t* row;
    int elementIndex;
    float maxHeight;
    float tableSpacing;
    
    if (!table || !table->rows || table->numRows <= 0) {
        return;
    }

    // 💖 probably need to remove
    // // Render table background and border
    CG_CHUDTableRenderBackgroundAndBorder(table); 
    
    // Render all rows and their elements
    for (i = 0; i < table->numRows; i++) {
        row = &table->rows[i];
        
        // Check if row is valid
        if (!row) continue;
        
        
        // Render row background and border (if configured)
        CG_CHUDTableRowRenderBackgroundAndBorder(table, row);
        
        // Render all elements in this row using unified rendering system
        element = row->elements;

        while (element) {
            if (element) {
                
                // Recalculate layout for element with current row position
                cherryhudLayoutContext_t layoutContext;
                layoutContext.parentX = table->context.baseX;
                layoutContext.parentY = row->yPosition;
                layoutContext.parentWidth = table->context.width;
                layoutContext.parentHeight = table->context.rowHeight;
                layoutContext.availableWidth = table->context.width;
                layoutContext.availableHeight = table->context.rowHeight;
                layoutContext.currentTime = cg.time;
                layoutContext.forceRecalc = qtrue;
                layoutContext.clientNum = row->playerData.clientNum;
                
                CG_CHUDLayoutCalculateElement(element, &layoutContext);
                
                CG_CHUDRenderElementWithMode(element, table->tableType, table->context.height);
            }
            element = element->next;
            elementIndex++;
        }
    }
}

// Render table border
static void CG_CHUDTableRenderBorder(cherryhudTable_t* table) {
    cherryhudConfig_t config;
    
    if (!table) return;
    
    // Copy table config and apply defaults
    memcpy(&config, &table->config, sizeof(cherryhudConfig_t));
    CG_CHUDConfigDefaultsCheck(&config);
    
    // Check if border is configured
    if (!config.border.isSet || !config.border.color.isSet) {
        return;
    }
    
    
    // Use the existing border function which handles coordinates properly
    CG_CHUDDrawBorder(&config);
}

// ============================================================================
// UNIFIED RENDERING FUNCTIONS
// ============================================================================

// Render table background and border
qboolean CG_CHUDTableRenderBackgroundAndBorder(cherryhudTable_t* table) {
    vec4_t bgColor;
    vec4_t borderColor;
    qboolean rendered = qfalse;
    
    if (!table) {
        return qfalse;
    }
    
    // Render background and border using unified function
    CG_CHUDRenderBackgroundAndBorder(&table->config, table->context.baseX, table->context.baseY, 
                                    table->context.width, table->context.height);
    rendered = qtrue;
    
    return rendered;
}

// Render row background and border directly
qboolean CG_CHUDTableRowRenderBackgroundAndBorder(cherryhudTable_t* table, cherryhudTableRow_t* row) {
    const cherryhudConfig_t* rowConfig;
    vec4_t bgColor;
    vec4_t borderColor;
    qboolean rendered = qfalse;
    
    if (!table || !row) {
        return qfalse;
    }
    
    // Get appropriate row template
    if (row->type == CHERRYHUD_ROW_TYPE_PLAYER || row->type == CHERRYHUD_ROW_TYPE_SELF) {
        rowConfig = CG_CHUDGetPlayerRowTemplate();
    } else {
        rowConfig = CG_CHUDGetSpectatorRowTemplate();
    }
    
    if (!rowConfig) {
        return qfalse;
    }
    
    // Render background and border using unified function
    CG_CHUDRenderBackgroundAndBorder(rowConfig, table->context.baseX, row->yPosition, 
                                    table->context.width, table->context.rowHeight);
    rendered = qtrue;
    
    return rendered;
}


// ============================================================================
// SCOREBOARD RENDERING
// ============================================================================

// Helper function to render title block
static float CG_CHUDRenderTitleBlock(const cherryhudScoreboardBlock_t* block, float scoreboardX, float currentY) {
    cherryhudConfig_t adjustedConfig;
    float titleHeight;
    float blockSpacing;
    qboolean shouldRender = qtrue;
    
    // Check if this title block should be rendered
    if (block->titleBlockIndex >= 0) {
        const cherryhudTitleBlock_t* titleBlock = CG_CHUDGetTitleBlock(block->titleBlockIndex);
        if (titleBlock && CG_CHUDTitleBlockHasContainer(titleBlock)) {
            shouldRender = CG_CHUDContainerHasContent(titleBlock->containerType);
        } else {
        }
    }
    
    if (!shouldRender) {
        return currentY;
    }
    
    // Copy and adjust config
    memcpy(&adjustedConfig, &block->config, sizeof(cherryhudConfig_t));
    adjustedConfig.pos.value[0] = scoreboardX + (block->config.pos.isSet ? block->config.pos.value[0] : 0.0f);
    adjustedConfig.pos.value[1] = currentY + (block->config.pos.isSet ? block->config.pos.value[1] : 0.0f);
    adjustedConfig.pos.isSet = qtrue;
    
    // Render title block
    CG_CHUDTableRenderTitleBlockByIndex(block->titleBlockIndex, &adjustedConfig);
    
    // Calculate height and spacing for next block
    titleHeight = block->config.size.isSet ? block->config.size.value[1] : 12.0f;
    blockSpacing = block->config.rowSpacing.isSet ? block->config.rowSpacing.value : 0.0f;
    
    return currentY + titleHeight + blockSpacing;
}

// Helper function to render players block
static float CG_CHUDRenderPlayersBlock(const cherryhudScoreboardBlock_t* block, float scoreboardX, float currentY) {
    cherryhudConfig_t adjustedConfig;
    cherryhudTable_t* playersTable;
    float rowHeight, rowSpacing;
    float playersHeight;
    float blockSpacing;
    const cherryhudConfig_t* playerRowTemplate;
    vec4_t activePos;
    vec4_t activeSize;
    float currentHeight;
    
    playersTable = CG_CHUDGetPlayersTable();
    if (!playersTable) {
        return currentY;
    }
    
    // Copy and adjust config
    memcpy(&adjustedConfig, &block->config, sizeof(cherryhudConfig_t));
    
    // Calculate current height for extendedType mode detection
    currentHeight = playersTable->context.height > 0.0f ? playersTable->context.height : 200.0f;
    
    // Get active position based on height (for extendedType support)
    CG_CHUDGetActivePos(&block->config, currentHeight, activePos);
    if (activePos[0] != 0.0f || activePos[1] != 0.0f || activePos[2] != 0.0f || activePos[3] != 0.0f) {
        adjustedConfig.pos.value[0] = scoreboardX + activePos[0];
        adjustedConfig.pos.value[1] = currentY + activePos[1];
    } else {
        adjustedConfig.pos.value[0] = scoreboardX + (block->config.pos.isSet ? block->config.pos.value[0] : 0.0f);
        adjustedConfig.pos.value[1] = currentY + (block->config.pos.isSet ? block->config.pos.value[1] : 0.0f);
    }
    adjustedConfig.pos.isSet = qtrue;
    
    // Get active size based on height (for extendedType support)
    CG_CHUDGetActiveSize(&block->config, currentHeight, activeSize);
    if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || activeSize[2] != 0.0f || activeSize[3] != 0.0f) {
        adjustedConfig.size.value[0] = activeSize[0];
        adjustedConfig.size.value[1] = activeSize[1];
        adjustedConfig.size.isSet = qtrue;
    }
    
    // Apply horizontal alignment if set
    if (adjustedConfig.alignH.isSet) {
        float tableWidth = adjustedConfig.size.isSet ? adjustedConfig.size.value[0] : 640.0f;
        adjustedConfig.pos.value[0] = CG_CHUDApplyAlignH(adjustedConfig.pos.value[0], tableWidth, adjustedConfig.alignH.value);
    }
    
    // Note: table position will be updated after building and height calculation
    
    // Render container background if configured
    if (CG_CHUDFill(&adjustedConfig)) {
        CG_CHUDRenderContainerBackgroundWithHeight(&adjustedConfig, 
            adjustedConfig.pos.value[0], 
            adjustedConfig.pos.value[1], 
            adjustedConfig.size.value[0], 
            adjustedConfig.size.value[1],
            currentHeight);
    }
    
    // Build table and calculate height
    CG_CHUDTableBuild(CG_CHUDGetTableManager(), playersTable);
    
    playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
    if (playerRowTemplate) {
        vec4_t activeSize;
        CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS), currentHeight, activeSize);
        
        if (activeSize[1] != 0.0f) {
            rowHeight = activeSize[1];
        } else if (playerRowTemplate->size.isSet) {
            rowHeight = playerRowTemplate->size.value[1];
        } else {
            rowHeight = g_playerRowHeight;
        }
    } else {
        rowHeight = g_playerRowHeight;
    }
    rowSpacing = playersTable->context.rowSpacing;
    playersHeight = (playersTable->numRows * rowHeight) + ((playersTable->numRows - 1) * rowSpacing);
    
    // Update table with calculated height
    adjustedConfig.size.value[1] = playersHeight;
    adjustedConfig.size.isSet = qtrue;
    playersTable->context.rowHeight = rowHeight;
    CG_CHUDTableUpdatePosition(playersTable, &adjustedConfig);
    
    // Render table
    CG_CHUDTableRender(playersTable);
    
    // Calculate spacing for next block
    // block->config.rowSpacing is for spacing AFTER this block (between blocks)
    blockSpacing = block->config.rowSpacing.isSet ? block->config.rowSpacing.value : 0.0f;
    
    return currentY + playersHeight + blockSpacing;
}

// Helper function to render spectators block
static float CG_CHUDRenderSpectatorsBlock(const cherryhudScoreboardBlock_t* block, float scoreboardX, float currentY) {
    cherryhudConfig_t adjustedConfig;
    cherryhudTable_t* spectatorsTable;
    float rowHeight, rowSpacing;
    float spectatorsHeight;
    float blockSpacing;
    const cherryhudConfig_t* spectatorRowTemplate;
    const cherryhudConfig_t* playerRowTemplate;
    vec4_t activePos;
    vec4_t activeSize;
    float currentHeight;
    
    spectatorsTable = CG_CHUDGetSpectatorsTable();
    if (!spectatorsTable) {
        return currentY;
    }
    
    // Copy and adjust config
    memcpy(&adjustedConfig, &block->config, sizeof(cherryhudConfig_t));
    
    // Calculate current height for extendedType mode detection
    currentHeight = spectatorsTable->context.height > 0.0f ? spectatorsTable->context.height : 200.0f;
    
    // Get active position based on height (for extendedType support)
    CG_CHUDGetActivePos(&block->config, currentHeight, activePos);
    if (activePos[0] != 0.0f || activePos[1] != 0.0f || activePos[2] != 0.0f || activePos[3] != 0.0f) {
        adjustedConfig.pos.value[0] = scoreboardX + activePos[0];
        adjustedConfig.pos.value[1] = currentY + activePos[1];
    } else {
        adjustedConfig.pos.value[0] = scoreboardX + (block->config.pos.isSet ? block->config.pos.value[0] : 0.0f);
        adjustedConfig.pos.value[1] = currentY + (block->config.pos.isSet ? block->config.pos.value[1] : 0.0f);
    }
    adjustedConfig.pos.isSet = qtrue;
    
    // Get active size based on height (for extendedType support)
    CG_CHUDGetActiveSize(&block->config, currentHeight, activeSize);
    if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || activeSize[2] != 0.0f || activeSize[3] != 0.0f) {
        adjustedConfig.size.value[0] = activeSize[0];
        adjustedConfig.size.value[1] = activeSize[1];
        adjustedConfig.size.isSet = qtrue;
    }
    
    // Apply horizontal alignment if set
    if (adjustedConfig.alignH.isSet) {
        float tableWidth = adjustedConfig.size.isSet ? adjustedConfig.size.value[0] : 640.0f;
        adjustedConfig.pos.value[0] = CG_CHUDApplyAlignH(adjustedConfig.pos.value[0], tableWidth, adjustedConfig.alignH.value);
    }
    
    // Note: table position will be updated after building and height calculation
    
    // Render container background if configured
    if (CG_CHUDFill(&adjustedConfig)) {
        CG_CHUDRenderContainerBackgroundWithHeight(&adjustedConfig, 
            adjustedConfig.pos.value[0], 
            adjustedConfig.pos.value[1], 
            adjustedConfig.size.value[0], 
            adjustedConfig.size.value[1],
            currentHeight);
    }
    
    // Build table and calculate height
    CG_CHUDTableBuild(CG_CHUDGetTableManager(), spectatorsTable);
    
    spectatorRowTemplate = CG_CHUDGetSpectatorRowTemplate();
    playerRowTemplate = CG_CHUDGetPlayerRowTemplate();
    if (spectatorRowTemplate) {
        vec4_t activeSize;
        CG_CHUDGetActiveSizeFromContainer(spectatorRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS), currentHeight, activeSize);
        
        if (activeSize[1] != 0.0f) {
            rowHeight = activeSize[1];
        } else if (spectatorRowTemplate->size.isSet) {
            rowHeight = spectatorRowTemplate->size.value[1];
        } else if (playerRowTemplate) {
            vec4_t playerActiveSize;
            CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS), currentHeight, playerActiveSize);
            
            if (playerActiveSize[1] != 0.0f) {
                rowHeight = playerActiveSize[1];
            } else if (playerRowTemplate->size.isSet) {
                rowHeight = playerRowTemplate->size.value[1];
            } else {
                rowHeight = g_playerRowHeight;
            }
        } else {
            rowHeight = g_playerRowHeight;
        }
    } else if (playerRowTemplate) {
        vec4_t activeSize;
        CG_CHUDGetActiveSizeFromContainer(playerRowTemplate, CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS), currentHeight, activeSize);
        
        if (activeSize[1] != 0.0f) {
            rowHeight = activeSize[1];
        } else if (playerRowTemplate->size.isSet) {
            rowHeight = playerRowTemplate->size.value[1];
        } else {
            rowHeight = g_playerRowHeight;
        }
    } else {
        rowHeight = g_playerRowHeight;
    }
    rowSpacing = spectatorsTable->context.rowSpacing;
    spectatorsHeight = (spectatorsTable->numRows * rowHeight) + ((spectatorsTable->numRows - 1) * rowSpacing);
    
    // Update table with calculated height
    adjustedConfig.size.value[1] = spectatorsHeight;
    adjustedConfig.size.isSet = qtrue;
    spectatorsTable->context.rowHeight = rowHeight;
    CG_CHUDTableUpdatePosition(spectatorsTable, &adjustedConfig);
    
    // Render table
    CG_CHUDTableRender(spectatorsTable);
    
    // Calculate spacing for next block
    // block->config.rowSpacing is for spacing AFTER this block (between blocks)
    blockSpacing = block->config.rowSpacing.isSet ? block->config.rowSpacing.value : 0.0f;
    
    return currentY + spectatorsHeight + blockSpacing;
}

// Render scoreboard with all blocks in order
// Helper function to validate scoreboard config and visibility
static qboolean CG_CHUDRenderScoreboardValidateConfig(const cherryhudConfig_t** scoreboardConfig) {
    *scoreboardConfig = CG_CHUDGetScoreboardConfig();
    if (!*scoreboardConfig) {
        return qfalse;
    }
    
    // Check scoreboard container visibility using visflags
    if (!CG_CHUDCheckElementVisibility(*scoreboardConfig, cg.clientNum, NULL)) {
        return qfalse; // Don't render scoreboard based on visflags
    }
    
    // Check scoreboard container hideflags
    if (CG_CHUDCheckElementHideFlags(*scoreboardConfig, cg.clientNum, NULL)) {
        return qfalse; // Hide scoreboard based on hideflags
    }
    
    return qtrue;
}

// Helper function to request scoreboard data from server
static void CG_CHUDRenderScoreboardRequestData(void) {
    if (cgs.osp.chud.scoreboard) {
        return; // Already have data
    }
    
    if (!cg.demoPlayback && cg.scoresRequestTime < cg.time) {
        cg.scoresRequestTime = cg.time + 2000;
        cg.realNumClients = CG_CountRealClients();
        trap_SendClientCommand("score");
    }
    
    if (!cg.demoPlayback && cg.statsAllRequestTime < cg.time) {
        cg.statsAllRequestTime = cg.time + 2100;
        trap_SendClientCommand("statsall");
        cgs.be.statsAllRequested = qtrue;
    }
}

// Helper function to prepare scoreboard config
static qboolean CG_CHUDRenderScoreboardPrepareConfig(const cherryhudConfig_t* scoreboardConfig, 
                                                    cherryhudConfig_t* config) {
    cherryhudTable_t* globalTable;
    float dynamicHeight;
    
    // Get global table to use its baseX/baseY
    globalTable = CG_CHUDGetGlobalTable();
    if (!globalTable) {
        return qfalse;
    }
    
    // Copy scoreboard config and apply defaults
    memcpy(config, scoreboardConfig, sizeof(cherryhudConfig_t));
    CG_CHUDConfigDefaultsCheck(config);
    
    // Calculate dynamic scoreboard height
    dynamicHeight = CG_CHUDScoreboardCalculateHeight();
    config->size.value[1] = dynamicHeight;
    config->size.isSet = qtrue;
    
    return qtrue;
}

void CG_CHUDRenderScoreboard(void) {
    const cherryhudConfig_t* scoreboardConfig;
    cherryhudConfig_t config;
    
    // Validate config and visibility
    if (!CG_CHUDRenderScoreboardValidateConfig(&scoreboardConfig)) {
        return;
    }
    
    // Request data from server if needed
    CG_CHUDRenderScoreboardRequestData();
    
    // Prepare config
    if (!CG_CHUDRenderScoreboardPrepareConfig(scoreboardConfig, &config)) {
        return;
    }
    
    // Render elements in correct order based on config parsing order
    // (background and border will be rendered inside with proper alignment)
    CG_CHUDRenderScoreboardElementsInOrder(scoreboardConfig);
}

// Render scoreboard elements in correct order based on config parsing order
void CG_CHUDRenderScoreboardElementsInOrder(const cherryhudConfig_t* scoreboardConfig) {
    float currentY;
    float scoreboardX, scoreboardY;
    const cherryhudScoreboardOrder_t* order;
    cherryhudTable_t* globalTable = CG_CHUDGetGlobalTable();
    float dynamicHeight;
    int i;
    
    // Get scoreboard position as base for relative positioning
    scoreboardX = scoreboardConfig->pos.isSet ? scoreboardConfig->pos.value[0] : 0.0f;
    scoreboardY = scoreboardConfig->pos.isSet ? scoreboardConfig->pos.value[1] : 0.0f;
    
    // Apply scoreboard alignment if set
    if (scoreboardConfig->alignH.isSet) {
        float scoreboardWidth = scoreboardConfig->size.isSet ? scoreboardConfig->size.value[0] : 640.0f;
        scoreboardX = CG_CHUDApplyAlignH(scoreboardX, scoreboardWidth, scoreboardConfig->alignH.value);
    }
    
    // Initialize starting position
    currentY = scoreboardY;
    
    // Calculate dynamic scoreboard height
    dynamicHeight = CG_CHUDScoreboardCalculateHeight();
    

    // Render scoreboard background and border with aligned position directly
    CG_CHUDRenderBackgroundAndBorder(scoreboardConfig, scoreboardX, scoreboardY, 
                                    scoreboardConfig->size.value[0], dynamicHeight);
    
    // Get the order of blocks from config
    order = CG_CHUDScoreboardOrderGet();
    if (!order || order->count == 0) {
        return;
    }


    // Render blocks in the order they were parsed from config
    for (i = 0; i < order->count; i++) {
        const cherryhudScoreboardBlock_t* block = &order->blocks[i];
        
        switch (block->type) {
            case CHERRYHUD_BLOCK_TYPE_TITLE: {
                currentY = CG_CHUDRenderTitleBlock(block, scoreboardX, currentY);
                break;
            }
            
            case CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS: {
                currentY = CG_CHUDRenderPlayersBlock(block, scoreboardX, currentY);
                break;
            }
            
            case CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS: {
                currentY = CG_CHUDRenderSpectatorsBlock(block, scoreboardX, currentY);
                break;
            }
        }
    }
    
}


// Render title block by index
void CG_CHUDTableRenderTitleBlockByIndex(int titleBlockIndex, const cherryhudConfig_t* titleConfig) {
    cherryhudConfig_t config;
    int i;
    int elementCount;
    cherryhudTable_t* table;
    const cherryhudTitleBlock_t* titleBlock;
    cherryhudElement_t* element;
    cherryhudLayoutContext_t elementContext;

    if (!titleConfig || titleBlockIndex < 0) return;
    
    // Get title block by index
    titleBlock = CG_CHUDGetTitleBlock(titleBlockIndex);
    if (!titleBlock) {
        return;
    }
    
    // Check if this title block is associated with a container
    if (CG_CHUDTitleBlockHasContainer(titleBlock)) {
        // Only render if the associated container has content
        if (!CG_CHUDContainerHasContent(titleBlock->containerType)) {
            return;
        }
    }
    
    
    // Copy title block config and apply defaults
    memcpy(&config, &titleBlock->config, sizeof(cherryhudConfig_t));
    
    
    // Use the adjusted position from scoreboard FIRST (already calculated in CG_CHUDRenderScoreboardElementsInOrder)
    if (titleConfig->pos.isSet) {
        config.pos = titleConfig->pos;
    }
    
    // Apply defaults for properties that are still not set (but preserve position)
    CG_CHUDConfigDefaultsCheck(&config);
    
    
    
    // Width inheritance: if title width is 0, inherit from scoreboard config
    CG_CHUDInheritRowWidth(&config);
    
    // Height should come only from config, no inheritance
    // But ensure height is set correctly from title block config
    if (titleBlock->config.size.isSet && titleBlock->config.size.value[1] > 0.0f) {
        config.size.value[1] = titleBlock->config.size.value[1];
        config.size.isSet = qtrue;
    }
    
    
    // Render title row background and border (if configured)
    CG_CHUDRenderBackgroundAndBorder(&config, config.pos.value[0], config.pos.value[1], 
                                    config.size.value[0], config.size.value[1]);
    
    // Render title block elements
    elementCount = titleBlock->elementCount;
    for (i = 0; i < elementCount; i++) {
        element = titleBlock->elements[i];
        if (!element) continue;

        // Clear any cached layout data for this element to avoid stale data
        CG_CHUDLayoutRemoveElement((void*)element);

        // Calculate layout bounds for the element (same as in table rows)
        // Initialize elementContext to avoid garbage data
        memset(&elementContext, 0, sizeof(cherryhudLayoutContext_t));
        elementContext.parentX = config.pos.value[0];
        elementContext.parentY = config.pos.value[1];
        elementContext.parentWidth = config.size.value[0];
        elementContext.parentHeight = config.size.value[1];
        elementContext.availableWidth = config.size.value[0];
        elementContext.availableHeight = config.size.value[1];
        elementContext.currentTime = cg.time;
        elementContext.forceRecalc = qtrue;
        elementContext.clientNum = -1;
        
        
        CG_CHUDLayoutCalculateElement(element, &elementContext);
        
        
        // Render element
        CG_CHUDRenderElement(element);
    }
}

// Render title row
void CG_CHUDTableRenderTitle(cherryhudTable_t* table) {
    int i;
    int elementCount;
    cherryhudElement_t* element;
    
    // Title can be rendered independently of table
    if (!table) {
        return;
    }
    
    // Render title row background and border directly from table config
    CG_CHUDTableRenderBackgroundAndBorder(table);
    
    // Render title row elements
    elementCount = CG_CHUDGetTitleRowElementCount();
    for (i = 0; i < elementCount; i++) {
        const cherryhudConfig_t* elementConfig;
        cherryhudConfig_t elementConfigCopy;
        
        elementConfig = CG_CHUDGetTitleRowElement(i);
        if (!elementConfig) continue;
        
        // Copy element config and inherit from title row
        memcpy(&elementConfigCopy, elementConfig, sizeof(cherryhudConfig_t));
        CG_CHUDInheritConfig(&elementConfigCopy, &table->config);
        
        // Adjust element position relative to title row
        if (elementConfigCopy.pos.isSet && table->config.pos.isSet) {
            elementConfigCopy.pos.value[0] += table->context.baseX;
            elementConfigCopy.pos.value[1] += table->config.pos.value[1];
        } else if (table->config.pos.isSet) {
            elementConfigCopy.pos.value[0] = table->context.baseX;
            elementConfigCopy.pos.value[1] = table->config.pos.value[1];
            elementConfigCopy.pos.isSet = qtrue;
        }
        
        CG_CHUDConfigDefaultsCheck(&elementConfigCopy);
        
        
        // Create and render element using unified factory
        element = CG_CHUDCreateElementFromConfig(&elementConfigCopy, -1, "title"); // -1 for title elements
        if (element) {
            CG_CHUDRenderElement(element);
            CG_CHUDDestroyElement(element);
        }
    }
}

// ============================================================================
// GLOBAL TABLE MANAGEMENT
// ============================================================================

// Add element to a specific row
void CG_CHUDTableAddElementToRow(cherryhudTable_t* table, int rowIndex, cherryhudElement_t* element) {
    cherryhudTableRow_t* row;
    cherryhudElement_t* last;
    
    if (!table || !table->rows || rowIndex < 0 || rowIndex >= table->numRows || !element) return;
    
    row = &table->rows[rowIndex];
    if (!row) return;
    
    // Add to the end of the elements list
    if (!row->elements) {
        row->elements = element;
    } else {
        last = row->elements;
        while (last && last->next) last = last->next;
        if (last) {
            last->next = element;
        }
    }
}

// Get row data
cherryhudTableRow_t* CG_CHUDTableGetRow(cherryhudTable_t* table, int rowIndex) {
    if (!table || !table->rows || rowIndex < 0 || rowIndex >= table->numRows) return NULL;
    return &table->rows[rowIndex];
}

// Get number of rows
int CG_CHUDTableGetNumRows(cherryhudTable_t* table) {
    return table ? table->numRows : 0;
}

// Set table properties
void CG_CHUDTableSetProperties(cherryhudTable_t* table, float baseX, float baseY, float rowHeight, float rowSpacing) {
    if (!table) return;
    
    CG_CHUDTableContextSetPosition(&table->context, baseX, baseY);
    CG_CHUDTableContextSetRowProperties(&table->context, rowHeight, rowSpacing);
}

// Global table management (now uses TableManager)
cherryhudTable_t* CG_CHUDGetGlobalTable(void) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    return manager ? manager->globalTable : NULL;
}

cherryhudTable_t* CG_CHUDGetPlayersTable(void) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    return manager ? manager->playersTable : NULL;
}

cherryhudTable_t* CG_CHUDGetSpectatorsTable(void) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    return manager ? manager->spectatorsTable : NULL;
}

void CG_CHUDSetGlobalTable(cherryhudTable_t* table) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    if (manager) {
        manager->globalTable = table;
    }
}

// Initialize global table
void CG_CHUDTableInit(void) {
    cherryhudConfig_t defaultConfig;
    cherryhudTableManager_t* manager;
    
    
    // Initialize scoreboard element order system
    CG_CHUDScoreboardOrderInit();
    
    // Get or create TableManager
    manager = CG_CHUDGetTableManager();
    if (!manager) {
        return;
    }
    
    // Templates are already loaded by CG_CHUDLoadConfig()
    
    // Always destroy existing table before creating new one
    if (manager->globalTable) {
        CG_CHUDTableDestroy(manager->globalTable);
        manager->globalTable = NULL;
    }
    
    // Get configuration with fallback priority
    CG_CHUDGetConfigWithFallback(&defaultConfig, NULL);
    
    manager->globalTable = CG_CHUDTableCreate(&defaultConfig);
    if (!manager->globalTable) {
        return;
    }
    
    // Build the table to populate it with data
    CG_CHUDTableBuild(manager, manager->globalTable);
    
    // Initialize tables for all loaded config types
    CG_CHUDTableInitByType("scoreboard");
    CG_CHUDTableInitByType("chat");
    CG_CHUDTableInitByType("hud");
}

// Shutdown global table
void CG_CHUDTableShutdown(void) {
    if (g_tableManager) {
        CG_CHUDTableManagerDestroy(g_tableManager);
        g_tableManager = NULL;
    }
}

// Set default table properties
static void CG_CHUDSetDefaultTableProperties(cherryhudTable_t* table) {
    if (!table) return;
    
    // Set default values in context only if not already set
    if (table->context.baseX == 0.0f) {
        table->context.baseX = 0.0f;
    }
    if (table->context.baseY == 0.0f) {
        table->context.baseY = 100.0f;
    }
    if (table->context.rowHeight == 0.0f) {
        table->context.rowHeight = 20.0f;
    }
    if (table->context.rowSpacing == 0.0f) {
        table->context.rowSpacing = 0.0f; // Default: no spacing between rows
    }
    
    // Ensure table has a position in context
    if (!table->config.pos.isSet) {
        table->context.baseX = 0.0f;  // X
        table->context.baseY = 100.0f;  // Y
    } else {
        // Update context from config if config is set
        table->context.baseX = table->config.pos.value[0];
        table->context.baseY = table->config.pos.value[1];
    }
    
    // Ensure table has a size in context
    if (!table->config.size.isSet) {
        // For scoreboard, use size from scoreboard config if available
        const cherryhudConfig_t* scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig && scoreboardConfig->size.isSet) {
            table->context.width = scoreboardConfig->size.value[0];
            table->context.height = scoreboardConfig->size.value[1];
        } else {
            table->context.width = 640.0f;  // Width - full screen width
            table->context.height = 200.0f;  // Height - reasonable default
        }
    } else {
        // Update context from config using active values
        vec4_t activeSize;
        float currentHeight;
        
        currentHeight = table->context.height > 0.0f ? table->context.height : 200.0f;
        CG_CHUDGetActiveSize(&table->config, currentHeight, activeSize);
        
        table->context.width = activeSize[0];
        table->context.height = activeSize[1];
    }
}

// Width inheritance utility - inherits width from scoreboard config
static void CG_CHUDInheritWidth(cherryhudConfig_t* config) {
    const cherryhudConfig_t* scoreboardConfig;
    cherryhudTable_t* table;
    vec4_t activeSize;
    float currentHeight;
    
    if (!config || !config->size.isSet || config->size.value[0] != 0.0f) return;
    
    scoreboardConfig = CG_CHUDGetScoreboardConfig();
    if (scoreboardConfig && scoreboardConfig->size.isSet && scoreboardConfig->size.value[0] > 0.0f) {
        // Use active size from scoreboard config
        currentHeight = 400.0f; // Default height for inheritance
        CG_CHUDGetActiveSize(scoreboardConfig, currentHeight, activeSize);
        config->size.value[0] = activeSize[0];
    } else {
        // Fallback to table width if scoreboard config not available
        table = CG_CHUDGetGlobalTable();
        if (table && table->context.width > 0.0f) {
            config->size.value[0] = table->context.width;
        }
    }
}

void CG_CHUDInheritRowWidth(cherryhudConfig_t* config) {
    CG_CHUDInheritWidth(config);
}

// Get configuration with fallback priority
static void CG_CHUDGetConfigWithFallback(cherryhudConfig_t* config, const char* type) {
    const cherryhudConfig_t* typeConfig;
    const cherryhudConfig_t* defaultContextConfig;
    cherryhudConfig_t* playerRowTemplate;
    
    if (!config) return;
    
    memset(config, 0, sizeof(cherryhudConfig_t));
    
    if (type) {
        // Try to get config for specific type
        typeConfig = CG_CHUDGetConfigByType(type);
        if (typeConfig) {
            memcpy(config, typeConfig, sizeof(cherryhudConfig_t));
            return;
        }
    }
    
    // Fallback to scoreboard config
    typeConfig = CG_CHUDGetScoreboardConfig();
    if (typeConfig) {
        memcpy(config, typeConfig, sizeof(cherryhudConfig_t));
        return;
    }
    
    // Fallback to default config from context
    defaultContextConfig = CG_CHUDGetDefaultConfig();
    if (defaultContextConfig) {
        memcpy(config, defaultContextConfig, sizeof(cherryhudConfig_t));
        return;
    }
    
    // Last resort: try to get config from playerRow template
    playerRowTemplate = (cherryhudConfig_t*)CG_CHUDGetPlayerRowTemplate();
    if (playerRowTemplate) {
        memcpy(config, playerRowTemplate, sizeof(cherryhudConfig_t));
    }
}

// Set default element properties
void CG_CHUDSetDefaultElementProperties(cherryhudConfig_t* config) {
    if (!config) return;
    
    // Use the comprehensive default check function instead of duplicating logic
    CG_CHUDConfigDefaultsCheck(config);
}

// Initialize table by type
// Update table position from playersRows/spectatorsRows config
void CG_CHUDTableUpdatePosition(cherryhudTable_t* table, const cherryhudConfig_t* containerConfig) {
    int i;
    cherryhudTableRow_t* row;
    qboolean positionChanged;
    qboolean sizeChanged;
    qboolean rowSpacingChanged;
    vec4_t activePos;
    vec4_t activeSize;
    float currentHeight;
    
    if (!table || !containerConfig) return;
    
    positionChanged = qfalse;
    sizeChanged = qfalse;
    rowSpacingChanged = qfalse;
    
    // Calculate current height for extendedType mode detection
    currentHeight = table->context.height > 0.0f ? table->context.height : 200.0f;
    
    // Get active position based on height (for extendedType support)
    CG_CHUDGetActivePos(containerConfig, currentHeight, activePos);
    if (activePos[0] != 0.0f || activePos[1] != 0.0f || activePos[2] != 0.0f || activePos[3] != 0.0f) {
        if (table->context.baseX != activePos[0] ||
            table->context.baseY != activePos[1]) {
            positionChanged = qtrue;
        }
        CG_CHUDTableContextSetPosition(&table->context, activePos[0], activePos[1]);
    } else if (containerConfig->pos.isSet) {
        // Fallback to default position
        if (table->context.baseX != containerConfig->pos.value[0] ||
            table->context.baseY != containerConfig->pos.value[1]) {
            positionChanged = qtrue;
        }
        CG_CHUDTableContextSetPosition(&table->context, containerConfig->pos.value[0], containerConfig->pos.value[1]);
    }
    
    // Get active size based on height (for extendedType support)
    CG_CHUDGetActiveSize(containerConfig, currentHeight, activeSize);
    if (activeSize[0] != 0.0f || activeSize[1] != 0.0f || activeSize[2] != 0.0f || activeSize[3] != 0.0f) {
        if (table->context.width != activeSize[0] ||
            table->context.height != activeSize[1]) {
            sizeChanged = qtrue;
        }
        CG_CHUDTableContextSetSize(&table->context, activeSize[0], activeSize[1]);
    } else if (containerConfig->size.isSet) {
        // Fallback to default size
        if (table->context.width != containerConfig->size.value[0] ||
            table->context.height != containerConfig->size.value[1]) {
            sizeChanged = qtrue;
        }
        CG_CHUDTableContextSetSize(&table->context, containerConfig->size.value[0], containerConfig->size.value[1]);
    }
    
    // Width inheritance: if table width is 0, inherit from scoreboard
    if (table->context.width <= 0.0f) {
        const cherryhudConfig_t* scoreboardConfig = CG_CHUDGetScoreboardConfig();
        if (scoreboardConfig && scoreboardConfig->size.isSet && scoreboardConfig->size.value[0] > 0.0f) {
            table->context.width = scoreboardConfig->size.value[0];
        }
    }
    
    // Note: rowSpacing is now managed only by templates (playerRow, spectatorRow)
    // containerConfig->rowSpacing is used only for spacing AFTER the block (between blocks)
    // if (containerConfig->rowSpacing.isSet) {
    //     if (Q_stricmp(table->tableType, "spectators") == 0) {
    //         // For spectators, only update if spectatorRowTemplate doesn't override it
    //         const cherryhudConfig_t* spectatorRowTemplate = CG_CHUDGetSpectatorRowTemplate();
    //         if (!spectatorRowTemplate || !spectatorRowTemplate->rowSpacing.isSet) {
    //             if (table->context.rowSpacing != containerConfig->rowSpacing.value) {
    //                 rowSpacingChanged = qtrue;
    //             }
    //             table->context.rowSpacing = containerConfig->rowSpacing.value;
    //         }
    //     } else {
    //         // For players, always update from containerConfig
    //         if (table->context.rowSpacing != containerConfig->rowSpacing.value) {
    //             rowSpacingChanged = qtrue;
    //         }
    //         table->context.rowSpacing = containerConfig->rowSpacing.value;
    //     }
    // }
    
    // If position, size, or rowSpacing changed, recalculate all row positions
    if ((positionChanged || sizeChanged || rowSpacingChanged) && table->rows && table->numRows > 0) {
        for (i = 0; i < table->numRows; i++) {
            row = &table->rows[i];
            if (row) {
                // Recalculate row position
                row->yPosition = table->context.baseY + (i * (table->context.rowHeight + table->context.rowSpacing));
                
                // Force layout recalculation for all elements in this row
                if (row->elements) {
                    cherryhudElement_t* element = row->elements;
                    while (element) {
                        // Remove from layout cache to force recalculation
                        CG_CHUDLayoutRemoveElement((void*)element);
                        element = element->next;
                    }
                }
            }
        }
        
        // Force layout system recalculation for this table
        CG_CHUDLayoutForceRecalcForTable(table);
    }
}

void CG_CHUDTableInitByType(const char* type) {
    cherryhudConfig_t defaultConfig;
    cherryhudTableManager_t* manager;
    int i;
    
    if (!type) {
        return;
    }
    
    manager = CG_CHUDGetTableManager();
    if (!manager) {
        return;
    }
    
    // Check if table with this type already exists
    for (i = 0; i < manager->tableTypeCount; i++) {
        if (Q_stricmp(manager->tablesByType[i].type, type) == 0) {
            return;
        }
    }
    
    // Add new table type if we have space
    if (manager->tableTypeCount >= MAX_TABLE_TYPES) {
        return;
    }
    
    // Get configuration with fallback priority
    CG_CHUDGetConfigWithFallback(&defaultConfig, type);
    
    // Create table
    manager->tablesByType[manager->tableTypeCount].table = CG_CHUDTableCreate(&defaultConfig);
    if (!manager->tablesByType[manager->tableTypeCount].table) {
        return;
    }
    
    Q_strncpyz(manager->tablesByType[manager->tableTypeCount].type, type, MAX_QPATH);
    manager->tablesByType[manager->tableTypeCount].initialized = qtrue;
    manager->tableTypeCount++;
}

// Get table by type
cherryhudTable_t* CG_CHUDGetTableByType(const char* type) {
    cherryhudTableManager_t* manager;
    int i;
    
    if (!type) {
        return NULL;
    }
    
    manager = CG_CHUDGetTableManager();
    if (!manager) {
        return NULL;
    }
    
    // Find table by type
    for (i = 0; i < manager->tableTypeCount; i++) {
        if (Q_stricmp(manager->tablesByType[i].type, type) == 0 && manager->tablesByType[i].initialized) {
            return manager->tablesByType[i].table;
        }
    }
    
    return NULL;
}

// ============================================================================
// SCOREBOARD ORDER MANAGEMENT
// ============================================================================

void CG_CHUDScoreboardOrderInit(void) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    if (manager && manager->scoreboardOrder) {
        memset(manager->scoreboardOrder, 0, sizeof(cherryhudScoreboardOrder_t));
        manager->blockOrderCounter = 0;
    }
}

void CG_CHUDScoreboardOrderAdd(cherryhudBlockType_t type, const cherryhudConfig_t* config) {
    cherryhudScoreboardBlock_t* block;
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    
    if (!manager || !manager->scoreboardOrder || manager->scoreboardOrder->count >= MAX_SCOREBOARD_BLOCKS) {
        return;
    }
    
    block = &manager->scoreboardOrder->blocks[manager->scoreboardOrder->count];
    block->type = type;
    block->order = manager->blockOrderCounter++;
    memcpy(&block->config, config, sizeof(cherryhudConfig_t));
    
    // No table ID needed for this implementation
    
    // For title blocks, store the index of the title block
    if (type == CHERRYHUD_BLOCK_TYPE_TITLE) {
        // Use the current title block index that was set during parsing
        extern int g_currentTitleBlockIndex;
        block->titleBlockIndex = g_currentTitleBlockIndex;
    } else {
        block->titleBlockIndex = -1; // Not applicable for non-title blocks
    }
    
    manager->scoreboardOrder->count++;
}

void CG_CHUDScoreboardOrderClear(void) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    if (manager && manager->scoreboardOrder) {
        memset(manager->scoreboardOrder, 0, sizeof(cherryhudScoreboardOrder_t));
        manager->blockOrderCounter = 0;
    }
}

const cherryhudScoreboardOrder_t* CG_CHUDScoreboardOrderGet(void) {
    cherryhudTableManager_t* manager = CG_CHUDGetTableManager();
    return manager ? manager->scoreboardOrder : NULL;
}

// Assuming CG_CHUDClearTitleBlocks exists; add element destruction
void CG_CHUDClearTitleBlocks(void) {
    int i, j;
    int blockCount = CG_CHUDGetTitleBlockCount();
    
    for (i = 0; i < blockCount; i++) {
        cherryhudTitleBlock_t* block = (cherryhudTitleBlock_t*)CG_CHUDGetTitleBlock(i);
        if (block) {
            for (j = 0; j < block->elementCount; j++) {
                if (block->elements[j]) {
                    CG_CHUDDestroyElement(block->elements[j]);
                    block->elements[j] = NULL;
                }
            }
            block->elementCount = 0;
        }
    }
    
    // Reset title block count to allow proper recreation
    CG_CHUDResetTitleBlockCount();
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Helper functions for player counting
static void CG_CHUDCountPlayers(int* playerCount, int* spectatorCount) {
    int i;
    clientInfo_t* ci;
    
    *playerCount = 0;
    *spectatorCount = 0;
    
    for (i = 0; i < MAX_CLIENTS; i++) {
        ci = &cgs.clientinfo[i];
        
        // Strict validation - only include valid clients
        if (!ci->infoValid || !ci->name || !ci->name[0]) continue;
        
        // Classify player type based on game mode
        if (ci->team == TEAM_SPECTATOR) {
            (*spectatorCount)++;
        } else if (cgs.gametype <= GT_SINGLE_PLAYER) {
            // FFA mode - all non-spectators are players
            (*playerCount)++;
        } else if (cgs.gametype >= GT_TEAM) {
            // Team mode - only RED and BLUE are players
            if (ci->team == TEAM_RED || ci->team == TEAM_BLUE) {
                (*playerCount)++;
            } else {
                (*spectatorCount)++;
            }
        }
    }
}

int CG_CHUDGetPlayerCount(void) {
    int playerCount, spectatorCount;
    CG_CHUDCountPlayers(&playerCount, &spectatorCount);
    return playerCount;
}

int CG_CHUDGetSpectatorCount(void) {
    int playerCount, spectatorCount;
    CG_CHUDCountPlayers(&playerCount, &spectatorCount);
    return spectatorCount;
}

qboolean CG_CHUDTableExceedsMaxHeight(cherryhudTable_t* table, float maxHeight) {
    float tableHeight;
    
    if (!table) {
        return qfalse;
    }
    
    tableHeight = CG_CHUDTableCalculateHeight(table);
    return (tableHeight > maxHeight);
}
