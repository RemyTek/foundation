#include "cg_local.h"

int customScoreboardColorIsSet;
vec4_t scoreboard_colorBody;
vec4_t scoreboard_colorHeader;


typedef struct {
    int clientNum;
    int score;
} ScoreSort_t;


#define BERUN (be_run.integer & 1)
int ScoreSort_Compare(const void *a, const void *b)
{
    const ScoreSort_t *sa = (const ScoreSort_t *)a;
    const ScoreSort_t *sb = (const ScoreSort_t *)b;
    return sb->score - sa->score;
}

#define SCOREBOARD_DIVISOR_LARGE 10
#define SCOREBOARD_DIVISOR_TIME 6
#define SCOREBOARD_DIVISOR_PING 10
#define SCOREBOARD_SPACE 4
#define SCOREBOARD_BIG_SPACE (SCOREBOARD_SPACE * 2)

#define SCALE_NORMAL  1.0f
#define SCALE_SMALL   0.8f
#define SCALE_TINY    0.55f

// Game type names array
const char* gametypeNames[] = {
    "Free for all",
    "Tournament",
    "Single Player",
    "Team Deathmatch",
    "Capture The flag",
    "Clan Arena",
    "Freeze Tag",         // GT_TEAM + isFreeze
    "Freeze tag CTF",     // GT_CTF + isFreeze
    "^1Unknown^7"         // Unknown
};

// Scoreboard character structure for fonts
typedef struct {
    float width;
    float height;
    vec4_t color;
    vec4_t shadowColor;
} scoreboardChar_t;

// Main scoreboard context structure
typedef struct {
    // ====== Base Positioning and Dimensions ======
    float         baseX;
    float         rightBaseX;
    float         baseY;
    float         width;
    float         halfWidth;

    // ====== Row and Header Dimensions ======
    float         rowHeight;
    float         rowSpacing;
    float         headerHeight;
    float         bottomHeaderHeight;
    float         headSize;

    // ====== Vertical Text Positions ======
    float         textY;
    float         textSmallY;
    float         headY;
    float         playerIdY;

    // ====== Horizontal Column Positions ======
    float         headX;
    float         nameX;
    float         eyeX;
    float         specLineY;
    float         firstX;
    float         secondX;
    float         thirdX;
    float         fourthX;
    float         fifthX;
    float         timeX;
    float         pingX;
    float         dmgRatioX;

    // ====== Width Constraints ======
    float         nameMaxWidth;
    float         maxWidth;

    // ====== Rendering Settings ======
    int         font;
    int         flags;
    int         numScoreColumns;

    // ====== Spacing and Scale ======
    float         space;
    float         bigSpace;
    float         scale;

    // ====== Font Structures ======
    scoreboardChar_t title;
    scoreboardChar_t playerScore;

    // ====== Main Text Colors ======
    vec4_t      textColor;
    vec4_t      shadowColor;

    // ====== Background and Border Colors ======
    vec4_t      background;
    vec4_t      playersEvenBackground;
    vec4_t      border;
    vec4_t      borderColor;
    vec4_t      defaultBorder;

    vec4_t      headerBgColor;
    vec4_t      bodyBgColor;

    // ====== Small Element Dimensions and Positions ======
    float         smallWidth;
    float         smallHeight;
    float         halfSmallWidth;
    float         frameX;
    float         frameY;
    float         frameW;
    float         headerY;
    vec4_t      lightWhite;

    // ====== Statistics Display Colors ======
    vec4_t      statLightBlue;
    vec4_t      statLightYellow;
    vec4_t      otherFragsColor;

    // ====== Ratio Color Coding ======
    vec4_t      ratioRed;
    vec4_t      ratioYellow;
    vec4_t      ratioGreen;
    vec4_t      ratioCyan;
    vec4_t      ratioMagenta;
    vec4_t      mdGray;
} scoreboardContext_t;

// Spectator column positions structure
typedef struct {
    int headX;
    int nameX;
    int timeX;
    int pingX;
} SpecColumnPositions;

// Global scoreboard contexts
scoreboardContext_t scoreboardSettings;
scoreboardContext_t* sbSet = &scoreboardSettings;
scoreboardContext_t scoreboardSettingsRight;
scoreboardContext_t* sbSetRight = &scoreboardSettingsRight;

// ====== Helper Functions ======

// Get color based on ratio value
static vec4_t *GetRatioColor(float ratio) {
    if (ratio < 0.6f) return &sbSet->ratioRed;
    else if (ratio < 1.0f) return &sbSet->ratioYellow;
    else if (ratio < 1.49f) return &sbSet->ratioGreen;
    else if (ratio < 2.2f) return &sbSet->ratioCyan;
    else return &sbSet->ratioMagenta;
}

// Draw scoreboard text with specified parameters
static void CG_DrawScoreboardText(float x, float y, const char *text, vec4_t textColor, int maxWidth, float scale, int dsflags_modifier, vec4_t bgColor) {
    int final_flags = sbSet->flags | dsflags_modifier;
    CG_OSPDrawStringNew(
        x, y, text,
        textColor, sbSet->playerScore.shadowColor,
        sbSet->playerScore.width * scale, sbSet->playerScore.height * scale,
        maxWidth, final_flags,
        bgColor, NULL, NULL
    );
}

// Draw scoreboard image with alignment
void CG_DrawScoreboardImage(float x, float y, float width, float height, vec4_t color, qhandle_t shader, int align) {
    float iconX, iconY;
    if (color == NULL) {
        color = sbSet->playerScore.color;
    }
    if (align == DS_HCENTER) {
    iconX = x - width * 0.5f;
    iconY = y;
    } else if (align == DS_HRIGHT) {
    iconX = x - width;
    iconY = y;
    } else { // DS_HLEFT or default
    iconX = x;
    iconY = y;
    }
    CG_DrawPicWithColor(iconX, iconY, width, height, color, shader);
}

// Draw icon with value text
static void CG_DrawIconAndValue(float x, float y, vec4_t iconColor, qhandle_t icon, int value, vec4_t color) {
    char text[16];
    float miniIconH = sbSet->smallHeight;
    float miniIconW = sbSet->smallWidth;

    // ICON (left)
    CG_DrawScoreboardImage(x, y, miniIconW, miniIconH, iconColor, icon, DS_HLEFT);

    // TEXT (right)
    Com_sprintf(text, sizeof(text), "%i", value);
    CG_DrawScoreboardText(x + miniIconW + (SCOREBOARD_SPACE / 4), y, text, color, sbSet->maxWidth, SCALE_TINY * sbSet->scale, DS_HLEFT, NULL);
}

// Draw stat triple: left value, icon, right value
void DrawStatTriple(int x, int y, int leftValue, float rightValue, qhandle_t icon) {
    char text[16];
    float miniIconH = sbSet->smallHeight;
    float miniIconW = sbSet->smallWidth;

    // TEXT (left value)
    Com_sprintf(text, sizeof(text), "%i", leftValue);
    CG_DrawScoreboardText(x - sbSet->halfSmallWidth, y, text, sbSet->statLightBlue, sbSet->maxWidth, SCALE_TINY * sbSet->scale, DS_HRIGHT, NULL);
    // ICON (center)
    CG_DrawScoreboardImage(x - (sbSet->halfSmallWidth/4), y, miniIconW, miniIconH, NULL, icon, DS_HCENTER);

    // CG_DrawPic(x, y, miniIconW, miniIconH, icon);
    // TEXT (right value)
    Com_sprintf(text, sizeof(text), "%.0f%%", rightValue);
    CG_DrawScoreboardText(x + sbSet->halfSmallWidth, y, text, sbSet->statLightYellow, sbSet->maxWidth, SCALE_TINY * sbSet->scale, DS_HLEFT, NULL);
}

// Draw weapon accuracy line for specific weapon
static void DrawWeaponAccuracyLine(float x, float y, int clientNum, int weaponType, int yOffset) {
    int kills = cgs.be.statsAll[clientNum].stats[weaponType].kills;
    float accuracy = cgs.be.statsAll[clientNum].stats[weaponType].accuracy;
    qhandle_t icon = cg_weapons[weaponType].weaponIcon;
    DrawStatTriple(x, y + yOffset, kills, accuracy, icon);
}

// ====== Player Drawing Functions ======

// Draw player head model
static void CG_DrawPlayerHead(float y, int clientNum, score_t *score, scoreboardContext_t *sb) {
    vec3_t headAngles;
    float headY = y + sb->headY;
    VectorClear(headAngles);
    if (clientNum == cg.snap->ps.clientNum) {
        headAngles[YAW] = (float)cg.time / 14 + 180.0f;
    } else {
        headAngles[YAW] = 180.0f;
    }
    CG_DrawHead(sb->headX, headY, sb->headSize, sb->headSize, clientNum, headAngles);
}

// Draw player ID number
static void CG_DrawPlayerId(float y, int clientNum, scoreboardContext_t *sb) {
    char playerIdText[4];
    float headY = y + sb->headY;
    Com_sprintf(playerIdText, sizeof(playerIdText), "%i", clientNum);
    CG_DrawScoreboardText(sb->headX, headY + sb->headSize, playerIdText, sb->textColor, sb->maxWidth, SCALE_SMALL * sb->scale, DS_VTOP, sb->background);
}

// Draw player name
static void CG_DrawPlayerName(float y, int clientNum, scoreboardContext_t *sb) {
    clientInfo_t *ci = &cgs.clientinfo[clientNum];
    float textY = y + sb->textY;
    CG_DrawScoreboardText(sb->nameX, textY, ci->name, sb->textColor, sb->nameMaxWidth, SCALE_NORMAL * sb->scale, 0, NULL);
}

// Draw player time
static void CG_DrawPlayerTime(float y, int clientNum, score_t *score, scoreboardContext_t *sb) {
    float textY_small = y + sb->textSmallY;
        if (score) {
        CG_DrawIconAndValue(sb->timeX, textY_small, sb->textColor, cgs.media.scoreboardBEClock, score->time, sb->textColor);
    } else {
        CG_DrawIconAndValue(sb->timeX, textY_small, sb->textColor, cgs.media.scoreboardBEClock, 0, sb->textColor);
    }
}

// Draw player ping with color coding
static void CG_DrawPlayerPing(float y, int clientNum, score_t *score, scoreboardContext_t *sb) {
    float textY_small = y + sb->textSmallY;
        if (score) {
        if (score->ping == 999) {
            CG_DrawPic(sb->pingX, textY_small, sb->smallWidth, sb->smallHeight, cgs.media.scoreboardBENoSignal);
        } else {
            vec4_t color;
            if((score->ping > 20))
            {
                Vector4Copy(colorYellow, color);
            }
            else if (score->ping > 49)
            {
                Vector4Copy(colorOrange, color);
            }
            else if (score->ping > 99)
            {
                Vector4Copy(colorRed, color);
            }
            else
            {
                Vector4Copy(colorGreen, color);
            }
            CG_DrawIconAndValue(sb->pingX, textY_small, color, cgs.media.scoreboardBESignal, score->ping, sb->textColor);
        }
    } else 
    {
        CG_DrawIconAndValue(sb->pingX, textY_small, colorWhite, cgs.media.scoreboardBESignal, 0, sb->textColor);
    }
}

// Draw player damage stats
static void CG_DrawPlayerDamage(float y, int clientNum, scoreboardContext_t *sb) {
    char text[64];
    float textY_small = y + sb->textSmallY;
    float dm;
    vec4_t *dmColor;
    CG_DrawIconAndValue(sb->firstX - sb->bigSpace, textY_small, colorGreen, cgs.media.arrowUp, cgs.be.statsAll[clientNum].dmgGiven, sb->textColor);
    CG_DrawIconAndValue(sb->secondX - sb->bigSpace, textY_small, colorRed, cgs.media.arrowDown, cgs.be.statsAll[clientNum].dmgReceived, sb->textColor);
        // Damage Ratio
    dm = cgs.be.statsAll[clientNum].damageRatio;
    if (dm >= 10.0f) {
        Com_sprintf(text, sizeof(text), "9.9+");
    } else {
        Com_sprintf(text, sizeof(text), "%.2f", dm);
    }
    dmColor = GetRatioColor(dm);
    CG_DrawScoreboardText(sb->thirdX, textY_small, text, *dmColor, sb->maxWidth, SCALE_TINY * sb->scale, DS_HRIGHT, NULL);
}

// Draw player weapon accuracy
static void CG_DrawPlayerWeaponAccuracy(float y, int clientNum, score_t *score, scoreboardContext_t *sb) {
    float textY_small = y + sb->textSmallY;
        if (score && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << score->client))) {
        CG_DrawScoreboardText(sb->pingX + (sb->bigSpace * 5), textY_small, "^BReady", colorYellow, sbSet->maxWidth, SCALE_TINY * sbSet->scale, DS_HLEFT, NULL);
    } else {
        if (cgs.be.statsAll[clientNum].stats[WP_RAILGUN].shots > 0) {
            DrawWeaponAccuracyLine(sb->pingX + (sb->bigSpace * 8), textY_small, clientNum, WP_RAILGUN, 0);
        }
        if (cgs.be.statsAll[clientNum].stats[WP_LIGHTNING].shots > 0) {
            DrawWeaponAccuracyLine(sb->pingX + (sb->bigSpace * 4), textY_small, clientNum, WP_LIGHTNING, 0);
        }
    }
}

// Draw player score and stats
static void CG_DrawPlayerScore(float y, int clientNum, score_t *score, scoreboardContext_t *sb) {
    char text[64];
    float textY = y + sb->textY;
    float textY_small = y + sb->textSmallY;
     vec4_t *kdColor;
    if (score) {
        /* Frags */
        Com_sprintf(text, sizeof(text), "%i", score->score);
        CG_DrawScoreboardText(sb->firstX, textY, text, sb->textColor, sb->maxWidth, SCALE_NORMAL * sb->scale, DS_HCENTER, NULL);
        /* Deaths */
        Com_sprintf(text, sizeof(text), "%i", cgs.be.statsAll[clientNum].deaths);
        CG_DrawScoreboardText(sb->secondX, textY, text, sb->mdGray, sb->maxWidth, SCALE_NORMAL * sb->scale, DS_HCENTER, NULL);
        /* KD Ratio */
        Com_sprintf(text, sizeof(text), "%.1f", cgs.be.statsAll[clientNum].kdratio);
        kdColor = GetRatioColor(cgs.be.statsAll[clientNum].kdratio);
        CG_DrawScoreboardText(sb->thirdX, textY, text, *kdColor, sb->maxWidth, SCALE_NORMAL * sb->scale, DS_HRIGHT, NULL);
    } else {
        /* No score? Use kills instead */
        Com_sprintf(text, sizeof(text), "%i", cgs.be.statsAll[clientNum].kills);
        CG_DrawScoreboardText(sb->firstX, textY, text, sb->otherFragsColor, sb->maxWidth, SCALE_NORMAL * sb->scale, DS_HCENTER, NULL);
        /* Deaths */
        Com_sprintf(text, sizeof(text), "%i", cgs.be.statsAll[clientNum].deaths);
        CG_DrawScoreboardText(sb->secondX, textY, text, sb->mdGray, sb->maxWidth, SCALE_NORMAL * sb->scale, DS_HCENTER, NULL);
        /* KD Ratio */
        Com_sprintf(text, sizeof(text), "%.1f", cgs.be.statsAll[clientNum].kdratio);
        kdColor = GetRatioColor(cgs.be.statsAll[clientNum].kdratio);
        CG_DrawScoreboardText(sb->thirdX, textY, text, *kdColor, sb->maxWidth, SCALE_NORMAL * sb->scale, DS_HRIGHT, NULL);
    }
}

// Draw main player info: head, ID, name, time, ping
static void CG_DrawPlayerMainInfo(float y, int clientNum, score_t *score, scoreboardContext_t *sb,  short connected, short isSpectator) {
    CG_DrawPlayerHead(y, clientNum, score, sb);
    CG_DrawPlayerId(y, clientNum, sb);
    CG_DrawPlayerName(y, clientNum, sb);
    if (connected) 
    {
        float width = sb->thirdX - sb->timeX + (sb->space / 2);
        float textY_small = y + sb->textSmallY;
        if (isSpectator)
        {
            width = sb->pingX + sb->smallWidth + (sb->space * 4) - sb->timeX;
        }
        // CG_OSPDrawFrameAdjusted(sb->nameX, textY_small,
        //     width, sb->smallHeight,
        //     sb->defaultBorder, colorBlack, qfalse);

        CG_DrawPlayerTime(y, clientNum, score, sb);
        CG_DrawPlayerPing(y, clientNum, score, sb);
    }
}

// ====== Initialization and Context Functions ======

// Init Scoreboard Colors from cvars
static void CG_InitScoreboardColors(void) {
CG_ParseCvarTwoColorsSimple(&cg_scoreboardColors, cgs.be.sbSettings.ffaColors.headerBg, cgs.be.sbSettings.ffaColors.bodyBg);
    customScoreboardColorIsSet = 1;
}


// Calculate scoreboard column widths
static void CG_CalcScoreboardColumnWidths(float baseX, scoreboardContext_t *sb) {
    float nameMaxWidth = (sb->width * 0.55f);
    float spacing;
    sb->headX = baseX;
    sb->nameX = baseX + sb->headSize + sb->space;
    sb->firstX = sb->nameX + nameMaxWidth + sb->space;
    sb->fifthX = baseX + sb->width - sb->space / 2.0f;
    spacing = (sb->fifthX - sb->firstX) / (sb->numScoreColumns - 1);
    if(sb->numScoreColumns >= 2) sb->secondX = sb->firstX + spacing - (5 * sb->scale);
    if(sb->numScoreColumns >= 3) sb->thirdX = sb->firstX + 2 * spacing;
    if(sb->numScoreColumns >= 4) sb->fourthX = sb->firstX + 3 * spacing;
    if(sb->numScoreColumns >= 5) sb->fifthX = sb->firstX + (sb->numScoreColumns - 1) * spacing;
    sb->nameMaxWidth = nameMaxWidth;

    // Ping positions
    sb->timeX = sb->nameX + (sb->space / 2);
    sb->pingX = sb->nameX + sb->smallWidth + (sb->bigSpace * 2);
}

// Initialize scoreboard context for game type

static void CG_ScoreboardContext_Init_GT(void)
{
    // if (cgs.gametype == GT_FFA)
        sbSet->numScoreColumns = 3;
} 

static void CG_ScoreboardContext_Init_ShopwMode(void)
{
    // if (cgs.gametype == GT_TD)
    sbSet->nameX = sbSet->baseX + sbSet->width/*  - sbSet->headSize */ - sbSet->nameMaxWidth;
}

// Initialize scoreboard context with default values
void CG_ScoreboardContext_Init(void) {
    float scale;



    // ====== Text Size and Scale Settings ======
    scale = cg_scoreboardScale.value;
    sbSet->playerScore.width  = cgs.be.sbSettings.textSize[0] * scale;
    sbSet->playerScore.height = cgs.be.sbSettings.textSize[1] * scale;
    sbSet->scale = scale;

    // ====== Spacing and Positioning ======
    sbSet->space = SCOREBOARD_SPACE * scale;
    sbSet->bigSpace = SCOREBOARD_BIG_SPACE * scale;

    // ====== Base Positioning and Dimensions ======
    // Calculate scoreboard width and center positioning
    sbSet->width = (cg_scoreboardWidth.value * scale);
    sbSet->baseX = SCREEN_WIDTH / 2 - sbSet->width;
    sbSet->rightBaseX = SCREEN_WIDTH / 2;

    // Calculate half width for column calculations
    sbSet->halfWidth = sbSet->width / 2;

    // ====== Vertical Layout ======
    // Set base Y position and calculate row dimensions
    sbSet->baseY = SCREEN_HEIGHT / sbSet->bigSpace;
    sbSet->rowHeight = (sbSet->playerScore.height * 1.85f);
    sbSet->headSize = (sbSet->rowHeight) * scale;
    sbSet->rowSpacing = sbSet->space / 2; // Space between rows

    // ====== Horizontal Layout ======
    // Position head and name columns
    sbSet->headX = sbSet->baseX;
    sbSet->nameX = sbSet->headX + sbSet->headSize + (sbSet->space / 2);
    

    // ====== Font and Rendering Settings ======
    sbSet->font = cg_scoreboardFont.integer;
    sbSet->flags = DS_SHADOW | DS_PROPORTIONAL;
    
    // ====== Text Vertical Positioning ======
    sbSet->textY = (2 - (SCOREBOARD_SPACE / 2) * sbSet->scale);

    // ====== Text Colors and Shadows ======
    Vector4Copy(colorWhite, sbSet->playerScore.color);
    Vector4Copy(colorBlack, sbSet->playerScore.shadowColor);

    // Title text settings (larger than regular text)
    sbSet->title.width = sbSet->playerScore.width * 1.15f;
    sbSet->title.height = sbSet->playerScore.height * 1.15f;

    // Small text vertical position
    sbSet->textSmallY = (((sbSet->textY + sbSet->playerScore.height) + (SCOREBOARD_SPACE / 2)) * sbSet->scale);

    // Head vertical centering
    sbSet->headY = (sbSet->rowHeight - sbSet->headSize) / 2; // Center head vertically
    sbSet->playerIdY = sbSet->headY + sbSet->headSize; // Initialize playerIdY for ID text position

    // Spectator
    sbSet->eyeX = (sbSet->headX + sbSet->headSize + (sbSet->space / 2)) - sbSet->space;
    sbSet->specLineY = sbSet->baseY + sbSet->rowHeight + (SCOREBOARD_SPACE * 2 * sbSet->scale);
    // Title colors
    Vector4Copy(colorWhite, sbSet->title.color);
    Vector4Copy(colorBlack, sbSet->title.shadowColor);

    // Main text colors
    Vector4Copy(colorWhite, sbSet->textColor);
    Vector4Copy(colorBlack, sbSet->shadowColor);

    // ====== Background and Border Colors ======
    Vector4Set(sbSet->defaultBorder, 1, 1, 1, 1);
    Vector4Set(sbSet->background, 0, 0, 0, 0.66f);
    Vector4Set(sbSet->playersEvenBackground, 0, 0, 0, 0.33f);
    Vector4Set(sbSet->border, 1, 1, 1, 1);
    Vector4Set(sbSet->borderColor, 1.0f, 1.0f, 1.0f, 0.5f);

    // ====== Layout Constraints ======
    sbSet->maxWidth = SCREEN_WIDTH;

    // ====== Small Element Dimensions ======
    sbSet->smallWidth = sbSet->playerScore.width * SCALE_TINY * sbSet->scale;
    sbSet->smallHeight = sbSet->playerScore.height * SCALE_TINY * sbSet->scale;

    // ====== Frame Positioning ======
    sbSet->frameX = sbSet->baseX - sbSet->space;
    sbSet->frameY = sbSet->baseY - sbSet->space;
    sbSet->frameW = sbSet->width + sbSet->space * 2;

    // ====== Header Positioning ======
    sbSet->headerY = sbSet->baseY - (sbSet->space / 2);
    sbSet->headerHeight = sbSet->headerY - sbSet->frameY + sbSet->title.height + sbSet->smallHeight + sbSet->space;
    sbSet->bottomHeaderHeight = sbSet->smallHeight;

    // ====== Time and Ping Column Positions ======
    sbSet->timeX = sbSet->nameX + (sbSet->space / 2);
    sbSet->pingX = sbSet->nameX + sbSet->smallWidth + (sbSet->bigSpace * 2);

    // ====== Special Colors ======
    Vector4Copy(colorMdGrey, sbSet->mdGray);
    Vector4Set(sbSet->lightWhite, 0.6f, 0.6f, 0.6f, 0.3f);

    // Half small width for icon positioning
    sbSet->halfSmallWidth = sbSet->smallWidth / 2;

    // ====== Statistics Display Colors ======
    Vector4Set(sbSet->statLightBlue, 0.8f, 0.9f, 1.0f, 1.0f);
    Vector4Set(sbSet->statLightYellow, 1.0f, 1.0f, 0.8f, 1.0f);
    Vector4Set(sbSet->otherFragsColor, 1.0f, 0.7f, 0.7f, 1);

    // ====== Ratio Color Coding ======
    Vector4Set(sbSet->ratioRed, 1.0f, 0.85f, 0.85f, 1.0f);
    Vector4Set(sbSet->ratioYellow, 1.0f, 1.0f, 0.85f, 1.0f);
    Vector4Set(sbSet->ratioGreen, 0.80f, 1.0f, 0.80f, 1.0f);
    Vector4Set(sbSet->ratioCyan, 0.75f, 1.0f, 1.0f, 1.0f);
    Vector4Set(sbSet->ratioMagenta, 1.0f, 0.65f, 1.0f, 1.0f);

    // // ====== Right Scoreboard Setup ======
    memcpy(sbSetRight, sbSet, sizeof(scoreboardContext_t));

    // Calculate column widths for left scoreboard
    CG_CalcScoreboardColumnWidths(sbSet->baseX, sbSet);
    CG_ScoreboardContext_Init_GT();
    CG_ScoreboardContext_Init_ShopwMode();
    // CG_InitScoreboardColors();
}

// Set scoreboard positions for double/single mode
static void CG_SetScoreboardPositions(short useDouble) {
    float availableWidth, colSpacing;
    int numCols;
    float baseX, rightBaseX;
    // Calculate available width for columns
    numCols = sbSet->numScoreColumns;
    availableWidth = sbSet->width - sbSet->nameMaxWidth - sbSet->headSize - sbSet->space;
    colSpacing = (numCols > 1) ? availableWidth / (numCols) : availableWidth;

    if (useDouble) {
        baseX = (SCREEN_WIDTH / 2 - sbSet->width) - (sbSet->space / 2);
        rightBaseX = (SCREEN_WIDTH / 2) + (sbSet->space / 2);

        // Left scoreboard
        sbSet->baseX = baseX;
        sbSet->rightBaseX = rightBaseX;
        sbSet->headX = baseX;
        sbSet->nameX = baseX + sbSet->headSize + sbSet->space;

        // Distribute columns evenly
        sbSet->firstX  = sbSet->nameX + sbSet->nameMaxWidth + sbSet->space;
        sbSet->secondX = sbSet->firstX + colSpacing;
        sbSet->thirdX  = sbSet->firstX + 2 * colSpacing;
        sbSet->fourthX = sbSet->firstX + 3 * colSpacing;
        sbSet->fifthX  = sbSet->firstX + 4 * colSpacing;

        sbSet->timeX = sbSet->nameX + (sbSet->space / 2);
        sbSet->pingX = sbSet->nameX + sbSet->smallWidth + (sbSet->bigSpace * 2);

        sbSet->frameX = baseX - sbSet->space;
        sbSet->frameW = sbSet->width + sbSet->space * 2;
        sbSet->textSmallY = sbSet->textSmallY; // unchanged

        // Right scoreboard
        sbSetRight->baseX = rightBaseX;
        sbSetRight->rightBaseX = rightBaseX;
        sbSetRight->headX = rightBaseX;
        sbSetRight->nameX = rightBaseX + sbSetRight->headSize + sbSetRight->space;

        sbSetRight->firstX  = sbSetRight->nameX + sbSetRight->nameMaxWidth + sbSetRight->space;
        sbSetRight->secondX = sbSetRight->firstX + colSpacing;
        sbSetRight->thirdX  = sbSetRight->firstX + 2 * colSpacing;
        sbSetRight->fourthX = sbSetRight->firstX + 3 * colSpacing;
        sbSetRight->fifthX  = sbSetRight->firstX + 4 * colSpacing;

        sbSetRight->timeX = sbSetRight->nameX + (sbSetRight->space / 2);
        sbSetRight->pingX = sbSetRight->nameX + sbSetRight->smallWidth + (sbSetRight->bigSpace * 2);

        sbSetRight->frameX = rightBaseX - sbSetRight->space;
        sbSetRight->frameW = sbSetRight->width + sbSetRight->space * 2;
        sbSetRight->textSmallY = sbSet->textSmallY;
    } else {
        baseX = (SCREEN_WIDTH - sbSet->width) / 2;
        sbSet->baseX = baseX;
        sbSet->rightBaseX = baseX;
        sbSet->headX = baseX;
        sbSet->nameX = baseX + sbSet->headSize + sbSet->space;

        sbSet->firstX  = sbSet->nameX + sbSet->nameMaxWidth + sbSet->space;
        sbSet->secondX = sbSet->firstX + colSpacing;
        sbSet->thirdX  = sbSet->firstX + 2 * colSpacing;
        sbSet->fourthX = sbSet->firstX + 3 * colSpacing;
        sbSet->fifthX  = sbSet->firstX + 4 * colSpacing;

        sbSet->timeX = sbSet->nameX + (sbSet->space / 2);
        sbSet->pingX = sbSet->nameX + sbSet->smallWidth + (sbSet->bigSpace * 2);

        sbSet->frameX = baseX - sbSet->space;
        sbSet->frameW = sbSet->width + sbSet->space * 2;
        sbSet->textSmallY = sbSet->textSmallY;
    }
}

// Get scoreboard row height (теперь только rowHeight, без rowSpacing)
static int CG_CalcScoreboardRowHeight(void) {
    return sbSet->rowHeight;
}

// Calculate actual scoreboard height
static int CG_CalcActualScoreboardHeight(short isDouble) {
    int i, k;
    int numPlayers = 0;
    int numSpectators = 0;
    int playerRowsHeight = 0;
    int spectatorRowsHeight = 0;
    int totalHeight;
    clientInfo_t *ci;

    // Count players and spectators
    for (i = 0; i < MAX_CLIENTS; i++) {
        ci = &cgs.clientinfo[i];
        if (!ci->infoValid) continue;
        if (ci->team == TEAM_SPECTATOR) {
            numSpectators++;
        } else {
            numPlayers++;
        }
    }

    if (isDouble) {
        int halfPlayers = numPlayers / 2;
        int leftPlayers = halfPlayers;
        int rightPlayers = numPlayers - halfPlayers;
        int halfSpectators = (numSpectators + 1) / 2;
        int leftSpectatorRows = (halfSpectators + 1) / 2; // ceil(halfSpectators / 2)
        int rightSpectatorRows = ((numSpectators - halfSpectators) + 1) / 2; // ceil((numSpectators - halfSpectators) / 2)
        int maxSpectatorRows = (leftSpectatorRows > rightSpectatorRows ? leftSpectatorRows : rightSpectatorRows);

        int maxPlayers = (leftPlayers > rightPlayers ? leftPlayers : rightPlayers);
        playerRowsHeight = maxPlayers * sbSet->rowHeight + (maxPlayers > 0 ? (maxPlayers - 1) * sbSet->rowSpacing : 0);
        spectatorRowsHeight = maxSpectatorRows * sbSet->rowHeight + (maxSpectatorRows > 0 ? (maxSpectatorRows - 1) * sbSet->rowSpacing : 0) + sbSet->space / 2;
    } else {
        playerRowsHeight = numPlayers * sbSet->rowHeight + (numPlayers > 0 ? (numPlayers - 1) * sbSet->rowSpacing : 0);
        if (numSpectators > 0) {
            int halfSpectators = (numSpectators + 1) / 2;
            spectatorRowsHeight = halfSpectators * sbSet->rowHeight + (halfSpectators > 0 ? (halfSpectators - 1) * sbSet->rowSpacing : 0) + sbSet->space / 2;
        }
    }

    // Total height: header + players + spectators + bottom header + space at end
    totalHeight = sbSet->headerHeight + playerRowsHeight + spectatorRowsHeight + sbSet->bottomHeaderHeight + (sbSet->space * 2);

    return totalHeight;
}

// Restore spectator positions
static void CG_RestoreSpectatorPositions(float baseX) {
    sbSet->headX = baseX;
    sbSet->nameX = baseX + sbSet->headSize + sbSet->space;
    sbSet->timeX = sbSet->nameX;
    sbSet->pingX = sbSet->nameX + sbSet->smallWidth + (sbSet->bigSpace * 2);
    sbSet->nameMaxWidth = (sbSet->width - sbSet->headSize - sbSet->space);
}
// ====== Row Drawing Functions ======

// Draw FFA player row
void CG_DrawScoreLineFFA(int y, int clientNum, int rowIndex, float baseX) {
    score_t *score = NULL;
    clientInfo_t *ci = &cgs.clientinfo[clientNum];
    int i;
    scoreboardContext_t *sb = (baseX == sbSet->baseX) ? sbSet : sbSetRight;
    short connected;
    CG_CalcScoreboardColumnWidths(baseX, sb);

    // Find score for client
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (cg.scores[i].client == clientNum) {
            score = &cg.scores[i];
            break;
        }
    }

    if (score && score->ping != -1) {
        connected = qtrue;
    } else {
        connected = qfalse;
    }

    CG_FontSelect(sb->font);

    // if (rowIndex % 2 == 1) {
    //     CG_FillRect(sb->headX, y, sb->width, sb->rowHeight, sb->playersEvenBackground);
    // }



    // Local client
    if (clientNum == cg.snap->ps.clientNum) {
        
        CG_FillRect(sb->headX, y, sb->width, sb->rowHeight, sb->lightWhite);
    }

    CG_DrawPlayerMainInfo(y, clientNum, score, sb, connected, qfalse);

    if (connected)
    {
        CG_DrawPlayerDamage(y, clientNum, sb);
        CG_DrawPlayerWeaponAccuracy(y, clientNum, score, sb);
        CG_DrawPlayerScore(y, clientNum, score, sb);
    }
    else
    {
        int textY = y + sb->textY;
        CG_DrawScoreboardText(sb->nameX + sb->nameMaxWidth + sb->space, textY, "Connecting", sb->textColor, sb->maxWidth, SCALE_NORMAL * sb->scale, 0, NULL);
    }
    // Local client frame
    if (clientNum == cg.snap->ps.clientNum) {
        vec4_t animatedColor;
        float factor = 0.5f * (1.0f + sin(M_PI * 2 * cg.time / 1500.0f));
        animatedColor[0] = animatedColor[1] = animatedColor[2] = 0.2f + 0.8f * factor;
        animatedColor[3] = 0.5f;
        CG_OSPDrawFrameAdjusted(sb->headX, y, sb->width, sb->rowHeight, colorBlack, animatedColor, qfalse);
        CG_OSPDrawFrameAdjusted(sb->headX, y, sb->width, sb->rowHeight, sb->defaultBorder, animatedColor, qtrue);
    }
    // All players frame
    CG_OSPDrawFrameAdjusted(sb->headX, y, sb->width, sb->rowHeight, sb->defaultBorder, sb->borderColor, qtrue);
}

// Draw spectator row
void CG_DrawSpectatorLine(int y, int clientNum, float baseX) {
    score_t *score = NULL;
    clientInfo_t *ci = &cgs.clientinfo[clientNum];
    int i;
    scoreboardContext_t *sb = (baseX == sbSet->baseX) ? sbSet : sbSetRight;
    short connected;

    // Find score for client
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (cg.scores[i].client == clientNum) {
            score = &cg.scores[i];
            break;
        }
    }

    if (score && score->ping != -1) {
        connected = qtrue;
    } else {
        connected = qfalse;
    }

    CG_FontSelect(sb->font);

    if (clientNum == cg.snap->ps.clientNum && !(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
        // Black frame for local spectator
        vec4_t animatedColor;
        float factor = 0.5f * (1.0f + sin(M_PI * 2 * cg.time / 2000.0f));
        animatedColor[0] = animatedColor[1] = animatedColor[2] = 0.5f + 0.5f * factor;
        animatedColor[3] = 1.0f;
        CG_OSPDrawFrameAdjusted(sb->headX, y, sb->halfWidth, sb->rowHeight, sb->defaultBorder, animatedColor, qfalse);
    }

    CG_DrawPlayerMainInfo(y, clientNum, score, sb, connected, qtrue);
    
    if (!connected) {
        int textY = y + sb->textY;
        CG_DrawScoreboardText(sb->nameX + sb->nameMaxWidth + sb->space, textY, "Connecting", sb->textColor, sb->width / 4, SCALE_TINY * sb->scale, 0, NULL);
        return;
    }  
    // Frame for all spectators
    CG_OSPDrawFrameAdjusted(sb->headX, y, sb->halfWidth, sb->rowHeight, sb->defaultBorder, sb->borderColor, qtrue);
}

// ====== List Functions ======

// Draw FFA scoreboard list
int CG_DrawScoreboardFFA(short isDouble) {
    int i, k;
    int y;
    int sortedClients[MAX_CLIENTS];
    int numSorted;
    ScoreSort_t scoreArray[MAX_CLIENTS];
    int half;
    int y_left;
    int y_right;
    int max_y;

    y = sbSet->baseY + sbSet->headerHeight; // Start after header
    numSorted = 0;

    for (i = 0; i < MAX_CLIENTS; i++) {
        clientInfo_t *ci = &cgs.clientinfo[i];
        if (!ci->infoValid || ci->team == TEAM_SPECTATOR) continue;
        sortedClients[numSorted] = i;
        scoreArray[numSorted].clientNum = i;
        scoreArray[numSorted].score = cgs.be.statsAll[i].kills;
        for (k = 0; k < MAX_CLIENTS; k++) {
            if (cg.scores[k].client == i) {
                scoreArray[numSorted].score = cg.scores[k].score;
                break;
            }
        }
        numSorted++;
    }

    qsort(scoreArray, numSorted, sizeof(ScoreSort_t), ScoreSort_Compare);
    
    if (isDouble) {
        // Split players into two tables: top in left
        half = (numSorted + 1) / 2;
        y_left = y;
        y_right = y;

        for (i = 0; i < half; i++) {
            CG_DrawScoreLineFFA(y_left, scoreArray[i].clientNum, i, sbSet->baseX);
            y_left += sbSet->rowHeight;
            if (i < half - 1) y_left += sbSet->rowSpacing;  // Spacing только между строками
        }

        for (i = half; i < numSorted; i++) {
            CG_DrawScoreLineFFA(y_right, scoreArray[i].clientNum, i - half, sbSet->rightBaseX);
            y_right += sbSet->rowHeight;
            if (i < numSorted - 1) y_right += sbSet->rowSpacing;  // Spacing только между строками
        }

        max_y = (y_left > y_right ? y_left : y_right);
        max_y = CG_DrawSpectatorList(max_y, sbSet->baseX, isDouble);
    } else {
        for (i = 0; i < numSorted; i++) {
            CG_DrawScoreLineFFA(y, scoreArray[i].clientNum, i, sbSet->baseX);
            y += sbSet->rowHeight;
            if (i < numSorted - 1) y += sbSet->rowSpacing;  // Spacing только между строками
        }
        y = CG_DrawSpectatorList(y, sbSet->baseX, isDouble);
        max_y = y;
    }
    return max_y;
}

// Set spectator column positions
static void SetSpectatorColumnPositions(float baseX, SpecColumnPositions* left, SpecColumnPositions* right, scoreboardContext_t* sb) {
    left->headX = baseX;
    left->nameX = left->headX + sb->headSize + sb->space;
    left->timeX = left->nameX + (sb->space / 2);
    left->pingX = left->nameX + sb->smallWidth + (sb->bigSpace * 2);

    right->headX = baseX + sb->halfWidth;
    right->nameX = right->headX + sb->headSize + sb->space;
    right->timeX = left->timeX + sb->halfWidth + (sb->space / 2);
    right->pingX = left->pingX + sb->halfWidth;
}

// Draw spectator two columns
static void DrawSpectatorTwoColumns(int startIndex, int numToDraw, int* spectatorList, float baseX, scoreboardContext_t* sb, int* leftY, int* rightY, SpecColumnPositions left, SpecColumnPositions right) {
    int i;
    for (i = 0; i < numToDraw; i += 2) {
        int actualIndex = startIndex + i;
        // Left column
        if (actualIndex < startIndex + numToDraw) {
            sb->headX = left.headX;
            sb->nameX = left.nameX;
            sb->timeX = left.timeX;
            sb->pingX = left.pingX;
            sb->nameMaxWidth = sb->halfWidth - sb->headSize - (sb->space / 2);
            CG_DrawSpectatorLine(*leftY, spectatorList[actualIndex], baseX);
            *leftY += sb->rowHeight;
            if (i < numToDraw - 2) *leftY += sb->rowSpacing;
        }
        // Right column
        if (actualIndex + 1 < startIndex + numToDraw) {
            sb->headX = right.headX;
            sb->nameX = right.nameX;
            sb->timeX = right.timeX;
            sb->pingX = right.pingX;
            sb->nameMaxWidth = sb->halfWidth - sb->headSize - (sb->space / 2);
            CG_DrawSpectatorLine(*rightY, spectatorList[actualIndex + 1], baseX);
            *rightY += sb->rowHeight;
            if (i < numToDraw - 2) *rightY += sb->rowSpacing;
        }
    }
}

// Separate function to draw spectator list
int CG_DrawSpectatorList(int yStart, float baseX, short isDouble) {
    int i;
    int spectatorList[MAX_CLIENTS];
    int numSpectators;
    int y;
    float lineY;
    vec4_t bottomLine;

    numSpectators = 0;
    y = yStart;

    // Collect spectator list
    for (i = 0; i < MAX_CLIENTS; i++) {
        clientInfo_t *ci = &cgs.clientinfo[i];
        if (!ci->infoValid || ci->team != TEAM_SPECTATOR) continue;
        spectatorList[numSpectators++] = i;
    }

    if (numSpectators > 0) {
        if (isDouble) {
            // Draw in two columns for double mode
            int half = (numSpectators + 1) / 2;
            int leftY = y + (sbSet->space / 2);
            int rightY = y + (sbSet->space / 2);
            SpecColumnPositions left, right;
            int maxYLeft, maxYRight;
            lineY = y;
            bottomLine[0] = 0;
            bottomLine[1] = 0;
            bottomLine[2] = 0;
            bottomLine[3] = 1;

            SetSpectatorColumnPositions(baseX, &left, &right, sbSet);

            DrawSpectatorTwoColumns(0, half, spectatorList, baseX, sbSet, &leftY, &rightY, left, right);

            maxYLeft = (leftY > rightY ? leftY : rightY);
            CG_OSPDrawFrameAdjusted(baseX - sbSet->space, 0,
                                   sbSet->width + sbSet->space * 2, lineY,
                                   bottomLine, sbSet->borderColor, qfalse);

            CG_FillRect(sbSet->eyeX - (sbSet->space / 2), lineY - (sbSet->space / 2), sbSet->space, sbSet->space, sbSet->background);
            
            CG_DrawPic(sbSet->eyeX, lineY, sbSet->title.width, sbSet->title.height, cgs.media.scoreboardBEEye);

            if (numSpectators > half) {
                int numRight = numSpectators - half;
                leftY = y + (sbSet->space / 2);
                rightY = y + (sbSet->space / 2);
                SetSpectatorColumnPositions(sbSet->rightBaseX, &left, &right, sbSet);

                DrawSpectatorTwoColumns(half, numRight, spectatorList, sbSet->rightBaseX, sbSetRight, &leftY, &rightY, left, right);

                maxYRight = (leftY > rightY ? leftY : rightY);
                CG_OSPDrawFrameAdjusted(sbSet->rightBaseX - sbSet->space, 0,
                                       sbSet->width + sbSet->space * 2, lineY,
                                       bottomLine, sbSet->borderColor, qfalse);
                CG_DrawPic(sbSet->eyeX, lineY, sbSet->title.width, sbSet->title.height, cgs.media.scoreboardBEEye);
            } else {
                maxYRight = y + (sbSet->space / 2);
            }
            y = (maxYLeft > maxYRight ? maxYLeft : maxYRight);
            // Restore original positions
            CG_RestoreSpectatorPositions(baseX);
        } else {
            int leftY = y + (sbSet->space / 2);
            int rightY = y + (sbSet->space / 2);
            SpecColumnPositions left, right;
            lineY = y;
            bottomLine[0] = 0;
            bottomLine[1] = 0;
            bottomLine[2] = 0;
            bottomLine[3] = 1;

            SetSpectatorColumnPositions(baseX, &left, &right, sbSet);

            DrawSpectatorTwoColumns(0, numSpectators, spectatorList, baseX, sbSet, &leftY, &rightY, left, right);

            // Separator line
            CG_OSPDrawFrameAdjusted(baseX - sbSet->space, 0,
                                   sbSet->width + sbSet->space * 2, lineY,
                                   bottomLine, sbSet->borderColor, qfalse);
            // Restore original positions
            CG_RestoreSpectatorPositions(baseX);
            CG_DrawPic(sbSet->nameX - (sbSet->playerScore.width + sbSet->space / 2), lineY - (sbSet->title.height / 2), sbSet->title.width, sbSet->title.height, cgs.media.scoreboardBEEye);

            y = (leftY > rightY ? leftY : rightY);
        }
    } else {
        y = yStart;
    }
    return y;
}

// ====== Header Functions ======

// Draw scoreboard header text
void CG_DrawScoreboardHeader(int x, int y, const char *text, vec4_t color, int maxWidth, int dsflags_modifier) {
    int final_flags = sbSet->flags | dsflags_modifier;
    CG_OSPDrawStringNew(x, y, text,
                        color, sbSet->title.shadowColor,
                        sbSet->title.width, sbSet->title.height,
                        maxWidth, final_flags,
                        NULL, NULL, NULL);
}

// Draw scoreboard header image
void CG_DrawScoreboardHeaderImage(int x, int y, float width, float height, qhandle_t shader, int align) {
    float iconX, iconY;
    if (align == DS_HCENTER) {
    iconX = x - width * 0.5f;
    iconY = y + (sbSet->title.height - height) * 0.5f;
    } else if (align == DS_HRIGHT) {
    iconX = x - width;
    iconY = y + (sbSet->title.height - height) * 0.5f;
    } else { // DS_HLEFT or default
    iconX = x;
    iconY = y + (sbSet->title.height - height) * 0.5f;
    }
    CG_DrawPicWithColor(iconX, iconY, width, height, sbSet->title.color, shader);
}

// ====== Helper Functions ======

// Draw the column headers for a scoreboard column
static void DrawScoreboardColumnHeaders(scoreboardContext_t* sb, float baseX, const char* columnNum, short isRight, short drawColumnNum) {
    scoreboardContext_t* targetSb = isRight ? sbSetRight : sb;

    if (drawColumnNum) {
        CG_DrawScoreboardHeader(targetSb->headX + sb->space, sb->headerY + sb->smallHeight + sb->space, columnNum, sb->mdGray, sb->width, DS_HLEFT);
    }
    CG_DrawScoreboardHeader(targetSb->nameX, sb->headerY + sb->smallHeight + sb->space, "Name", sb->title.color, targetSb->nameMaxWidth, 0);
    CG_DrawScoreboardHeaderImage(targetSb->firstX, sb->headerY + sb->smallHeight + sb->space, sb->title.width, sb->title.height, cgs.media.scoreboardBEScore, DS_HCENTER);
    if (sb->numScoreColumns >= 2) {
        CG_DrawScoreboardHeaderImage(targetSb->secondX, sb->headerY + sb->smallHeight + sb->space, sb->title.width, sb->title.height, cgs.media.obituariesSkull, DS_HCENTER);
    }
    if (sb->numScoreColumns >= 3) {
        CG_DrawScoreboardHeaderImage(targetSb->thirdX, sb->headerY + sb->smallHeight + sb->space, sb->title.width, sb->title.height, cgs.media.scoreboardBELeaderboard, DS_HRIGHT);
    }
}

// Draw the background and frame for scoreboard (unified for all gametypes)
static void DrawScoreboardBackgroundAndFrame(float frameX, float frameY, float frameW, float frameH, scoreboardContext_t* sb, int gametype, int team) 
{
    vec4_t bgColor, hdrColor;
    vec4_t tempbgColor, temphdrColor;

    // Set team-specific colors for team-based gametypes
    if (gametype >= GT_TEAM) {
        if (team == TEAM_RED) {
            if (cgs.be.sbSettings.redColors.bodyBg[3] >= 0.0f)
            {
                Vector4Copy(cgs.be.sbSettings.redColors.bodyBg, bgColor);
            }
            else
            {
                Vector4Copy(scoreboard_rtColor, bgColor);
            }
            if (cgs.be.sbSettings.redColors.headerBg[3] >= 0.0f)
            {
                Vector4Copy(cgs.be.sbSettings.redColors.headerBg, hdrColor);
            }
            else
            {
                Vector4Copy(scoreboard_rtColor, hdrColor);
            }

        } else if (team == TEAM_BLUE) {
            if (cgs.be.sbSettings.blueColors.bodyBg[3] >= 0.0f)
            {
                Vector4Copy(cgs.be.sbSettings.blueColors.bodyBg, bgColor);
            }
            else
            {
                Vector4Copy(scoreboard_btColor, bgColor);
            }
            if (cgs.be.sbSettings.blueColors.headerBg[3] >= 0.0f)
            {
                Vector4Copy(cgs.be.sbSettings.blueColors.headerBg, hdrColor);
            }
            else
            {
                Vector4Copy(scoreboard_btColor, hdrColor);
            }

        }
    }
    // Handle single player and other non-team modes
    else if (gametype <= GT_SINGLE_PLAYER) {

         if (cgs.be.sbSettings.ffaColors.bodyBg[3] >= 0.0f)
         {
             Vector4Copy(cgs.be.sbSettings.ffaColors.bodyBg, bgColor);
         }
         else
         {
             Vector4Copy(sbSet->background, bgColor);
         }
           
        if (cgs.be.sbSettings.ffaColors.headerBg[3] >= 0.0f)
        {
            Vector4Copy(cgs.be.sbSettings.ffaColors.headerBg, hdrColor);
        }
        else
        {
            Vector4Copy(sbSet->background, hdrColor);
        }
    }

    // this should work for all gametypes
    if (team == TEAM_SPECTATOR) {
        if (cgs.be.sbSettings.specColors.bodyBg[3] >= 0.0f)
        {
            Vector4Copy(cgs.be.sbSettings.specColors.bodyBg, bgColor);
        }
        else
        {
            Vector4Copy(sb->background, bgColor);
        }
    }

    // Apply the colors
    CG_OSPAdjustTeamColor(bgColor, tempbgColor);
    CG_OSPAdjustTeamColor(hdrColor, temphdrColor);

    CG_FillRect(frameX, frameY, frameW, frameH, temphdrColor);
    CG_FillRect(frameX, frameY, frameW, sb->headerHeight, tempbgColor);

    CG_OSPDrawFrameAdjusted(frameX, frameY, frameW, frameH, sb->defaultBorder, sb->borderColor, qtrue);
    CG_OSPDrawFrameAdjusted(frameX, frameY, frameW, frameH, sb->defaultBorder, colorBlack, qfalse);
}

// Helper function for drawing scoreboard frame (unified for FFA and Team)
static void CG_DrawScoreboardFrameUnified(short isTeamMode, short forceDouble, int (*drawListFunc)(short)) {
    int i;
    int y = sbSet->baseY;
    int numPlayers = 0;
    int numSpectators = 0;
    float frameX_left = sbSet->baseX - (sbSet->space / 2);
    float frameX_right = sbSet->rightBaseX - (sbSet->space / 2);
    float frameY = sbSet->baseY - sbSet->space;
    float frameW = sbSet->width + sbSet->space;
    float frameH;
    const char* info = CG_ConfigString(CS_SERVERINFO);
    char* hostname = Info_ValueForKey(info, "sv_hostname");
    char* mapname = Info_ValueForKey(info, "mapname");
    short isFreeze = cgs.osp.gameTypeFreeze;
    int gt = cgs.gametype;
    int endY;
    char limitStr[64];
    int limit;
    char topHeader[256];
    char mapnameBuf[64];
    vec4_t infoColor = {0.75f, 0.75f, 0.75f, 1};
    char gametypeStr[64];
    short useDouble;

    // Build gametype string based on game type and freeze mode
    if (gt >= 0 && gt < GT_MAX_GAME_TYPE) {
        if (isFreeze && gt == GT_TEAM) {
            Q_strncpyz(gametypeStr, gametypeNames[6], sizeof(gametypeStr));
        } else if (isFreeze && gt == GT_CTF) {
            Q_strncpyz(gametypeStr, gametypeNames[7], sizeof(gametypeStr));
        } else {
            Q_strncpyz(gametypeStr, gametypeNames[gt], sizeof(gametypeStr));
        }
    } else {
        Q_strncpyz(gametypeStr, gametypeNames[8], sizeof(gametypeStr));
    }

    // Count players and spectators
    for (i = 0; i < MAX_CLIENTS; i++) {
        if (cgs.clientinfo[i].infoValid) {
            if (cgs.clientinfo[i].team == TEAM_SPECTATOR) {
                numSpectators++;
            } else {
                numPlayers++;
            }
        }
    }

    // Determine if double column mode is needed
    if (forceDouble) {
        useDouble = qtrue;
    } else {
        int singleHeight = CG_CalcActualScoreboardHeight(qfalse);
        int doubleHeight = CG_CalcActualScoreboardHeight(qtrue);
        int threshold = SCREEN_HEIGHT - 2 * sbSet->baseY;
        useDouble = (singleHeight >= threshold) ? qtrue : qfalse;
    }

    frameH = CG_CalcActualScoreboardHeight(useDouble);
    CG_SetScoreboardPositions(useDouble);

    // Recalculate frame positions after setting positions
    frameX_left = sbSet->baseX - (sbSet->space / 2);
    frameX_right = sbSet->rightBaseX - (sbSet->space / 2);

    // Prepare common strings
    limit = (isFreeze || gt == GT_CTF) ? cgs.capturelimit : cgs.fraglimit;
    Com_sprintf(limitStr, sizeof(limitStr), "Limits: %d, %d min", limit, cgs.timelimit);
    Com_sprintf(topHeader, sizeof(topHeader), "%s", gametypeStr);

    CG_FontSelect(sbSet->font);

    if (useDouble) {
        // Double column mode
        CG_CalcScoreboardColumnWidths(sbSet->baseX, sbSet);

        if (isTeamMode) {
            DrawScoreboardBackgroundAndFrame(frameX_left, frameY, frameW, frameH, sbSet, cgs.gametype, TEAM_RED); // Red for left
            DrawScoreboardBackgroundAndFrame(frameX_right, frameY, frameW, frameH, sbSet, cgs.gametype, TEAM_BLUE); // Blue for right
        } else {
            DrawScoreboardBackgroundAndFrame(frameX_left, frameY, frameW, frameH, sbSet, cgs.gametype, TEAM_SPECTATOR);
            DrawScoreboardBackgroundAndFrame(frameX_right, frameY, frameW, frameH, sbSet, cgs.gametype, TEAM_SPECTATOR);
        }

        // Draw header info: topHeader on left, limitStr on right
        CG_OSPDrawStringNew(sbSet->baseX, sbSet->headerY, topHeader, infoColor, sbSet->title.shadowColor,
                            sbSet->smallWidth, sbSet->smallHeight, sbSet->rightBaseX + sbSet->width, sbSet->flags, NULL, NULL, NULL);
        CG_OSPDrawStringNew(sbSet->rightBaseX + sbSet->width, sbSet->headerY, limitStr, infoColor, sbSet->title.shadowColor,
                            sbSet->smallWidth, sbSet->smallHeight, sbSet->maxWidth, sbSet->flags | DS_HRIGHT, NULL, NULL, NULL);

        if (cgs.be.supportedServer) {
            CG_DrawScoreboardImage(sbSet->baseX, sbSet->baseY - sbSet->smallHeight, sbSet->smallWidth, sbSet->smallHeight, colorGreen, cgs.media.scoreboardBEReady, DS_HCENTER);
        }

        DrawScoreboardColumnHeaders(sbSet, sbSet->baseX, isTeamMode ? "" : "1", qfalse, !isTeamMode);

        CG_CalcScoreboardColumnWidths(sbSet->rightBaseX, sbSetRight);
        DrawScoreboardColumnHeaders(sbSet, sbSet->rightBaseX, isTeamMode ? "" : "2", qtrue, !isTeamMode);

        y += sbSet->headerHeight;
        endY = drawListFunc(useDouble);

    } else {
        // Single column mode
        CG_CalcScoreboardColumnWidths(sbSet->baseX, sbSet);

        DrawScoreboardBackgroundAndFrame(frameX_left, frameY, frameW, frameH, sbSet, qfalse, 0);

        // Draw header info: both on left base
        CG_OSPDrawStringNew(sbSet->baseX, sbSet->headerY, topHeader, infoColor, sbSet->title.shadowColor,
                            sbSet->smallWidth, sbSet->smallHeight, (sbSet->baseX + sbSet->width), sbSet->flags, NULL, NULL, NULL);
        CG_OSPDrawStringNew(sbSet->baseX + sbSet->width, sbSet->headerY, limitStr, infoColor, sbSet->title.shadowColor,
                            sbSet->smallWidth, sbSet->smallHeight, sbSet->maxWidth, sbSet->flags | DS_HRIGHT, NULL, NULL, NULL);

        if (cgs.be.supportedServer) {
            CG_DrawScoreboardImage(sbSet->baseX, sbSet->baseY - sbSet->smallHeight, sbSet->smallWidth, sbSet->smallHeight, colorGreen, cgs.media.scoreboardBEReady, DS_HCENTER);
        }

        DrawScoreboardColumnHeaders(sbSet, sbSet->baseX, "", qfalse, qfalse);  // No column number for single mode

        y += sbSet->headerHeight;
        endY = drawListFunc(useDouble);
    }

    // Draw bottom header
    {
        float bottomY = endY + sbSet->space;
        float bottomHeaderY = bottomY + sbSet->bottomHeaderHeight - (sbSet->smallHeight + sbSet->space / 4);

        Com_sprintf(mapnameBuf, sizeof(mapnameBuf), "%s", mapname);

        // Draw background for bottom header
        CG_FillRect(frameX_left, bottomY, frameW, sbSet->bottomHeaderHeight, sbSet->headerBgColor);

        // Draw server name on left
        CG_OSPDrawStringNew(sbSet->baseX, bottomHeaderY, mapname, infoColor, sbSet->title.shadowColor,
                            sbSet->smallWidth, sbSet->smallHeight, sbSet->width, sbSet->flags, NULL, NULL, NULL);

        // Draw map on right
        CG_OSPDrawStringNew(sbSet->baseX + sbSet->width, bottomHeaderY, hostname, infoColor, sbSet->title.shadowColor,
                            sbSet->smallWidth, sbSet->smallHeight, sbSet->maxWidth, sbSet->flags | DS_HRIGHT, NULL, NULL, NULL);
    }
}

// ====== Main Function ======

// Main scoreboard drawing function for FFA mode
void CG_DrawScoreboardFFAFrame(void) {
    CG_DrawScoreboardFrameUnified(qfalse, qfalse, CG_DrawScoreboardFFA);
}

// ====== Team Scoreboard Functions ======

// Draw team scoreboard list
int CG_DrawScoreboardTeam(short isDouble) {
    int i, k;
    int y;
    int sortedClientsRed[MAX_CLIENTS];
    int sortedClientsBlue[MAX_CLIENTS];
    int numSortedRed = 0;
    int numSortedBlue = 0;
    ScoreSort_t scoreArrayRed[MAX_CLIENTS];
    ScoreSort_t scoreArrayBlue[MAX_CLIENTS];
    int y_left;
    int y_right;
    int max_y;

    y = sbSet->baseY + sbSet->headerHeight; // Start after header

    // Collect and sort red team players
    for (i = 0; i < MAX_CLIENTS; i++) {
        clientInfo_t *ci = &cgs.clientinfo[i];
        if (!ci->infoValid || ci->team != TEAM_RED) continue;
        sortedClientsRed[numSortedRed] = i;
        scoreArrayRed[numSortedRed].clientNum = i;
        scoreArrayRed[numSortedRed].score = cgs.be.statsAll[i].kills;
        for (k = 0; k < MAX_CLIENTS; k++) {
            if (cg.scores[k].client == i) {
                scoreArrayRed[numSortedRed].score = cg.scores[k].score;
                break;
            }
        }
        numSortedRed++;
    }
    qsort(scoreArrayRed, numSortedRed, sizeof(ScoreSort_t), ScoreSort_Compare);

    // Collect and sort blue team players
    for (i = 0; i < MAX_CLIENTS; i++) {
        clientInfo_t *ci = &cgs.clientinfo[i];
        if (!ci->infoValid || ci->team != TEAM_BLUE) continue;
        sortedClientsBlue[numSortedBlue] = i;
        scoreArrayBlue[numSortedBlue].clientNum = i;
        scoreArrayBlue[numSortedBlue].score = cgs.be.statsAll[i].kills;
        for (k = 0; k < MAX_CLIENTS; k++) {
            if (cg.scores[k].client == i) {
                scoreArrayBlue[numSortedBlue].score = cg.scores[k].score;
                break;
            }
        }
        numSortedBlue++;
    }
    qsort(scoreArrayBlue, numSortedBlue, sizeof(ScoreSort_t), ScoreSort_Compare);

    // Always double mode for teams
    y_left = y;
    y_right = y;

    // Draw red team on left
    for (i = 0; i < numSortedRed; i++) {
        CG_DrawScoreLineFFA(y_left, scoreArrayRed[i].clientNum, i, sbSet->baseX);
        y_left += sbSet->rowHeight;
        if (i < numSortedRed - 1) y_left += sbSet->rowSpacing;
    }

    // Draw blue team on right
    for (i = 0; i < numSortedBlue; i++) {
        CG_DrawScoreLineFFA(y_right, scoreArrayBlue[i].clientNum, i, sbSet->rightBaseX);
        y_right += sbSet->rowHeight;
        if (i < numSortedBlue - 1) y_right += sbSet->rowSpacing;
    }

    max_y = (y_left > y_right ? y_left : y_right);
    max_y = CG_DrawSpectatorList(max_y, sbSet->baseX, qtrue); // Always double for spectators too
    return max_y;
}

// Main scoreboard drawing function for Team mode
void CG_DrawScoreboardTeamFrame(void) {
    CG_DrawScoreboardFrameUnified(qtrue, qtrue, CG_DrawScoreboardTeam);
}

// ====== Spectator Functions ======

// Set spectator column positions
// Helper function for drawing scoreboard frame (unified for FFA and Team)
