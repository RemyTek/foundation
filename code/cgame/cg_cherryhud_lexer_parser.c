#include "cg_cherryhud_private.h"
#include "cg_local.h"
#include "../qcommon/qcommon.h"

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseElementContent(cherryhud_parser_t* parser, cherryhudConfig_t* config);

static int g_currentRowContext = 0;
int g_currentTitleBlockIndex = -1;
static int g_titleBlockDepth = 0;
int g_scoreboardBlockDepth = 0; // Made global for use in handlers
static int g_tempTitleElementCount = 0;

// Local default system - stack of default configs for nested contexts
#define MAX_DEFAULT_STACK_DEPTH 16
static struct {
    cherryhudConfig_t configs[MAX_DEFAULT_STACK_DEPTH];
    qboolean loaded[MAX_DEFAULT_STACK_DEPTH];
    int depth;
} g_defaultStack;

// Current active default config (like SuperHUD behavior)
static cherryhudConfig_t g_currentDefaultConfig;
static qboolean g_hasActiveDefault = qfalse;

// Default stack management functions
static void CG_CHUDDefaultStackInit(void) {
    memset(&g_defaultStack, 0, sizeof(g_defaultStack));
    g_defaultStack.depth = 0;
    
    // Initialize current default config
    memset(&g_currentDefaultConfig, 0, sizeof(g_currentDefaultConfig));
    g_hasActiveDefault = qfalse;
}

// Reset parser static variables - CRITICAL for preventing memory leaks
void CG_CHUDLexerParserResetStatics(void) {
    g_currentRowContext = 0;
    g_currentTitleBlockIndex = -1;
    g_titleBlockDepth = 0;
    g_scoreboardBlockDepth = 0;
    g_tempTitleElementCount = 0;
    
    // Reset default stack
    CG_CHUDDefaultStackInit();
}

void CG_CHUDDefaultStackPush(const cherryhudConfig_t* config) {
    if (g_defaultStack.depth < MAX_DEFAULT_STACK_DEPTH) {
        memcpy(&g_defaultStack.configs[g_defaultStack.depth], config, sizeof(cherryhudConfig_t));
        g_defaultStack.loaded[g_defaultStack.depth] = qtrue;
        g_defaultStack.depth++;
    }
}

void CG_CHUDDefaultStackPop(void) {
    if (g_defaultStack.depth > 0) {
        g_defaultStack.depth--;
        g_defaultStack.loaded[g_defaultStack.depth] = qfalse;
    }
}

const cherryhudConfig_t* CG_CHUDDefaultStackGetCurrent(void) {
    if (g_defaultStack.depth > 0 && g_defaultStack.loaded[g_defaultStack.depth - 1]) {
        return &g_defaultStack.configs[g_defaultStack.depth - 1];
    }
    return NULL;
}

qboolean CG_CHUDDefaultStackHasDefaults(void) {
    return g_defaultStack.depth > 0 && g_defaultStack.loaded[g_defaultStack.depth - 1];
}

// SuperHUD-style default management functions
void CG_CHUDSetCurrentDefaultConfig(const cherryhudConfig_t* config) {
    if (config) {
        memcpy(&g_currentDefaultConfig, config, sizeof(cherryhudConfig_t));
        g_hasActiveDefault = qtrue;
    } else {
        // Clear current default (like !default{})
        memset(&g_currentDefaultConfig, 0, sizeof(cherryhudConfig_t));
        g_hasActiveDefault = qfalse;
    }
}

const cherryhudConfig_t* CG_CHUDGetCurrentDefaultConfig(void) {
    return g_hasActiveDefault ? &g_currentDefaultConfig : NULL;
}

qboolean CG_CHUDHasActiveDefault(void) {
    return g_hasActiveDefault;
}

// Debug function to print current default config
void CG_CHUDDebugPrintCurrentDefault(void) {
    if (g_hasActiveDefault) {
        if (g_currentDefaultConfig.font.isSet) {
        }
        if (g_currentDefaultConfig.textExt.size.isSet) {
        }
        if (g_currentDefaultConfig.textExt.alignH.isSet) {
        }
        if (g_currentDefaultConfig.color.isSet) {
        }
    } else {
    }
}

static qboolean CG_CHUDLexerParserFastElementMatch(const char* token_value, const char* name) {
    if (!token_value || !name) return qfalse;
    
    if (name[0] && !name[1]) {
        return (token_value[0] == name[0] || 
                (token_value[0] >= 'A' && token_value[0] <= 'Z' && 
                 token_value[0] + 32 == name[0]) ||
                (token_value[0] >= 'a' && token_value[0] <= 'z' && 
                 token_value[0] - 32 == name[0])) && !token_value[1];
    }
    
    return Q_stricmp(token_value, name) == 0;
}


cherryhud_parser_t* CG_CHUDLexerParserCreate(const char* input) {
    cherryhud_parser_t* parser;
    
    if (!input) return NULL;
    
    parser = (cherryhud_parser_t*)Z_Malloc(sizeof(cherryhud_parser_t));
    OSP_MEMORY_CHECK(parser);
    memset(parser, 0, sizeof(cherryhud_parser_t));
    
    parser->lexer = CG_CHUDLexerCreate(input);
    if (!parser->lexer) {
        Z_Free(parser);
        return NULL;
    }
    
    CG_CHUDLexerTokenize(parser->lexer);
    parser->token_count = CG_CHUDLexerGetTokenCount(parser->lexer);
    parser->current_token = 0;
    
    parser->cached_token = NULL;
    parser->cached_token_index = -1;
    
    // Initialize configuration storage for this parsing session
    CG_CHUDConfigStorageInit();
    
    // Initialize default stack
    CG_CHUDDefaultStackInit();
    
    // Reset context at the beginning of parsing
    g_currentRowContext = 0;
    g_currentTitleBlockIndex = -1;
    g_titleBlockDepth = 0;
    g_tempTitleElementCount = 0;
    
    return parser;
}

void CG_CHUDLexerParserDestroy(cherryhud_parser_t* parser) {
    if (!parser) return;
    
    if (parser->lexer) {
        CG_CHUDLexerDestroy(parser->lexer);
    }
    
    Z_Free(parser);
}

cherryhud_token_t* CG_CHUDLexerParserGetCurrentToken(cherryhud_parser_t* parser) {
    if (!parser || parser->current_token >= parser->token_count) {
        return NULL;
    }
    
    if (parser->cached_token && parser->cached_token_index == parser->current_token) {
        return parser->cached_token;
    }
    
    parser->cached_token = CG_CHUDLexerGetToken(parser->lexer, parser->current_token);
    parser->cached_token_index = parser->current_token;
    
    return parser->cached_token;
}

cherryhud_token_t* CG_CHUDLexerParserPeekToken(cherryhud_parser_t* parser, int offset) {
    int index = parser->current_token + offset;
    if (!parser || index >= parser->token_count) {
        return NULL;
    }
    return CG_CHUDLexerGetToken(parser->lexer, index);
}

qboolean CG_CHUDLexerParserIsAtEnd(cherryhud_parser_t* parser) {
    return !parser || parser->current_token >= parser->token_count;
}

qboolean CG_CHUDLexerParserMatch(cherryhud_parser_t* parser, cherryhud_token_type_t type) {
    cherryhud_token_t* token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) return qfalse;
    return token->type == type;
}

qboolean CG_CHUDLexerParserMatchElement(cherryhud_parser_t* parser, const char* name) {
    cherryhud_token_t* token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token || token->type != TOKEN_ELEMENT) return qfalse;
    if (!token->value || !name) return qfalse;
    return CG_CHUDLexerParserFastElementMatch(token->value, name);
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserSkipToNextToken(cherryhud_parser_t* parser) {
    cherryhud_token_t* token;
    
    if (!parser) return CHERRYHUD_CONFIG_END_OF_FILE;
    
    if (parser->current_token < parser->token_count) {
        token = CG_CHUDLexerGetToken(parser->lexer, parser->current_token);
        if (token && token->type != TOKEN_WHITESPACE && 
            token->type != TOKEN_NEWLINE && token->type != TOKEN_COMMENT) {
            parser->cached_token = token;
            parser->cached_token_index = parser->current_token;
            return CHERRYHUD_CONFIG_OK;
        }
    }
    
    while (parser->current_token < parser->token_count) {
        token = CG_CHUDLexerGetToken(parser->lexer, parser->current_token);
        if (!token) break;
        
        if (token->type == TOKEN_WHITESPACE || 
            token->type == TOKEN_NEWLINE || 
            token->type == TOKEN_COMMENT) {
            parser->current_token++;
            continue;
        }
        
        parser->cached_token = token;
        parser->cached_token_index = parser->current_token;
        return CHERRYHUD_CONFIG_OK;
    }
    
    return CHERRYHUD_CONFIG_END_OF_FILE;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseValue(cherryhud_parser_t* parser, char* value, int maxlen) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    
    if (token->type == TOKEN_STRING || token->type == TOKEN_ELEMENT) {
        // Allow empty strings - set empty string if token->value is NULL or empty
        if (token->value) {
            Q_strncpyz(value, token->value, maxlen);
        } else {
            value[0] = '\0';
        }
        parser->current_token++;
        return CHERRYHUD_CONFIG_OK;
    }
    
    return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFloat(cherryhud_parser_t* parser, float* value) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    
    if (token->type == TOKEN_NUMBER && token->value) {
        *value = atof(token->value);
        parser->current_token++;
        return CHERRYHUD_CONFIG_OK;
    }
    
    return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseInt(cherryhud_parser_t* parser, int* value) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    
    if (token->type == TOKEN_NUMBER && token->value) {
        *value = atoi(token->value);
        parser->current_token++;
        return CHERRYHUD_CONFIG_OK;
    }
    
    return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVec2(cherryhud_parser_t* parser, vec2_t value) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[0]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[1]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    return CHERRYHUD_CONFIG_OK;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVec3(cherryhud_parser_t* parser, vec3_t value) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[0]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[1]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[2]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    return CHERRYHUD_CONFIG_OK;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVec4(cherryhud_parser_t* parser, vec4_t value) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[0]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[1]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[2]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    status = CG_CHUDLexerParserParseFloat(parser, &value[3]);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    return CHERRYHUD_CONFIG_OK;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    char c;
    
    config->color.isSet = qfalse;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    
    if (token->type == TOKEN_ELEMENT && token->value && strlen(token->value) == 1) {
        c = tolower(token->value[0]);
        switch (c) {
            case 't':
                config->color.value.type = CHERRYHUD_COLOR_T;
                parser->current_token++;
                config->color.isSet = qtrue;
                return CHERRYHUD_CONFIG_OK;
            case 'e':
                config->color.value.type = CHERRYHUD_COLOR_E;
                parser->current_token++;
                config->color.isSet = qtrue;
                return CHERRYHUD_CONFIG_OK;
            case 'i':
                config->color.value.type = CHERRYHUD_COLOR_I;
                parser->current_token++;
                config->color.isSet = qtrue;
                return CHERRYHUD_CONFIG_OK;
            case 'u':
                config->color.value.type = CHERRYHUD_COLOR_U;
                parser->current_token++;
                config->color.isSet = qtrue;
                return CHERRYHUD_CONFIG_OK;
        }
    }
    
    // Parse as RGBA
    config->color.value.type = CHERRYHUD_COLOR_RGBA;
    status = CG_CHUDLexerParserParseVec4(parser, config->color.value.rgba);
    if (status == CHERRYHUD_CONFIG_OK) config->color.isSet = qtrue;
    return status;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseCommand(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    cherryhudConfigParseStatus_t (*parseFunc)(cherryhud_parser_t* parser, cherryhudConfig_t* config);
    const char* commandName;
    char* subCommand;
    char buffer[MAX_QPATH];
    cherryhudConfig_t* targetConfig;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token || token->type != TOKEN_ELEMENT) {
        return CHERRYHUD_CONFIG_WRONG_COMMAND_NAME;
    }
    
    commandName = token->value;
    
    // Check for compact.* or double.* prefixes
    if (Q_strncmp(commandName, "compact.", 8) == 0) {
        // Parse compact.* command
        Q_strncpyz(buffer, commandName, sizeof(buffer));
        subCommand = buffer + 8; // Skip "compact."
        
        
        // Find the base command parser
        parseFunc = CG_CHUDLexerFindCommand(subCommand);
        if (!parseFunc) {
            return CHERRYHUD_CONFIG_WRONG_COMMAND_NAME;
        }
        
        // Get local compact config for this element
        targetConfig = CG_CHUDGetPrefixedConfig(config, "compact");
        if (!targetConfig) {
            return CHERRYHUD_CONFIG_WRONG_COMMAND_NAME;
        }
        
        
        // Advance to next token
        parser->current_token++;
        
        return parseFunc(parser, targetConfig);
        
    } else if (Q_strncmp(commandName, "double.", 7) == 0) {
        // Parse double.* command
        Q_strncpyz(buffer, commandName, sizeof(buffer));
        subCommand = buffer + 7; // Skip "double."
        
        // Find the base command parser
        parseFunc = CG_CHUDLexerFindCommand(subCommand);
        if (!parseFunc) {
            return CHERRYHUD_CONFIG_WRONG_COMMAND_NAME;
        }
        
        // Get local double config for this element
        targetConfig = CG_CHUDGetPrefixedConfig(config, "double");
        if (!targetConfig) {
            return CHERRYHUD_CONFIG_WRONG_COMMAND_NAME;
        }
        
        // Advance to next token
        parser->current_token++;
        
        return parseFunc(parser, targetConfig);
        
    } else {
        // Regular command - use original logic
        parseFunc = CG_CHUDLexerFindCommand(commandName);
        if (!parseFunc) {
            return CHERRYHUD_CONFIG_WRONG_COMMAND_NAME;
        }
        
        // Advance to next token
        parser->current_token++;
        
        return parseFunc(parser, config);
    }
}


cherryhudConfigParseStatus_t CG_CHUDLexerParserParse(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    if (!parser || !config) {
        return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    }
    
    
    while (!CG_CHUDLexerParserIsAtEnd(parser)) {
        status = CG_CHUDLexerParserParseUniversal(parser, config);
        if (status == CHERRYHUD_CONFIG_END_OF_FILE) {
            break;
        }
        if (status != CHERRYHUD_CONFIG_OK) {
            return status;
        }
    }
    
    return CHERRYHUD_CONFIG_OK;
}

cherryhudConfigParseResult_t CG_CHUDParseConfigWithLexer(const char* input, cherryhudConfig_t* config) {
    cherryhud_parser_t* parser;
    cherryhudConfigParseResult_t result;
    cherryhud_token_t* currentToken;
    
    result.status = CHERRYHUD_CONFIG_OK;
    result.line = 0;
    result.column = 0;
    result.tokenValue = NULL;
    
    if (!input || !config) {
        result.status = CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
        return result;
    }
    
    parser = CG_CHUDLexerParserCreate(input);
    if (!parser) {
        result.status = CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
        return result;
    }
    
    result.status = CG_CHUDLexerParserParse(parser, config);
    
    // Process stored element configurations after parsing is complete
    if (result.status == CHERRYHUD_CONFIG_OK) {
        CG_CHUDConfigStorageProcessElements();
    }
    
    if (result.status != CHERRYHUD_CONFIG_OK) {
        currentToken = CG_CHUDLexerParserGetCurrentToken(parser);
        if (currentToken) {
            result.line = currentToken->line;
            result.column = currentToken->column;
            result.tokenValue = currentToken->value;
        }
    }
    
    CG_CHUDLexerParserDestroy(parser);
    
    return result;
}



cherryhudConfigParseStatus_t CG_CHUDLexerParserParseElementContent(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    cherryhudConfig_t elementConfig;
    cherryhud_token_t* token;
    cherryhud_token_t* currentToken;
    const char* elementType;
    const char* elementText;
    qboolean isDefaultElement;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token || !token->value) {
        return CHERRYHUD_CONFIG_WRONG_ELEMENT_NAME;
    }
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    if (!CG_CHUDLexerParserMatch(parser, TOKEN_BLOCK_START)) {
        return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    }
    parser->current_token++;
    
    memset(&elementConfig, 0, sizeof(elementConfig));
    
    // Check if this is a !default element
    isDefaultElement = (token->value && Q_stricmp(token->value, "!default") == 0);
    
    // Only inherit config for !default elements, other elements will get defaults applied later
    if (isDefaultElement) {
        CG_CHUDInheritConfig(&elementConfig, config);
    }
    // For non-default elements, we start with empty config and will apply defaults later
    
    // For title context, clear inherited position to prevent scoreboard position inheritance
    if (g_currentRowContext == CHERRYHUD_CONTEXT_TITLE) {
        // Don't reset position for title elements - preserve user-specified Y position
        // elementConfig.pos is preserved as set by user
    }
    
    while (!CG_CHUDLexerParserIsAtEnd(parser)) {
        status = CG_CHUDLexerParserSkipToNextToken(parser);
        if (status != CHERRYHUD_CONFIG_OK) {
            break;
        }
        
        currentToken = CG_CHUDLexerParserGetCurrentToken(parser);
        
        if (CG_CHUDLexerParserMatch(parser, TOKEN_BLOCK_END)) {
            
            if (g_scoreboardBlockDepth > 0) {
                g_scoreboardBlockDepth--;

                if (g_scoreboardBlockDepth == 0) {
                    // Exiting scoreboard block - reset all contexts
                    g_currentRowContext = 0;
                    g_currentTitleBlockIndex = -1;
                    g_titleBlockDepth = 0;
                    g_tempTitleElementCount = 0;
                } else {
                }
            } else if (g_currentRowContext == CHERRYHUD_CONTEXT_TITLE && g_titleBlockDepth > 0) {
                g_titleBlockDepth--;
                // Context remains as title (3) until we exit the entire scoreboard block
            } else {
                // Reset context when exiting other block types
                // This ensures that title blocks are created as regular blocks by default
                if (g_currentRowContext != 0) {
                    g_currentRowContext = 0;
                }
            }
            
            // Pop local defaults when exiting a block
            CG_CHUDDefaultStackPop();

            parser->current_token++;
            break;
        }
        
        status = CG_CHUDLexerParserParseCommand(parser, &elementConfig);
        if (status == CHERRYHUD_CONFIG_WRONG_COMMAND_NAME) {
            status = CG_CHUDLexerParserParseUniversal(parser, &elementConfig);
            if (status != CHERRYHUD_CONFIG_OK) {
                return status;
            }
        } else if (status != CHERRYHUD_CONFIG_OK) {
            return status;
        }
    }
    
    return CHERRYHUD_CONFIG_OK;
}

/* Helper function to process serverInfo elements */
static cherryhudConfigParseStatus_t CG_CHUDProcessServerInfoElement(const char* elementName, cherryhudConfig_t* elementConfig) {
    char originalType[MAX_QPATH] = "";
    
    
    if (elementConfig->type.isSet) {
        Q_strncpyz(originalType, elementConfig->type.value, sizeof(originalType));
    }
    
    /* Keep the original type in type.value - decor element will process it */
    if (originalType[0]) {
        /* Keep the original type - decor element will process it */
        Q_strncpyz(elementConfig->type.value, originalType, sizeof(elementConfig->type.value));
        elementConfig->type.isSet = qtrue;
    } else {
        /* No original type - set default */
        Q_strncpyz(elementConfig->type.value, "player.name", sizeof(elementConfig->type.value));
        elementConfig->type.isSet = qtrue;
    }
    
    return CHERRYHUD_CONFIG_OK;
}

/* Helper function to set context for block elements */
static void CG_CHUDSetBlockContext(const cherryhudElementTypeInfo_t* contextTypeInfo, const char* blockName) {
    if (contextTypeInfo && (contextTypeInfo->category == CHERRYHUD_TYPE_BLOCK || contextTypeInfo->category == CHERRYHUD_TYPE_CONTAINER)) {
        int bonusFlag = contextTypeInfo->bonusFlag;
        int context = bonusFlag & (CHERRYHUD_CONTEXT_PLAYER_ROW | CHERRYHUD_CONTEXT_SPECTATOR_ROW | CHERRYHUD_CONTEXT_TITLE);
        
        
        if (context >= CHERRYHUD_CONTEXT_PLAYER_ROW && context <= CHERRYHUD_CONTEXT_TITLE) {
            int oldContext = g_currentRowContext;
            g_currentRowContext = context;
        } else {
        }
    } else {
    }
}

/* Helper function to process element based on type info */
static cherryhudConfigParseStatus_t CG_CHUDProcessElementByType(const cherryhudElementTypeInfo_t* typeInfo, const char* elementName, cherryhudConfig_t* elementConfig) {
    int bonusFlag;
    qboolean isDefault;
    
    // Check if this is a !default element
    isDefault = (elementName && Q_stricmp(elementName, "!default") == 0);
    
    // Apply default configuration for all elements except !default itself
    if (!isDefault) {
        CG_CHUDApplyDefaultConfig(elementConfig);
    }
    
    if (!typeInfo) {
        // For title context, add element directly to current title block
        if (g_currentRowContext == CHERRYHUD_CONTEXT_TITLE && g_currentTitleBlockIndex >= 0) {
            CG_CHUDAddTitleBlockElement(g_currentTitleBlockIndex, elementConfig);
        } else {
            CG_CHUDConfigStorageAddElement(elementName, elementConfig, g_currentRowContext);
        }
        return CHERRYHUD_CONFIG_OK;
    }
    
    bonusFlag = typeInfo->bonusFlag;
    
    switch (typeInfo->category) {
        case CHERRYHUD_TYPE_ELEMENT:
            
            /* No special handling needed for element groups */
            
            // For title context, add element directly to current title block
            if (g_currentRowContext == CHERRYHUD_CONTEXT_TITLE && g_currentTitleBlockIndex >= 0) {
                CG_CHUDAddTitleBlockElement(g_currentTitleBlockIndex, elementConfig);
            } else {
                /* Store element configuration instead of creating it immediately */
                CG_CHUDConfigStorageAddElement(elementName, elementConfig, g_currentRowContext);
            }
            break;
            
        case CHERRYHUD_TYPE_BLOCK:
            
            /* Check if this is a template block - if so, only set template, don't render */
            if (typeInfo && CG_CHUDIsTemplateBlock(typeInfo->bonusFlag)) {
                /* Use centralized handler system */
                CG_CHUDHandleElement(elementName, elementConfig);
            } else {
                /* Use centralized handler system */
                CG_CHUDHandleElement(elementName, elementConfig);
            }
            break;
            
        case CHERRYHUD_TYPE_CONTAINER:
            /* Use centralized handler system */
            CG_CHUDHandleElement(elementName, elementConfig);
            break;
            
        case CHERRYHUD_TYPE_CONFIG:
            /* Use centralized handler system */
            CG_CHUDHandleElement(elementName, elementConfig);
            break;
            
        default:
            break;
    }
    
    return CHERRYHUD_CONFIG_OK;
}

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseUniversal(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    cherryhudConfig_t elementConfig;
    cherryhudTable_t* table;
    cherryhudTitleBlock_t* block;
    const cherryhudElementTypeInfo_t* contextTypeInfo;
    const cherryhudElementTypeInfo_t* typeInfo;
    qboolean isTitleBlock;
    qboolean isDefaultElement;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token || token->type == TOKEN_EOF) {
        return CHERRYHUD_CONFIG_END_OF_FILE;
    }
    
    if (token->type != TOKEN_ELEMENT) {
        return CHERRYHUD_CONFIG_WRONG_ELEMENT_NAME;
    }
    
    
    parser->current_token++;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
        if (CG_CHUDLexerParserMatch(parser, TOKEN_BLOCK_START)) {
            int contextBeforeBlock;
            parser->current_token++;
            
            if (g_currentRowContext == 3) {
                g_titleBlockDepth++;
            }
        
        /* Set context BEFORE parsing contents for row blocks using centralized type system */
        contextTypeInfo = CG_CHUDGetElementTypeInfo(token->value);
        
        // Store the current context BEFORE setting new context for title blocks
        contextBeforeBlock = g_currentRowContext;
        
        
        CG_CHUDSetBlockContext(contextTypeInfo, token->value);
        
        
        memset(&elementConfig, 0, sizeof(elementConfig));
        
        // Check if this is a !default element
        isDefaultElement = (token->value && Q_stricmp(token->value, "!default") == 0);
        
        // Only inherit config for !default elements, other elements will get defaults applied later
        if (isDefaultElement) {
            // For !default elements, inherit from parent config
            if (g_currentRowContext == CHERRYHUD_CONTEXT_TITLE) {
                // For title context, inherit only non-position and non-size properties
                // Create a temporary config without position and size for inheritance
                cherryhudConfig_t tempConfig;
                memcpy(&tempConfig, config, sizeof(cherryhudConfig_t));
                tempConfig.pos.isSet = qfalse;
                tempConfig.size.isSet = qfalse;
                CG_CHUDInheritConfig(&elementConfig, &tempConfig);
                
            } else {
                // For non-title context, inherit normally
                CG_CHUDInheritConfig(&elementConfig, config);
            }
        }
        // For non-default elements, we start with empty config and will apply defaults later
        
        // Check if this is a title block, but don't create it yet
        // We need to parse all commands first to get the complete config
        isTitleBlock = qfalse;
        if (contextTypeInfo && contextTypeInfo->category == CHERRYHUD_TYPE_BLOCK && 
            (contextTypeInfo->bonusFlag & CHERRYHUD_CONTEXT_TITLE) == CHERRYHUD_CONTEXT_TITLE) {
            int parentContext;
            isTitleBlock = qtrue;
            
            // For title blocks, we need to determine if they are container titles
            // based on the current parsing context, not the stored context
            parentContext = contextBeforeBlock;
            
            
            // Set context for title elements to be added to this block
            g_currentRowContext = CHERRYHUD_CONTEXT_TITLE;
            g_titleBlockDepth++;
            
            // Check if we're inside a container context
            // A title block is a container title ONLY if we're currently parsing inside a container
            // We can determine this by checking if we're in a row context AND the context is valid
            if (parentContext == CHERRYHUD_CONTEXT_PLAYER_ROW || 
                parentContext == CHERRYHUD_CONTEXT_SPECTATOR_ROW) {
                // This title block is inside a container - create it as a container title
                const char* containerType = NULL;
                if (parentContext == CHERRYHUD_CONTEXT_PLAYER_ROW) {
                    containerType = CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS);
                } else if (parentContext == CHERRYHUD_CONTEXT_SPECTATOR_ROW) {
                    containerType = CG_CHUDGetContainerName(CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS);
                }
                CG_CHUDCreateTitleBlockForContainer(&elementConfig, containerType);
                g_currentTitleBlockIndex = CG_CHUDGetTitleBlockCount() - 1;
            } else {
                // Regular title block - this is the default case
                CG_CHUDCreateTitleBlock(&elementConfig);
                g_currentTitleBlockIndex = CG_CHUDGetTitleBlockCount() - 1;
            }
        }

        // Parse contents of the block FIRST to get complete config
        while (!CG_CHUDLexerParserIsAtEnd(parser)) {
            status = CG_CHUDLexerParserSkipToNextToken(parser);
            if (status != CHERRYHUD_CONFIG_OK) break;

            if (CG_CHUDLexerParserMatch(parser, TOKEN_BLOCK_END)) {
                // Pop local defaults when exiting a block
                CG_CHUDDefaultStackPop();
                parser->current_token++;
                break;
            }

            status = CG_CHUDLexerParserParseCommand(parser, &elementConfig);
            if (status == CHERRYHUD_CONFIG_WRONG_COMMAND_NAME) {
                status = CG_CHUDLexerParserParseUniversal(parser, &elementConfig);
                if (status != CHERRYHUD_CONFIG_OK) {
                    return status;
                }
            } else if (status != CHERRYHUD_CONFIG_OK) {
                return status;
            }
        }

        // NOW update title block config and add to scoreboard order (title block was already created earlier)
        if (isTitleBlock) {
            
            // Update the title block config with the complete configuration
            CG_CHUDUpdateTitleBlockConfig(g_currentTitleBlockIndex, &elementConfig);
            
            // Add to scoreboard order
            CG_CHUDScoreboardOrderAdd(CHERRYHUD_BLOCK_TYPE_TITLE, &elementConfig);
        }

        
        /* Get element type info for special handling */
        typeInfo = CG_CHUDGetElementTypeInfo(token->value);
        
        /* No special handling needed - all elements use unified decor system */
        
        /* Process element based on type info */
        CG_CHUDProcessElementByType(typeInfo, token->value, &elementConfig);
        
    } else {
        // Simple element without block
    }
    
    return CHERRYHUD_CONFIG_OK;
}
