// Silly Quad model shader used by item_silly (models/powerups/instant/qua2*).
models/powerups/instant/qua2
{
	{
		map textures/effects/envmaplightred.jpg
		blendfunc GL_ONE GL_ZERO
		tcGen environment
		rgbGen identity
	}
}

models/powerups/instant/qua2_ring
{
	{
		map textures/effects/envmaplightred.jpg
		blendfunc add
		rgbGen identity
		tcGen environment
	}
}
