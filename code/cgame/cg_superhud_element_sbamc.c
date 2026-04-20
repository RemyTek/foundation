#include "cg_local.h"
#include "cg_superhud_private.h"
#include "../qcommon/qcommon.h"

typedef struct
{
	superhudConfig_t config;
	superhudTextContext_t ctx;
	qhandle_t infiniteShader;
	superhudDrawContext_t infiniteDraw;
} shudElementStatusbarAmmoCount;

void* CG_SHUDElementSBAmCCreate(const superhudConfig_t* config)
{
	shudElementStatusbarAmmoCount* element;

	SHUD_ELEMENT_INIT(element, config);

	element->infiniteShader = trap_R_RegisterShader("icons/infinite");

	//load defaults
	if (!element->config.color.isSet)
	{
		element->config.color.isSet = qtrue;
		element->config.color.value.type = SUPERHUD_COLOR_RGBA;
		Vector4Set(element->config.color.value.rgba, 1, 0.7, 0, 1);
	}

	if (!element->config.text.isSet)
	{
		element->config.text.isSet = qtrue;
		Q_strncpyz(element->config.text.value, "%i", sizeof(element->config.text.value));
	}

	CG_SHUDTextMakeContext(&element->config, &element->ctx);
	CG_SHUDFillAndFrameForText(&element->config, &element->ctx);

	return element;
}

void CG_SHUDElementSBAmCRoutine(void* context)
{
	shudElementStatusbarAmmoCount* element = (shudElementStatusbarAmmoCount*)context;
	int wp;
	int  isFiring;
	int ammo;
	vec4_t finalColor;
	if (!cg.snap)
	{
		return;
	}

	wp = cg.snap->ps.weapon;

	isFiring = (cg.predictedPlayerState.weaponstate == WEAPON_FIRING) ? 1 : 0;

	if (wp == WP_NONE || wp == WP_GAUNTLET) return;

	ammo = CG_SHUDGetAmmo(wp);

	if (ammo == 0)
	{
		Vector4Copy(colorRed, finalColor);
		finalColor[3] = element->config.color.value.rgba[3];
	}
	else
	{
		if (isFiring && !element->config.style.isSet)
		{
			if (element->config.color2.isSet)
			{
				Vector4Copy(element->config.color2.value.rgba, finalColor);
			}
			else
			{
				Vector4Copy(colorDkGrey, finalColor);
			}
		}
		else
		{
			Vector4Copy(element->config.color.value.rgba, finalColor);
		}
	}

	Vector4Copy(finalColor, element->ctx.color);

	if (ammo > 500)
	{
		float iconSize = element->config.fontsize.isSet ? element->config.fontsize.value[1] : 20.0f;
		float iconX = element->config.rect.value[0] - iconSize * 0.5f;
		float iconY = element->ctx.coord.named.y;
		// Mirror the vertical alignment the text renderer applies via DS_ flags
		if (element->ctx.flags & DS_VCENTER)
			iconY -= iconSize * 0.5f;
		else if (element->ctx.flags & DS_VTOP)
			iconY -= iconSize;

		memset(&element->infiniteDraw, 0, sizeof(element->infiniteDraw));
		element->infiniteDraw.coord.named.x = iconX;
		element->infiniteDraw.coord.named.y = iconY;
		element->infiniteDraw.coord.named.w = iconSize;
		element->infiniteDraw.coord.named.h = iconSize;
		element->infiniteDraw.coordPicture.named.x = 0.0f;
		element->infiniteDraw.coordPicture.named.y = 0.0f;
		element->infiniteDraw.coordPicture.named.w = 1.0f;
		element->infiniteDraw.coordPicture.named.h = 1.0f;
		element->infiniteDraw.image = element->infiniteShader;
		Vector4Set(element->infiniteDraw.color, 1, 1, 1, 1);

		CG_SHUDDrawStretchPic(element->infiniteDraw.coord, element->infiniteDraw.coordPicture, element->infiniteDraw.color, element->infiniteDraw.image);
	}
	else
	{
		element->ctx.text = va(element->config.text.value, ammo > 0 ? ammo : 0);
		CG_SHUDTextPrintNew(&element->config, &element->ctx, qfalse);
	}
}

void CG_SHUDElementSBAmCDestroy(void* context)
{
	if (context)
	{
		Z_Free(context);
	}
}
