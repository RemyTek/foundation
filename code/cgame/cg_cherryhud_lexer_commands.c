#include "cg_cherryhud_private.h"
#include "cg_local.h"
#include "../qcommon/qcommon.h"


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseEnum(cherryhud_parser_t* parser, const char** enumValues, int enumCount, int* result);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseSimple(cherryhud_parser_t* parser, void* target, int type);

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseAlignH(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseAlignV(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseAngles(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBgColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBgColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseDirection(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFade(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFadeDelay(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFill(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImage(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageTC(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseItTeam(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMargins(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseModel(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMonospace(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseOffset(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseText(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlign(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlignV(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextOffset(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTime(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVisFlags(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseHideFlag(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseHlColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseHlSize(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBorder(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBorderColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBorderColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseStyle(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseDoublebar(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseType(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParsePos(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseSize(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseContent(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMaxWidth(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMaxValue(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBgColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBorder(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBorderColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageBgColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageBorder(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageBorderColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseRowSpacing(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseWeapon(cherryhud_parser_t* parser, cherryhudConfig_t* config);
// Extended text parameter parsers
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextSize(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextPos(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBackground(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlignV(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlignH(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextShadow(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextShadowColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextFont(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextFormat(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseExtendedType(cherryhud_parser_t* parser, cherryhudConfig_t* config);
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTableSpacing(cherryhud_parser_t* parser, cherryhudConfig_t* config);


static struct {
    const char* name;
    cherryhudConfigParseStatus_t (*parse)(cherryhud_parser_t* parser, cherryhudConfig_t* config);
} lexerCommandTable[] = {
    { "alignh", CG_CHUDLexerParserParseAlignH },
    { "alignv", CG_CHUDLexerParserParseAlignV },
    { "angles", CG_CHUDLexerParserParseAngles },
    { "bgcolor", CG_CHUDLexerParserParseBgColor },
    { "bgcolor2", CG_CHUDLexerParserParseBgColor2 },
    { "hlcolor", CG_CHUDLexerParserParseHlColor },
    { "color", CG_CHUDLexerParserParseColor },
    { "color2", CG_CHUDLexerParserParseColor2 },
    { "direction", CG_CHUDLexerParserParseDirection },
    { "doublebar", CG_CHUDLexerParserParseDoublebar },
    { "fade", CG_CHUDLexerParserParseFade },
    { "fadedelay", CG_CHUDLexerParserParseFadeDelay },
    { "fill", CG_CHUDLexerParserParseFill },
    { "font", CG_CHUDLexerParserParseTextFont },
    { "image", CG_CHUDLexerParserParseImage },
    { "imagetc", CG_CHUDLexerParserParseImageTC },
    { "itteam", CG_CHUDLexerParserParseItTeam },
    { "margins", CG_CHUDLexerParserParseMargins },
    { "model", CG_CHUDLexerParserParseModel },
    { "monospace", CG_CHUDLexerParserParseMonospace },
    { "offset", CG_CHUDLexerParserParseOffset },
    { "text", CG_CHUDLexerParserParseText },
    { "textoffset", CG_CHUDLexerParserParseTextOffset },
    { "time", CG_CHUDLexerParserParseTime },
    { "visflags", CG_CHUDLexerParserParseVisFlags },
    { "hideflags", CG_CHUDLexerParserParseHideFlag },
    { "hlsize", CG_CHUDLexerParserParseHlSize },
    { "border", CG_CHUDLexerParserParseBorder },
    { "borderColor", CG_CHUDLexerParserParseBorderColor },
    { "borderColor2", CG_CHUDLexerParserParseBorderColor2 },
    { "style", CG_CHUDLexerParserParseStyle },
    { "type", CG_CHUDLexerParserParseType },
    { "pos", CG_CHUDLexerParserParsePos },
    { "size", CG_CHUDLexerParserParseSize },
    { "content", CG_CHUDLexerParserParseContent },
    { "maxwidth", CG_CHUDLexerParserParseMaxWidth },
    { "maxvalue", CG_CHUDLexerParserParseMaxValue },
    { "text.bgcolor", CG_CHUDLexerParserParseTextBgColor },
    { "image.bgcolor", CG_CHUDLexerParserParseImageBgColor },
    { "image.border", CG_CHUDLexerParserParseImageBorder },
    { "image.bordercolor", CG_CHUDLexerParserParseImageBorderColor },
    { "rowSpacing", CG_CHUDLexerParserParseRowSpacing },
    { "weapon", CG_CHUDLexerParserParseWeapon },
    { "extendedType", CG_CHUDLexerParserParseExtendedType },
    { "tableSpacing", CG_CHUDLexerParserParseTableSpacing },
    // Extended text parameters
    { "text.size", CG_CHUDLexerParserParseTextSize },
    { "text.pos", CG_CHUDLexerParserParseTextPos },
    { "text.color", CG_CHUDLexerParserParseTextColor },
    { "text.color2", CG_CHUDLexerParserParseTextColor2 },
    { "text.background", CG_CHUDLexerParserParseTextBackground },
    { "text.border", CG_CHUDLexerParserParseTextBorder },
    { "text.bordercolor", CG_CHUDLexerParserParseTextBorderColor },
    { "text.alignv", CG_CHUDLexerParserParseTextAlignV },
    { "text.alignh", CG_CHUDLexerParserParseTextAlignH },
    { "text.shadow", CG_CHUDLexerParserParseTextShadow },
    { "text.shadow.color", CG_CHUDLexerParserParseTextShadowColor },
    { "text.font", CG_CHUDLexerParserParseTextFont },
    { "text.format", CG_CHUDLexerParserParseTextFormat },
    { NULL, NULL }
};

cherryhudConfigParseStatus_t (*CG_CHUDLexerFindCommand(const char* name))(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    int i;
        
    for (i = 0; lexerCommandTable[i].name; i++) {
        if (Q_stricmp(lexerCommandTable[i].name, name) == 0) {
            return lexerCommandTable[i].parse;
        }
    }
    return NULL;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseColorHelper(cherryhud_parser_t* parser, cherryhudColor_t* outColor) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    char c;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    
    if (token->type == TOKEN_ELEMENT && token->value && strlen(token->value) == 1) {
        c = tolower(token->value[0]);
        switch (c) {
            case 't':
                outColor->type = CHERRYHUD_COLOR_T;
                parser->current_token++;
                return CHERRYHUD_CONFIG_OK;
            case 'e':
                outColor->type = CHERRYHUD_COLOR_E;
                parser->current_token++;
                return CHERRYHUD_CONFIG_OK;
            case 'i':
                outColor->type = CHERRYHUD_COLOR_I;
                parser->current_token++;
                return CHERRYHUD_CONFIG_OK;
            case 'u':
                outColor->type = CHERRYHUD_COLOR_U;
                parser->current_token++;
                return CHERRYHUD_CONFIG_OK;
        }
    }
    
    // Parse as RGBA
    outColor->type = CHERRYHUD_COLOR_RGBA;
    return CG_CHUDLexerParserParseVec4(parser, outColor->rgba);
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVisFlagsHelper(cherryhud_parser_t* parser, cherryhudVisFlags_t* visflags) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    int flagsFound = 0;
    int currentFlags = 0;
    qboolean hasPlusOperator = qfalse;
    
    // Initialize visflags
    visflags->flags = 0;
    visflags->op = CHERRYHUD_VISFLAGS_OP_OR; // Default to OR operation
    visflags->isSet = qfalse;
    
    // Parse all tokens until we hit a block end or semicolon
    while (!CG_CHUDLexerParserIsAtEnd(parser)) {
        status = CG_CHUDLexerParserSkipToNextToken(parser);
        if (status != CHERRYHUD_CONFIG_OK) break;
        
        token = CG_CHUDLexerParserGetCurrentToken(parser);
        if (!token) break;
        
        // Stop at block end or semicolon
        if (token->type == TOKEN_BLOCK_END || token->type == TOKEN_SEMICOLON) {
            break;
        }
        
        if (token->type == TOKEN_ELEMENT && token->value) {
            char* tokenValue = token->value;
            char* plusPos;
            
            // Check if this token contains + (indicates AND operation)
            if (strchr(tokenValue, '+')) {
                hasPlusOperator = qtrue;
            }
            
            // Check for + operator (AND operation)
            plusPos = strchr(tokenValue, '+');
            if (plusPos) {
                char* firstPart;
                char* secondPart;
                visflags->op = CHERRYHUD_VISFLAGS_OP_AND;
                
                // Split the token by + and parse each part
                firstPart = tokenValue;
                secondPart = plusPos + 1;
                *plusPos = '\0'; // Null-terminate first part
                
                // Parse first part
                if (Q_stricmp(firstPart, SE_IM_STR) == 0) {
                    currentFlags |= SE_IM;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_TEAM_ONLY_STR) == 0) {
                    currentFlags |= SE_TEAM_ONLY;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_SPECT_STR) == 0) {
                    currentFlags |= SE_SPECT;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_DEAD_STR) == 0) {
                    currentFlags |= SE_DEAD;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_DEMO_HIDE_STR) == 0) {
                    currentFlags |= SE_DEMO_HIDE;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_SCORES_HIDE_STR) == 0) {
                    currentFlags |= SE_SCORES_HIDE;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_KEY1_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY1_SHOW;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_KEY2_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY2_SHOW;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_KEY3_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY3_SHOW;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_KEY4_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY4_SHOW;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_GT_FFA_STR) == 0) {
                    currentFlags |= SE_GT_FFA;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_GT_TDM_STR) == 0) {
                    currentFlags |= SE_GT_TDM;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_GT_CTF_STR) == 0) {
                    currentFlags |= SE_GT_CTF;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_GT_TOURNEY_STR) == 0) {
                    currentFlags |= SE_GT_TOURNEY;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_GT_FREEZETAG_STR) == 0) {
                    currentFlags |= SE_GT_FREEZETAG;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_GT_CLANARENA_STR) == 0) {
                    currentFlags |= SE_GT_CLANARENA;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_SHOW_EMPTY_STR) == 0) {
                    currentFlags |= SE_SHOW_EMPTY;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_LOCAL_CLIENT_STR) == 0 || 
                           Q_stricmp(firstPart, SE_LOCAL_CLIENT_STR_ALT) == 0) {
                    currentFlags |= SE_LOCAL_CLIENT;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_ACTIVE_PLAYER_STR) == 0) {
                    currentFlags |= SE_ACTIVE_PLAYER;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_READY_STR) == 0) {
                    currentFlags |= SE_READY;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_FROZEN_STR) == 0) {
                    currentFlags |= SE_FROZEN;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_COMPACT_MODE_STR) == 0) {
                    currentFlags |= SE_COMPACT_MODE;
                flagsFound++;
                } else if (Q_stricmp(firstPart, SE_DOUBLE_MODE_STR) == 0) {
                    currentFlags |= SE_DOUBLE_MODE;
                flagsFound++;
                }
                
                // Parse second part
                if (Q_stricmp(secondPart, SE_IM_STR) == 0) {
                    currentFlags |= SE_IM;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_TEAM_ONLY_STR) == 0) {
                    currentFlags |= SE_TEAM_ONLY;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_SPECT_STR) == 0) {
                    currentFlags |= SE_SPECT;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_DEAD_STR) == 0) {
                    currentFlags |= SE_DEAD;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_DEMO_HIDE_STR) == 0) {
                    currentFlags |= SE_DEMO_HIDE;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_SCORES_HIDE_STR) == 0) {
                    currentFlags |= SE_SCORES_HIDE;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_KEY1_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY1_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_KEY2_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY2_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_KEY3_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY3_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_KEY4_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY4_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_GT_FFA_STR) == 0) {
                    currentFlags |= SE_GT_FFA;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_GT_TDM_STR) == 0) {
                    currentFlags |= SE_GT_TDM;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_GT_CTF_STR) == 0) {
                    currentFlags |= SE_GT_CTF;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_GT_TOURNEY_STR) == 0) {
                    currentFlags |= SE_GT_TOURNEY;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_GT_FREEZETAG_STR) == 0) {
                    currentFlags |= SE_GT_FREEZETAG;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_GT_CLANARENA_STR) == 0) {
                    currentFlags |= SE_GT_CLANARENA;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_SHOW_EMPTY_STR) == 0) {
                    currentFlags |= SE_SHOW_EMPTY;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_LOCAL_CLIENT_STR) == 0 || 
                           Q_stricmp(secondPart, SE_LOCAL_CLIENT_STR_ALT) == 0) {
                    currentFlags |= SE_LOCAL_CLIENT;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_ACTIVE_PLAYER_STR) == 0) {
                    currentFlags |= SE_ACTIVE_PLAYER;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_READY_STR) == 0) {
                    currentFlags |= SE_READY;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_FROZEN_STR) == 0) {
                    currentFlags |= SE_FROZEN;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_COMPACT_MODE_STR) == 0) {
                    currentFlags |= SE_COMPACT_MODE;
                    flagsFound++;
                } else if (Q_stricmp(secondPart, SE_DOUBLE_MODE_STR) == 0) {
                    currentFlags |= SE_DOUBLE_MODE;
                    flagsFound++;
                }
                
                // Restore the + character
                *plusPos = '+';
            } else {
                // No + operator, parse as single flag
                if (Q_stricmp(tokenValue, SE_IM_STR) == 0) {
                    currentFlags |= SE_IM;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_TEAM_ONLY_STR) == 0) {
                    currentFlags |= SE_TEAM_ONLY;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_SPECT_STR) == 0) {
                    currentFlags |= SE_SPECT;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_DEAD_STR) == 0) {
                    currentFlags |= SE_DEAD;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_DEMO_HIDE_STR) == 0) {
                    currentFlags |= SE_DEMO_HIDE;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_SCORES_HIDE_STR) == 0) {
                    currentFlags |= SE_SCORES_HIDE;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_KEY1_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY1_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_KEY2_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY2_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_KEY3_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY3_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_KEY4_SHOW_STR) == 0) {
                    currentFlags |= SE_KEY4_SHOW;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_GT_FFA_STR) == 0) {
                    currentFlags |= SE_GT_FFA;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_GT_TDM_STR) == 0) {
                    currentFlags |= SE_GT_TDM;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_GT_CTF_STR) == 0) {
                    currentFlags |= SE_GT_CTF;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_GT_TOURNEY_STR) == 0) {
                    currentFlags |= SE_GT_TOURNEY;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_GT_FREEZETAG_STR) == 0) {
                    currentFlags |= SE_GT_FREEZETAG;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_GT_CLANARENA_STR) == 0) {
                    currentFlags |= SE_GT_CLANARENA;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_SHOW_EMPTY_STR) == 0) {
                    currentFlags |= SE_SHOW_EMPTY;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_LOCAL_CLIENT_STR) == 0 || 
                           Q_stricmp(tokenValue, SE_LOCAL_CLIENT_STR_ALT) == 0) {
                    currentFlags |= SE_LOCAL_CLIENT;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_ACTIVE_PLAYER_STR) == 0) {
                    currentFlags |= SE_ACTIVE_PLAYER;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_READY_STR) == 0) {
                    currentFlags |= SE_READY;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_FROZEN_STR) == 0) {
                    currentFlags |= SE_FROZEN;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_COMPACT_MODE_STR) == 0) {
                    currentFlags |= SE_COMPACT_MODE;
                    flagsFound++;
                } else if (Q_stricmp(tokenValue, SE_DOUBLE_MODE_STR) == 0) {
                    currentFlags |= SE_DOUBLE_MODE;
                    flagsFound++;
            } else {
                // Unknown command - stop parsing and let the next command handle it
                parser->current_token--;
                break;
                }
            }
        }
        
        parser->current_token++;
    }
    
    visflags->flags = currentFlags;
    visflags->isSet = (flagsFound > 0);
    
    // Set AND operation if + was detected anywhere
    if (hasPlusOperator) {
        visflags->op = CHERRYHUD_VISFLAGS_OP_AND;
    }
    
    return flagsFound > 0 ? CHERRYHUD_CONFIG_OK : CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
}


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseEnum(cherryhud_parser_t* parser, const char** enumValues, int enumCount, int* result) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    char c;
    int i;
    
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token || token->type != TOKEN_ELEMENT) return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
    
    if (token->value) {
        // Check for single character values (backward compatibility)
        if (strlen(token->value) == 1) {
            c = tolower(token->value[0]);
            for (i = 0; i < enumCount; i++) {
                if (enumValues[i] && strlen(enumValues[i]) == 1 && tolower(enumValues[i][0]) == c) {
                    *result = i;
                    parser->current_token++;
                    return CHERRYHUD_CONFIG_OK;
                }
            }
        }
        
        // Check for multi-character values
        for (i = 0; i < enumCount; i++) {
            if (enumValues[i] && Q_stricmp(token->value, enumValues[i]) == 0) {
                *result = i;
                parser->current_token++;
                return CHERRYHUD_CONFIG_OK;
            }
        }
    }
    
    return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseSimple(cherryhud_parser_t* parser, void* target, int type) {
    if (type == 0) {
        return CG_CHUDLexerParserParseValue(parser, (char*)target, MAX_QPATH);
    }
    return CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseOptionalString(cherryhud_parser_t* parser, char* target, int maxlen) {
    cherryhudConfigParseStatus_t status;
    cherryhud_token_t* token;
    
    // Try to get next token without advancing
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status != CHERRYHUD_CONFIG_OK) {
        // No more tokens - treat as empty string
        target[0] = '\0';
        return CHERRYHUD_CONFIG_OK;
    }
    
    token = CG_CHUDLexerParserGetCurrentToken(parser);
    if (!token) {
        // No token - treat as empty string
        target[0] = '\0';
        return CHERRYHUD_CONFIG_OK;
    }
    
    // Check if next token is a command (starts with letter and not a string)
    if (token->type == TOKEN_ELEMENT) {
        // Next token is a command - treat as empty string, don't advance
        target[0] = '\0';
        return CHERRYHUD_CONFIG_OK;
    }
    
    // Token is a string or element - parse it
    if (token->type == TOKEN_STRING || token->type == TOKEN_ELEMENT) {
        if (token->value) {
            Q_strncpyz(target, token->value, maxlen);
        } else {
            target[0] = '\0';
        }
        parser->current_token++;
        return CHERRYHUD_CONFIG_OK;
    }
    
    // Unexpected token type - treat as empty string
    target[0] = '\0';
    return CHERRYHUD_CONFIG_OK;
}



static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->color2.isSet = qfalse;
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->color2.value);
    if (status == CHERRYHUD_CONFIG_OK) config->color2.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseAlignH(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* alignValues[] = {"l", "c", "r"};
    int result;
    
    config->alignH.isSet = qfalse;
    
    status = CG_CHUDLexerParserParseEnum(parser, alignValues, 3, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->alignH.value = result; // l=0, c=1, r=2
        config->alignH.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseAlignV(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* alignValues[] = {"t", "c", "b"};
    int result;
    
    config->alignV.isSet = qfalse;
    
    status = CG_CHUDLexerParserParseEnum(parser, alignValues, 3, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->alignV.value = result; // t=0, c=1, b=2
        config->alignV.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseAngles(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    float e = 0.0f;
    
    config->angles.isSet = qfalse;
    
    // Parse p y r
    status = CG_CHUDLexerParserParseVec3(parser, config->angles.value);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    // Try to parse optional e value
    status = CG_CHUDLexerParserParseFloat(parser, &e);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->angles.value[3] = e;
    } else {
        config->angles.value[3] = 0.0f;
    }
    
    config->angles.isSet = qtrue;
    return CHERRYHUD_CONFIG_OK;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBgColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->background.color);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->background.isSet = qtrue;
        config->background.color.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBgColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->background.color2);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->background.isSet = qtrue;
        config->background.color2.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseDirection(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* directionValues[] = {"l", "r", "t", "b"};
    int result;
    
    config->direction.isSet = qfalse;
    
    status = CG_CHUDLexerParserParseEnum(parser, directionValues, 4, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->direction.value = result; // l=0, r=1, t=2, b=3
        config->direction.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFade(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->fade.isSet = qtrue;
    status = CG_CHUDLexerParserParseVec4(parser, config->fade.value);
    if (status != CHERRYHUD_CONFIG_OK) {
        // It's ok if there are no colors
        Vector4Clear(config->fade.value);
    }
    return CHERRYHUD_CONFIG_OK;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFadeDelay(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    float fadeDelay;
    
    config->fadedelay.isSet = qfalse;
    status = CG_CHUDLexerParserParseFloat(parser, &fadeDelay);
    if (status != CHERRYHUD_CONFIG_OK) return status;
    
    config->fadedelay.isSet = qtrue;
    config->fadedelay.value = (int)(fadeDelay);
    return CHERRYHUD_CONFIG_OK;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFill(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    config->fill.isSet = qtrue;
    (void) parser;
    (void) config;
    return CHERRYHUD_CONFIG_OK;
}


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImage(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->image.isSet = qfalse;
    status = CG_CHUDLexerParserParseOptionalString(parser, config->image.value, MAX_QPATH);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->image.isSet = qtrue;
        // Always set isSet to true, even for empty strings
        // This allows users to explicitly set empty image to override auto-generation
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageTC(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->imagetc.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec4(parser, config->imagetc.value);
    if (status == CHERRYHUD_CONFIG_OK) config->imagetc.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseItTeam(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* teamValues[] = {"b", "r", "n", "o", "e"};
    int result;
    
    config->itTeam.isSet = qfalse;
    
    status = CG_CHUDLexerParserParseEnum(parser, teamValues, 5, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->itTeam.value = result; // b=0, r=1, n=2, o=3, e=4
        config->itTeam.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMargins(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->margins.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec4(parser, config->margins.value);
    if (status == CHERRYHUD_CONFIG_OK) config->margins.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseModel(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->model.isSet = qfalse;
    status = CG_CHUDLexerParserParseSimple(parser, config->model.value, 0);
    if (status == CHERRYHUD_CONFIG_OK) config->model.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMonospace(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    config->monospace.isSet = qtrue;
    (void) parser; // Parser not used for this boolean flag
    return CHERRYHUD_CONFIG_OK;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseOffset(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->offset.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec3(parser, config->offset.value);
    if (status == CHERRYHUD_CONFIG_OK) config->offset.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseText(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->text.isSet = qfalse;
    status = CG_CHUDLexerParserParseOptionalString(parser, config->text.value, MAX_QPATH);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->text.isSet = qtrue;
        // Always set isSet to true, even for empty strings
        // This allows users to explicitly set empty text to override auto-generation
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlign(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* alignValues[] = {"l", "c", "r"};
    int result;
    
    config->textExt.alignH.isSet = qfalse;
    
    status = CG_CHUDLexerParserParseEnum(parser, alignValues, 3, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.alignH.value = result; // l=0, c=1, r=2
        config->textExt.alignH.isSet = qtrue;
    }
    return status;
}


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextOffset(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textOffset.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec2(parser, config->textOffset.value);
    if (status == CHERRYHUD_CONFIG_OK) config->textOffset.isSet = qtrue;
    return status;
}


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTime(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->time.isSet = qfalse;
    status = CG_CHUDLexerParserParseInt(parser, &config->time.value);
    if (status == CHERRYHUD_CONFIG_OK) config->time.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVisFlags(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->visflags.isSet = qfalse;
    status = CG_CHUDLexerParserParseVisFlagsHelper(parser, &config->visflags);
    
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseHlColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->hlcolor.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec4(parser, config->hlcolor.value);
    if (status == CHERRYHUD_CONFIG_OK) config->hlcolor.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseHlSize(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->hlsize.isSet = qfalse;
    status = CG_CHUDLexerParserParseInt(parser, &config->hlsize.value);
    if (status == CHERRYHUD_CONFIG_OK) config->hlsize.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBorder(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->border.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec4(parser, config->border.sideSizes);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->border.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBorderColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->border.color);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->border.isSet = qtrue;
        config->border.color.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseBorderColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->border.color2);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->border.isSet = qtrue;
        config->border.color2.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseStyle(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->style.isSet = qfalse;
    status = CG_CHUDLexerParserParseInt(parser, &config->style.value);
    if (status == CHERRYHUD_CONFIG_OK) config->style.isSet = qtrue;
    return status;
}


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseDoublebar(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    config->doublebar.isSet = qtrue;
    (void) parser;
    (void) config;
    return CHERRYHUD_CONFIG_OK;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseType(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->type.isSet = qfalse;
    status = CG_CHUDLexerParserParseSimple(parser, config->type.value, 0);
    if (status == CHERRYHUD_CONFIG_OK) config->type.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParsePos(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->pos.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec2(parser, config->pos.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->pos.isSet = qtrue;
    } else {
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseSize(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    
    config->size.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec2(parser, config->size.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->size.isSet = qtrue;
    } else {
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseContent(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->text.isSet = qfalse;
    status = CG_CHUDLexerParserParseSimple(parser, config->text.value, 0);
    if (status == CHERRYHUD_CONFIG_OK) config->text.isSet = qtrue;
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMaxWidth(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->maxWidth.isSet = qfalse;
    status = CG_CHUDLexerParserParseFloat(parser, &config->maxWidth.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->maxWidth.isSet = qtrue;
    } else {
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseMaxValue(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->maxValue.isSet = qfalse;
    status = CG_CHUDLexerParserParseFloat(parser, &config->maxValue.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->maxValue.isSet = qtrue;
    } else {
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBgColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textBgColor.isSet = qfalse;
    status = CG_CHUDLexerParserParseColor(parser, config);
    if (status == CHERRYHUD_CONFIG_OK) {
        // Copy parsed color to textBgColor
        memcpy(&config->textBgColor.value, &config->color.value, sizeof(cherryhudColor_t));
        config->textBgColor.isSet = qtrue;
    }
    return status;
}


static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageBgColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->imageBgColor.isSet = qfalse;
    status = CG_CHUDLexerParserParseColor(parser, config);
    if (status == CHERRYHUD_CONFIG_OK) {
        // Copy parsed color to imageBgColor
        memcpy(&config->imageBgColor.value, &config->color.value, sizeof(cherryhudColor_t));
        config->imageBgColor.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageBorder(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->imageBorder.isSet = qfalse;
    status = CG_CHUDLexerParserParseBorder(parser, config);
    if (status == CHERRYHUD_CONFIG_OK) {
        // Copy parsed border to imageBorder
        memcpy(&config->imageBorder, &config->border, sizeof(config->border));
        config->imageBorder.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseImageBorderColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->imageBorderColor.isSet = qfalse;
    status = CG_CHUDLexerParserParseColor(parser, config);
    if (status == CHERRYHUD_CONFIG_OK) {
        // Copy parsed color to imageBorderColor
        memcpy(&config->imageBorderColor.value, &config->color.value, sizeof(cherryhudColor_t));
        config->imageBorderColor.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseRowSpacing(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->rowSpacing.isSet = qfalse;
    status = CG_CHUDLexerParserParseFloat(parser, &config->rowSpacing.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->rowSpacing.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseWeapon(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->weapon.isSet = qfalse;
    status = CG_CHUDLexerParserParseSimple(parser, config->weapon.value, 0);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->weapon.isSet = qtrue;
    }
    return status;
}

// Extended text parameter parsing functions
static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextSize(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.size.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec2(parser, config->textExt.size.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.size.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextPos(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.pos.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec2(parser, config->textExt.pos.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.pos.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.color.isSet = qfalse;
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->textExt.color.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.color.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextColor2(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.color2.isSet = qfalse;
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->textExt.color2.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.color2.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBackground(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.background.isSet = qfalse;
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->textExt.background.color);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.background.isSet = qtrue;
        config->textExt.background.color.isSet = qtrue;
        config->textExt.background.type = CHERRYHUD_BACKGROUND_DEFAULT;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBorder(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.border.isSet = qfalse;
    status = CG_CHUDLexerParserParseVec4(parser, config->textExt.border.sideSizes);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.border.isSet = qtrue;
        config->textExt.border.type = CHERRYHUD_BORDER_DEFAULT;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextBorderColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.bordercolor.isSet = qfalse;
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->textExt.bordercolor.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.bordercolor.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlignV(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* alignValues[] = {"t", "c", "b"};
    int result;
    
    config->textExt.alignV.isSet = qfalse;
    status = CG_CHUDLexerParserParseEnum(parser, alignValues, 3, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.alignV.value = result; // t=0, c=1, b=2
        config->textExt.alignV.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextAlignH(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    const char* alignValues[] = {"l", "c", "r"};
    int result;
    
    config->textExt.alignH.isSet = qfalse;
    status = CG_CHUDLexerParserParseEnum(parser, alignValues, 3, &result);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.alignH.value = result; // l=0, c=1, r=2
        config->textExt.alignH.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextShadow(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.shadow.isSet = qfalse;
    status = CG_CHUDLexerParserSkipToNextToken(parser);
    if (status == CHERRYHUD_CONFIG_OK) {
        // For boolean flags, just set to true if the command is present
        config->textExt.shadow.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextShadowColor(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.shadowColor.isSet = qfalse;
    status = CG_CHUDLexerParserParseColorHelper(parser, &config->textExt.shadowColor.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.shadowColor.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextFont(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.font.isSet = qfalse;
    status = CG_CHUDLexerParserParseSimple(parser, config->textExt.font.value, 0);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.font.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTextFormat(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->textExt.format.isSet = qfalse;
    status = CG_CHUDLexerParserParseSimple(parser, config->textExt.format.value, 0);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->textExt.format.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseExtendedType(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    const char* extendedTypeValues[] = {
        "none",
        "double", 
        "compact",
        NULL
    };
    
    int result;
    cherryhudConfigParseStatus_t status = CG_CHUDLexerParserParseEnum(parser, extendedTypeValues, 3, &result);

    if (status == CHERRYHUD_CONFIG_OK) {
        config->extendedType.value = (cherryhudExtendedType_t)result;
        config->extendedType.isSet = qtrue;
    } else {
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseTableSpacing(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->tableSpacing.isSet = qfalse;
    status = CG_CHUDLexerParserParseFloat(parser, &config->tableSpacing.value);
    if (status == CHERRYHUD_CONFIG_OK) {
        config->tableSpacing.isSet = qtrue;
    }
    return status;
}

static cherryhudConfigParseStatus_t CG_CHUDLexerParserParseHideFlag(cherryhud_parser_t* parser, cherryhudConfig_t* config) {
    cherryhudConfigParseStatus_t status;
    
    config->hideflags.isSet = qfalse;
    status = CG_CHUDLexerParserParseVisFlagsHelper(parser, &config->hideflags);
    
    return status;
}

