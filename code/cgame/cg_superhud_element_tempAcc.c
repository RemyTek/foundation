#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

#define SHUD_STYLE_FLOAT       (1 << 0)
#define SHUD_STYLE_COLORIZED   (1 << 1)

typedef enum
{
	SHUD_ELEMENT_TEMPACC_TEXT,
	SHUD_ELEMENT_TEMPACC_ICON
} shudElementTempAccType_t;

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t textCtx;
	superhudDrawContext_t drawCtx;
	superhudGlobalContext_t* gctx;
	shudElementTempAccType_t type;
}
shudElementTempAcc_t;

void* CG_SHUDElementTempAccCreate(const superhudConfig_t* config, shudElementTempAccType_t type)
{
	shudElementTempAcc_t* element;

	SHUD_ELEMENT_INIT(element, config);

	element->type = type;

	element->gctx = CG_SHUDGetContext();
	if (element->type == SHUD_ELEMENT_TEMPACC_TEXT)
	{
		CG_SHUDTextMakeContext(&element->config, &element->textCtx);
		CG_SHUDFillAndFrameForText(&element->config, &element->textCtx);
	}
	else
	{
		CG_SHUDDrawMakeContext(&element->config, &element->drawCtx);
	}
	return element;
}

void* CG_SHUDElementTempAccTextCreate(const superhudConfig_t* config)
{
	return CG_SHUDElementTempAccCreate(config, SHUD_ELEMENT_TEMPACC_TEXT);
}
void* CG_SHUDElementTempAccIconCreate(const superhudConfig_t* config)
{
	return CG_SHUDElementTempAccCreate(config, SHUD_ELEMENT_TEMPACC_ICON);
}

static void CG_SHUDEStylesTempAcc_Color(vec4_t color, const superhudConfig_t* config, float accuracy)
{
	if (config->style.isSet && (config->style.value & SHUD_STYLE_COLORIZED))
	{
		if (accuracy < 30.0f)
		{
			Vector4Copy(colorRed, color);
		}
		else if (accuracy < 50.0f)
		{
			Vector4Copy(colorYellow, color);
		}
		else if (accuracy < 60.0f)
		{
			Vector4Copy(colorGreen, color);
		}
		else
		{
			Vector4Copy(colorMagenta, color);
		}
	}
	else
	{
		Vector4Copy(config->color.value.rgba, color);
	}
}


void CG_SHUDElementTempAccRoutine(void* context)
{
	shudElementTempAcc_t* element = (shudElementTempAcc_t*)context;
	superhudTempAccEntry_t* entry = &element->gctx->tempAcc.weapon[WP_LIGHTNING];
	vec4_t color;
	char accuracyStr[8];

	CG_GetWeaponTempAccuracy(WP_LIGHTNING);

	if (entry->tempAccuracy <= 0.0f &&
	        (!(SHUD_CHECK_SHOW_EMPTY(element))))
	{
		return;
	}

	if (element->type == SHUD_ELEMENT_TEMPACC_TEXT)
	{
		if (element->config.style.isSet && (element->config.style.value & SHUD_STYLE_FLOAT))
			Com_sprintf(accuracyStr, sizeof(accuracyStr), "%.1f%%", entry->tempAccuracy);
		else
			Com_sprintf(accuracyStr, sizeof(accuracyStr), "%.0f%%", entry->tempAccuracy);

		element->textCtx.text = va("%s", accuracyStr);


		CG_SHUDEStylesTempAcc_Color(color, &element->config, entry->tempAccuracy);
		Vector4Copy(color, element->textCtx.color);

		CG_SHUDFillAndFrameForText(&element->config, &element->textCtx);
		CG_SHUDTextPrintNew(&element->config, &element->textCtx, qfalse);
	}
	else if (element->type == SHUD_ELEMENT_TEMPACC_ICON)
	{
		element->drawCtx.image = cgs.media.tempAccIcon;

		CG_SHUDDrawBorder(&element->config);
		CG_SHUDFill(&element->config);
		CG_SHUDDrawStretchPicCtx(&element->config, &element->drawCtx);
	}
}

void CG_SHUDElementTempAccDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
