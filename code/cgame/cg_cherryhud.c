#include "cg_local.h"
#include "../qcommon/qcommon.h"
#include "cg_cherryhud_private.h"

void CG_CHUDDestroyBlock(cherryhudBlock_t* block);


void CG_CHUDRoutenesDestroy(cherryhudElement_t* chud) {
    cherryhudElement_t* tmp;
    
    if (!chud) return;
    
    
    while (chud) {
        if (chud->element.destroy) {
            chud->element.destroy(chud->element.context);
        }
        tmp = chud;
        chud = chud->next;
        Z_Free(tmp);
    }
}





qboolean CG_CHUDLoadConfigWithLexer(const char* filename) {
    fileHandle_t fileHandle;
    int fileContentSize;
    int tableCount, elementCount;
    char* fileContent;
    char cherryhudFilename[MAX_QPATH];
    cherryhudConfig_t config;
    cherryhudConfigParseStatus_t status;
    cherryhudConfigParseResult_t parseResult;
    cherryhudTable_t* table;

    // Check if filename is valid
    if (!filename || !filename[0]) {
        return qfalse;
    }
    
    // Load file
    Com_sprintf(cherryhudFilename, sizeof(cherryhudFilename), "chud/%s.cfg", filename);
    
    fileContentSize = trap_FS_FOpenFile(cherryhudFilename, &fileHandle, FS_READ);
    if (fileContentSize <= 0) {
        return qfalse;
    }
    
    fileContent = (char*)Z_Malloc(fileContentSize + 1);
    OSP_MEMORY_CHECK(fileContent);
    trap_FS_Read(fileContent, fileContentSize, fileHandle);
    fileContent[fileContentSize] = 0;
    trap_FS_FCloseFile(fileHandle);
    
    // Initialize config
    memset(&config, 0, sizeof(config));
    
    // Parse with lexer
    parseResult = CG_CHUDParseConfigWithLexer(fileContent, &config);
    
    Z_Free(fileContent);
    
    if (parseResult.status != CHERRYHUD_CONFIG_OK) {
        const char* errorMsg = "Unknown error";
        switch (parseResult.status) {
            case CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER:
                errorMsg = "Unexpected character or syntax error";
                break;
            case CHERRYHUD_CONFIG_WRONG_ELEMENT_NAME:
                errorMsg = "Unknown element name";
                break;
            case CHERRYHUD_CONFIG_WRONG_COMMAND_NAME:
                errorMsg = "Unknown command or parameter";
                break;
            case CHERRYHUD_CONFIG_LOST_ELEMENT_BODY:
                errorMsg = "Missing element body or closing brace";
                break;
            case CHERRYHUD_CONFIG_END_OF_FILE:
                errorMsg = "Unexpected end of file";
                break;
            default:
                errorMsg = "Parse error";
                break;
        }
        
        // CRITICAL: Clear config to prevent memory leak on parse error
        CG_CHUDClearConfig(&config);
        
        if (parseResult.line > 0) {
            if (parseResult.tokenValue) {
                CG_Printf("^1CherryHUD: Error loading %s - %s at line %d, column %d (token: '%s')\n", 
                          filename, errorMsg, parseResult.line, parseResult.column, parseResult.tokenValue);
            } else {
                CG_Printf("^1CherryHUD: Error loading %s - %s at line %d, column %d\n", 
                          filename, errorMsg, parseResult.line, parseResult.column);
            }
        } else {
            CG_Printf("^1CherryHUD: Error loading ^3%s ^7- ^1%s\n", filename, errorMsg);
        }
        return qfalse;
    }
    
    // Count elements and tables for success message
    elementCount = CG_CHUDGetPlayerInfoTemplateCount();
    table = CG_CHUDGetGlobalTable();
    if (table) {
        tableCount = 1;
    }
    
    // CRITICAL: Clear config to prevent memory leak on successful parse
    CG_CHUDClearConfig(&config);
    
    return qtrue;
}

/*
 * Load SuperHUD config
 */
void CG_CHUDLoadConfig(void)
{
    const char* configFile = "default";
    
    
    // // CRITICAL: Clear old title blocks BEFORE loading new config to prevent memory leaks
    // CG_CHUDClearTitleBlocks();
    
    // Always reset system before loading new config
    CG_CHUDResetSystem();
    
    
    // Load default templates AFTER reset but BEFORE initializing tables
    CG_CHUDLoadDefaultTemplates();
    
    // Initialize table builder - always reinitialize for config reloads
    CG_CHUDTableInit(); // Now uses new clean architecture
    
    // Initialize layout system
    CG_CHUDLayoutInit();
    
    // Check if chud_file is properly initialized
    if (chud_file.string && chud_file.string[0]) {
        configFile = chud_file.string;
    }
    
    // Use only lexer-based parser (legacy parser disabled)
    if (CG_CHUDLoadConfigWithLexer(configFile))
    {
        int elementCount, tableCount;
        cherryhudTable_t* table;
        
               // Count elements and tables for success message
               elementCount = CG_CHUDGetPlayerRowElementCount();
               tableCount = 0;
               table = CG_CHUDGetGlobalTable();
               if (table) {
                   tableCount = 1;
               }
               
        
        // Title blocks are cleared in CG_CHUDResetSystem() before loading new config
        
        CG_Printf("^2CherryHUD: %s loaded successfully - %d tables, %d elements\n", 
                  configFile, tableCount, elementCount);
    }
    else
    {
        if (CG_CHUDLoadConfigWithLexer("default"))
        {
            // Count elements and tables for success message
            int elementCount = CG_CHUDGetPlayerInfoTemplateCount();
            int tableCount = 0;
            cherryhudTable_t* table = CG_CHUDGetGlobalTable();
            if (table) {
                tableCount = 1;
            }
            
            CG_Printf("^2CherryHUD: default loaded successfully - %d tables, %d elements\n", 
                      tableCount, elementCount);
        }
        else
        {
            trap_Cvar_Set("CG_CHUD", "0");
        }
    }
}

void CG_CHUDRoutine(void) {
    // Always check visflags/hideflags first, regardless of button state
    CG_CHUDRenderScoreboard();
}

void CG_CHUDToggleScoreboard(void) {
    static qboolean scoreboardVisible = qfalse;
    cherryhudTable_t* table = CG_CHUDGetGlobalTable();
    
    if (table) {
        scoreboardVisible = !scoreboardVisible;
        
        // For now, we'll always render the table when it exists
        // In the future, we can add a visibility flag to the table structure
    }
}





void CG_CHUDDestroyAllWindows(void) {
    
    // Clear all title blocks to prevent memory leaks
    CG_CHUDClearTitleBlocks();
    
    // Shutdown layout system
    CG_CHUDLayoutShutdown();
    
}

void CG_CHUDRenderBlock(cherryhudBlock_t* block, float parentX, float parentY) {
    cherryhudElement_t* elem;
    cherryhudBlock_t* subBlock;
    cherryhudLayoutContext_t layoutContext;
    cherryhudLayoutBounds_t* blockBounds;

    if (!block) return;


    // Calculate layout for this block
    layoutContext.parentX = parentX;
    layoutContext.parentY = parentY;
    layoutContext.parentWidth = 640.0f; // Default screen width
    layoutContext.parentHeight = 480.0f; // Default screen height
    layoutContext.availableWidth = 640.0f;
    layoutContext.availableHeight = 480.0f;
    layoutContext.currentTime = cg.time;
    layoutContext.forceRecalc = qfalse;
    layoutContext.clientNum = 0; // Default clientNum for non-player elements

    blockBounds = CG_CHUDLayoutCalculateBlock(block, &layoutContext);
    if (!blockBounds || !blockBounds->isValid) {
        return;
    }

    // Render block's background and border
    CG_CHUDFill(&block->config);
    // Use centralized border drawing
    if (block->config.border.isSet && block->config.border.color.isSet) {
        vec4_t borderColor;
        CG_CHUDConfigPickBorderColor(&block->config, borderColor, qfalse);
        CG_CHUDDrawBorderCentralized(block->config.pos.value[0], block->config.pos.value[1], 
                                     block->config.size.value[0], block->config.size.value[1],
                                     block->config.border.sideSizes, borderColor);
    }

    // Calculate layout for elements
    elem = block->elements;
    while (elem) {
        cherryhudLayoutContext_t elementContext;
        cherryhudLayoutBounds_t* elementBounds;
        
        elementContext.parentX = blockBounds->x;
        elementContext.parentY = blockBounds->y;
        elementContext.parentWidth = blockBounds->width;
        elementContext.parentHeight = blockBounds->height;
        elementContext.availableWidth = blockBounds->width;
        elementContext.availableHeight = blockBounds->height;
        elementContext.currentTime = cg.time;
        elementContext.forceRecalc = qfalse;
        
        elementBounds = CG_CHUDLayoutCalculateElement(elem, &elementContext);
        if (elementBounds && elementBounds->isValid) {
            if (elem->element.routine) {
                elem->element.routine(elem->element.context);
            }
        }
        elem = elem->next;
    }

    // Render sub-blocks recursively
    subBlock = block->subblocks;
    while (subBlock) {
        CG_CHUDRenderBlock(subBlock, blockBounds->x, blockBounds->y);
        subBlock = subBlock->next;
    }
}

