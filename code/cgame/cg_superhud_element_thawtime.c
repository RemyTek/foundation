#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	int timePrev;
	superhudTextContext_t ctx;
} shudElementThawTime_t;

void* CG_SHUDElementThawTimeCreate(const superhudConfig_t* config)
{
	shudElementThawTime_t* element;

	SHUD_ELEMENT_INIT(element, config);

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	CG_SHUDFillAndFrameForText(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementThawTimeRoutine(void* context)
{
	shudElementThawTime_t* element = (shudElementThawTime_t*)context;
	int remainingTime;
	int seconds, mins;
	float percent = 0.0f;

	if (CG_OSPIsGameTypeFreeze() && (cg_drawThawTimer.integer || cg_drawThawPercentage.integer))
	{
		if (!Q_Isfreeze(cg.clientNum))
		{
			return;
		}

		remainingTime = cg.thawTime - cg.time;
		if (remainingTime < 0)
		{
			return;
		}

		if (cg_drawThawPercentage.integer)
		{
			int percentInt;
			int autoThawTime = cgs.autoThawTime * 1000;

			if (autoThawTime > 0)
			{
				percent = 100.0f * (1.0f - (float)remainingTime / (float)autoThawTime);
				if (percent < 0.0f) percent = 0.0f;
				if (percent > 100.0f) percent = 100.0f;
			}
			percentInt = (int)(percent + 0.5f);

			if (percentInt == 100)
			{
				element->ctx.text = va("THAW %03d%%", percentInt);
			}
			else if (percentInt > 9)
			{
				element->ctx.text = va("THAW %02d%%", percentInt);
			}
			else
			{
				element->ctx.text = va("THAW %d%%", percentInt);
			}
		}
		else if (cg_drawThawTimer.integer)
		{
			seconds = (remainingTime + 999) / 1000;
			mins = seconds / 60;
			seconds -= mins * 60;
			if (mins > 0)
			{
				element->ctx.text = va("THAW %d:%02d", mins, seconds);
			}
			else if (seconds > 9)
			{
				element->ctx.text = va("THAW %02d", seconds);
			}
			else
			{
				element->ctx.text = va("THAW %d", seconds);
			}
		}

		CG_SHUDTextPrint(&element->config, &element->ctx);
	}
}

void CG_SHUDElementThawTimeDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
