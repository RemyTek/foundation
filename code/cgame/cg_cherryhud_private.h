#ifndef CG_CHERRYHUD_PRIVATE_H
#define CG_CHERRYHUD_PRIVATE_H

#include "cg_local.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SUPERHUD_DEFAULT_FADEDELAY 1000.0

// Simple bitmask: context + template flag
// Context: 1=player, 2=spectator, 3=title
// Template flag: 4 (bit 2)

#define CHERRYHUD_CONTEXT_PLAYER_ROW    1
#define CHERRYHUD_CONTEXT_SPECTATOR_ROW 2
#define CHERRYHUD_CONTEXT_TITLE         3
#define CHERRYHUD_CONTEXT_MAIN_CONTAINER 4
// Bonus flags for elements
#define CHERRYHUD_FLAG_TEMPLATE         4
#define CHERRYHUD_FLAG_AUTO_DETECT      8 


#define MAX_PREFIXED_MODES 4

typedef enum
{
	CHERRYHUD_ALIGNH_LEFT,
	CHERRYHUD_ALIGNH_CENTER,
	CHERRYHUD_ALIGNH_RIGHT,
} cherryhudAlignH_t;

typedef enum
{
	CHERRYHUD_ALIGNV_TOP,
	CHERRYHUD_ALIGNV_CENTER,
	CHERRYHUD_ALIGNV_BOTTOM,
} cherryhudAlignV_t;

typedef enum
{
	CHERRYHUD_COLOR_RGBA,
	CHERRYHUD_COLOR_T,
	CHERRYHUD_COLOR_E,
	CHERRYHUD_COLOR_I,
	CHERRYHUD_COLOR_U,
} cherryhudColorType_t;


typedef struct
{
    cherryhudColorType_t type;
    vec4_t rgba;
	qboolean isSet;
} cherryhudColor_t;

typedef enum
{
    CHERRYHUD_BACKGROUND_DEFAULT,
    CHERRYHUD_BACKGROUND_FADING,
    CHERRYHUD_BACKGROUND_BLINKING
} cherryhudBackgroundType_t;

typedef struct 
{
    cherryhudBackgroundType_t type;
    cherryhudColor_t color;
} cherryhudBackground_t;

typedef enum
{
    CHERRYHUD_BORDER_DEFAULT,
    CHERRYHUD_BORDER_FADING,
    CHERRYHUD_BORDER_BLINKING
} cherryhudBorderType_t;

typedef struct 
{
    cherryhudBorderType_t type;
    vec4_t sideSizes;
    cherryhudColor_t color;
    cherryhudColor_t color2;
} cherryhudBorder_t;

typedef enum
{
	CHERRYHUD_DIR_LEFT_TO_RIGHT,
	CHERRYHUD_DIR_RIGHT_TO_LEFT,
	CHERRYHUD_DIR_TOP_TO_BOTTOM,
	CHERRYHUD_DIR_BOTTOM_TO_TOP,
} cherryhudDirection_t;

typedef enum
{
	CHERRYHUD_CONFIG_OK,
	CHERRYHUD_CONFIG_UNEXPECTED_CHARACTER,
	CHERRYHUD_CONFIG_WRONG_ELEMENT_NAME,
	CHERRYHUD_CONFIG_WRONG_COMMAND_NAME,
	CHERRYHUD_CONFIG_LOST_ELEMENT_BODY,
	CHERRYHUD_CONFIG_END_OF_FILE,
} cherryhudConfigParseStatus_t;

typedef struct
{
	cherryhudConfigParseStatus_t status;
	int line;
	int column;
	const char* tokenValue;
} cherryhudConfigParseResult_t;

typedef enum
{
	CHERRYHUD_ITTEAM_BLUE,
	CHERRYHUD_ITTEAM_RED,
	CHERRYHUD_ITTEAM_NEUTRAL,
	CHERRYHUD_ITTEAM_OWN,
	CHERRYHUD_ITTEAM_ENEMY,
} cherryhudItTeam_t;

typedef enum
{
	CHERRYHUD_EXTENDED_NONE,
	CHERRYHUD_EXTENDED_DOUBLE,
	CHERRYHUD_EXTENDED_COMPACT,
} cherryhudExtendedType_t;

typedef enum
{
	CHERRYHUD_VISFLAGS_OP_OR,    // Space - any flag active
	CHERRYHUD_VISFLAGS_OP_AND,   // Plus - all flags active
} cherryhudVisFlagsOp_t;

typedef struct
{
	int flags;                    // Combined flags
	cherryhudVisFlagsOp_t op;     // Operation type
	qboolean isSet;
} cherryhudVisFlags_t;

typedef struct
{
	struct
	{
		cherryhudAlignH_t value;
		qboolean isSet;
	} alignH;
	struct
	{
		cherryhudAlignV_t value;
		qboolean isSet;
	} alignV;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} angles;
	struct
	{
		cherryhudBackgroundType_t type;
    	cherryhudColor_t color;
		cherryhudColor_t color2;
		qboolean isSet;
	} background;
	struct
	{
		cherryhudBorderType_t type;
		vec4_t sideSizes;
		cherryhudColor_t color;
		cherryhudColor_t color2;
		qboolean isSet;
	} border;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} hlcolor;
	struct
	{
		cherryhudColor_t value;
		qboolean isSet;
	} color;
	struct
	{
		cherryhudColor_t value;
		qboolean isSet;
	} color2;
	struct
	{
		cherryhudDirection_t value;
		qboolean isSet;
	} direction;
	struct
	{
		qboolean isSet;
	} doublebar;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} fade;
	struct
	{
		int value;
		qboolean isSet;
	} fadedelay;
	struct
	{
		qboolean isSet;
	} fill;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} font;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} image;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} imagetc;
	struct
	{
		cherryhudItTeam_t value;
		qboolean isSet;
	} itTeam;
	struct
	{
		vec4_t value;
		qboolean isSet;
	} margins;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} model;
	struct
	{
		qboolean isSet;
	} monospace;
	struct
	{
		vec3_t value;
		qboolean isSet;
	} offset;
	struct
	{
		vec2_t value;
		qboolean isSet;
	} pos;
	struct
	{
		vec2_t value;
		qboolean isSet;
	} size;
	struct
	{
		float value;
		qboolean isSet;
	} rowSpacing;
	struct
	{
		int value;
		qboolean isSet;
	} style;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} text;
	struct
	{
		vec2_t value;
		qboolean isSet;
	} textOffset;
	struct
	{
		int value;
		qboolean isSet;
	} time;
	cherryhudVisFlags_t visflags;
	cherryhudVisFlags_t hideflags;
	struct
	{
		int value;
		qboolean isSet;
	} hlsize;
	struct
	{
		int value;
		qboolean isSet;
	} visiblity;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} type;
	struct
	{
		float value;
		qboolean isSet;
	} maxWidth;
	// Text-specific parameters
	struct
	{
		cherryhudColor_t value;
		qboolean isSet;
	} textBgColor;
	struct
	{
		cherryhudBorderType_t type;
		vec4_t sideSizes;
		cherryhudColor_t color;
		cherryhudColor_t color2;
		qboolean isSet;
	} textBorder;
	struct
	{
		cherryhudColor_t value;
		qboolean isSet;
	} textBorderColor;
	// Image-specific parameters
	struct
	{
		cherryhudColor_t value;
		qboolean isSet;
	} imageBgColor;
	struct
	{
		cherryhudBorderType_t type;
		vec4_t sideSizes;
		cherryhudColor_t color;
		cherryhudColor_t color2;
		qboolean isSet;
	} imageBorder;
	struct
	{
		cherryhudColor_t value;
		qboolean isSet;
	} imageBorderColor;
	struct
	{
		char value[MAX_QPATH];
		qboolean isSet;
	} weapon;
	struct
	{
		cherryhudExtendedType_t value;
		qboolean isSet;
	} extendedType;
	struct
	{
		float value;
		qboolean isSet;
	} tableSpacing;
	// Extended text parameters (textExt)
	struct
	{
		struct
		{
			vec2_t value;
			qboolean isSet;
		} size;
		struct
		{
			vec2_t value;
			qboolean isSet;
		} pos;
		struct
		{
			cherryhudColor_t value;
			qboolean isSet;
		} color;
		struct
		{
			cherryhudColor_t value;
			qboolean isSet;
		} color2;
		struct
		{
			cherryhudBackgroundType_t type;
			cherryhudColor_t color;
			cherryhudColor_t color2;
			qboolean isSet;
		} background;
		struct
		{
			cherryhudBorderType_t type;
			vec4_t sideSizes;
			cherryhudColor_t color;
			cherryhudColor_t color2;
			qboolean isSet;
		} border;
		struct
		{
			cherryhudColor_t value;
			qboolean isSet;
		} bordercolor;
		struct
		{
			cherryhudAlignV_t value;
			qboolean isSet;
		} alignV;
		struct
		{
			cherryhudAlignH_t value;
			qboolean isSet;
		} alignH;
		struct
		{
			qboolean isSet;
		} shadow;
		struct
		{
			cherryhudColor_t value;
			qboolean isSet;
		} shadowColor;
		struct
		{
			char value[MAX_QPATH];
			qboolean isSet;
		} font;
		struct
		{
			char value[MAX_QPATH];
			qboolean isSet;
		} format;
	} textExt;
	struct
	{
		float value;
		qboolean isSet;
	} maxValue;
	
	// Prefixed mode configurations (compact.*, double.*, etc.)
	struct
	{
		struct {
			char prefix[MAX_QPATH];           // "compact", "double", etc.
			int configIndex;                  // Index into global prefixed configs array
			qboolean loaded;                  // Are there variables with this prefix
		} prefixed[MAX_PREFIXED_MODES];
		int prefixedCount;
	} prefixed;
} cherryhudConfig_t;

typedef cherryhudConfig_t cherryhudElementDefault_t;

typedef struct configFileLine_s
{
	char* line;
	int size;
	int line_number;
	struct configFileLine_s* next;
} configFileLine_t;

typedef struct configFileInfo_s
{
	int pos; /* position in line */
	configFileLine_t* root;
	configFileLine_t* last_line;
} configFileInfo_t;

#define CG_CHUD_CONFIG_INFO_GET_CHAR(CFI) (CFI)->last_line->line[(CFI)->pos]
#define CG_CHUD_CONFIG_INFO_NEXT_CHAR(CFI) ++(CFI)->pos
#define CG_CHUD_CONFIG_INFO_NEXT_LINE(CFI) do { (CFI)->last_line = (CFI)->last_line->next; (CFI)->pos = 0; } while(0)
#define CG_CHUD_CONFIG_INFO_IS_END_OF_FILE(CFI) ((CFI)->last_line == NULL)

#define SE_IM         (1 << 0)  // 0x00000001
#define SE_IM_STR "im"
#define SE_TEAM_ONLY  (1 << 1)  // 0x00000002
#define SE_TEAM_ONLY_STR "teamonly"
#define SE_SPECT      (1 << 2)  // 0x00000004
#define SE_SPECT_STR      "spectator"
#define SE_DEAD       (1 << 3)  // 0x00000008
#define SE_DEAD_STR       "dead"
#define SE_DEMO_HIDE  (1 << 4)  // 0x00000010
#define SE_DEMO_HIDE_STR  "demohide"
#define SE_SCORES_HIDE  (1 << 5)  // 0x00000020
#define SE_SCORES_HIDE_STR  "scoreshide"
#define SE_KEY1_SHOW  (1 << 6)  // 0x00000040
#define SE_KEY1_SHOW_STR  "key1show"
#define SE_KEY2_SHOW  (1 << 7)  // 0x00000080
#define SE_KEY2_SHOW_STR  "key2show"
#define SE_KEY3_SHOW  (1 << 8)  // 0x00000100
#define SE_KEY3_SHOW_STR  "key3show"
#define SE_KEY4_SHOW  (1 << 9)  // 0x00000200
#define SE_KEY4_SHOW_STR  "key4show"
#define SE_CHUDSCOREBOARD_SHOW  (1 << 10) // 0x00000400
#define SE_CHUDSCOREBOARD_SHOW_STR  "chudscoreboard"
#define SE_SHOW_EMPTY  (1 << 23) // 0x00800000
#define SE_SHOW_EMPTY_STR  "showempty"
#define SE_GT_FFA 	(1 << 11) // 0x00000800
#define SE_GT_FFA_STR "gt_ffa"
#define SE_GT_TOURNEY   (1 << 12) // 0x00001000
#define SE_GT_TOURNEY_STR "gt_tourney"
#define SE_GT_TDM      (1 << 13) // 0x00002000
#define SE_GT_TDM_STR  "gt_tdm"
#define SE_GT_CTF       (1 << 14) // 0x00004000
#define SE_GT_CTF_STR   "gt_ctf"
#define SE_GT_FREEZETAG (1 << 15) // 0x00008000
#define SE_GT_FREEZETAG_STR "gt_freezetag"
#define SE_GT_CLANARENA (1 << 16) // 0x00010000
#define SE_GT_CLANARENA_STR "gt_clanarena"
#define SE_LOCAL_CLIENT (1 << 17) // 0x00020000
#define SE_LOCAL_CLIENT_STR "localClient"
#define SE_LOCAL_CLIENT_STR_ALT "localPlayer"
#define SE_COMPACT_MODE (1 << 18) // 0x00040000
#define SE_COMPACT_MODE_STR "compact"
#define SE_DOUBLE_MODE (1 << 19) // 0x00080000
#define SE_DOUBLE_MODE_STR "double"
#define SE_ACTIVE_PLAYER (1 << 20) // 0x00100000
#define SE_ACTIVE_PLAYER_STR "activePlayer"
#define SE_READY (1 << 21) // 0x00200000
#define SE_READY_STR "ready"
#define SE_FROZEN (1 << 22) // 0x00400000
#define SE_FROZEN_STR "frozen"

// Hideflag constants (same as visflags but with opposite logic)
#define HF_LOCAL_CLIENT (1 << 17) // 0x00020000
#define HF_LOCAL_CLIENT_STR "localClient"
#define HF_LOCAL_CLIENT_STR_ALT "localPlayer"
#define HF_ACTIVE_PLAYER (1 << 20) // 0x00100000
#define HF_ACTIVE_PLAYER_STR "activePlayer"
#define HF_READY (1 << 21) // 0x00200000
#define HF_READY_STR "ready"
#define HF_FROZEN (1 << 22) // 0x00400000
#define HF_FROZEN_STR "frozen"
#define HF_COMPACT_MODE (1 << 18) // 0x00040000
#define HF_COMPACT_MODE_STR "compact"
#define HF_DOUBLE_MODE (1 << 19) // 0x00080000
#define HF_DOUBLE_MODE_STR "double"

#define CHUD_CHECK_SHOW_EMPTY(element) ( \
    ((element) != NULL) && \
    ((element)->config.visflags.isSet) && \
    (((element)->config.visflags.value & SE_SHOW_EMPTY) != 0) \
)

typedef struct cherryHUDConfigElement_s
{
	const char* name;
	int visibility;
	void* (*create)(const cherryhudConfig_t* config, const char* containerType);
	void (*routine)(void* context);
	void (*destroy)(void* context);
	void* context;
	int order;
} cherryHUDConfigElement_t;


typedef struct cherryhudElement_s
{
	cherryHUDConfigElement_t element;
	cherryhudConfig_t config;
	struct cherryhudElement_s* next;
	
	// Информация о контейнере
	const char* containerType;         // "!scoreboard", "playersRows", "spectatorsRows"
}
cherryhudElement_t;

typedef struct cherryHUDConfigCommand_s
{
	const char* name;
	cherryhudConfigParseStatus_t (*parse)(configFileInfo_t* finfo, cherryhudConfig_t* config);
	struct cherryHUDConfigCommand_s* next;
} cherryHUDConfigCommand_t;



// Old element functions removed - now using unified decor element

cherryhudElement_t* CG_CHUDCreateElementFromConfig(const cherryhudConfig_t* config, int clientNum, const char* containerType);
void CG_CHUDDestroyElement(cherryhudElement_t* element);
void CG_CHUDRenderElement(cherryhudElement_t* element);

// ServerInfo element removed - now using unified decor element

void* CG_CHUDElementDrawCreate(const cherryhudConfig_t* config, const char* containerType);
void CG_CHUDElementDrawRoutine(void* context);
void CG_CHUDElementDrawDestroy(void* context);

void* CG_CHUDElementDecorCreate(const cherryhudConfig_t* config, const char* containerType);
void CG_CHUDElementDecorRoutine(void* context);
void CG_CHUDElementDecorDestroy(void* context);
void CG_CHUDElementDecorSetClientNum(void* context, int clientNum);


#define SUPERHUD_UPDATE_TIME 50




/*
 * cg_cherryhud_util.c
 */

typedef struct
{
	vec2_t pos;        // x, y position
	vec2_t size;       // width, height
	int flags;
	vec4_t color_origin;
	vec4_t color;
	vec4_t shadowColor;
	vec4_t background;
	vec4_t border;
	vec4_t borderColor;
	int width;
	int fontIndex;
	const char* text;
} cherryhudTextContext_t;

typedef struct
{
	vec2_t pos;        // x, y position
	vec2_t size;       // width, height
	vec2_t picPos;     // picture x, y position
	vec2_t picSize;    // picture width, height
	qhandle_t image;
	vec4_t color;
	vec4_t color_origin;
} cherryhudDrawContext_t;

typedef struct
{
	cherryhudDirection_t direction;
	float max; // maximum coord for bar
	float koeff; //multiplier
	vec4_t bar[2]; // coord of two bars
	vec4_t color_top; // color of bar
	vec4_t color2_top; // color of top bar of doublebar
	vec4_t color_back; // color of background
	qboolean two_bars; // one or two bars
} cherryhudBarContext_t;

#define OSPHUD_TEAMOVERLAY_STR_SIZE 128

void CG_CHUDBarMakeContext(const cherryhudConfig_t* in, cherryhudBarContext_t* out, float max);
void CG_CHUDTextMakeContext(const cherryhudConfig_t* in, cherryhudTextContext_t* out);
void CG_CHUDTextMakeAdjustCoords(const cherryhudConfig_t* in, float* out_x, float* out_y);
void CG_CHUDDrawMakeAdjustCoords(const cherryhudConfig_t* in, float* out_x, float* out_y);
void CG_CHUDDrawStretchPic(float x, float y, float w, float h, 
                          float picX, float picY, float picW, float picH, 
                          const float* color, qhandle_t shader);
void CG_CHUDDrawMakeContext(const cherryhudConfig_t* cfg, cherryhudDrawContext_t* out);

void CG_CHUDTextPrintNew(const cherryhudConfig_t* cfg, cherryhudTextContext_t* pos, qboolean colorOverride);
void CG_CHUDDrawStretchPicCtx(const cherryhudConfig_t* cfg, cherryhudDrawContext_t* out);
void CG_CHUDBarPrint(const cherryhudConfig_t* cfg, cherryhudBarContext_t* ctx, float value);
qboolean CG_CHUDFill(const cherryhudConfig_t* cfg);
void CG_CHUDFillAndFrameForText(cherryhudConfig_t* cfg, cherryhudTextContext_t* ctx);
void CG_CHUDFillAndFrameForImage(const cherryhudConfig_t* cfg, cherryhudDrawContext_t* ctx);

team_t CG_CHUDGetOurActiveTeam(void);
qboolean CG_CHUDGetFadeColor(const vec4_t from_color, vec4_t out, const cherryhudConfig_t* cfg, int startTime);
void CG_CHUDConfigPickBgColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride);
void CG_CHUDRenderContainerBackground(const cherryhudConfig_t* config, float x, float y, float w, float h);
void CG_CHUDRenderContainerBackgroundWithHeight(const cherryhudConfig_t* config, float x, float y, float w, float h, float currentHeight);
void CG_CHUDConfigDefaultsCheck(cherryhudConfig_t* config);

void CG_CHUDConfigPickTextBgColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride);
void CG_CHUDConfigPickTextBorderColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride);
qboolean CG_CHUDConfigHasTextBackground(const cherryhudConfig_t* config);
qboolean CG_CHUDConfigHasTextBorder(const cherryhudConfig_t* config);

void CG_CHUDConfigPickImageBgColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride);
void CG_CHUDConfigPickImageBorderColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride);
qboolean CG_CHUDConfigHasImageBackground(const cherryhudConfig_t* config);
qboolean CG_CHUDConfigHasImageBorder(const cherryhudConfig_t* config);

void CG_CHUDConfigPickBorderColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride);



typedef struct
{
	char message[MAX_SAY_TEXT];
	int time;
} cherryhudChatEntry_t;

typedef struct
{
	int time;
	int attacker;
	int target;
	int attackerTeam;
	int targetTeam;
	int mod;
	qboolean unfrozen;
	struct
	{
		qboolean isInitialized;
		qhandle_t iconShader;
		vec4_t attackerColor;
		vec4_t targetColor;
		char attackerName[MAX_QPATH];
		char targetName[MAX_QPATH];
		int maxVisibleChars;
		float baseX;
		float attackerWidth;
		float targetWidth;
		float spacing;
		int maxNameLenPix;
	} runtime;
} cherryhudObituariesEntry_t;

typedef struct
{
	float tempAccuracy;
} cherryhudTempAccEntry_t;

typedef struct
{
	int lastTrackedWeapon;
	float lastAccuracy;
	float kdratio;
	struct
	{
		float accuracy;
		int kills;
		int deaths;
		int hits;
		int shots;
		int pickUps;
		int drops;
	} stats[WP_NUM_WEAPONS];
} customStats_t;

#define SHUD_MAX_OBITUARIES_LINES 8
#define SHUD_MAX_CHAT_LINES 16
#define SHUD_MAX_POWERUPS 8

typedef struct
{
	struct
	{
		int time;
		char message[256];
	} fragmessage;
	struct
	{
		int time;
		char message[256];
	} rankmessage;
	struct
	{
		cherryhudChatEntry_t line[SHUD_MAX_CHAT_LINES];
		unsigned int index;
	} chat;
	struct
	{
		cherryhudObituariesEntry_t line[SHUD_MAX_OBITUARIES_LINES];
		unsigned int index;
	} obituaries;
	struct cherryhudPowerupsCache_t
	{
		struct cherryhudPowerupElement_t
		{
			int time;
			int powerup;
			qboolean isHoldable;
		} element[SHUD_MAX_POWERUPS];
		int numberOfActive;
		int lastUpdateTime;
	} powerupsCache;
	struct
	{
		cherryhudTempAccEntry_t weapon[WP_NUM_WEAPONS];
	} tempAcc;
	customStats_t customStats;
	
	// Configuration data
	struct
	{
		cherryhudConfig_t scoreboard;  // !scoreboard config
		cherryhudConfig_t defaultConfig;  // !default config
		cherryhudConfig_t compactConfig;  // compact.* config
		cherryhudConfig_t doubleConfig;   // double.* config
		qboolean scoreboardLoaded;
		qboolean defaultLoaded;
		qboolean compactLoaded;
		qboolean doubleLoaded;
		
		// Dynamic config storage by type
		#define MAX_CONFIG_TYPES 16
		struct {
			char type[MAX_QPATH];
			cherryhudConfig_t config;
			qboolean loaded;
		} configs[MAX_CONFIG_TYPES];
		int configCount;
	} config;
} cherryhudGlobalContext_t;

cherryhudGlobalContext_t* CG_CHUDGetContext(void);

void CG_CHUDSetPlayerRowTemplate(const cherryhudConfig_t* config);
void CG_CHUDSetPlayerInfoTemplate(const cherryhudConfig_t* config);
void CG_CHUDSetSpectatorRowTemplate(const cherryhudConfig_t* config);
void CG_CHUDSetTitleRowTemplate(const cherryhudConfig_t* config);
const cherryhudConfig_t* CG_CHUDGetPlayerRowTemplate(void);
const cherryhudConfig_t* CG_CHUDGetSpectatorRowTemplate(void);
const cherryhudConfig_t* CG_CHUDGetTitleRowTemplate(void);
const cherryhudConfig_t* CG_CHUDGetPlayerInfoTemplate(int index);
int CG_CHUDGetPlayerInfoTemplateCount(void);
void CG_CHUDClearPlayerInfoTemplates(void);

void CG_CHUDAddPlayerRowElement(const cherryhudConfig_t* config);
void CG_CHUDAddSpectatorRowElement(const cherryhudConfig_t* config);
void CG_CHUDAddTitleRowElement(const cherryhudConfig_t* config);
void CG_CHUDResetElementIndexes(void);
const cherryhudConfig_t* CG_CHUDGetPlayerRowElement(int index);
const cherryhudConfig_t* CG_CHUDGetSpectatorRowElement(int index);
const cherryhudConfig_t* CG_CHUDGetTitleRowElement(int index);
int CG_CHUDGetPlayerRowElementCount(void);
int CG_CHUDGetSpectatorRowElementCount(void);
int CG_CHUDGetTitleRowElementCount(void);

#define MAX_TITLE_ELEMENTS 512
#define MAX_TITLE_BLOCKS 16

typedef struct {
    cherryhudConfig_t config;
    cherryhudElement_t* elements[MAX_TITLE_ELEMENTS];
    int elementCount;
    char containerType[MAX_QPATH];  // Associated container type (playersRows, spectatorsRows, etc.)
    qboolean isContainerTitle;      // Whether this title is associated with a container
} cherryhudTitleBlock_t;

void CG_CHUDSetDefaultElementProperties(cherryhudConfig_t* config);
void CG_CHUDCreateTitleBlock(const cherryhudConfig_t* config);
void CG_CHUDCreateTitleBlockForContainer(const cherryhudConfig_t* config, const char* containerType);
void CG_CHUDUpdateTitleBlockConfig(int blockIndex, const cherryhudConfig_t* config);
void CG_CHUDClearTitleBlocks(void);
int CG_CHUDGetTitleBlockCount(void);
const cherryhudTitleBlock_t* CG_CHUDGetTitleBlock(int index);
void CG_CHUDAddTitleBlockElement(int blockIndex, const cherryhudConfig_t* config);
qboolean CG_CHUDTitleBlockHasContainer(const cherryhudTitleBlock_t* titleBlock);
qboolean CG_CHUDContainerHasContent(const char* containerType);

void CG_CHUDSetScoreboardConfig(const cherryhudConfig_t* config);
void CG_CHUDSetDefaultConfig(const cherryhudConfig_t* config);
const cherryhudConfig_t* CG_CHUDGetScoreboardConfig(void);
const cherryhudConfig_t* CG_CHUDGetDefaultConfig(void);
qboolean CG_CHUDIsScoreboardConfigLoaded(void);
qboolean CG_CHUDIsDefaultConfigLoaded(void);

void CG_CHUDSetConfigByType(const char* type, const cherryhudConfig_t* config);
const cherryhudConfig_t* CG_CHUDGetConfigByType(const char* type);
qboolean CG_CHUDIsConfigLoadedByType(const char* type);
void CG_CHUDClearConfigByType(const char* type);
void CG_CHUDClearConfig(cherryhudConfig_t* config);
void CG_CHUDClearAllConfigs(void);

// Compact and double config management
void CG_CHUDSetCompactConfig(const cherryhudConfig_t* config);
void CG_CHUDSetDoubleConfig(const cherryhudConfig_t* config);
const cherryhudConfig_t* CG_CHUDGetCompactConfig(void);
const cherryhudConfig_t* CG_CHUDGetDoubleConfig(void);
qboolean CG_CHUDIsCompactConfigLoaded(void);
qboolean CG_CHUDIsDoubleConfigLoaded(void);

// Prefixed mode config management
cherryhudConfig_t* CG_CHUDGetPrefixedConfig(const cherryhudConfig_t* baseConfig, const char* prefix);
qboolean CG_CHUDIsModeActive(const cherryhudConfig_t* config, const char* mode, float currentHeight);
const char* CG_CHUDGetActiveMode(const cherryhudConfig_t* config, float currentHeight);
void CG_CHUDGetActivePos(const cherryhudConfig_t* config, float currentHeight, vec4_t result);
void CG_CHUDGetActiveSize(const cherryhudConfig_t* config, float currentHeight, vec4_t result);
void CG_CHUDGetActivePosFromContainer(const cherryhudConfig_t* config, const char* containerType, float currentHeight, vec4_t result);
void CG_CHUDGetActiveSizeFromContainer(const cherryhudConfig_t* config, const char* containerType, float currentHeight, vec4_t result);
void CG_CHUDGetActiveColor(const cherryhudConfig_t* config, float currentHeight, cherryhudColor_t* result);
void CG_CHUDGetActiveColor2(const cherryhudConfig_t* config, float currentHeight, cherryhudColor_t* result);
void CG_CHUDGetActiveBgColor(const cherryhudConfig_t* config, float currentHeight, float* result);


void CG_CHUDResetSystem(void);
void CG_CHUDLoadDefaultTemplates(void);



typedef struct cherryhudBlock_s {
    char type[MAX_QPATH];  // e.g., "row", "playersRows"
    cherryhudConfig_t config;
    struct cherryhudElement_s *elements;  // list of elements in this block
    struct cherryhudBlock_s *subblocks;   // list of sub-blocks
    struct cherryhudBlock_s *next;        // for linking blocks
    struct cherryhudBlock_s *parent;      // parent block for nesting
} cherryhudBlock_t;


typedef enum {
    CHERRYHUD_ROW_TYPE_TITLE,
    CHERRYHUD_ROW_TYPE_SELF,
    CHERRYHUD_ROW_TYPE_PLAYER,
    CHERRYHUD_ROW_TYPE_CLIENT,
    CHERRYHUD_ROW_TYPE_SPECTATOR
} cherryhudRowType_t;

typedef struct {
    int clientNum;
    int score;
    int ping;
    int team;
    qboolean connected;
    qboolean isSpectator;
    clientInfo_t* clientInfo;
    score_t* scoreData;
} cherryhudPlayerData_t;

// Table row structure
typedef struct {
    cherryhudRowType_t type;
    cherryhudPlayerData_t playerData;
    float yPosition;
    int rowIndex;
    cherryhudElement_t* elements; // List of elements in this row
} cherryhudTableRow_t;


// Runtime context for table (separate from config)
typedef struct {
    float baseX, baseY;           // Current position
    float width, height;          // Current size
    float rowHeight;              // Current row height
    float rowSpacing;             // Current row spacing
    int lastBuildTime;            // Last time this table was built (for throttling)
    qboolean needsRecalc;         // Whether table needs recalculation
    qboolean isDirty;             // Whether table data has changed
} cherryhudTableContext_t;

// Scoreboard order structures (needed for TableManager)
typedef enum {
    CHERRYHUD_BLOCK_TYPE_TITLE,
    CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS,
    CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS
} cherryhudBlockType_t;

typedef struct {
    cherryhudBlockType_t type;
    cherryhudConfig_t config;
    int order; // Order in which block was parsed
    int titleBlockIndex; // Index of title block (only for title blocks)
} cherryhudScoreboardBlock_t;

#define MAX_SCOREBOARD_BLOCKS 10
typedef struct {
    cherryhudScoreboardBlock_t blocks[MAX_SCOREBOARD_BLOCKS];
    int count;
} cherryhudScoreboardOrder_t;

// Table structure (needed for tableTypeEntry_t)
typedef struct {
    cherryhudTableRow_t* rows;
    int numRows;
    int maxRows;
    int playerRowsStart;  // Index where player rows start (after title + self)
    qboolean isDouble;
    char tableType[MAX_QPATH];  // "players", "spectators", or "mixed"
    cherryhudConfig_t config;   // Configuration (read-only after creation)
    cherryhudTableContext_t context; // Runtime context (managed by tableBuilder)
} cherryhudTable_t;

// Table type management
#define MAX_TABLE_TYPES 16
typedef struct {
    char type[MAX_QPATH];
    cherryhudTable_t* table;
    qboolean initialized;
} tableTypeEntry_t;

// Table Manager - replaces global state
typedef struct {
    cherryhudTable_t* globalTable;
    cherryhudTable_t* playersTable;
    cherryhudTable_t* spectatorsTable;
    qboolean tableDirty;          // Flag to track if rebuild is needed
    int lastPlayerHash;           // Simple hash of player data for change detection
    cherryhudScoreboardOrder_t* scoreboardOrder;
    tableTypeEntry_t tablesByType[MAX_TABLE_TYPES];
    int tableTypeCount;
    int blockOrderCounter;
} cherryhudTableManager_t;

// Service interfaces for separation of concerns
typedef struct {
    void (*collectData)(cherryhudPlayerData_t* players, int* numPlayers, const char* tableType);
    void (*sortData)(cherryhudPlayerData_t* players, int numPlayers);
    int (*calculateHash)(const cherryhudPlayerData_t* players, int numPlayers);
} cherryhudDataService_t;

typedef struct {
    void (*manageMemory)(cherryhudTable_t* table, int requiredRows);
    void (*updateRows)(cherryhudTable_t* table, const cherryhudPlayerData_t* players, int numPlayers);
    void (*finalizeTable)(cherryhudTable_t* table);
} cherryhudMemoryService_t;

typedef struct {
    void (*updateProperties)(cherryhudTable_t* table);
    qboolean (*needsRebuild)(cherryhudTable_t* table);
    qboolean (*dataChanged)(cherryhudTableManager_t* manager, cherryhudTable_t* table, int playerHash, int requiredRows);
} cherryhudValidationService_t;


// Block Management Prototypes
void CG_CHUDAddElementToBlock(cherryhudBlock_t* block, cherryhudElement_t* element);

// Rendering functions
void CG_CHUDRenderBlock(cherryhudBlock_t* block, float parentX, float parentY);
void CG_CHUDRenderElementWithMode(cherryhudElement_t* element, const char* containerType, float currentHeight);

// Configuration inheritance
void CG_CHUDInheritConfig(cherryhudConfig_t* child, const cherryhudConfig_t* parent);
void CG_CHUDApplyDefaultConfig(cherryhudConfig_t* config);

// Local default stack functions (internal)
const cherryhudConfig_t* CG_CHUDDefaultStackGetCurrent(void);
qboolean CG_CHUDDefaultStackHasDefaults(void);
void CG_CHUDDefaultStackPush(const cherryhudConfig_t* config);
void CG_CHUDDefaultStackPop(void);

// SuperHUD-style default management functions
void CG_CHUDSetCurrentDefaultConfig(const cherryhudConfig_t* config);
const cherryhudConfig_t* CG_CHUDGetCurrentDefaultConfig(void);
qboolean CG_CHUDHasActiveDefault(void);
void CG_CHUDDebugPrintCurrentDefault(void);

// Cleanup
void CG_CHUDDestroyBlock(cherryhudBlock_t* block);
void CG_CHUDDestroyAllWindows(void);

// Table Manager functions
cherryhudTableManager_t* CG_CHUDTableManagerCreate(void);
void CG_CHUDTableManagerDestroy(cherryhudTableManager_t* manager);
cherryhudTableManager_t* CG_CHUDGetTableManager(void);

// Service creation functions
cherryhudDataService_t* CG_CHUDDataServiceCreate(void);
cherryhudMemoryService_t* CG_CHUDMemoryServiceCreate(void);
cherryhudValidationService_t* CG_CHUDValidationServiceCreate(void);
void CG_CHUDDataServiceDestroy(cherryhudDataService_t* service);
void CG_CHUDMemoryServiceDestroy(cherryhudMemoryService_t* service);
void CG_CHUDValidationServiceDestroy(cherryhudValidationService_t* service);

// Table builder functions
cherryhudTable_t* CG_CHUDTableCreate(const cherryhudConfig_t* config);
void CG_CHUDTableDestroy(cherryhudTable_t* table);
void CG_CHUDTableBuild(cherryhudTableManager_t* manager, cherryhudTable_t* table);
void CG_CHUDTableRender(cherryhudTable_t* table);
void CG_CHUDTableRenderTitle(cherryhudTable_t* table);
void CG_CHUDTableAddElementToRow(cherryhudTable_t* table, int rowIndex, cherryhudElement_t* element);
cherryhudTableRow_t* CG_CHUDTableGetRow(cherryhudTable_t* table, int rowIndex);
int CG_CHUDTableGetNumRows(cherryhudTable_t* table);
void CG_CHUDTableSetProperties(cherryhudTable_t* table, float baseX, float baseY, float rowHeight, float rowSpacing);
void CG_CHUDTableUpdatePosition(cherryhudTable_t* table, const cherryhudConfig_t* containerConfig);

// Table building helper functions
void CG_CHUDTableCollectPlayerData(cherryhudPlayerData_t* players, int* numPlayers, const char* tableType);
qboolean CG_CHUDTableNeedsRebuild(cherryhudTable_t* table);
void CG_CHUDTableUpdateProperties(cherryhudTable_t* table);
qboolean CG_CHUDTableDataChanged(cherryhudTableManager_t* manager, cherryhudTable_t* table, int playerHash, int requiredRows);
void CG_CHUDTableManageRows(cherryhudTable_t* table, int requiredRows);
void CG_CHUDTableUpdateAllRows(cherryhudTable_t* table, const cherryhudPlayerData_t* players, int numPlayers);
void CG_CHUDTableFinalize(cherryhudTable_t* table);

// Table context management functions
void CG_CHUDTableContextInit(cherryhudTableContext_t* context);
void CG_CHUDTableContextUpdateFromConfig(cherryhudTableContext_t* context, const cherryhudConfig_t* config);
void CG_CHUDTableContextSetPosition(cherryhudTableContext_t* context, float x, float y);
void CG_CHUDTableContextSetSize(cherryhudTableContext_t* context, float width, float height);
void CG_CHUDTableContextSetRowProperties(cherryhudTableContext_t* context, float rowHeight, float rowSpacing);
void CG_CHUDTableContextMarkDirty(cherryhudTableContext_t* context);
void CG_CHUDTableContextMarkClean(cherryhudTableContext_t* context);
qboolean CG_CHUDTableContextNeedsRecalc(cherryhudTableContext_t* context);

// Direct table rendering functions (no config lookups)

// Scoreboard rendering
void CG_CHUDRenderScoreboard(void);
void CG_CHUDTableRenderTitleBlockByIndex(int titleBlockIndex, const cherryhudConfig_t* titleConfig);
void CG_CHUDRenderScoreboardElementsInOrder(const cherryhudConfig_t* scoreboardConfig);

// Scoreboard block order management

// Configuration storage for deferred element creation
typedef struct {
    char elementName[MAX_QPATH];  // Name of the element (playerInfo, serverInfo, etc.)
    cherryhudConfig_t config;     // Element configuration
    int context;                  // Context: 0=default, 1=playerRow, 2=spectatorRow, 3=title
    int order;                    // Order in which element was parsed
} cherryhudElementConfig_t;

typedef struct {
    cherryhudElementConfig_t elements[MAX_TITLE_ELEMENTS * 4]; // Store elements for all contexts// Lol X4
    int elementCount;
    qboolean isParsed;  // Whether parsing is complete
} cherryhudConfigStorage_t;


// Global table management
cherryhudTable_t* CG_CHUDGetGlobalTable(void);
cherryhudTable_t* CG_CHUDGetPlayersTable(void);
cherryhudTable_t* CG_CHUDGetSpectatorsTable(void);
void CG_CHUDSetGlobalTable(cherryhudTable_t* table);
void CG_CHUDTableInit(void);
void CG_CHUDTableInitByType(const char* type);
cherryhudTable_t* CG_CHUDGetTableByType(const char* type);
cherryhudTable_t* CG_CHUDGetTableByContext(int contextFlag);
void CG_CHUDTableShutdown(void);

// Dynamic height calculation
float CG_CHUDTableCalculateHeight(cherryhudTable_t* table);
float CG_CHUDContainerCalculateHeight(const char* containerType);
float CG_CHUDScoreboardCalculateHeight(void);

// Scoreboard element order management
void CG_CHUDScoreboardOrderInit(void);
void CG_CHUDScoreboardOrderAdd(cherryhudBlockType_t type, const cherryhudConfig_t* config);
void CG_CHUDScoreboardOrderClear(void);

// Configuration storage management
void CG_CHUDConfigStorageInit(void);
void CG_CHUDConfigStorageClear(void);
void CG_CHUDResetTitleElementIndex(void);
void CG_CHUDResetTitleBlockCount(void);
void CG_CHUDConfigStorageAddElement(const char* elementName, const cherryhudConfig_t* config, int context);
void CG_CHUDConfigStorageProcessElements(void);
cherryhudConfigStorage_t* CG_CHUDGetConfigStorage(void);

// Unified element type system
typedef enum {
    CHERRYHUD_TYPE_ELEMENT,      // Regular elements (playerInfo, serverInfo, print, text, etc.)
    CHERRYHUD_TYPE_BLOCK,        // Block/container types (playerRow, spectatorRow, title, etc.)
    CHERRYHUD_TYPE_CONTAINER,    // Container types (!scoreboard, playersRows, spectatorsRows)
    CHERRYHUD_TYPE_CONFIG       // Config types (!default)
} cherryhudElementTypeCategory_t;

typedef struct {
    const char* name;                                    // Element name
    cherryhudElementTypeCategory_t category;            // Type category
    qboolean isHandler;                                  // Has handler function
    void (*handler)(const char* elementName, cherryhudConfig_t* config); // Handler function
    void* (*create)(const cherryhudConfig_t* config, const char* containerType);    // Element create function
    void (*routine)(void* context);                      // Element routine function (render)
    void (*destroy)(void* context);                      // Element destroy function
    int bonusFlag;                                       // Bonus flag for additional info
} cherryhudElementTypeInfo_t;

// Element type management
qboolean CG_CHUDIsElementType(const char* elementName);
qboolean CG_CHUDIsContainerType(const char* elementName);
qboolean CG_CHUDIsConfigType(const char* elementName);
qboolean CG_CHUDIsBlockType(const char* elementName);
// CG_CHUDIsServerInfoType removed - now using unified decor element
const cherryhudElementTypeInfo_t* CG_CHUDGetElementTypeInfo(const char* elementName);
const cherryhudElementTypeInfo_t* CG_CHUDAutoDetectElementType(const cherryhudConfig_t* config);

// Helper functions for working with bonus flags
qboolean CG_CHUDIsTemplateBlock(int bonusFlag);
const char* CG_CHUDGetContainerName(cherryhudBlockType_t type);

// Element handler system
typedef enum {
    CHERRYHUD_HANDLER_ELEMENT,
    CHERRYHUD_HANDLER_CONTAINER,
    CHERRYHUD_HANDLER_CONFIG
} cherryhudHandlerType_t;

typedef struct {
    const char* name;
    cherryhudHandlerType_t type;
    void (*handler)(const char* elementName, cherryhudConfig_t* config);
} cherryhudElementHandler_t;

void CG_CHUDRegisterElementHandlers(void);
cherryhudElementHandler_t* CG_CHUDFindElementHandler(const char* elementName);
void CG_CHUDHandleElement(const char* elementName, cherryhudConfig_t* config);

// Individual handler functions
void CG_CHUDHandlePlayerRow(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandleSpectatorRow(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandleTitle(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandlePlayersRows(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandleSpectatorsRows(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandleScoreboardConfig(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandleDefaultConfig(const char* elementName, cherryhudConfig_t* config);
void CG_CHUDHandleCustomConfig(const char* elementName, cherryhudConfig_t* config);
const cherryhudScoreboardOrder_t* CG_CHUDScoreboardOrderGet(void);

// Player counting functions
int CG_CHUDGetPlayerCount(void);
int CG_CHUDGetSpectatorCount(void);

// Global row height from playerRow config
extern float g_playerRowHeight;

// Global variables for parser state
extern int g_scoreboardBlockDepth;

// Global variable for current clientNum in color context
extern int g_currentColorClientNum;


// Utility functions
void CG_CHUDRoutenesDestroy(cherryhudElement_t* chud);
// Centralized border drawing function with negative/positive size logic
void CG_CHUDDrawBorderCentralized(float x, float y, float w, float h, const vec4_t borderSize, const vec4_t borderColor);
// Centralized background fill function
void CG_CHUDFillCentralized(float x, float y, float w, float h, const float* bgColor);
qboolean CG_CHUDDrawBorder(const cherryhudConfig_t* cfg);

// Unified rendering functions
qboolean CG_CHUDRenderBackground(const cherryhudConfig_t* config, float x, float y, float w, float h);
qboolean CG_CHUDRenderBorder(const cherryhudConfig_t* config, float x, float y, float w, float h);
void CG_CHUDRenderBackgroundAndBorder(const cherryhudConfig_t* config, float x, float y, float w, float h);

// Player info element functions - now handled by decor element
void CG_CHUDElementDecorSetClientNum(void* context, int clientNum);

//
// cg_cherryhud_lexer
//
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_BLOCK_START,      // {
    TOKEN_BLOCK_END,        // }
    TOKEN_ELEMENT,          // playerInfo, print, etc.
    TOKEN_COMMAND,          // pos, size, color, etc.
    TOKEN_STRING,           // "text" or text
    TOKEN_NUMBER,           // 123.45
    TOKEN_COMMA,            // ,
    TOKEN_SEMICOLON,        // ;
    TOKEN_COMMENT,          // # comment
    TOKEN_NEWLINE,          // \n
    TOKEN_WHITESPACE,       // spaces, tabs
    TOKEN_ERROR             // Invalid token
} cherryhud_token_type_t;

typedef struct cherryhud_token_s {
    cherryhud_token_type_t type;
    char* value;
    int line;
    int column;
    int length;
} cherryhud_token_t;

typedef struct cherryhud_lexer_s {
    const char* input;
    int input_length;
    int position;
    int line;
    int column;
    cherryhud_token_t* tokens;
    int token_count;
    int token_capacity;
} cherryhud_lexer_t;

cherryhud_lexer_t* CG_CHUDLexerCreate(const char* input);
void CG_CHUDLexerDestroy(cherryhud_lexer_t* lexer);
cherryhud_token_t* CG_CHUDLexerTokenize(cherryhud_lexer_t* lexer);
cherryhud_token_t* CG_CHUDLexerGetToken(cherryhud_lexer_t* lexer, int index);
int CG_CHUDLexerGetTokenCount(cherryhud_lexer_t* lexer);


//
// cg_cherryhud_lexer_parser
//
typedef struct cherryhud_parser_s {
    cherryhud_lexer_t* lexer;
    int current_token;
    int token_count;
    cherryhud_token_t* cached_token;
    int cached_token_index;
} cherryhud_parser_t;

cherryhud_parser_t* CG_CHUDLexerParserCreate(const char* input);
void CG_CHUDLexerParserDestroy(cherryhud_parser_t* parser);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParse(cherryhud_parser_t* parser, cherryhudConfig_t* config);

void CG_CHUDLexerParserResetStatics(void);
void CG_CHUDResetElementCounter(void);

cherryhud_token_t* CG_CHUDLexerParserGetCurrentToken(cherryhud_parser_t* parser);
cherryhud_token_t* CG_CHUDLexerParserPeekToken(cherryhud_parser_t* parser, int offset);
qboolean CG_CHUDLexerParserIsAtEnd(cherryhud_parser_t* parser);
qboolean CG_CHUDLexerParserMatch(cherryhud_parser_t* parser, cherryhud_token_type_t type);
qboolean CG_CHUDLexerParserMatchElement(cherryhud_parser_t* parser, const char* name);
cherryhudConfigParseStatus_t CG_CHUDLexerParserSkipToNextToken(cherryhud_parser_t* parser);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseValue(cherryhud_parser_t* parser, char* value, int maxlen);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseFloat(cherryhud_parser_t* parser, float* value);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseInt(cherryhud_parser_t* parser, int* value);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVec2(cherryhud_parser_t* parser, vec2_t value);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVec3(cherryhud_parser_t* parser, vec3_t value);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseVec4(cherryhud_parser_t* parser, vec4_t value);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseColor(cherryhud_parser_t* parser, cherryhudConfig_t* config);
cherryhudConfigParseStatus_t CG_CHUDLexerParserParseCommand(cherryhud_parser_t* parser, cherryhudConfig_t* config);

cherryhudConfigParseResult_t CG_CHUDParseConfigWithLexer(const char* input, cherryhudConfig_t* config);

cherryhudConfigParseStatus_t CG_CHUDLexerParserParseUniversal(cherryhud_parser_t* parser, cherryhudConfig_t* config);

cherryhudConfigParseStatus_t (*CG_CHUDLexerFindCommand(const char* name))(cherryhud_parser_t* parser, cherryhudConfig_t* config);

// Layout system integration
//
// cg_cherryhud_layout
//
typedef enum {
    CHERRYHUD_LAYOUT_MODE_ABSOLUTE,    // Absolute positioning
    CHERRYHUD_LAYOUT_MODE_RELATIVE,    // Relative to parent
    CHERRYHUD_LAYOUT_MODE_FLOW,        // Flow layout (horizontal/vertical)
    CHERRYHUD_LAYOUT_MODE_GRID,        // Grid layout
    CHERRYHUD_LAYOUT_MODE_TABLE        // Table layout (for scoreboard)
} cherryhudLayoutMode_t;

// Layout direction for flow mode
typedef enum {
    CHERRYHUD_LAYOUT_DIR_HORIZONTAL,
    CHERRYHUD_LAYOUT_DIR_VERTICAL
} cherryhudLayoutDirection_t;

// Layout constraints
typedef struct {
    float minWidth;
    float maxWidth;
    float minHeight;
    float maxHeight;
    qboolean isSet;
} cherryhudLayoutConstraints_t;

// Layout properties for each element
typedef struct {
    cherryhudLayoutMode_t mode;
    cherryhudLayoutDirection_t direction;
    cherryhudLayoutConstraints_t constraints;
    
    // Flow layout properties
    float spacing;          // Space between elements in flow mode
    float padding;          // Padding around elements
    
    // Grid layout properties
    int gridColumns;        // Number of columns in grid
    int gridRows;           // Number of rows in grid
    float gridSpacingX;     // Horizontal spacing in grid
    float gridSpacingY;     // Vertical spacing in grid
    
    // Table layout properties
    int tableColumn;        // Column index in table
    int tableRow;           // Row index in table
    float tableCellWidth;   // Width of table cell
    float tableCellHeight;  // Height of table cell
    
    qboolean isSet;
} cherryhudLayoutProperties_t;

// Calculated layout bounds
typedef struct {
    float x, y, width, height;  // Final calculated bounds
    float contentX, contentY;   // Content area (excluding padding)
    float contentWidth, contentHeight;
    qboolean isValid;           // Whether layout is valid
    int lastUpdateTime;         // When this layout was last calculated
    int clientNum;              // Client number for playerInfo elements
} cherryhudLayoutBounds_t;

// Layout context for calculations
typedef struct {
    float parentX, parentY;     // Parent position
    float parentWidth, parentHeight; // Parent size
    float availableWidth, availableHeight; // Available space
    int currentTime;            // Current time for caching
    qboolean forceRecalc;       // Force recalculation
    int clientNum;              // Client number for playerInfo elements
} cherryhudLayoutContext_t;

// Layout manager state
typedef struct {
    cherryhudLayoutBounds_t* bounds;  // Cached bounds for all elements
    int boundsCount;
    int boundsCapacity;
    int lastLayoutTime;        // When layout was last calculated
    qboolean needsRecalc;      // Whether layout needs recalculation
} cherryhudLayoutManager_t;

// Layout calculation functions
void CG_CHUDLayoutInit(void);
void CG_CHUDLayoutShutdown(void);
void CG_CHUDLayoutRecalculate(void);
void CG_CHUDLayoutCleanupUnused(void);

// Layout property management
void CG_CHUDLayoutSetProperties(cherryhudElement_t* element, const cherryhudLayoutProperties_t* props);
void CG_CHUDLayoutGetProperties(const cherryhudElement_t* element, cherryhudLayoutProperties_t* props);
void CG_CHUDLayoutSetDefaultProperties(cherryhudLayoutProperties_t* props);

// Layout calculation functions
cherryhudLayoutBounds_t* CG_CHUDLayoutCalculateElement(cherryhudElement_t* element, const cherryhudLayoutContext_t* context);
cherryhudLayoutBounds_t* CG_CHUDLayoutCalculateBlock(cherryhudBlock_t* block, const cherryhudLayoutContext_t* context);

// Layout bounds management
cherryhudLayoutBounds_t* CG_CHUDLayoutGetBoundsByContext(void* context);
void CG_CHUDLayoutRemoveElement(void* elementPtr);
void CG_CHUDLayoutForceRecalcForTable(cherryhudTable_t* table);


// Layout mode specific calculations
void CG_CHUDLayoutCalculateAbsolute(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds);
void CG_CHUDLayoutCalculateTable(cherryhudElement_t* element, const cherryhudLayoutContext_t* context, cherryhudLayoutBounds_t* bounds);

// Layout utility functions
void CG_CHUDLayoutApplyConstraints(cherryhudLayoutBounds_t* bounds, const cherryhudLayoutConstraints_t* constraints);

// Width inheritance utility - inherits width from scoreboard config
void CG_CHUDInheritRowWidth(cherryhudConfig_t* config);

// Common element helper functions
void* CG_CHUDCreateElementBase(const cherryhudConfig_t* config, int elementSize);
void CG_CHUDDestroyElementBase(void* context);
qboolean CG_CHUDValidateElementContext(void* context, const cherryhudConfig_t* config);
qboolean CG_CHUDValidateElementBounds(cherryhudLayoutBounds_t* bounds);

// Common validation utilities
qboolean CG_CHUDValidateElementAndConfig(void* element, const cherryhudConfig_t* config);
qboolean CG_CHUDValidateClientNumber(int clientNum);
qboolean CG_CHUDValidateBounds(cherryhudLayoutBounds_t* bounds);

// Common rendering utilities
void CG_CHUDRenderElementWithValidation(void* element, const cherryhudConfig_t* config, 
                                       cherryhudLayoutBounds_t* bounds, int clientNum);
void CG_CHUDRenderElementBackgroundAndBorder(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds);
void CG_CHUDSetElementPositionFromBounds(cherryhudTextContext_t* textCtx, cherryhudLayoutBounds_t* bounds);
void CG_CHUDRenderContainerBackgroundFromBounds(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds);
void CG_CHUDRenderContainerBackgroundFromBoundsWithHeight(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds, float currentHeight);
qboolean CG_CHUDValidateClientNum(int clientNum);
void CG_CHUDRenderElementText(const cherryhudConfig_t* config, cherryhudTextContext_t* textCtx, const char* displayText);

void CG_CHUDSetTextContextFromBounds(cherryhudTextContext_t* textCtx, cherryhudLayoutBounds_t* bounds);
void CG_CHUDSetDrawContextFromBounds(cherryhudDrawContext_t* drawCtx, cherryhudLayoutBounds_t* bounds);

// Generic type handler lookup structure
typedef struct {
    const char* typeName;
    void (*handler)(void* element);
} cherryhudTypeHandler_t;

// ============================================================================
// ALIGNMENT HELPER FUNCTIONS
// ============================================================================

// Apply horizontal alignment to X position
float CG_CHUDApplyAlignH(float x, float width, cherryhudAlignH_t alignH);

// Apply vertical alignment to Y position
float CG_CHUDApplyAlignV(float y, float height, cherryhudAlignV_t alignV);

// Apply both horizontal and vertical alignment to position
void CG_CHUDApplyAlignment(float* x, float* y, float width, float height, 
                          cherryhudAlignH_t alignH, cherryhudAlignV_t alignV);

// Get alignment values from config with defaults
void CG_CHUDGetAlignmentFromConfig(const cherryhudConfig_t* config, 
                                  cherryhudAlignH_t* alignH, cherryhudAlignV_t* alignV);

// Get textExt alignment values from config with defaults
void CG_CHUDGetTextExtAlignmentFromConfig(const cherryhudConfig_t* config, 
                                         cherryhudAlignH_t* alignH, cherryhudAlignV_t* alignV);

// Apply alignment to text context
void CG_CHUDApplyAlignmentToTextContext(cherryhudTextContext_t* textCtx, 
                                       cherryhudAlignH_t alignH, cherryhudAlignV_t alignV);

// Apply alignment to draw context
void CG_CHUDApplyAlignmentToDrawContext(cherryhudDrawContext_t* drawCtx, 
                                       cherryhudAlignH_t alignH, cherryhudAlignV_t alignV);

// Apply alignment to layout bounds
void CG_CHUDApplyAlignmentToBounds(cherryhudLayoutBounds_t* bounds, 
                                  cherryhudAlignH_t alignH, cherryhudAlignV_t alignV);

// Apply alignment from config to layout bounds
void CG_CHUDApplyConfigAlignmentToBounds(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds);

// Apply textExt alignment from config to layout bounds
void CG_CHUDApplyTextExtAlignmentToBounds(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds);

// Format number according to format string
void CG_CHUDFormatNumber(float value, const char* format, char* output, int maxlen);

// Centralized visibility checking function (similar to SuperHUD)
qboolean CG_CHUDCheckElementVisibility(const cherryhudConfig_t* config, int clientNum, cherryhudElement_t* element);

// Centralized hideflags checking function (opposite logic to visflags)
qboolean CG_CHUDCheckElementHideFlags(const cherryhudConfig_t* config, int clientNum, cherryhudElement_t* element);

// Get container mode for element (0=none, 1=compact, 2=double)
int CG_CHUDGetElementContainerMode(cherryhudElement_t* element);

// Get active mode from container for child elements
const char* CG_CHUDGetContainerActiveMode(const char* containerType, float currentHeight);

// Check if element should be visible based on localClient visflag
qboolean CG_CHUDCheckLocalClientVisibility(const cherryhudConfig_t* config, int clientNum);

// Check if element should be visible based on hideflags (opposite logic)
qboolean CG_CHUDCheckHideFlagVisibility(const cherryhudConfig_t* config, int clientNum);

// Extended table rendering functions
qboolean CG_CHUDTableExceedsMaxHeight(cherryhudTable_t* table, float maxHeight);

// Generic type handler lookup function
void* CG_CHUDFindTypeHandler(const cherryhudTypeHandler_t* handlers, const char* typeName);

// ============================================================================
// TABLE MANAGEMENT - Old system functions
// ============================================================================

#ifdef __cplusplus
}
#endif

#endif
