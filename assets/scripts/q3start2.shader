// Threewave Portal maps shaders - Do not modify

textures/q3start2/redsky
{
//	qer_editorimage textures/q3start2/redsky.tga
	qer_editorimage textures/skies/dimclouds.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_surfacelight 25
	q3map_lightsubdivide 512
	q3map_sun	1 1 1 32 90 90
//	{
//		map	textures/q3start2/redsky.tga
//	}
	skyparms - 512 -
	{
		map textures/skies/killsky_1.tga
		tcMod scroll 0.05 .1
		tcMod scale 2 2
		depthWrite
	}
	{
		map textures/skies/killsky_2.tga
		blendfunc GL_ONE GL_ONE
		tcMod scroll 0.05 0.06
		tcMod scale 3 2
	}

}

textures/q3start2/redsky2
{
//	qer_editorimage textures/q3start2/redsky.tga
	qer_editorimage textures/skies/dimclouds.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_surfacelight 50
	q3map_lightsubdivide 512
	q3map_sun	1 1 1 56 90 80
//	{
//		map	textures/q3start2/redsky.tga
//	}
	skyparms - 512 -
	{
		map textures/skies/killsky_1.tga
		tcMod scroll 0.05 .1
		tcMod scale 2 2
		depthWrite
	}
	{
		map textures/skies/killsky_2.tga
		blendfunc GL_ONE GL_ONE
		tcMod scroll 0.05 0.06
		tcMod scale 3 2
	}
}

textures/q3start2/redsky_nolight
{
//	qer_editorimage textures/q3start2/redsky.tga
	qer_editorimage textures/skies/dimclouds.tga
	surfaceparm noimpact
	surfaceparm nolightmap
//	q3map_sun	1 1 1 32 90 90
//	{
//		map	textures/q3start2/redsky.tga
//	}
	skyparms - 512 -
	{
		map textures/skies/killsky_1.tga
		tcMod scroll 0.05 .1
		tcMod scale 2 2
		depthWrite
	}
	{
		map textures/skies/killsky_2.tga
		blendfunc GL_ONE GL_ONE
		tcMod scroll 0.05 0.06
		tcMod scale 3 2
	}

}


textures/q3start2/xian_dm3padwall_nofx
	{
	q3map_surfacelight 100
	q3map_lightimage textures/sfx/xian_dm3padwallglow.tga
	qer_editorimage textures/sfx/xian_dm3padwall.tga
	
	{
		map textures/sfx/xian_dm3padwall.tga
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
}
textures/q3start2/xian_dm3padwall_nolight
	{
//	q3map_surfacelight 100
//	q3map_lightimage textures/sfx/xian_dm3padwallglow.tga
	qer_editorimage textures/sfx/xian_dm3padwall.tga
	
	{
		map textures/sfx/xian_dm3padwall.tga
		rgbGen identity
	}
	
	{
		map $lightmap
		rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
}

textures/q3start2/slime_lit
	{
		qer_editorimage textures/liquids/slime7.tga
		q3map_lightimage textures/liquids/slime7.tga
		q3map_globaltexture
		qer_trans .5

//		q3map_lightsubdivide 512
		surfaceparm noimpact
		surfaceparm slime
		surfaceparm nolightmap
		surfaceparm trans		

		q3map_surfacelight 200
		tessSize 256
		cull disable

		deformVertexes wave 100 sin 0 1 .5 .5

		{
			map textures/liquids/slime7c.tga
			tcMod turb .3 .2 1 .05
			tcMod scroll .01 .01
		}
	
		{
			map textures/liquids/slime7.tga
			blendfunc GL_ONE GL_ONE
			tcMod turb .2 .1 1 .05
			tcMod scale .5 .5
			tcMod scroll .01 .01
		}

		{
			map textures/liquids/bubbles.tga
			blendfunc GL_ZERO GL_SRC_COLOR
			tcMod turb .2 .1 .1 .2
			tcMod scale .05 .05
			tcMod scroll .001 .001
		}		
}
// non-solid blackness
textures/q3start2/black_nonsolid
{
	qer_editorimage textures/sfx/blackness.tga
	surfaceparm nolightmap
	surfaceparm noimpact
	surfaceparm nomarks
	surfaceparm nonsolid	
	{
		map textures/sfx/blackness.tga
	}
}

textures/q3start2/patch10_pj_lite2_b_500
{
	qer_editorimage textures/q3start2/patch10_pj_lite2_b.tga
	q3map_surfacelight 500
	surfaceparm nomarks
	//light 1
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start2/patch10_pj_lite2_b.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/q3start2/patch10_pj_lite2.blend.fix.tga
		blendFunc GL_ONE GL_ONE
	}
}

textures/q3start2/squisher
{
	qer_editorimage textures/q3start2/redsky.tga
	surfaceparm nodraw
	surfaceparm nomarks
        surfaceparm nolightmap
	surfaceparm playerclip
	surfaceparm noimpact
}


textures/q3start2/clanggrate_slick
{
	qer_editorimage textures/base_floor/clanggrate.tga
	surfaceparm nomarks
	surfaceparm trans
	surfaceparm slick
	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_floor/clanggrate.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
}


textures/q3start2/big_blue_telep
{ 
	qer_editorimage textures/ctf/blue_telep.tga
        cull disable	
        surfaceparm nomarks
        surfaceparm trans
        sort additive	

        {
	        
	        clampmap textures/ctf/blue_telep.tga
		blendFunc add
                //depthWrite
                //tcMod stretch sin .9 0.1 0 .5
                tcmod rotate 327
                rgbGen identity
	}
        {
	        clampmap textures/ctf/blue_telep2.tga
		blendFunc add
                //depthWrite
                //tcMod stretch sin .9 0.1 0 .1
                tcmod rotate -211
                rgbGen identity
        }
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcmod rotate 20
	        rgbGen identity
	}
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcMod stretch sin .7 0 0 0
                tcmod rotate -20
                rgbGen identity
	}
        {
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/q3start2/big_red_telep
{ 
	qer_editorimage textures/ctf/red_telep.tga
        cull disable	
        surfaceparm nomarks
        surfaceparm trans
        sort additive	

        {
	        
	        clampmap textures/ctf/red_telep.tga
		blendFunc add
                //depthWrite
                //tcMod stretch sin .9 0.1 0 .5
                tcmod rotate 327
                rgbGen identity
	}
        {
	        clampmap textures/ctf/red_telep2.tga
		blendFunc add
                //depthWrite
                //tcMod stretch sin .9 0.1 0 .1
                tcmod rotate -211
                rgbGen identity
        }
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcmod rotate 20
	        rgbGen identity
	}
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcMod stretch sin .7 0 0 0
                tcmod rotate -20
                rgbGen identity
	}
        {
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/q3start2/qpong_pewter
{	
	qer_editorimage textures/base_trim/pewter_spec.tga
	{
		map textures/base_trim/pewter_spec.tga
		rgbGen identity
		alphaGen lightingSpecular
	}
      
}
textures/q3start2/s_portfx
{
		qer_editorimage textures/q3start2/s_teledoor.tga
		q3map_surfacelight 50
		surfaceparm noimpact
		surfaceparm nolightmap
		cull disable
	{
		map textures/q3start2/s_teledoor.tga
		tcMod turb 0 .5 0 .1		
		rgbGen wave sin .7 .5 .0 .3
		blendfunc gl_one gl_zero
	}
	{
		map textures/q3start2/s_teledoor.tga
		tcMod turb 0 .35 .5 .2
		rgbGen wave sin .8 .2 .5 .3
		blendfunc gl_one gl_one
	}
}
textures/q3start2/logo_q3f
{    
	qer_editorimage textures/q3start2/logo_q3f.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start2/logo_q3f.tga
                blendFunc add
		rgbGen vertex
	}
}

textures/q3start2/lightblack_x
{
	qer_editorimage textures/q3start/black.tga
	q3map_lightimage textures/q3start/grey.tga
	q3map_surfacelight 300
	q3map_lightsubdivide 512
	q3map_backsplash -1 32
	surfaceparm nomarks
	surfaceparm nodlight
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_novertexshadows
	{
		map textures/q3start/black.tga
		rgbGen vertex
	}
}

textures/q3start2/lightblack_x2
{
	qer_editorimage textures/q3start/black.tga
	q3map_lightimage textures/q3start/grey.tga
	q3map_surfacelight 60
	q3map_lightsubdivide 512
	q3map_backsplash -1 32
	surfaceparm nomarks
	surfaceparm nodlight
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_novertexshadows
	{
		map textures/q3start/black.tga
		rgbGen vertex
	}
}

textures/q3start2/green_telep_x
{ 
	qer_editorimage textures/q3start2/green_telep.tga
	cull disable	
        surfaceparm nomarks
        surfaceparm trans
        surfaceparm nonsolid
	sort additive	
        {
	        
	        clampmap textures/q3start2/green_telep.tga
		blendFunc add
                tcmod rotate 327
                rgbGen identity
	}
        {
	        clampmap textures/q3start2/green_telep2.tga
		blendFunc add
                tcmod rotate -211
                rgbGen identity
        }
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcmod rotate 20
	        rgbGen identity
	}
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcMod stretch sin .7 0 0 0
                tcmod rotate -20
                rgbGen identity
	}
        {
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/q3start2/red_telep_x
{ 
	qer_editorimage textures/ctf/red_telep.tga
	cull disable	
        surfaceparm nomarks
        surfaceparm trans
        surfaceparm nonsolid
	sort additive	
        {
	        
	        clampmap textures/ctf/red_telep.tga
		blendFunc add
                tcmod rotate 327
                rgbGen identity
	}
        {
	        clampmap textures/ctf/red_telep2.tga
		blendFunc add
                tcmod rotate -211
                rgbGen identity
        }
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcmod rotate 20
	        rgbGen identity
	}
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcMod stretch sin .7 0 0 0
                tcmod rotate -20
                rgbGen identity
	}
        {
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/q3start2/blue_telep_x
{ 
	qer_editorimage textures/ctf/blue_telep.tga
	cull disable	
        surfaceparm nomarks
        surfaceparm trans
        surfaceparm nonsolid
	sort additive	
        {
	        
	        clampmap textures/ctf/blue_telep.tga
		blendFunc add
                tcmod rotate 327
                rgbGen identity
	}
        {
	        clampmap textures/ctf/blue_telep2.tga
		blendFunc add
                tcmod rotate -211
                rgbGen identity
        }
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcmod rotate 20
	        rgbGen identity
	}
        {
	        clampmap textures/ctf/telep.tga
		alphaFunc GE128
                depthWrite
                tcMod stretch sin .7 0 0 0
                tcmod rotate -20
                rgbGen identity
	}
        {
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/q3start2/fan_nonsolid
{
	qer_editorimage textures/sfx/fan.tga
	surfaceparm trans
        surfaceparm nomarks
        surfaceparm nonsolid
	cull none
        nopicmip
	{
		clampmap textures/sfx/fan.tga
		tcMod rotate 256 
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendFunc GL_DST_COLOR GL_ZERO
		depthFunc equal
	}
}

textures/q3start2/flourescent_1000_x
{
	qer_editorimage textures/base_light/light5.tga
	surfaceparm nomarks
        surfaceparm nonsolid
	q3map_surfacelight 500

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/xlight5.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/base_light/xlight5.blend.tga
		blendFunc GL_ONE GL_ONE
		rgbGen wave noise 0.8 0.07 0 40
	}
}
