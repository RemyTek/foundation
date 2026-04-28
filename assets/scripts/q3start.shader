// Threewave Portal maps shaders - Do not modify


textures/q3start/clippy
// same as common-clip,... 
// but for some reason clip keeps dissapearing from my editor texture window?
{
	qer_editorimage textures/common/clip.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm playerclip
	surfaceparm noimpact
}

textures/q3start/grateclip
// clips, weapclips, metalsteps - all in one
{
	qer_trans 0.4
	qer_editorimage textures/common/clip.tga
	surfaceparm metalsteps	
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nodraw
}

textures/q3start/nodraw_transparent
{
	qer_editorimage textures/common/nodraw.tga
	qer_trans 0.4
	surfaceparm nodraw
	surfaceparm nonsolid
	surfaceparm trans
	surfaceparm nomarks
}

textures/q3start/logodecal
{    
	qer_editorimage textures/q3start/logo256.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/logo256.tga
                blendFunc add
		rgbGen vertex
	}
}

textures/q3start/logo_ra3
{    
	qer_editorimage textures/q3start/logo_ra3.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip
	
	{
		map textures/q3start/logo_ra3.tga
                blendFunc blend
		rgbGen vertex
	}
}

textures/q3start/logo_q3rally
{    
	qer_editorimage textures/q3start/logo_q3rally.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/logo_q3rally.tga
                blendFunc add
		rgbGen vertex
	}
}

textures/q3start/logo_ut
{    
	qer_editorimage textures/q3start/logo_ut.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/logo_ut.tga
                blendFunc blend
		rgbGen vertex
	}
}

textures/q3start/logo_cctf
{    
	qer_editorimage textures/q3start/tv_red.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/tv_red.tga
                blendFunc add
		rgbGen vertex
	}
}

textures/q3start/logo_ctfs
{    
	qer_editorimage textures/q3start/logo_ctfs.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/logo_ctfs.tga
                blendFunc add
		rgbGen vertex
	}
}

textures/q3start/logo_alliance
{    
	qer_editorimage textures/q3start/logo_alliance.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/logo_alliance.tga
                blendFunc add
		rgbGen vertex
	}
}

textures/q3start/logo_osp
{    
	qer_editorimage textures/q3start/logo_osp.tga 
	surfaceparm nomarks   
	surfaceparm trans
	surfaceparm pointlight
	nopicmip

	{
		map textures/q3start/logo_osp.tga
		blendFunc blend
		rgbGen vertex
		alphaFunc GE128
	}
}

textures/q3start/deksky_1a
{
	qer_editorimage textures/ctf_cas_l/qer_bigblacksky.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_backsplash -1 32
	q3map_lightsubdivide 512
	q3map_surfacelight 30
	q3map_sun .6 .6 .6 30 35 78
	skyparms env/dek1 - -
}

textures/q3start/deksky_1b
{
	qer_editorimage textures/ctf_cas_l/qer_bigblacksky.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_backsplash -1 32
	q3map_lightsubdivide 1024
	q3map_surfacelight 20
	skyparms env/dek1 - -
}

textures/q3start/deksky_1c
{
	qer_editorimage textures/ctf_cas_l/qer_bigblacksky.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	skyparms env/dek1 - -
}

textures/q3start/deksky_2a
{
	qer_editorimage textures/ctf_cas_l/qer_bigblacksky.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_backsplash -1 32
	q3map_lightsubdivide 512
	q3map_surfacelight 40
	q3map_sun .6 .6 .6 30 35 78
	skyparms env/dek1 - -
}

textures/q3start/deksky_2b
{
	qer_editorimage textures/ctf_cas_l/qer_bigblacksky.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_backsplash -1 32
	q3map_lightsubdivide 1024
	q3map_surfacelight 30
	skyparms env/dek1 - -
}

textures/q3start/deksky_2c
{
	qer_editorimage textures/ctf_cas_l/qer_bigblacksky.tga
	surfaceparm noimpact
	surfaceparm nolightmap
	skyparms env/dek1 - -
}


textures/q3start/lightblack_1a
{
	qer_editorimage textures/q3start/black.tga
	q3map_lightimage textures/q3start/grey.tga
	q3map_surfacelight 30
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

textures/q3start/lightblack_1b
{
	qer_editorimage textures/q3start/black.tga
	q3map_lightimage textures/q3start/grey.tga
	q3map_surfacelight 20
	q3map_lightsubdivide 1024
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

textures/q3start/fastblack
{
	qer_editorimage textures/q3start/black.tga
	surfaceparm nomarks
	surfaceparm nodlight
	surfaceparm noimpact
	surfaceparm nolightmap
	q3map_backsplash -1 32
	q3map_novertexshadows

	{
		map textures/q3start/black.tga
		rgbGen vertex
	}
}



textures/q3start/lightblack
{
	qer_editorimage textures/q3start/black.tga
	q3map_lightimage textures/q3start/grey.tga
	q3map_surfacelight 80
	q3map_lightsubdivide 384
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

textures/q3start/lightblack2
{
	qer_editorimage textures/q3start/black.tga
	q3map_lightimage textures/q3start/grey.tga
	q3map_surfacelight 150
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

textures/q3start/glass01
{
	qer_editorimage textures/effects/tinfx2b.tga
	surfaceparm trans	
	surfaceparm nolightmap
	surfaceparm nomarks
	qer_trans 0.4
	cull none

	{
		map textures/effects/tinfx2b.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/glass02
{
	qer_editorimage textures/effects/tinfx2.tga
	surfaceparm trans	
	surfaceparm nolightmap
	surfaceparm nomarks
	qer_trans 0.4
	cull none

	{
		map textures/effects/tinfx2.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/bounce_neutral
{
	qer_editorimage textures/q3start/qer_bounce.tga
	q3map_lightimage textures/q3start/bounce_fx.tga
	q3map_surfacelight 400
	surfaceparm nodamage
	surfaceparm nomarks
	surfaceparm trans

	{
		map textures/q3start/bounce_base.tga
		rgbGen identity
		alphaFunc GE128
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
		depthFunc equal
	}
	{
		map textures/q3start/bounce_glow.tga
		blendfunc add
		rgbGen wave sin 0.5 0.5 0 1.5
	}
	{
		clampmap textures/q3start/bounce_fx.tga
		blendfunc add
		tcMod stretch sin 1.2 0.8 0 1.5
		rgbGen wave square 0.5 0.5 0.25 1.5
	}
	{
		map textures/q3start/bounce_shadow.tga
		blendfunc filter
		rgbGen identity
	}
}

textures/q3start/weapfloor_neutral
{
	qer_editorimage textures/q3start/qer_weapfloor.tga
	surfaceparm metalsteps
	surfaceparm nomarks
	surfaceparm trans

	{
		clampmap textures/q3start/weapfloor_fx.tga
		rgbGen wave sin 0.9 0.2 0 0.7
		tcMod rotate 180
		depthWrite
		alphaFunc GE128
	}
	{
		map textures/q3start/weapfloor_1.tga
		blendfunc blend
		rgbGen identity
		depthFunc equal
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
		tcGen lightmap
		depthFunc equal
	}
	{
		map textures/q3start/weapfloor_shadow.tga
		blendfunc filter
		rgbGen identity
	}
}

textures/q3start/baslt4_1_2k
{
	qer_editorimage textures/base_light/baslt4_1.tga
	surfaceparm nomarks
	q3map_surfacelight 2000

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/baslt4_1.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/base_light/baslt4_1.blend.tga
		blendfunc GL_ONE GL_ONE
	}
}

textures/q3start/cornerlight01
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 1000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .09 0 7
	}		
}

textures/q3start/cornerlight02
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 1000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .05 0.1 6
	}		
}

textures/q3start/cornerlight03
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 1000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .07 0.15 7
	}		
}

textures/q3start/cornerlight04
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 1000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .08 0.2 9
	}		
}

textures/q3start/cornerlight05
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 1000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .06 0.3 8
	}		
}

textures/q3start/cornerlight_2000
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 2000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .07 0.15 7
	}		
}

textures/q3start/cornerlight_2500
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 2500

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .07 0.15 7
	}		
}

textures/q3start/cornerlight_5k
{
	qer_editorimage textures/base_light/cornerlight.tga
	q3map_lightimage textures/base_light/cornerlight.glow.tga
	surfaceparm nomarks
	q3map_surfacelight 5000

	{
		map textures/base_light/cornerlight.tga
		rgbGen identity
	}
	{
		map $lightmap
		blendfunc filter
		rgbGen identity
	}
	{	
		map textures/base_light/cornerlight.glow.tga
		blendfunc GL_one gl_one
		rgbgen wave sawtooth .6 .07 0.15 7
	}		
}

textures/q3start/white1500
{
	qer_editorimage textures/base_light/ceil1_38.tga
	surfaceparm nomarks
	q3map_surfacelight 1500

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_38.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_38.blend.tga
		blendfunc GL_ONE GL_ONE
	}
}

textures/q3start/white2k
{
	qer_editorimage textures/base_light/ceil1_38.tga
	surfaceparm nomarks
	q3map_surfacelight 2000

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_38.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_38.blend.tga
		blendfunc GL_ONE GL_ONE
	}
}

textures/q3start/light5_200
{
	qer_editorimage textures/base_light/light5.tga
	surfaceparm nomarks
	q3map_surfacelight 200

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
	}
}

textures/q3start/light5_500
{
	qer_editorimage textures/base_light/light5.tga
	surfaceparm nomarks
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
	}
}

textures/q3start/flourescent_1000
{
	qer_editorimage textures/base_light/light5.tga
	surfaceparm nomarks
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

textures/q3start/flourescent_5k
{
	qer_editorimage textures/base_light/light5.tga
	surfaceparm nomarks
	q3map_surfacelight 5000

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

textures/q3start/flourescent_10k
{
	qer_editorimage textures/base_light/light5.tga
	surfaceparm nomarks
	q3map_surfacelight 10000

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

textures/q3start/s_beam_green
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave sin .4 .3 .0 .1

	}
}

textures/q3start/s_beam_green01
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave square 1 .7 0 .35

	}
}

textures/q3start/s_beam_green02
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave square 1 .7 .1 .35

	}
}

textures/q3start/s_beam_green03
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave square 1 .7 .2 .35

	}
}

textures/q3start/s_beam_green04
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave square 1 .7 .3 .35

	}
}

textures/q3start/s_beam_greenx1
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave square 1 0.7 0 0.5

	}
}

textures/q3start/s_beam_greenx2
{
	qer_editorimage textures/q3start/s_beam_green.tga
	q3map_lightimage textures/q3start/s_beam_green_blend.tga
	q3map_surfacelight 512
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/q3start/s_beam_green_blend.tga
		blendfunc GL_ONE GL_ONE
		tcmod scroll 0.2 0.0
		rgbGen wave square 1 0.7 0.5 0.5

	}
}

textures/q3start/ceil1_4_5k
{
	qer_editorimage textures/base_light/ceil1_4.tga
	surfaceparm nomarks
	q3map_surfacelight 5000

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_4.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_4.blend.tga
		blendfunc GL_ONE GL_ONE
	}
}

textures/q3start/ceil1_4_10k
{
	qer_editorimage textures/base_light/ceil1_4.tga
	surfaceparm nomarks
	q3map_surfacelight 10000

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_4.tga
		blendFunc filter
		rgbGen identity
	}
	{
		map textures/base_light/ceil1_4.blend.tga
		blendfunc GL_ONE GL_ONE
	}
}

textures/q3start/stripe_blue
{
	qer_editorimage textures/base_trim/border11c.tga
	q3map_lightimage textures/base_trim/border11c_pulse1.tga
	q3map_surfacelight 300
	
	{
		map textures/base_trim/border11c.tga
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
	}
	{
		map textures/base_trim/border11c_light.tga
		blendfunc gl_one gl_one
		rgbgen wave sin 1 .1 0 5
	}
	{
		map textures/base_trim/border11c_pulse1b.tga
		blendfunc gl_one gl_one
		tcmod scale .08 1
		tcmod scroll -0.5 0
	}
}

textures/q3start/gothic_light2_6k
{
	qer_editorimage textures/gothic_light/gothic_light2.tga
	q3map_surfacelight 6000
	surfaceparm nomarks

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/gothic_light/gothic_light2.tga
		blendFunc GL_DST_COLOR GL_ZERO
		rgbGen identity
	}
	{
		map textures/gothic_light/gothic_light2_blend.tga
		rgbGen wave sin .6 .1 .1 .1
		blendfunc GL_ONE GL_ONE
	}
}

textures/q3start/spinlight_amber
{
	qer_editorimage textures/q3start/spin_fx_amber.tga
	qer_trans 0.4
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm trans

	{
		clampmap textures/q3start/spin_fx_amber.tga
		blendfunc add
		tcMod rotate 360
	}
}

textures/q3start/spinflare_amber 
{
	qer_editorimage textures/q3start/flare_amber.tga
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nolightmap
	deformVertexes autosprite
	cull none
	{
		map textures/q3start/flare_amber.tga
                blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/spinflare_amber_2 
{
	qer_editorimage textures/q3start/flare_amber.tga
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nolightmap
	DeformVertexes autosprite

	{
		map textures/q3start/flare_amber.tga
                blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/spinflare_amber_3
{
	qer_editorimage textures/q3start/flare_amber.tga
	surfaceparm trans
	surfaceparm nomarks
	surfaceparm nonsolid
	surfaceparm noimpact
	surfaceparm nolightmap

	{
		map textures/q3start/flare_amber.tga
                blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/glass_amber
{
	qer_editorimage textures/q3start/transparency2.tga
	surfaceparm nomarks
	surfaceparm trans
	cull none
	qer_trans 0.5
     
        {
		map textures/q3start/transparency2.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
		rgbGen wave sin 0.5 0 0 0
	}
}

textures/q3start/numdecals
{       	
	qer_editorimage textures/q3start/nombers.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm trans
	cull none
	nopicmip

	{
		map textures/q3start/nombers.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gatedecal
{       	
	qer_editorimage textures/q3start/gate.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm trans
	cull none
	nopicmip

	{
		map textures/q3start/gate.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/randomdecal
{       	
	qer_editorimage textures/q3start/random.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	surfaceparm trans
	cull none
	nopicmip

	{
		map textures/q3start/random.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/clangdark_nonsolid
{
	qer_editorimage textures/base_floor/clangdark.tga
	surfaceparm metalsteps
	surfaceparm nonsolid

	{
		map $lightmap
		rgbGen identity
	}
	{
		map textures/base_floor/clangdark.tga
		blendFunc filter
		rgbGen identity
	}
}

textures/q3start/bluey
{
	qer_editorimage textures/q3start/blue.tga
	q3map_surfacelight 100
	surfaceparm trans	
	surfaceparm nonsolid
	surfaceparm nolightmap
	surfaceparm nomarks
	cull none

	{
		map textures/q3start/blue.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/redy
{
	qer_editorimage textures/q3start/red.tga
	q3map_surfacelight 100
	surfaceparm trans	
	surfaceparm nonsolid
	surfaceparm nolightmap
	surfaceparm nomarks
	cull none

	{
		map textures/q3start/red.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/greeny
{
	qer_editorimage textures/q3start/green.tga
	q3map_surfacelight 100
	surfaceparm trans	
	surfaceparm nonsolid
	surfaceparm nolightmap
	surfaceparm nomarks
	cull none

	{
		map textures/q3start/green.tga
		tcgen environment
		blendFunc GL_ONE GL_ONE
	}
}

textures/q3start/screen01
{
	qer_editorimage textures/q3start/comp_screen.tga
	q3map_surfacelight 100
	surfaceparm nomarks

	{
		animMap .25 textures/q3start/logo128.tga textures/q3start/logo128.tga textures/q3start/logo128.tga 
		rgbGen wave sawtooth 0 1 0 .25
	
	}
	{
		map textures/base_wall/comp3textd.tga
		blendfunc add
		rgbGen wave inversesawtooth 0 1 0 .25
		tcmod scroll 5 .25
	}
	{
		map textures/base_wall/chrome_env.tga
		tcgen environment
		tcmod scale .5 .5
		blendfunc add
		rgbgen wave triangle .5 .05 0 10
	}
	{
		map textures/q3start/comp_screen.tga
		blendfunc blend
		rgbgen identity
	}
	{
		map $lightmap
		blendfunc gl_dst_color gl_zero
		rgbgen identity
	}
}

textures/q3start/screen02
{
	qer_editorimage textures/q3start/comp_screen.tga
	q3map_surfacelight 100
	surfaceparm nomarks

	{
		animMap .25 textures/q3start/logo128.tga textures/q3start/logo128.tga textures/q3start/logo128.tga 
		rgbGen wave sawtooth 0 1 0 .25
	
	}
	{
		map textures/base_wall/comp3textd.tga
		blendfunc add
		rgbGen wave inversesawtooth 0 1 0 .25
		tcmod scroll 5 .25
	}
	{
		map textures/base_wall/chrome_env.tga
		tcgen environment
		tcmod scale .5 .5
		blendfunc add
		rgbgen wave triangle .5 .05 0 10
	}
	{
		map textures/q3start/comp_screen.tga
		blendfunc blend
		rgbgen identity
	}
	{
		map $lightmap
		blendfunc gl_dst_color gl_zero
		rgbgen identity
	}
}

textures/q3start/trippy01
{
	qer_editorimage textures/q3start/bluestuff.tga
	surfaceparm nolightmap

	{
		map textures/q3start/bluestuff.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcMod scroll 0.3 1
		tcMod turb 0 0.25 0 0.05
	}
	{
		map textures/q3start/redstuff.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcMod scroll -0.1 0.7
	}
	{
		map textures/q3start/greenstuff.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcMod scroll -0.2 0.5
		tcMod turb 0 0.25 0 0.05
	}
}

textures/q3start/trippy02
{
	qer_editorimage textures/q3start/bluestuff.tga
	surfaceparm nolightmap

	{
		map textures/q3start/bluestuff.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcMod scroll 0.3 1
		tcMod turb 0 0.25 0 0.05
	}
	{
		map textures/q3start/redstuff.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcMod scroll 0.5 0.1
	}
	{
		map textures/q3start/greenstuff.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcMod scroll -0.2 0.5
		tcMod turb 0 0.25 0 0.05
	}
}

textures/q3start/monkeyshines01
{
	qer_editorimage textures/sfx/tesla1.tga
	q3map_lightimage textures/sfx/tesla1.tga	
	q3map_surfacelight 100
	cull none

	{
		map $lightmap
		tcgen environment
		blendfunc filter
	}
	{
		map textures/sfx/tesla1.tga
		blendfunc add
		rgbgen wave sawtooth 0 1 0 5
		tcmod scale 1 .5
		tcmod turb 0 .1 .2 1
		tcMod scroll -1 -1
	}
	{
		map textures/sfx/electricslime.tga
		blendfunc add
		rgbgen wave sin 0 .5 .2 1
		tcmod scale .5 .5
		tcmod turb 0 .1 0 1
		tcmod rotate 180
		tcmod scroll -1 -1
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
}

textures/q3start/monkeyshines02
{
	qer_editorimage textures/sfx/tesla1.tga
	q3map_lightimage textures/sfx/tesla1.tga	
	q3map_surfacelight 100
	cull none

	{
		map $lightmap
		tcgen environment
		blendfunc filter
	}
	{
		map textures/sfx/tesla1.tga
		blendfunc add
		rgbgen wave sawtooth 0 1 0 5
		tcmod scale 1 .5
		tcmod turb 0 .1 .4 1
		tcMod scroll -1 -1
	}
	{
		map textures/sfx/electricslime.tga
		blendfunc add
		rgbgen wave sin 0 .5 .4 1
		tcmod scale .5 .5
		tcmod turb 0 .1 0 1
		tcmod rotate 180
		tcmod scroll -1 -1
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
}

textures/q3start/monkeyshines03
{
	qer_editorimage textures/sfx/tesla1.tga
	q3map_lightimage textures/sfx/tesla1.tga	
	q3map_surfacelight 100
	cull none

	{
		map $lightmap
		tcgen environment
		blendfunc filter
	}
	{
		map textures/sfx/tesla1.tga
		blendfunc add
		rgbgen wave sawtooth 0 1 0 5
		tcmod scale 1 .5
		tcmod turb 0 .1 .6 1
		tcMod scroll -1 -1
	}
	{
		map textures/sfx/electricslime.tga
		blendfunc add
		rgbgen wave sin 0 .5 .6 1
		tcmod scale .5 .5
		tcmod turb 0 .1 0 1
		tcmod rotate 180
		tcmod scroll -1 -1
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
}

textures/q3start/monkeyshines04
{
	qer_editorimage textures/sfx/tesla1.tga
	q3map_lightimage textures/sfx/tesla1.tga	
	q3map_surfacelight 100
	cull none

	{
		map $lightmap
		tcgen environment
		blendfunc filter
	}
	{
		map textures/sfx/tesla1.tga
		blendfunc add
		rgbgen wave sawtooth 0 1 0 5
		tcmod scale 1 .5
		tcmod turb 0 .1 .8 1
		tcMod scroll -1 -1
	}
	{
		map textures/sfx/electricslime.tga
		blendfunc add
		rgbgen wave sin 0 .5 .8 1
		tcmod scale .5 .5
		tcmod turb 0 .1 0 1
		tcmod rotate 180
		tcmod scroll -1 -1
	}
	{
		map textures/sfx/cabletest2.tga
		blendfunc blend
	}
}

textures/q3start/slime_mg1
	{
		qer_editorimage textures/liquids/slime7.tga
		q3map_lightimage textures/liquids/slime7.tga
		q3map_globaltexture
		qer_trans .5
		surfaceparm slime
		surfaceparm nolightmap
		surfaceparm trans		
		q3map_surfacelight 100
		tessSize 32
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

textures/q3start/lavahell_mg1
{
	qer_editorimage textures/liquids/lavahell.tga
	q3map_globaltexture
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 200
	q3map_lightsubdivide 256
	cull disable
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1

	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .1 0 .05
	}
}

textures/q3start/lavahell_mg2
{
	qer_editorimage textures/liquids/lavahell.tga
	q3map_globaltexture
	surfaceparm trans
	surfaceparm noimpact
	surfaceparm lava
	surfaceparm nolightmap
	q3map_surfacelight 100
	q3map_lightsubdivide 256
	cull disable
	tesssize 128
	cull disable
	deformVertexes wave 100 sin 3 2 .1 0.1

	{
		map textures/liquids/lavahell.tga
		tcMod turb 0 .1 0 .05
	}
}

textures/q3start/tv_cctf
{
	qer_editorimage textures/q3start/tv_red.tga
	q3map_lightimage textures/q3start/tv_red.tga
        q3map_surfacelight 100

	{
		map textures/q3start/tv_red.tga
	        rgbGen wave square 0 1 0 .5
	}
	{
		map textures/base_wall/comp3text.tga
		blendfunc add
	        rgbGen identity
		tcmod scroll 3 3
	}
	{
		map textures/base_wall/comp3textb.tga
		blendfunc add
	        rgbGen identity
		tcmod scroll 3 3
	}
	{
		map $lightmap
	        rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
	{
		map $lightmap
		tcgen environment
		tcmod scale .5 .5
	        rgbGen wave sin .25 0 0 0
		blendfunc add
	}	          		
}     

textures/q3start/tv_ctfs
{
	qer_editorimage textures/q3start/logo_ctfs.tga
	q3map_lightimage textures/q3start/logo_ctfs.tga
        q3map_surfacelight 100

	{
		map textures/q3start/logo_ctfs.tga
	        rgbGen wave square 0 1 0 .5
	}
	{
		map textures/base_wall/comp3text.tga
		blendfunc add
	        rgbGen identity
		tcmod scroll 3 3
	}
	{
		map textures/base_wall/comp3textb.tga
		blendfunc add
	        rgbGen identity
		tcmod scroll 3 3
	}
	{
		map $lightmap
	        rgbGen identity
		blendfunc gl_dst_color gl_zero
	}
	{
		map $lightmap
		tcgen environment
		tcmod scale .5 .5
	        rgbGen wave sin .25 0 0 0
		blendfunc add
	}	          		
}     

textures/q3start/portal_x
{       	
	qer_editorimage textures/q3start/portal_x.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm noimpact
	surfaceparm nomarks
	surfaceparm trans
	cull none
	nopicmip

	{
		map textures/q3start/portal_x.tga
		blendFunc blend
		rgbgen wave sin 0.9 0 0 0
	}
}

textures/q3start/portal_juan
{
	qer_editorimage textures/common/qer_portal.tga
	surfaceparm nolightmap
	surfaceparm nomarks
	portal
	nopicmip
	{
		map textures/q3start/q1teleporter.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		alphagen portal 184
		rgbgen wave sin .7 .05 0 .3
		tcmod stretch sin 1 .1 0 0.6
		tcmod turb 1 .5 0 .1
		depthWrite
	}
	{
		map textures/q3start/q1teleporter.tga
		rgbgen wave sin .7 .09 0 .3
		tcmod turb 1 .5 .3 .1
		blendfunc GL_one gl_one
	}
	{
		map textures/q3start/q1teleporter.tga
		rgbgen wave sin .7 .1 0 .3
		tcmod turb 1 .5 .8 .1
		blendfunc GL_one gl_one
	}
}

textures/q3start/teleporter_clear
{
	qer_editorimage textures/q3start/q1teleporter.tga
	surfaceparm nolightmap
	surfaceparm playerclip
	surfaceparm noimpact
	surfaceparm nomarks
	surfaceparm trans
	nopicmip

	{
		map textures/q3start/q1teleporter.tga
		blendfunc GL_one gl_one
		tcMod stretch sin 1 0.1 1 0.6 
		tcMod turb 1 0.5 0 0.1
	}
	{
		map textures/q3start/q1teleporter.tga
		blendfunc GL_one gl_one
		tcMod turb 1 0.5 0.3 0.1
	}
	{
		map textures/q3start/q1teleporter.tga
		blendfunc GL_one gl_one
		tcMod turb 1 0.5 0.8 0.1
	}
}

textures/q3start/teleporter_levels
{
	qer_editorimage textures/q3start/q1teleporter.tga
	surfaceparm nolightmap
	surfaceparm playerclip
	surfaceparm noimpact
	surfaceparm nomarks
	surfaceparm trans
	nopicmip

	{
		map textures/q3start/q1teleporter.tga
		blendfunc GL_dst_color gl_one
		tcMod stretch sin 1 0.1 1 0.6 
		tcMod turb 1 0.5 0 0.1
	}
	{
		map textures/q3start/q1teleporter.tga
		blendfunc GL_dst_color gl_one
		tcMod turb 1 0.5 0.3 0.1
	}
	{
		map textures/q3start/q1teleporter.tga
		blendfunc GL_dst_color gl_one
		tcMod turb 1 0.5 0.8 0.1
	}
}

textures/q3start/gt0
{       	
	qer_editorimage textures/q3start/gt_00.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gt_00.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gt1
{       	
	qer_editorimage textures/q3start/gt_01.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gt_01.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gt3
{       	
	qer_editorimage textures/q3start/gt_03.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gt_03.tga
		blendFunc blend
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gt4
{       	
	qer_editorimage textures/q3start/gt_04.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gt_04.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gt9
{       	
	qer_editorimage textures/q3start/gt_09.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gt_09.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gt10
{       	
	qer_editorimage textures/q3start/gt_10.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gt_10.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/gvx
{       	
	qer_editorimage textures/q3start/gv_x.tga
	qer_trans 0.4
	surfaceparm nolightmap
	surfaceparm nomarks
	nopicmip

	{
		map textures/q3start/gv_x.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbgen identity
		depthWrite
	}
}

textures/q3start/x
{
	qer_editorimage textures/q3start/comp_screen.tga
	q3map_surfacelight 100
	surfaceparm nomarks
	surfaceparm nolightmap

	{
		map textures/q3start/grey.tga
		rgbGen Identity
	}
	{
		map textures/base_wall/comp3textd.tga
		blendfunc add
		rgbGen wave noise 0.5 0.6 0 7
		tcmod scroll 5 .25
	}
}
