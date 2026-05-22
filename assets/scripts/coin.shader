// Coin shaders imported from threewave/scripts/threewave.shader
// Coin models reference models/special/gold_coin and models/special/gold_flare.

models/special/gold_flare
{
	deformVertexes autoSprite
	cull disable
	nopicmip
	{
		clampmap models/powerups/special/gold_flare.jpg
		blendFunc GL_ONE GL_ONE
		tcMod rotate 200
		tcMod stretch sin 0.7 0.3 0 0.5
	}
}

models/special/gold_coin
{
	{
		map models/powerups/special/gold_coin.jpg
		blendfunc GL_ONE GL_ZERO
		tcGen environment
		rgbGen identity
	}
}

icons/coin_icon
{
	sort nearest
	nopicmip
	{
		map icons/coin_icon.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}
