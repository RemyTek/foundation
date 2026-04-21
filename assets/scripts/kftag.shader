//flags
models/flag3/b_flag3
{
	cull none
	nopicmip

	{
		map models/flag3/b_flag3_tint.tga
		blendfunc add
		rgbgen identity
	}
	{
		AnimMap 15 models/flag3/flag3_static_1.tga models/flag3/flag3_static_2.tga models/flag3/flag3_static_3.tga models/flag3/flag3_static_4.tga
		blendfunc add
		rgbGen wave sin 0.5 0.25 0 3
	}
	{
		map models/flag3/flag3.tga
		blendfunc blend
		rgbGen wave triangle 0.95 0.05 1 15
	}
}

models/flag3/d_flag3
{
	cull none
	nopicmip

	{
		map models/flag3/d_flag3_tint.tga
		blendfunc add
		rgbgen entity
	}
	{
		AnimMap 15 models/flag3/flag3_static_1.tga models/flag3/flag3_static_2.tga models/flag3/flag3_static_3.tga models/flag3/flag3_static_4.tga
		blendfunc add
		rgbGen wave sin 0.5 0.25 0 3
	}
	{
		map models/flag3/flag3.tga
		blendfunc blend
		rgbGen wave triangle 0.95 0.05 1 15
	}
}

models/flag3/f_flag3
{
	cull none
	nopicmip

	{
		map models/flag3/f_flag3_tint.tga
		blendfunc add
		rgbgen identity
	}
	{
		AnimMap 15 models/flag3/flag3_static_1.tga models/flag3/flag3_static_2.tga models/flag3/flag3_static_3.tga models/flag3/flag3_static_4.tga
		blendfunc add
		rgbGen wave sin 0.5 0.25 0 3
	}
	{
		map models/flag3/f_flag3.tga
		blendfunc blend
		rgbGen wave triangle 0.95 0.05 1 15
	}
}

models/flag3/g_flag3
{
	cull none
	nopicmip

	{
		map models/flag3/g_flag3_tint.tga
		blendfunc add
		rgbgen identity
	}
	{
		AnimMap 15 models/flag3/flag3_static_1.tga models/flag3/flag3_static_2.tga models/flag3/flag3_static_3.tga models/flag3/flag3_static_4.tga
		blendfunc add
		rgbGen wave sin 0.5 0.25 0 3
	}
	{
		map models/flag3/flag3.tga
		blendfunc blend
		rgbGen wave triangle 0.95 0.05 1 15
	}
}

models/flag3/n_flag3
{
	cull none
	nopicmip

	{
		map models/flag3/n_flag3_tint.tga
		blendfunc add
		rgbgen identity
	}
	{
		AnimMap 15 models/flag3/flag3_static_1.tga models/flag3/flag3_static_2.tga models/flag3/flag3_static_3.tga models/flag3/flag3_static_4.tga
		blendfunc add
		rgbGen wave sin 0.5 0.25 0 3
	}
	{
		map models/flag3/flag3.tga
		blendfunc blend
		rgbGen wave triangle 0.95 0.05 1 15
	}
}

models/flag3/r_flag3
{
	cull none
	nopicmip

	{
		map models/flag3/r_flag3_tint.tga
		blendfunc add
		rgbgen identity
	}
	{
		AnimMap 15 models/flag3/flag3_static_1.tga models/flag3/flag3_static_2.tga models/flag3/flag3_static_3.tga models/flag3/flag3_static_4.tga
		blendfunc add
		rgbGen wave sin 0.5 0.25 0 3
	}
	{
		map models/flag3/flag3.tga
		blendfunc blend
		rgbGen wave triangle 0.95 0.05 1 15
	}
}
//freeze
freezeShader
{
	nopicmip
	deformVertexes wave 100 sin 2 0 0 0
	{
		map gfx/misc/freeze.jpg
		blendfunc add
		rgbGen const ( 0.95 1 1 )
		tcMod scroll 0.1 0.1
		tcGen environment
	}
}

freezeMarkShader
{
	nopicmip
	polygonoffset
	{
		clampmap gfx/damage/freeze_stain.tga
		blendfunc gl_src_alpha gl_one_minus_src_alpha
		rgbgen identitylighting
		alphagen vertex
	}
}

spawnShader
{
        deformVertexes wave 100 sin 3 0 0 0
        {
                map menu/art/fx_grn.tga
                blendfunc GL_ONE GL_ONE
                tcGen environment
                tcmod rotate 30
                //tcMod turb 0 0.2 0 .2
                tcmod scroll 1 .1
        }
}

iceTrail
{

        nopicmip                        // make sure a border remains
        entityMergable          // allow all the sprites to be merged together
        {
                clampmap gfx/damage/blood_spurt.tga
                blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
                rgbGen          vertex
                alphaGen        vertex
        }
}

//fullBright
//{
	// use our supplied color...
//	{
//		map $whiteimage
//		blendFunc GL_ONE GL_ZERO
//		rgbGen entity
//	}

	// ...and divide it by the current brightness
//	{
//		map $whiteimage
//		blendFunc GL_DST_COLOR GL_ZERO
//		rgbGen identityLighting
//	}
//}

//fullBrightVertexLight
//{
//	{
//		map $whiteimage
//		blendFunc GL_ONE GL_ZERO
//		rgbGen entity
//	}
//}
fullBright
{
    {
        map gfx/colors/neutral.tga
        rgbGen entity
        blendFunc GL_ONE GL_ZERO
    }
}


//grapple
models/weapons2/grapple2/ball
{
	//novlcollapse
	nopicmip
	{
		map models/weapons2/grapple2/ball.jpg
		blendFunc GL_ONE GL_ZERO
		tcGen environment
		rgbGen lightingDiffuse
	}
}

models/weapons2/grapple2/spikes
{
	//novlcollapse
	nopicmip
	{
		map models/weapons2/grapple2/spikes.jpg
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		tcGen environment
		rgbGen lightingDiffuse
	}
}

grappleRope
{
	//novlcollapse
	nopicmip
	cull none
	{
		map models/weapons2/grapple2/grapplerope.tga
		blendfunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
	}
}
//players

// CPM skins
// KHB  9-Mar-01	single shader for both v and lm, all standard models now supported, and a WICKED trick...  :)

models/players/anarki/pm
{
	nopicmip
	{
		map models/players/anarki/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/anarki/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/anarki/pmh
{
	nopicmip
	{
		map models/players/anarki/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/anarki/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/anarki/pmg
{
	nopicmip
	{
		map models/players/anarki/color_x.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/anarki/color_x.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/biker/pm
{
	nopicmip
	{
		map models/players/biker/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/biker/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/biker/pmh
{
	nopicmip
	{
		map models/players/biker/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/biker/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/bitterman/pm
{
	nopicmip
	{
		map models/players/bitterman/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/bitterman/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/bitterman/pmh
{
	nopicmip
	{
		map models/players/bitterman/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/bitterman/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/bones/pm
{
	nopicmip
	{
		map models/players/bones/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/bones/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/crash/pm
{
	nopicmip
	{
		map models/players/crash/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/crash/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/crash/pmt
{
	nopicmip
	{
		map models/players/crash/color_t.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/crash/color_t.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/doom/pm
{
	nopicmip
	{
		map models/players/doom/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/doom/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/grunt/pm
{
	nopicmip
	{
		map models/players/grunt/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/grunt/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/grunt/pmh
{
	nopicmip
	{
		map models/players/grunt/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/grunt/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/hunter/pm
{
	nopicmip
	{
		map models/players/hunter/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/hunter/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/hunter/pmh
{
	nopicmip
	{
		map models/players/hunter/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/hunter/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/hunter/pmf
{
	nopicmip
	{
		map models/players/hunter/color_x.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
//	{
//		map models/players/hunter/color_x.tga
//		blendFunc GL_ONE GL_ZERO
//		alphaFunc LT128
//		rgbGen identity
//	}
}


models/players/keel/pm
{
	nopicmip
	{
		map models/players/keel/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/keel/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/keel/pmh
{
	nopicmip
	{
		map models/players/keel/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/keel/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/klesk/pm
{
	nopicmip
	{
		map models/players/klesk/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/klesk/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/klesk/pmh
{
	nopicmip
	{
		map models/players/klesk/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/klesk/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/lucy/pm
{
	nopicmip
	{
		map models/players/lucy/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/lucy/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/lucy/pmh
{
	nopicmip
	{
		map models/players/lucy/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/lucy/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/major/pm
{
	nopicmip
	{
		map models/players/major/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/major/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/major/pmh
{
	nopicmip
	{
		map models/players/major/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/major/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/mynx/pm
{
	nopicmip
	{
		map models/players/mynx/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/mynx/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/mynx/pmh
{
	nopicmip
	{
		map models/players/mynx/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/mynx/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/mynx/pmt
{
	nopicmip
	{
		map models/players/mynx/color_x.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/mynx/color_x.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/orbb/pm
{
	nopicmip
	{
		map models/players/orbb/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/orbb/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/orbb/pmh
{
	nopicmip
	{
		map models/players/orbb/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/orbb/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/ranger/pm
{
	nopicmip
	{
		map models/players/ranger/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/ranger/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/ranger/pmh
{
	nopicmip
	{
		map models/players/ranger/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/ranger/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/razor/pm
{
	nopicmip
	{
		map models/players/razor/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/razor/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/razor/pmh
{
	nopicmip
	{
		map models/players/razor/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/razor/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/sarge/pm
{
	nopicmip
	{
		map models/players/sarge/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/sarge/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/slash/pm
{
	nopicmip
	{
		map models/players/slash/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/slash/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/slash/pmh
{
	nopicmip
	{
		map models/players/slash/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/slash/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/slash/pms
{
	nopicmip
	sort additive
	cull disable
	{
		clampmap models/players/slash/color_skates.tga
		blendFunc GL_ONE GL_ONE
		tcMod stretch sin .9 0.1 0 1.1
		rgbGen entity
	}
	{
		clampmap models/players/slash/color_skates.tga
		blendFunc GL_ONE GL_ONE
		tcMod stretch sin .9 0.1 0 1.1
		rgbGen identity
	}
}


models/players/sorlag/pm
{
	nopicmip
	{
		map models/players/sorlag/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/sorlag/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/sorlag/pmt
{
	nopicmip
	{
		map models/players/sorlag/color_t.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/sorlag/color_t.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/tankjr/pm
{
	nopicmip
	{
		map models/players/tankjr/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/tankjr/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/uriel/pm
{
	nopicmip
	{
		map models/players/uriel/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/uriel/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/uriel/pmh
{
	nopicmip
	{
		map models/players/uriel/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/uriel/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/uriel/pmw
{
	nopicmip
	{
		map models/players/uriel/color_x.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
//	{
//		map models/players/uriel/color_x.tga
//		blendFunc GL_ONE GL_ZERO
//		alphaFunc LT128
//		rgbGen identity
//	}
}


models/players/visor/pm
{
	nopicmip
	{
		map models/players/visor/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/visor/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}


models/players/xaero/pm
{
	nopicmip
	{
		map models/players/xaero/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/xaero/color.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}

models/players/xaero/pmh
{
	nopicmip
	{
		map models/players/xaero/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc GE128
		rgbGen entity
	}
	{
		map models/players/xaero/color_h.tga
		blendFunc GL_ONE GL_ZERO
		alphaFunc LT128
		rgbGen identity
	}
}
//teams
"team_icon/the fallen_red"
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/thefallen_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
"team_icon/the fallen_blue"
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/thefallen_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/thefallen_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/thefallen_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/thefallen_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/thefallen_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/intruders_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/intruders_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/intruders_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/intruders_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/crusaders_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/crusaders_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/crusaders_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/crusaders_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/pagans_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/pagans_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/pagans_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/pagans_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/stroggs_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/stroggs_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/stroggs_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/stroggs_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}

//shortcuts by Kr3m

team_icon/t_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/thefallen_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/t_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/thefallen_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/i_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/intruders_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/i_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/intruders_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/c_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/crusaders_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/c_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/crusaders_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/p_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/pagans_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/p_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/pagans_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/s_red
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/stroggs_red.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
team_icon/s_blue
{
	cull none
        surfaceparm nolightmap
        surfaceparm trans
        surfaceparm nomarks
	{
		map team_icon/stroggs_blue.tga
		blendFunc Add
		rgbgen wave triangle 0.2 0.5 0 0.2

	}

}
sprites/infinity
{
	nopicmip
	{
		map sprites/infinity.tga
		blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
		rgbGen vertex
	}
}

powerups/spawnprotect
{
	deformVertexes wave 100 sin 3 0 0 0
	{
		map textures/effects/quadmap3.tga
		blendfunc GL_ONE GL_ONE
		tcGen environment
                tcmod rotate 30
		//tcMod turb 0 0.2 0 .2
                tcmod scroll 1 .1
	}
}
powerups/spawnProtectWeapon
{
	deformVertexes wave 100 sin 0.5 0 0 0
	{
		map textures/effects/quadmap3.tga
		blendfunc GL_ONE GL_ONE
		tcGen environment
                tcmod rotate 30
		//tcMod turb 0 0.2 0 .2
                tcmod scroll 1 .1
	}
}