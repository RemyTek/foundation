#include "./cg_cherryhud_private.h"
#include "cg_local.h"

// Global variable for current clientNum in color context
int g_currentColorClientNum = 0;

typedef struct
{
	vec4_t bar1_value;
	vec4_t bar1_bottom;
	vec4_t bar2_value;
	vec4_t bar2_bottom;
} drawBarCoords_t;


static void CG_CHUDConfigPickColorGeneric(
    const qboolean* mainIsSet,
    const cherryhudColor_t* mainValue,
    const qboolean* altIsSet,
    const cherryhudColor_t* altValue,
    float* color,
    qboolean alphaOverride)
{
	const cherryhudColor_t* in = mainValue;
	const float* target;
	team_t team;
	float finalAlpha = 1.0f;

	if (!(*mainIsSet))
	{
		if (alphaOverride)
		{
			Vector4Copy(colorWhite, color);
		}
		else
		{
			VectorCopy(colorWhite, color);
		}
		return;
	}

	switch (in->type)
	{
		case CHERRYHUD_COLOR_RGBA:
			target = in->rgba;
			finalAlpha = in->rgba[3];
			break;

		case CHERRYHUD_COLOR_T:
			team = CG_CHUDGetOurActiveTeam();
			target = (team == TEAM_BLUE) ? cgs.be.blueTeamColor : cgs.be.redTeamColor;
			break;

		case CHERRYHUD_COLOR_E:
			team = CG_CHUDGetOurActiveTeam();
			target = (team == TEAM_BLUE) ? cgs.be.redTeamColor : cgs.be.blueTeamColor;
			break;

		case CHERRYHUD_COLOR_U:
			// For unique color, use the global clientNum
			// If not set (0), use local client number
			if (g_currentColorClientNum > 0) {
				target = UNIQUE_COLOR(g_currentColorClientNum);
			} else {
				target = UNIQUE_COLOR(cg.clientNum);
			}
			break;

		case CHERRYHUD_COLOR_I:
		default:
			target = colorWhite;
			break;
	}

	if ((*altIsSet) && in->type != CHERRYHUD_COLOR_RGBA)
	{
		finalAlpha = altValue->rgba[3];
	}

	if (alphaOverride)
	{
		color[0] = target[0];
		color[1] = target[1];
		color[2] = target[2];
		color[3] = finalAlpha;
	}
	else
	{
		color[0] = target[0];
		color[1] = target[1];
		color[2] = target[2];
		color[3] = finalAlpha;
	}
}
void CG_CHUDConfigPickColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	CG_CHUDConfigPickColorGeneric(&config->color.isSet, &config->color.value,
	                              &config->color2.isSet, &config->color2.value,
	                              color, alphaOverride);
}

void CG_CHUDConfigPickTextColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	CG_CHUDConfigPickColorGeneric(&config->textExt.color.isSet, &config->textExt.color.value,
	                              &config->textExt.color2.isSet, &config->textExt.color2.value,
	                              color, alphaOverride);
	
}

void CG_CHUDConfigPickBgColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	CG_CHUDConfigPickColorGeneric(&config->background.color.isSet, &config->background.color,
	                              &config->background.color2.isSet, &config->background.color2,
	                              color, alphaOverride);
}

void CG_CHUDConfigPickBorderColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	CG_CHUDConfigPickColorGeneric(&config->border.color.isSet, &config->border.color,
	                              &config->border.color2.isSet, &config->border.color2,
	                              color, alphaOverride);
}



static void CG_CHUDTextDefaultChecks(cherryhudConfig_t* config)
{
	/* Set default textExt.size if not set */
	if (!config->textExt.size.isSet)
	{
		/* if textExt.size wasn't set, find max size for rect */
		config->textExt.size.value[1] = config->size.value[1];
		config->textExt.size.value[0] = config->textExt.size.value[1] / 1.618f;
		config->textExt.size.isSet = qtrue;
	}
	else
	{
		/* if textExt.size was set but only one value, use it for both width and height */
		if (config->textExt.size.value[1] == 0)
		{
			config->textExt.size.value[1] = config->textExt.size.value[0];
		}
	}

	// Set default textExt.pos if not set (default offset is 0,0)
	if (!config->textExt.pos.isSet)
	{
		config->textExt.pos.value[0] = 0.0f;
		config->textExt.pos.value[1] = 0.0f;
		config->textExt.pos.isSet = qtrue;
	}

    // Set default textExt.color if not set - copy from regular color
    // Only do this if textExt.color was not already set (e.g., through inheritance)
    // This ensures that inherited textExt.color values are not overwritten
    if (!config->textExt.color.isSet && config->color.isSet)
    {
        config->textExt.color.value = config->color.value;
        config->textExt.color.isSet = qtrue;
    } else if (config->textExt.color.isSet) {
    }

	// Set default textExt.color2 if not set - copy from regular color2
	// Only do this if textExt.color2 was not already set (e.g., through inheritance)
	// This ensures that inherited textExt.color2 values are not overwritten
	if (!config->textExt.color2.isSet && config->color2.isSet)
	{
		config->textExt.color2.value = config->color2.value;
		config->textExt.color2.isSet = qtrue;
	}

	// Set default textExt.font if not set - copy from regular font
	if (!config->textExt.font.isSet && config->font.isSet)
	{
		Q_strncpyz(config->textExt.font.value, config->font.value, sizeof(config->textExt.font.value));
		config->textExt.font.isSet = qtrue;
	}



}

void CG_CHUDConfigDefaultsCheck(cherryhudConfig_t* config)
{
	// Only apply defaults for properties that are NOT already set
	// This ensures that inherited values are not overwritten
	// This fixes the issue where title blocks lost their inherited position/size

	if (!config->pos.isSet)
	{
		config->pos.value[0] = 0.0f;
		config->pos.value[1] = 0.0f;
		config->pos.isSet = qtrue;
	}

	if (!config->size.isSet)
	{
		config->size.value[0] = 12.0f;
		config->size.value[1] = 12.0f;
		config->size.isSet = qtrue;
	}

	if (!config->color.isSet)
	{
		Vector4Copy(colorWhite, config->color.value.rgba);
		config->color.value.type = CHERRYHUD_COLOR_RGBA;
		config->color.isSet = qtrue;
	}
	
	if (!config->textExt.alignH.isSet)
	{
		config->textExt.alignH.value = CHERRYHUD_ALIGNH_LEFT;
		config->textExt.alignH.isSet = qtrue;
	}

	// textExt.shadow has no default - it's only set when explicitly specified in config

	// Handle all textExt parameters
	CG_CHUDTextDefaultChecks(config);

	// Set default alignH if not set
	if (!config->alignH.isSet)
	{
		config->alignH.value = CHERRYHUD_ALIGNH_LEFT;
		config->alignH.isSet = qtrue;
	}

	// Set default alignV if not set
	if (!config->alignV.isSet)
	{
		config->alignV.value = CHERRYHUD_ALIGNV_CENTER;
		config->alignV.isSet = qtrue;
	}

	// Set default font if not set
	if (!config->font.isSet)
	{
		Q_strncpyz(config->font.value, "sansman", sizeof(config->font.value));
		config->font.isSet = qtrue;
	}


	// Set default type if not set (for playerInfo elements)
	if (!config->type.isSet)
	{
		Q_strncpyz(config->type.value, "name", sizeof(config->type.value));
		config->type.isSet = qtrue;
	}

	// Set fill flag if background color or image is set (only if not already set)
	if (!config->fill.isSet && (config->background.color.isSet || config->image.isSet))
	{
		config->fill.isSet = qtrue;
	}
	
}


void CG_CHUDTextMakeAdjustCoords(const cherryhudConfig_t* in, float* out_x, float* out_y)
{
	cherryhudAlignH_t h;
	cherryhudAlignV_t v;

	if (!in->pos.isSet || !in->size.isSet)
	{
		return;
	}

	// Use textExt.alignH for horizontal positioning, fallback to regular alignH
	if (in->textExt.alignH.isSet)
	{
		h = in->textExt.alignH.value;
	}
	else if (in->alignH.isSet)
	{
		h = in->alignH.value;
	}
	else
	{
		h = CHERRYHUD_ALIGNH_LEFT;
	}

	// Use textExt.alignV for vertical positioning, fallback to regular alignV
	if (in->textExt.alignV.isSet)
	{
		v = in->textExt.alignV.value;
	}
	else if (in->alignV.isSet)
	{
		v = in->alignV.value;
	}
	else
	{
		v = CHERRYHUD_ALIGNV_TOP;
	}

	// Use centralized alignment helpers
	*out_x = in->pos.value[0];
	*out_y = in->pos.value[1];
	CG_CHUDApplyAlignment(out_x, out_y, in->size.value[0], in->size.value[1], h, v);

	// Add textExt.pos offset (text.pos works as offset relative to element pos)
	if (in->textExt.pos.isSet)
	{
		*out_x += in->textExt.pos.value[0];
		*out_y += in->textExt.pos.value[1];
	}
}

void CG_CHUDTextMakeContext(const cherryhudConfig_t* in, cherryhudTextContext_t* out)
{
	cherryhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_CHUDConfigDefaultsCheck(&config);

	// Use alignH/alignV for positioning text element

	CG_CHUDTextMakeAdjustCoords(&config, &out->pos[0], &out->pos[1]);


	// Use textExt.alignH for horizontal text alignment within the element
	switch (config.textExt.alignH.value)
	{
		default:
		case CHERRYHUD_ALIGNH_LEFT:
			out->flags |= DS_HLEFT;
			break;
		case CHERRYHUD_ALIGNH_CENTER:
			out->flags |= DS_HCENTER;
			break;
		case CHERRYHUD_ALIGNH_RIGHT:
			out->flags |= DS_HRIGHT;
			break;
	}

	out->size[0] = config.textExt.size.value[0];
	out->size[1] = config.textExt.size.value[1];

	// Use textExt.alignV for vertical text alignment within the element
	if (config.textExt.alignV.isSet)
	{
		switch (config.textExt.alignV.value)
		{
			default:
			case CHERRYHUD_ALIGNV_TOP:
				out->flags |= DS_VTOP;
				break;
			case CHERRYHUD_ALIGNV_CENTER:
				out->flags |= DS_VCENTER;
				break;
			case CHERRYHUD_ALIGNV_BOTTOM:
				out->flags |= DS_VBOTTOM;
				break;
		}
	}
 
	if (!config.monospace.isSet)
	{
		out->flags |= DS_PROPORTIONAL;
	}
	// Handle shadow logic
	if (config.textExt.shadowColor.isSet)
	{
		// If shadow color is set, enable shadow with that color
		out->flags |= DS_SHADOW;
		CG_CHUDConfigPickColorGeneric(&config.textExt.shadowColor.isSet, &config.textExt.shadowColor.value,
		                              &config.textExt.shadowColor.isSet, &config.textExt.shadowColor.value,
		                              out->shadowColor, qtrue);
	}
	else if (config.textExt.shadow.isSet)
	{
		// If only shadow flag is set, enable shadow with black color
		out->flags |= DS_SHADOW;
		Vector4Copy(colorBlack, out->shadowColor);
		if (config.color.isSet)
		{
			out->shadowColor[3] = config.color.value.rgba[3];
		}
	}
	else
	{
		// No shadow by default
		Vector4Copy(colorBlack, out->shadowColor);
	}

	// Use textExt.font if set, otherwise fallback to global font, then default
	if (config.textExt.font.isSet) {
		out->fontIndex = CG_FontIndexFromName(config.textExt.font.value);
	} else if (config.font.isSet) {
		out->fontIndex = CG_FontIndexFromName(config.font.value);
	} else {
		out->fontIndex = CG_FontIndexFromName("sansman");
	}
	
	// Set maxWidth from config
	if (config.maxWidth.isSet) {
		out->width = config.maxWidth.value;
	} else {
		out->width = SCREEN_WIDTH; // No limit by default
	}

	CG_CHUDConfigPickTextColor(&config, out->color, qtrue);
	Vector4Copy(out->color, out->color_origin);
	
	// Copy text-specific border, bordercolor, and bgcolor from config to text context
	// Use only textExt parameters for text (no fallback to regular parameters)
	if (config.textExt.border.isSet) {
		Vector4Copy(config.textExt.border.sideSizes, out->border);
	}
	
	if (config.textExt.bordercolor.isSet) {
		Vector4Copy(config.textExt.bordercolor.value.rgba, out->borderColor);
	}
	
	if (config.textExt.background.isSet) {
		Vector4Copy(config.textExt.background.color.rgba, out->background);
	}
}



void CG_CHUDDrawMakeAdjustCoords(const cherryhudConfig_t* in, float* out_x, float* out_y)
{
	cherryhudAlignH_t h;
	cherryhudAlignV_t v;

	if (!in->pos.isSet || !in->size.isSet)
	{
		return;
	}

	// Use centralized alignment helpers
	CG_CHUDGetAlignmentFromConfig(in, &h, &v);
	*out_x = in->pos.value[0];
	*out_y = in->pos.value[1];
	CG_CHUDApplyAlignment(out_x, out_y, in->size.value[0], in->size.value[1], h, v);
}

void CG_CHUDDrawMakeContext(const cherryhudConfig_t* in, cherryhudDrawContext_t* out)
{
	cherryhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_CHUDConfigDefaultsCheck(&config);

	// Use alignH/alignV for positioning draw element
	CG_CHUDDrawMakeAdjustCoords(&config, &out->pos[0], &out->pos[1]);

	out->size[0] = config.size.value[0];
	out->size[1] = config.size.value[1];

	out->picPos[0] = 0.0f;
	out->picPos[1] = 0.0f;
	out->picSize[0] = 1.0f;
	out->picSize[1] = 1.0f;

	CG_CHUDConfigPickColor(&config, out->color, qtrue);
	Vector4Copy(out->color, out->color_origin);
}

void CG_CHUDBarMakeContext(const cherryhudConfig_t* in, cherryhudBarContext_t* out, float max)
{
	float x = 0, y = 0;
	float bar_height, bar_width;
	cherryhudConfig_t config;
	memset(out, 0, sizeof(*out));
	memcpy(&config, in, sizeof(config));

	CG_CHUDConfigDefaultsCheck(&config);

	if (!config.direction.isSet)
	{
		config.direction.isSet = qtrue;
		config.direction.value = CHERRYHUD_DIR_LEFT_TO_RIGHT;
	}

	if (!config.style.isSet) // set default style
	{
		config.style.isSet = qtrue;
		config.style.value = 1;
	}
	else if (config.style.value < 1 || config.style.value > 3) // need to rework
	{
		config.style.value = 1;
	}

	out->direction = config.direction.value;

	x = config.pos.value[0];
	y = config.pos.value[1];

	if (config.doublebar.isSet)
	{
		static const float bar_gap = 4;
		out->two_bars = qtrue;
		if (out->direction == CHERRYHUD_DIR_LEFT_TO_RIGHT || out->direction == CHERRYHUD_DIR_RIGHT_TO_LEFT)
		{
			if (config.style.value == 1)    // style 1 - default: split into two bars
			{

				bar_height = config.size.value[1] / 2 - bar_gap / 2; //split horizontal
				bar_width = config.size.value[0];

				out->bar[0][0] = x;//x
				out->bar[0][1] = y;//y
				out->bar[0][2] = bar_width;//w
				out->bar[0][3] = bar_height;   // height is half of rect and minus half of the gap between two bars
				//
				out->bar[1][0] = x;//x
				out->bar[1][1] = y + bar_height + bar_gap;
				out->bar[1][2] = bar_width;//w
				out->bar[1][3] = bar_height;//h height is same as in first bar
			}
			else if (config.style.value == 2 || config.style.value == 3) // style 2 - same start point for both bars
			{
				// all the same for same bars
				out->bar[1][0] = out->bar[0][0] = x; // x
				out->bar[1][1] = out->bar[0][1] = y; // y
				out->bar[1][2] = out->bar[0][2] = config.size.value[0]; // w
				out->bar[1][3] = out->bar[0][3] = config.size.value[1]; // h
			}
			CG_AdjustFrom640(&out->bar[1][0], &out->bar[1][1], &out->bar[1][2], &out->bar[1][3]);
			CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
			out->max = out->bar[1][2];
			out->koeff = 2 * out->bar[1][2] / max;
		}
		else
		{
			if (config.style.value == 1)    // style 1 - default: split into two bars
			{
				bar_height = config.size.value[1];
				bar_width = config.size.value[0] / 2 - bar_gap / 2;

				out->bar[0][0] = x;//x
				out->bar[0][1] = y;//y
				out->bar[0][2] = bar_width;//w
				out->bar[0][3] = bar_height;   //h
				//
				out->bar[1][0] = x + bar_width + bar_gap;
				out->bar[1][1] = y;//y
				out->bar[1][2] = bar_width;//w
				out->bar[1][3] = bar_height;//h
			}
			else if (config.style.value == 2 || config.style.value == 3)   // style 2 - same start point for both bars
			{
				// all the same for same bars
				out->bar[1][0] = out->bar[0][0] = x; // x
				out->bar[1][1] = out->bar[0][1] = y; // y
				out->bar[1][2] = out->bar[0][2] = config.size.value[0]; // w
				out->bar[1][3] = out->bar[0][3] = config.size.value[1]; // h
			}
			CG_AdjustFrom640(&out->bar[1][0], &out->bar[1][1], &out->bar[1][2], &out->bar[1][3]);
			CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
			out->max = out->bar[1][3];
			out->koeff = 2 * out->bar[1][3] / max;
		}
	}
	else
	{
		// single bar
		out->bar[0][0] = x;
		out->bar[0][1] = y;
		out->bar[0][2] = config.size.value[0];
		out->bar[0][3] = config.size.value[1];
		CG_AdjustFrom640(&out->bar[0][0], &out->bar[0][1], &out->bar[0][2], &out->bar[0][3]);
		if (out->direction == CHERRYHUD_DIR_LEFT_TO_RIGHT || out->direction == CHERRYHUD_DIR_RIGHT_TO_LEFT)
		{
			out->max = out->bar[0][2]; // max / width
		}
		else
		{
			out->max = out->bar[0][3]; // max / height
		}
		out->koeff = out->max / max;
	}

	CG_CHUDConfigPickColor(&config, out->color_top, qtrue);
	if (config.border.color.isSet)
	{
		Vector4Copy(config.border.color.rgba, out->color_back);
	}
	else
	{
		Vector4Set(out->color_back, 0, 0, 0, 0);
	}
}

qboolean CG_CHUDIsTimeOut(const cherryhudConfig_t* cfg, int startTime)
{
	if (!startTime)
	{
		return qtrue;
	}
	if (cfg->time.isSet)
	{
		if ((cg.time - startTime) < cfg->time.value)
		{
			//fade time is not started;
			return qfalse;
		}
	}
	return qtrue;
}

/*
   *  Затухание
   *  возвращает qfalse если элемент потух
   */
qboolean CG_CHUDGetFadeColor(const vec4_t from_color, vec4_t out, const cherryhudConfig_t* cfg, int startTime)
{
	int time = 0;

	Vector4Copy(from_color, out);

	if (!CG_CHUDIsTimeOut(cfg, startTime))
	{
		return qtrue;
	}

	if (cfg->time.isSet)
	{
		time = cfg->time.value;
	}

	if (cfg->fade.isSet)
	{
		int fadetime;
		float fadedelay = SUPERHUD_DEFAULT_FADEDELAY;

		fadetime = cg.time - startTime - time;

		if (cfg->fadedelay.isSet)
		{
			fadedelay = (float)cfg->fadedelay.value;
		}

		if (fadetime > 0 && fadetime < fadedelay)
		{
			vec4_t tmpfade;
			float k = (float)fadetime / fadedelay;
			Vector4Copy(cfg->fade.value, tmpfade);
			Vector4Subtract(tmpfade, from_color, tmpfade);
			Vector4MA(from_color, k, tmpfade, out);
			return qtrue;
		}
		else
		{
			return qfalse;
		}
	}

	return qfalse;
}

void CG_CHUDTextPrintNew(const cherryhudConfig_t* cfg, cherryhudTextContext_t* ctx, qboolean colorOverride)
{
	if (!ctx->text || !ctx->text[0])
	{
		return;
	}
	if (colorOverride)
		CG_CHUDConfigPickColor(cfg, ctx->color, qfalse);


	CG_FontSelect(ctx->fontIndex);
	CG_OSPDrawStringNew(ctx->pos[0],
	                    ctx->pos[1],
	                    ctx->text,
	                    ctx->color,
	                    ctx->shadowColor, // default shadow color - black
	                    ctx->size[0],
	                    ctx->size[1],
	                    ctx->width,
	                    ctx->flags,
	                    ctx->background,
	                    ctx->border,
	                    ctx->borderColor);
}

static void CG_CHUDBarPreparePrintLTR(const cherryhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	if (ctx->two_bars)
	{
		float width;

		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		Vector4Copy(ctx->bar[1], coords->bar2_value);
		Vector4Copy(ctx->bar[1], coords->bar2_bottom);
		//bar1
		width = value * ctx->koeff;
		if (width > ctx->max)
		{
			width = ctx->max;
		}
		if (width < 0)
		{
			width = 0;
		}
		coords->bar1_value[2] = width;

		//bar2
		width = value * ctx->koeff;
		width -= ctx->max;

		if (width > ctx->max)
		{
			width = ctx->max;
		}
		if (width < 0)
		{
			width = 0;
		}

		coords->bar2_value[2] = width;
	}
	else
	{
		float width;
		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		width = value * ctx->koeff;
		if (width > ctx->max)
		{
			width = ctx->max;
		}
		coords->bar1_value[2] = width;
	}
}

static void CG_CHUDBarPreparePrintRTL(const cherryhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	//Just make left-to-right and mirror it
	CG_CHUDBarPreparePrintLTR(ctx, value, coords);

	coords->bar1_value[0] = coords->bar1_value[0] + ctx->bar[0][2] - coords->bar1_value[2]; //x = x + max_width - width
	coords->bar2_value[0] = coords->bar2_value[0] + ctx->bar[1][2] - coords->bar2_value[2]; //x = x + max_width - width
}

static void CG_CHUDBarPreparePrintTTB(const cherryhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	if (ctx->two_bars)
	{
		float height;

		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		Vector4Copy(ctx->bar[1], coords->bar2_value);
		Vector4Copy(ctx->bar[1], coords->bar2_bottom);
		//bar1
		height = value * ctx->koeff;
		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}
		coords->bar1_value[3] = height;

		//bar2
		height = value * ctx->koeff;
		height -= ctx->max;

		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}

		coords->bar2_value[3] = height;
	}
	else
	{
		float height;
		Vector4Copy(ctx->bar[0], coords->bar1_value);
		Vector4Copy(ctx->bar[0], coords->bar1_bottom);
		height = value * ctx->koeff;
		if (height > ctx->max)
		{
			height = ctx->max;
		}
		if (height < 0)
		{
			height = 0;
		}
		coords->bar1_value[2] = height;
	}
}

static void CG_CHUDBarPreparePrintBTT(const cherryhudBarContext_t* ctx, float value, drawBarCoords_t* coords)
{
	//Just make top to bottom and mirror it
	CG_CHUDBarPreparePrintTTB(ctx, value, coords);

	coords->bar1_value[1] += ctx->bar[0][3] - coords->bar1_value[3]; //y = y + max_height - height
	coords->bar2_value[1] += ctx->bar[1][3] - coords->bar2_value[3]; //y = y + max_height - height
}

void CG_CHUDBarPrint(const cherryhudConfig_t* cfg, cherryhudBarContext_t* ctx, float value)
{
	drawBarCoords_t coords;

	memset(&coords, 0, sizeof(coords));

	switch (ctx->direction)
	{
		default:
		case CHERRYHUD_DIR_LEFT_TO_RIGHT:
			CG_CHUDBarPreparePrintLTR(ctx, value, &coords);
			break;
		case CHERRYHUD_DIR_RIGHT_TO_LEFT:
			CG_CHUDBarPreparePrintRTL(ctx, value, &coords);
			break;
		case CHERRYHUD_DIR_TOP_TO_BOTTOM:
			CG_CHUDBarPreparePrintTTB(ctx, value, &coords);
			break;
		case CHERRYHUD_DIR_BOTTOM_TO_TOP:
			CG_CHUDBarPreparePrintBTT(ctx, value, &coords);
			break;
	}

	trap_R_SetColor(ctx->color_back);
	trap_R_DrawStretchPic(coords.bar1_bottom[0], coords.bar1_bottom[1], coords.bar1_bottom[2], coords.bar1_bottom[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		trap_R_DrawStretchPic(coords.bar2_bottom[0], coords.bar2_bottom[1], coords.bar2_bottom[2], coords.bar2_bottom[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(ctx->color_top);
	trap_R_DrawStretchPic(coords.bar1_value[0], coords.bar1_value[1], coords.bar1_value[2], coords.bar1_value[3],
	                      0, 0, 0, 0,
	                      cgs.media.whiteShader);
	if (ctx->two_bars)
	{
		if (cfg->style.value == 2 || cfg->style.value == 3)
		{
			trap_R_SetColor(ctx->color2_top); // 2nd bar color
		}

		trap_R_DrawStretchPic(coords.bar2_value[0], coords.bar2_value[1], coords.bar2_value[2], coords.bar2_value[3],
		                      0, 0, 0, 0,
		                      cgs.media.whiteShader);
	}
	trap_R_SetColor(NULL);
}

team_t CG_CHUDGetOurActiveTeam(void)
{
	const qboolean is_freeze = CG_OSPIsGameTypeFreeze();

	team_t our_team = TEAM_FREE;
	if (is_freeze)
	{
		our_team = cgs.clientinfo[cg.clientNum].rt;
	}
	else
	{
		our_team = cgs.clientinfo[cg.clientNum].team;
	}

	if (our_team == TEAM_SPECTATOR)
	{
		if (is_freeze)
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].rt;
		}
		else
		{
			our_team = cgs.clientinfo[cg.snap->ps.clientNum].team;
		}
	}
	return our_team;
}


qboolean CG_CHUDFill(const cherryhudConfig_t* cfg)
{
	float x, y, w, h;
	vec4_t bgColor;
	if (!cfg->fill.isSet || !cfg->pos.isSet || !cfg->size.isSet)
	{
		return qfalse;
	}

	x = cfg->pos.value[0];
	y = cfg->pos.value[1];
	w = cfg->size.value[0];
	h = cfg->size.value[1];

	CG_AdjustFrom640(&x, &y, &w, &h);

	if (cfg->background.color.isSet)
	{
		CG_CHUDConfigPickBgColor(cfg, bgColor, qfalse);

		trap_R_SetColor(bgColor);
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
		trap_R_SetColor(NULL);
		return qtrue;
	}

	if (cfg->image.isSet)
	{
		vec4_t color;
		qhandle_t image = trap_R_RegisterShader(cfg->image.value);
		if (!image)
		{
			return qfalse;
		}

		CG_CHUDConfigPickColor(cfg, color, qfalse);

		trap_R_SetColor(color);
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, image);
		trap_R_SetColor(NULL);
		return qtrue;
	}

	return qfalse;
}


void CG_CHUDFillDirect(float x, float y, float w, float h, float* color) {
	cherryhudConfig_t* cfg = NULL;
	CG_CHUDConfigPickColor(cfg, color, qfalse);
	trap_R_SetColor(color);
	trap_R_DrawStretchPic(x, y, w, h, 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

void CG_CHUDDrawStretchPic(float x, float y, float w, float h, 
                          float picX, float picY, float picW, float picH, 
                          const float* color, qhandle_t shader)
{
	if (!shader) return;

	trap_R_SetColor(color);
	CG_AdjustFrom640(&x, &y, &w, &h);
	trap_R_DrawStretchPic(x, y, w, h, picX, picY, picW, picH, shader);
	trap_R_SetColor(NULL);
}

// Centralized border drawing function with negative/positive size logic
void CG_CHUDDrawBorderCentralized(float x, float y, float w, float h, const vec4_t borderSize, const vec4_t borderColor)
{
	vec4_t adjustedBorderSize;
	vec4_t adjCoords;
	qboolean isInner;
	int i;
	
	if (!borderSize || !borderColor) {
		return;
	}
	
	// Check if any border size is non-zero
	if (borderSize[0] == 0.0f && borderSize[1] == 0.0f && borderSize[2] == 0.0f && borderSize[3] == 0.0f) {
		return;
	}
	adjCoords[0] = x;
	adjCoords[1] = y;
	adjCoords[2] = w;
	adjCoords[3] = h;
	CG_AdjustFrom640(&adjCoords[0], &adjCoords[1], &adjCoords[2], &adjCoords[3]);


	// Process border sizes: negative = inner, positive = outer
	Vector4Copy(borderSize, adjustedBorderSize);
	isInner = qfalse;
	
	for (i = 0; i < 4; i++) {
		if (adjustedBorderSize[i] < 0.0f) {
			// Negative value = inner border
			adjustedBorderSize[i] = -adjustedBorderSize[i]; // Make positive for drawing
			isInner = qtrue;
		} else if (adjustedBorderSize[i] > 0.0f) {
			// Positive value = outer border
			isInner = qfalse;
		}
	}
	
	// Draw the border (coordinates are already in correct scale)
	CG_OSPDrawFrame(adjCoords[0], adjCoords[1], adjCoords[2], adjCoords[3], adjustedBorderSize, (float*)borderColor, isInner);
}


// Centralized background fill function
void CG_CHUDFillCentralized(float x, float y, float w, float h, const float* bgColor)
{
	vec4_t adjCoords;
	if (!bgColor) {
		return;
	}
	adjCoords[0] = x;
	adjCoords[1] = y;
	adjCoords[2] = w;
	adjCoords[3] = h;
	CG_AdjustFrom640(&adjCoords[0], &adjCoords[1], &adjCoords[2], &adjCoords[3]);
	trap_R_SetColor(bgColor);
	trap_R_DrawStretchPic(adjCoords[0], adjCoords[1], adjCoords[2], adjCoords[3], 0, 0, 0, 0, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

qboolean CG_CHUDDrawBorder(const cherryhudConfig_t* cfg)
{
	vec4_t borderColor;
	vec4_t adjCoords;
	if (!cfg->border.isSet || !cfg->pos.isSet || !cfg->size.isSet || !cfg->border.color.isSet)
	{
		return qfalse;
	}

	adjCoords[0] = cfg->pos.value[0];
	adjCoords[1] = cfg->pos.value[1];
	adjCoords[2] = cfg->size.value[0];
	adjCoords[3] = cfg->size.value[1];
	// CG_AdjustFrom640(&adjCoords[0], &adjCoords[1], &adjCoords[2], &adjCoords[3]);

	CG_CHUDConfigPickBorderColor(cfg, borderColor, qfalse);

	CG_CHUDDrawBorderCentralized(adjCoords[0], adjCoords[1], adjCoords[2], adjCoords[3],
	                             cfg->border.sideSizes, borderColor);

	return qtrue;
}

void CG_CHUDFillAndFrameForText(cherryhudConfig_t* cfg, cherryhudTextContext_t* ctx)
{
	qboolean drawBackground;
	qboolean drawBorder;

	drawBackground = CG_CHUDConfigHasTextBackground(cfg);
	drawBorder = CG_CHUDConfigHasTextBorder(cfg);

	if (!drawBackground && !drawBorder)
	{
		return;
	}

	if (drawBackground)
	{
		CG_CHUDConfigPickTextBgColor(cfg, ctx->background, qfalse);
	}

	if (drawBorder)
	{
		// Use only text-specific border
		if (cfg->textExt.border.isSet) {
			Vector4Copy(cfg->textExt.border.sideSizes, ctx->border);
		}
		CG_CHUDConfigPickTextBorderColor(cfg, ctx->borderColor, qfalse);
	}
}

void CG_CHUDDrawStretchPicCtx(const cherryhudConfig_t* cfg, cherryhudDrawContext_t* ctx)
{
	// Draw background and border first (if configured)
	CG_CHUDFillAndFrameForImage(cfg, ctx);
	
	// we have to pick color again, because team could changed
	CG_CHUDConfigPickColor(cfg, ctx->color, qfalse);
	
	CG_CHUDDrawStretchPic(ctx->pos[0], ctx->pos[1], ctx->size[0], ctx->size[1],
	                     ctx->picPos[0], ctx->picPos[1], ctx->picSize[0], ctx->picSize[1],
	                     ctx->color, ctx->image);
}

void CG_CHUDFillAndFrameForImage(const cherryhudConfig_t* cfg, cherryhudDrawContext_t* ctx)
{
	qboolean drawBackground;
	qboolean drawBorder;
	vec4_t bgColor;
	vec4_t borderColor;
	
	drawBackground = CG_CHUDConfigHasImageBackground(cfg);
	drawBorder = CG_CHUDConfigHasImageBorder(cfg);
	
	if (!drawBackground && !drawBorder)
	{
		return;
	}
	
	if (drawBackground)
	{
		CG_CHUDConfigPickImageBgColor(cfg, bgColor, qfalse);
		CG_CHUDFillCentralized(ctx->pos[0], ctx->pos[1], ctx->size[0], ctx->size[1], bgColor);
	}
	
	if (drawBorder)
	{
		CG_CHUDConfigPickImageBorderColor(cfg, borderColor, qfalse);
		// Use image-specific border if available, otherwise fallback to global
		if (cfg->imageBorder.isSet) {
			CG_CHUDDrawBorderCentralized(ctx->pos[0], ctx->pos[1], ctx->size[0], ctx->size[1], 
			                            cfg->imageBorder.sideSizes, borderColor);
		} else {
			CG_CHUDDrawBorderCentralized(ctx->pos[0], ctx->pos[1], ctx->size[0], ctx->size[1], 
			                            cfg->border.sideSizes, borderColor);
		}
	}
}


void CG_CHUDConfigPickTextBgColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	if (config->textExt.background.isSet)
	{
		CG_CHUDConfigPickColorGeneric(&config->textExt.background.color.isSet, &config->textExt.background.color,
		                              &config->textExt.background.color2.isSet, &config->textExt.background.color2,
		                              color, alphaOverride);
	}
	else
	{
		// No fallback - use only text-specific background
		Vector4Set(color, 0.0f, 0.0f, 0.0f, 0.0f); // No background by default
	}
}

void CG_CHUDConfigPickTextBorderColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	if (config->textExt.bordercolor.isSet)
	{
		CG_CHUDConfigPickColorGeneric(&config->textExt.bordercolor.isSet, &config->textExt.bordercolor.value,
		                              &config->textExt.bordercolor.isSet, &config->textExt.bordercolor.value,
		                              color, alphaOverride);
	}
	else
	{
		// No fallback - use only text-specific border color
		Vector4Set(color, 0.0f, 0.0f, 0.0f, 0.0f); // No border by default
	}
}

qboolean CG_CHUDConfigHasTextBackground(const cherryhudConfig_t* config)
{
	return config->textExt.background.isSet;
}

qboolean CG_CHUDConfigHasTextBorder(const cherryhudConfig_t* config)
{
	return config->textExt.border.isSet;
}

void CG_CHUDConfigPickImageBgColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	if (config->imageBgColor.isSet)
	{
		CG_CHUDConfigPickColorGeneric(&config->imageBgColor.isSet, &config->imageBgColor.value,
		                              &config->imageBgColor.isSet, &config->imageBgColor.value,
		                              color, alphaOverride);
	}
	else
	{
		// Fallback to global background color
		CG_CHUDConfigPickBgColor(config, color, alphaOverride);
	}
}

void CG_CHUDConfigPickImageBorderColor(const cherryhudConfig_t* config, float* color, qboolean alphaOverride)
{
	if (config->imageBorderColor.isSet)
	{
		CG_CHUDConfigPickColorGeneric(&config->imageBorderColor.isSet, &config->imageBorderColor.value,
		                              &config->imageBorderColor.isSet, &config->imageBorderColor.value,
		                              color, alphaOverride);
	}
	else
	{
		// Fallback to global border color
		CG_CHUDConfigPickBorderColor(config, color, alphaOverride);
	}
}

qboolean CG_CHUDConfigHasImageBackground(const cherryhudConfig_t* config)
{
	return config->imageBgColor.isSet || config->background.color.isSet;
}

qboolean CG_CHUDConfigHasImageBorder(const cherryhudConfig_t* config)
{
	return (config->imageBorder.isSet && config->imageBorder.color.isSet) ||
	       (config->border.isSet && config->border.color.isSet);
}

void CG_CHUDRenderContainerBackground(const cherryhudConfig_t* config, float x, float y, float w, float h)
{
	float bgColor[4];
	
	CG_CHUDConfigPickBgColor(config, bgColor, qfalse);
	CG_CHUDFillCentralized(x, y, w, h, bgColor);
}

void CG_CHUDRenderContainerBackgroundWithHeight(const cherryhudConfig_t* config, float x, float y, float w, float h, float currentHeight)
{
	float bgColor[4];
	
	CG_CHUDGetActiveBgColor(config, currentHeight, bgColor);
	CG_CHUDFillCentralized(x, y, w, h, bgColor);
}

void CG_CHUDRenderContainerBackgroundFromBounds(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds)
{
	float bgColor[4];
	
	CG_CHUDConfigPickBgColor(config, bgColor, qfalse);
	CG_CHUDFillCentralized(bounds->x, bounds->y, bounds->width, bounds->height, bgColor);
}

void CG_CHUDRenderContainerBackgroundFromBoundsWithHeight(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds, float currentHeight)
{
	float bgColor[4];
	
	CG_CHUDGetActiveBgColor(config, currentHeight, bgColor);
	CG_CHUDFillCentralized(bounds->x, bounds->y, bounds->width, bounds->height, bgColor);
}

// ============================================================================
// UNIFIED RENDERING FUNCTIONS
// ============================================================================

// Universal function to render background only
qboolean CG_CHUDRenderBackground(const cherryhudConfig_t* config, float x, float y, float w, float h)
{
	vec4_t bgColor;
	
	if (!config || !config->background.color.isSet) {
		return qfalse;
	}
	
	CG_CHUDConfigPickBgColor(config, bgColor, qfalse);
	CG_CHUDFillCentralized(x, y, w, h, bgColor);
	return qtrue;
}

// Universal function to render border only
qboolean CG_CHUDRenderBorder(const cherryhudConfig_t* config, float x, float y, float w, float h)
{
	vec4_t borderColor;
	
	if (!config || !config->border.isSet || !config->border.color.isSet) {
		return qfalse;
	}
	
	CG_CHUDConfigPickBorderColor(config, borderColor, qfalse);
	CG_CHUDDrawBorderCentralized(x, y, w, h, config->border.sideSizes, borderColor);
	return qtrue;
}

// Universal function to render both background and border
void CG_CHUDRenderBackgroundAndBorder(const cherryhudConfig_t* config, float x, float y, float w, float h)
{
	if (!config) return;
	
	// Render background first
	CG_CHUDRenderBackground(config, x, y, w, h);
	
	// Then render border on top
	CG_CHUDRenderBorder(config, x, y, w, h);
}

// ============================================================================
// ALIGNMENT HELPER FUNCTIONS
// ============================================================================

// Apply horizontal alignment to X position
float CG_CHUDApplyAlignH(float x, float width, cherryhudAlignH_t alignH)
{
	switch (alignH) {
		case CHERRYHUD_ALIGNH_LEFT:
			return x;
		case CHERRYHUD_ALIGNH_CENTER:
			return x - width / 2.0f;
		case CHERRYHUD_ALIGNH_RIGHT:
			return x - width;
		default:
			return x;
	}
}

// Apply vertical alignment to Y position
float CG_CHUDApplyAlignV(float y, float height, cherryhudAlignV_t alignV)
{
	switch (alignV) {
		case CHERRYHUD_ALIGNV_TOP:
			return y;
		case CHERRYHUD_ALIGNV_CENTER:
			return y - height / 2.0f;
		case CHERRYHUD_ALIGNV_BOTTOM:
			return y - height;
		default:
			return y;
	}
}

// Apply both horizontal and vertical alignment to position
void CG_CHUDApplyAlignment(float* x, float* y, float width, float height, 
                          cherryhudAlignH_t alignH, cherryhudAlignV_t alignV)
{
	if (x) *x = CG_CHUDApplyAlignH(*x, width, alignH);
	if (y) *y = CG_CHUDApplyAlignV(*y, height, alignV);
}

// Get alignment values from config with defaults
void CG_CHUDGetAlignmentFromConfig(const cherryhudConfig_t* config, 
                                  cherryhudAlignH_t* alignH, cherryhudAlignV_t* alignV)
{
	if (!config) return;
	
	if (alignH) {
		*alignH = config->alignH.isSet ? config->alignH.value : CHERRYHUD_ALIGNH_LEFT;
	}
	if (alignV) {
		*alignV = config->alignV.isSet ? config->alignV.value : CHERRYHUD_ALIGNV_TOP;
	}
}

// Get textExt alignment values from config with defaults
void CG_CHUDGetTextExtAlignmentFromConfig(const cherryhudConfig_t* config, 
                                         cherryhudAlignH_t* alignH, cherryhudAlignV_t* alignV)
{
	if (!config) return;
	
	if (alignH) {
		*alignH = config->textExt.alignH.isSet ? config->textExt.alignH.value : CHERRYHUD_ALIGNH_LEFT;
	}
	if (alignV) {
		*alignV = config->textExt.alignV.isSet ? config->textExt.alignV.value : CHERRYHUD_ALIGNV_TOP;
	}
}

// Apply alignment to text context
void CG_CHUDApplyAlignmentToTextContext(cherryhudTextContext_t* textCtx, 
                                       cherryhudAlignH_t alignH, cherryhudAlignV_t alignV)
{
	if (!textCtx) return;
	
	textCtx->pos[0] = CG_CHUDApplyAlignH(textCtx->pos[0], textCtx->width, alignH);
	textCtx->pos[1] = CG_CHUDApplyAlignV(textCtx->pos[1], textCtx->width, alignV);
}

// Apply alignment to draw context
void CG_CHUDApplyAlignmentToDrawContext(cherryhudDrawContext_t* drawCtx, 
                                       cherryhudAlignH_t alignH, cherryhudAlignV_t alignV)
{
	if (!drawCtx) return;
	
	drawCtx->pos[0] = CG_CHUDApplyAlignH(drawCtx->pos[0], drawCtx->size[0], alignH);
	drawCtx->pos[1] = CG_CHUDApplyAlignV(drawCtx->pos[1], drawCtx->size[1], alignV);
}

// Apply alignment to layout bounds
void CG_CHUDApplyAlignmentToBounds(cherryhudLayoutBounds_t* bounds, 
                                  cherryhudAlignH_t alignH, cherryhudAlignV_t alignV)
{
	if (!bounds) return;
	
	bounds->x = CG_CHUDApplyAlignH(bounds->x, bounds->width, alignH);
	bounds->y = CG_CHUDApplyAlignV(bounds->y, bounds->height, alignV);
}

// Apply alignment from config to layout bounds
void CG_CHUDApplyConfigAlignmentToBounds(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds)
{
	cherryhudAlignH_t alignH;
	cherryhudAlignV_t alignV;
	
	if (!config || !bounds) return;
	
	CG_CHUDGetAlignmentFromConfig(config, &alignH, &alignV);
	CG_CHUDApplyAlignmentToBounds(bounds, alignH, alignV);
}

// Apply textExt alignment from config to layout bounds
void CG_CHUDApplyTextExtAlignmentToBounds(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds)
{
	cherryhudAlignH_t alignH;
	cherryhudAlignV_t alignV;
	
	if (!config || !bounds) return;
	
	CG_CHUDGetTextExtAlignmentFromConfig(config, &alignH, &alignV);
	CG_CHUDApplyAlignmentToBounds(bounds, alignH, alignV);
}

// Format number according to format string
// Supports formats like: .2% (2 decimal places + %), .0X (integer + X), .1f (1 decimal place)
void CG_CHUDFormatNumber(float value, const char* format, char* output, int maxlen)
{
	int decimalPlaces;
	char suffix[MAX_QPATH];
	char formatStr[32];
	int i;
	int len;
	qboolean hasDecimal;
	qboolean hasSuffix;
	
	if (!format || !output || maxlen <= 0) {
		if (output && maxlen > 0) {
			output[0] = '\0';
		}
		return;
	}
	
	// Initialize
	decimalPlaces = 0;
	suffix[0] = '\0';
	hasDecimal = qfalse;
	hasSuffix = qfalse;
	
	// Parse format string
	len = strlen(format);
	for (i = 0; i < len; i++) {
		if (format[i] == '.') {
			hasDecimal = qtrue;
			// Look for number after decimal point
			if (i + 1 < len && format[i + 1] >= '0' && format[i + 1] <= '9') {
				decimalPlaces = format[i + 1] - '0';
				i++; // Skip the digit
			}
		} else if (format[i] == '%' || format[i] == 'X' || format[i] == 'f') {
			// Found suffix
			suffix[0] = format[i];
			suffix[1] = '\0';
			hasSuffix = qtrue;
		} else if (format[i] >= 'A' && format[i] <= 'Z') {
			// Custom suffix (single character)
			suffix[0] = format[i];
			suffix[1] = '\0';
			hasSuffix = qtrue;
		}
	}
	
	// Build format string for sprintf
	if (hasDecimal) {
		if (hasSuffix && suffix[0] == '%') {
			// Special case for percentage - multiply by 100
			Com_sprintf(formatStr, sizeof(formatStr), "%%.%df%%%%", decimalPlaces);
			Com_sprintf(output, maxlen, formatStr, value);
		} else if (hasSuffix && suffix[0] == 'f') {
			// Float format
			Com_sprintf(formatStr, sizeof(formatStr), "%%.%df", decimalPlaces);
			Com_sprintf(output, maxlen, formatStr, value);
		} else if (hasSuffix) {
			// Integer format with suffix
			Com_sprintf(formatStr, sizeof(formatStr), "%%d%s", suffix);
			Com_sprintf(output, maxlen, formatStr, (int)value);
		} else {
			// Decimal format without suffix - use float
			Com_sprintf(formatStr, sizeof(formatStr), "%%.%df", decimalPlaces);
			Com_sprintf(output, maxlen, formatStr, value);
		}
	} else {
		// No decimal specified - default to integer
		if (hasSuffix) {
			Com_sprintf(formatStr, sizeof(formatStr), "%%d%s", suffix);
			Com_sprintf(output, maxlen, formatStr, (int)value);
		} else {
			Com_sprintf(output, maxlen, "%d", (int)value);
		}
	}
}

// Centralized visibility checking function (similar to SuperHUD)
// Common validation utilities
qboolean CG_CHUDValidateElementAndConfig(void* element, const cherryhudConfig_t* config) {
    return (element != NULL && config != NULL);
}

qboolean CG_CHUDValidateClientNumber(int clientNum) {
    return (clientNum >= 0 && clientNum < MAX_CLIENTS);
}

qboolean CG_CHUDValidateBounds(cherryhudLayoutBounds_t* bounds) {
    return (bounds != NULL && bounds->isValid);
}

// Common rendering utilities
void CG_CHUDRenderElementWithValidation(void* element, const cherryhudConfig_t* config, 
                                       cherryhudLayoutBounds_t* bounds, int clientNum) {
    // Validate inputs
    if (!CG_CHUDValidateElementAndConfig(element, config) || 
        !CG_CHUDValidateBounds(bounds) || 
        !CG_CHUDValidateClientNumber(clientNum)) {
        return;
    }
    
    // Check visibility
    if (!CG_CHUDCheckElementVisibility(config, clientNum, (cherryhudElement_t*)element)) {
        return;
    }
    
    // Check hide flags
    if (CG_CHUDCheckElementHideFlags(config, clientNum, (cherryhudElement_t*)element)) {
        return;
    }
}

void CG_CHUDRenderElementBackgroundAndBorder(const cherryhudConfig_t* config, cherryhudLayoutBounds_t* bounds) {
    if (!config || !bounds) return;
    
    // Render background
    if (config->background.color.isSet) {
        CG_CHUDRenderContainerBackgroundFromBounds(config, bounds);
    }
    
    // Render border
    if (config->border.isSet) {
        vec4_t borderColor;
        CG_CHUDConfigPickBorderColor(config, borderColor, qfalse);
        CG_CHUDDrawBorderCentralized(bounds->x, bounds->y, bounds->width, bounds->height,
                                     config->border.sideSizes, borderColor);
    }
}

qboolean CG_CHUDCheckElementVisibility(const cherryhudConfig_t* config, int clientNum, cherryhudElement_t* element)
{
    int vflags;
    qboolean skip;
    qboolean is_intermission;
    qboolean is_team_game;
    qboolean is_dead;
    qboolean is_spectator;
    qboolean is_scores;
    qboolean is_gt_ffa;
    qboolean is_gt_tdm;
    qboolean is_gt_ctf;
    qboolean is_gt_tourney;
    qboolean is_gt_ft;
    qboolean is_gt_ca;
    qboolean anyFlagActive;
    qboolean allFlagsActive;
    
    if (!config) {
        return qtrue; // No config, show by default
    }
    
    // Get visflags from config
    vflags = config->visflags.isSet ? config->visflags.flags : 0;
    
    // For scoreboard (element == NULL), always add chudscoreboard flag
    if (element == NULL) {
        vflags |= SE_CHUDSCOREBOARD_SHOW;
    }
    
    // If no visflags set, show by default
    if (vflags == 0) {
        return qtrue;
    }
    
    // Determine game state (similar to SuperHUD)
    is_intermission = (cg.intermissionStarted != 0);
    is_team_game = (cgs.gametype >= GT_TEAM);
    is_dead = (cg.snap && cg.snap->ps.pm_type == PM_DEAD);
    is_spectator = (cg.snap && cg.snap->ps.pm_type == PM_SPECTATOR);
    is_scores = (cg.showScores != 0);
    is_gt_ffa = (cgs.gametype == GT_FFA);
    is_gt_tdm = (cgs.gametype == GT_TEAM);
    is_gt_ctf = (cgs.gametype == GT_CTF);
    is_gt_tourney = (cgs.gametype == GT_TOURNAMENT);
    is_gt_ft = (cgs.gametype == GT_TEAM && cgs.osp.gameTypeFreeze);
    is_gt_ca = (cgs.gametype == GT_CA);
    
    // Check which flags are currently active
    anyFlagActive = ((vflags & SE_IM) && is_intermission) ||
                    ((vflags & SE_TEAM_ONLY) && is_team_game) ||
                    ((vflags & SE_DEAD) && is_dead) ||
                    ((vflags & SE_SPECT) && is_spectator) ||
                    ((vflags & SE_SCORES_HIDE) && is_scores) ||
                    ((vflags & SE_DEMO_HIDE) && cg.demoPlayback) ||
                    ((vflags & SE_KEY1_SHOW) && cgs.osp.chud.key[0]) ||
                    ((vflags & SE_KEY2_SHOW) && cgs.osp.chud.key[1]) ||
                    ((vflags & SE_KEY3_SHOW) && cgs.osp.chud.key[2]) ||
                    ((vflags & SE_KEY4_SHOW) && cgs.osp.chud.key[3]) ||
                    ((vflags & SE_CHUDSCOREBOARD_SHOW) && cgs.osp.chud.scoreboard) ||
                    ((vflags & SE_GT_FFA) && is_gt_ffa) ||
                    ((vflags & SE_GT_TDM) && is_gt_tdm) ||
                    ((vflags & SE_GT_CTF) && is_gt_ctf) ||
                    ((vflags & SE_GT_TOURNEY) && is_gt_tourney) ||
                    ((vflags & SE_GT_FREEZETAG) && is_gt_ft) ||
                    ((vflags & SE_GT_CLANARENA) && is_gt_ca) ||
                    ((vflags & SE_LOCAL_CLIENT) && (clientNum == cg.clientNum)) ||
                    ((vflags & SE_ACTIVE_PLAYER) && (clientNum == cg.snap->ps.clientNum)) ||
                    ((vflags & SE_READY) && (cg.warmup && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)))) ||
                    ((vflags & SE_FROZEN) && (!cg.warmup && CG_OSPIsGameTypeFreeze() && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum))));
    
    // Check if ALL flags are active (for AND operation)
    // Only check flags that are actually set in the visflags
    allFlagsActive = qtrue;
    if (vflags & SE_IM) allFlagsActive = allFlagsActive && is_intermission;
    if (vflags & SE_TEAM_ONLY) allFlagsActive = allFlagsActive && is_team_game;
    if (vflags & SE_DEAD) allFlagsActive = allFlagsActive && is_dead;
    if (vflags & SE_SPECT) allFlagsActive = allFlagsActive && is_spectator;
    if (vflags & SE_SCORES_HIDE) allFlagsActive = allFlagsActive && is_scores;
    if (vflags & SE_DEMO_HIDE) allFlagsActive = allFlagsActive && cg.demoPlayback;
    if (vflags & SE_KEY1_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[0];
    if (vflags & SE_KEY2_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[1];
    if (vflags & SE_KEY3_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[2];
    if (vflags & SE_KEY4_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[3];
    if (vflags & SE_CHUDSCOREBOARD_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.scoreboard;
    if (vflags & SE_GT_FFA) allFlagsActive = allFlagsActive && is_gt_ffa;
    if (vflags & SE_GT_TDM) allFlagsActive = allFlagsActive && is_gt_tdm;
    if (vflags & SE_GT_CTF) allFlagsActive = allFlagsActive && is_gt_ctf;
    if (vflags & SE_GT_TOURNEY) allFlagsActive = allFlagsActive && is_gt_tourney;
    if (vflags & SE_GT_FREEZETAG) allFlagsActive = allFlagsActive && is_gt_ft;
    if (vflags & SE_GT_CLANARENA) allFlagsActive = allFlagsActive && is_gt_ca;
    if (vflags & SE_LOCAL_CLIENT) allFlagsActive = allFlagsActive && (clientNum == cg.clientNum);
    if (vflags & SE_ACTIVE_PLAYER) allFlagsActive = allFlagsActive && (clientNum == cg.snap->ps.clientNum);
    if (vflags & SE_READY) allFlagsActive = allFlagsActive && (cg.warmup && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)));
    if (vflags & SE_FROZEN) allFlagsActive = allFlagsActive && (!cg.warmup && CG_OSPIsGameTypeFreeze() && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)));
    
    // Determine if we should show based on operation type
    if (config->visflags.op == CHERRYHUD_VISFLAGS_OP_AND) {
        skip = !allFlagsActive; // Show only if ALL flags are active
    } else {
        skip = !anyFlagActive;  // Show if ANY flag is active (OR operation)
    }
    
    // Check container mode flags
    if (!skip) {
        int elementMode = CG_CHUDGetElementContainerMode(element);
        
        // Check compact mode flag
        if ((vflags & SE_COMPACT_MODE) && elementMode != 1) {
            skip = qtrue; // Don't show if compact mode required but not active
        }
        
        // Check double mode flag
        if ((vflags & SE_DOUBLE_MODE) && elementMode != 2) {
            skip = qtrue; // Don't show if double mode required but not active
        }
    }
    
    return !skip; // Return true if not skipped (should be visible)
}


// Centralized hideflags checking function (opposite logic to visflags)
qboolean CG_CHUDCheckElementHideFlags(const cherryhudConfig_t* config, int clientNum, cherryhudElement_t* element)
{
    int hflags;
    qboolean hide;
    qboolean is_intermission;
    qboolean is_team_game;
    qboolean is_dead;
    qboolean is_spectator;
    qboolean is_scores;
    qboolean is_gt_ffa;
    qboolean is_gt_tdm;
    qboolean is_gt_ctf;
    qboolean is_gt_tourney;
    qboolean is_gt_ft;
    qboolean is_gt_ca;
    qboolean anyFlagActive;
    qboolean allFlagsActive;
    
    if (!config) {
        return qfalse; // No config, don't hide by default
    }
    
    // Get hideflags from config
    hflags = config->hideflags.isSet ? config->hideflags.flags : 0;
    
    // If no hideflags set, don't hide by default
    if (hflags == 0) {
        return qfalse;
    }
    
    // Determine game state (same as visflags)
    is_intermission = (cg.intermissionStarted != 0);
    is_team_game = (cgs.gametype >= GT_TEAM);
    is_dead = (cg.snap && cg.snap->ps.pm_type == PM_DEAD);
    is_spectator = (cg.snap && cg.snap->ps.pm_type == PM_SPECTATOR);
    is_scores = (cg.showScores != 0);
    is_gt_ffa = (cgs.gametype == GT_FFA);
    is_gt_tdm = (cgs.gametype == GT_TEAM);
    is_gt_ctf = (cgs.gametype == GT_CTF);
    is_gt_tourney = (cgs.gametype == GT_TOURNAMENT);
    is_gt_ft = (cgs.gametype == GT_TEAM && cgs.osp.gameTypeFreeze);
    is_gt_ca = (cgs.gametype == GT_CA);
    
    // Check which flags are currently active (same logic as visflags)
    anyFlagActive = ((hflags & SE_IM) && is_intermission) ||
                    ((hflags & SE_TEAM_ONLY) && is_team_game) ||
                    ((hflags & SE_DEAD) && is_dead) ||
                    ((hflags & SE_SPECT) && is_spectator) ||
                    ((hflags & SE_SCORES_HIDE) && is_scores) ||
                    ((hflags & SE_DEMO_HIDE) && cg.demoPlayback) ||
                    ((hflags & SE_KEY1_SHOW) && cgs.osp.chud.key[0]) ||
                    ((hflags & SE_KEY2_SHOW) && cgs.osp.chud.key[1]) ||
                    ((hflags & SE_KEY3_SHOW) && cgs.osp.chud.key[2]) ||
                    ((hflags & SE_KEY4_SHOW) && cgs.osp.chud.key[3]) ||
                    ((hflags & SE_CHUDSCOREBOARD_SHOW) && cgs.osp.chud.scoreboard) ||
                    ((hflags & SE_GT_FFA) && is_gt_ffa) ||
                    ((hflags & SE_GT_TDM) && is_gt_tdm) ||
                    ((hflags & SE_GT_CTF) && is_gt_ctf) ||
                    ((hflags & SE_GT_TOURNEY) && is_gt_tourney) ||
                    ((hflags & SE_GT_FREEZETAG) && is_gt_ft) ||
                    ((hflags & SE_GT_CLANARENA) && is_gt_ca) ||
                    ((hflags & SE_LOCAL_CLIENT) && (clientNum == cg.clientNum)) ||
                    ((hflags & SE_ACTIVE_PLAYER) && (clientNum == cg.snap->ps.clientNum)) ||
                    ((hflags & SE_READY) && (cg.warmup && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)))) ||
                    ((hflags & SE_FROZEN) && (!cg.warmup && CG_OSPIsGameTypeFreeze() && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum))));
    
    // Check if ALL flags are active (for AND operation)
    allFlagsActive = qtrue;
    if (hflags & SE_IM) allFlagsActive = allFlagsActive && is_intermission;
    if (hflags & SE_TEAM_ONLY) allFlagsActive = allFlagsActive && is_team_game;
    if (hflags & SE_DEAD) allFlagsActive = allFlagsActive && is_dead;
    if (hflags & SE_SPECT) allFlagsActive = allFlagsActive && is_spectator;
    if (hflags & SE_SCORES_HIDE) allFlagsActive = allFlagsActive && is_scores;
    if (hflags & SE_DEMO_HIDE) allFlagsActive = allFlagsActive && cg.demoPlayback;
    if (hflags & SE_KEY1_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[0];
    if (hflags & SE_KEY2_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[1];
    if (hflags & SE_KEY3_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[2];
    if (hflags & SE_KEY4_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.key[3];
    if (hflags & SE_CHUDSCOREBOARD_SHOW) allFlagsActive = allFlagsActive && cgs.osp.chud.scoreboard;
    if (hflags & SE_GT_FFA) allFlagsActive = allFlagsActive && is_gt_ffa;
    if (hflags & SE_GT_TDM) allFlagsActive = allFlagsActive && is_gt_tdm;
    if (hflags & SE_GT_CTF) allFlagsActive = allFlagsActive && is_gt_ctf;
    if (hflags & SE_GT_TOURNEY) allFlagsActive = allFlagsActive && is_gt_tourney;
    if (hflags & SE_GT_FREEZETAG) allFlagsActive = allFlagsActive && is_gt_ft;
    if (hflags & SE_GT_CLANARENA) allFlagsActive = allFlagsActive && is_gt_ca;
    if (hflags & SE_LOCAL_CLIENT) allFlagsActive = allFlagsActive && (clientNum == cg.clientNum);
    if (hflags & SE_ACTIVE_PLAYER) allFlagsActive = allFlagsActive && (clientNum == cg.snap->ps.clientNum);
    if (hflags & SE_READY) allFlagsActive = allFlagsActive && (cg.warmup && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)));
    if (hflags & SE_FROZEN) allFlagsActive = allFlagsActive && (!cg.warmup && CG_OSPIsGameTypeFreeze() && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)));
    
    // Determine if we should hide based on operation type
    if (config->hideflags.op == CHERRYHUD_VISFLAGS_OP_AND) {
        hide = allFlagsActive; // Hide if ALL flags are active
    } else {
        hide = anyFlagActive;  // Hide if ANY flag is active (OR operation)
    }
    
    // Check container mode flags
    if (!hide) {
        int elementMode = CG_CHUDGetElementContainerMode(element);
        
        // Check compact mode flag
        if ((hflags & SE_COMPACT_MODE) && elementMode == 1) {
            hide = qtrue; // Hide if compact mode is active
        }
        
        // Check double mode flag
        if ((hflags & SE_DOUBLE_MODE) && elementMode == 2) {
            hide = qtrue; // Hide if double mode is active
        }
    }
    
    return hide; // Return true if should be hidden
}

// Old function removed - using new architecture

// Legacy functions for backward compatibility
qboolean CG_CHUDCheckLocalClientVisibility(const cherryhudConfig_t* config, int clientNum)
{
    if (!config || !config->visflags.isSet) {
        return qtrue; // No visflags set, show by default
    }
    
    // Check if localClient flag is set
    if (config->visflags.flags & SE_LOCAL_CLIENT) {
        // Only show if this is the local client (our ID)
        return (clientNum == cg.clientNum);
    }
    
    // Check if activePlayer flag is set
    if (config->visflags.flags & SE_ACTIVE_PLAYER) {
        // Only show if this is the active player (who we're observing)
        return (clientNum == cg.snap->ps.clientNum);
    }
    
    // Check if ready flag is set
    if (config->visflags.flags & SE_READY) {
        // Only show if this player is ready AND we're in warmup
        return (cg.warmup && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)) != 0);
    }
    
    // Check if frozen flag is set
    if (config->visflags.flags & SE_FROZEN) {
        // Only show if this player is frozen (ready in freeze tag mode, not in warmup)
        return (!cg.warmup && CG_OSPIsGameTypeFreeze() && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)) != 0);
    }
    
    return qtrue; // No localClient restriction, show by default
}

// Get container mode for element (0=none, 1=compact, 2=double)
int CG_CHUDGetElementContainerMode(cherryhudElement_t* element) {
    if (!element || !element->containerType) {
        return 0; // No container, no mode
    }
    
    // For containers with extendedtype, determine mode dynamically
    if (Q_stricmp(element->containerType, "playersRows") == 0 ||
        Q_stricmp(element->containerType, "spectatorsRows") == 0) {
        
        // Get the container config to check extendedtype
        const cherryhudScoreboardOrder_t* scoreboardOrder = CG_CHUDScoreboardOrderGet();
        if (scoreboardOrder) {
			int i;
            for (i = 0; i < scoreboardOrder->count; i++) {
                if ((Q_stricmp(element->containerType, "playersRows") == 0 &&
                     scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_PLAYERS_ROWS) ||
                    (Q_stricmp(element->containerType, "spectatorsRows") == 0 &&
                     scoreboardOrder->blocks[i].type == CHERRYHUD_BLOCK_TYPE_SPECTATORS_ROWS)) {
                    
                    // Check if container has extendedtype and if it's currently active
                    if (scoreboardOrder->blocks[i].config.extendedType.isSet) {
                        const char* activeMode = CG_CHUDGetContainerActiveMode(element->containerType, 0.0f);

                        // Check if the mode is currently active (not just configured)
                        if (activeMode) {
                            if (Q_stricmp(activeMode, "compact") == 0) {
                                return 1; // compact mode
                            } else if (Q_stricmp(activeMode, "double") == 0) {
                                return 2; // double mode
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    return 0; // none mode
}

qboolean CG_CHUDCheckHideFlagVisibility(const cherryhudConfig_t* config, int clientNum)
{
    if (!config || !config->hideflags.isSet) {
        return qtrue; // No hideflags set, show by default
    }
    
    // Check if localClient flag is set in hideflags
    if (config->hideflags.flags & SE_LOCAL_CLIENT) {
        // Hide if this is the local client (our ID)
        return (clientNum != cg.clientNum);
    }
    
    // Check if activePlayer flag is set in hideflags
    if (config->hideflags.flags & SE_ACTIVE_PLAYER) {
        // Hide if this is the active player (who we're observing)
        return (clientNum != cg.snap->ps.clientNum);
    }
    
    // Check if ready flag is set in hideflags
    if (config->hideflags.flags & SE_READY) {
        // Hide if this player is ready AND we're in warmup
        return !(cg.warmup && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)) != 0);
    }
    
    // Check if frozen flag is set in hideflags
    if (config->hideflags.flags & SE_FROZEN) {
        // Hide if this player is frozen (ready in freeze tag mode, not in warmup)
        return !(!cg.warmup && CG_OSPIsGameTypeFreeze() && (cg.snap->ps.stats[STAT_CLIENTS_READY] & (1 << clientNum)) != 0);
    }
    
    return qtrue; // No localClient restriction, show by default
}


