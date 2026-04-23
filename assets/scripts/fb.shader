fBright
{
	// use color...
	{
		map $whiteimage
		blendFunc GL_ONE GL_ZERO
		rgbGen entity
	}

	// divide it by the current brightness
	{
		map $whiteimage
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identityLighting
	}
}

fBrightVertexLight
{
	{
		map $whiteimage
		blendFunc GL_ONE GL_ZERO
		rgbGen entity
	}
}
