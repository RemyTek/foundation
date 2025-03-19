models/weapons3/hmg/hmg
{
	cull disable
	nopicmip
	{
		map	models/weapons3/hmg/hmg.tga
		rgbGen lightingDiffuse	
	}
}

models/weapons3/hmg/f_hmg
{
	nopicmip
	sort additive
	cull disable
	{
		map	models/weapons3/hmg/f_hmg.tga
		blendfunc add
	}
}

models/powerups/ammo/hmgammo
{
	nopicmip
    {
	    map textures/effects/tinfx2d.tga
		tcGen environment
		blendfunc GL_ONE GL_ZERO
        rgbGen identity
    }
    {
	    map models/powerups/ammo/hmgammo.tga
		blendfunc blend
        rgbGen lightingDiffuse
    }
}

models/powerups/ammo/hmgammo1
{
	nopicmip
    {
	    map textures/effects/envmapmach.tga
		tcGen environment
		blendfunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/powerups/ammo/hmgammo2
{
	nopicmip
	{
		map models/powerups/ammo/hmgammo2.tga
		blendfunc blend
        rgbGen lightingDiffuse
	}
	
}

models/powerups/ammo/ammopack1
{
	nopicmip
    {
		map textures/effects/bronzefx.tga
		tcGen environment
		blendfunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
    }
}

models/powerups/ammo/ammopack
{
	nopicmip
       {
	        map textures/effects/bronzefx.tga
			tcGen environment
			blendfunc GL_ONE GL_ZERO
        rgbGen lightingDiffuse
       }
       {
	        map models/powerups/ammo/ammopack.tga
			blendfunc blend
            rgbGen lightingDiffuse
       }
}

models/powerups/ammo/ammopack2
{
	nopicmip
    {
	    map models/powerups/ammo/ammopack2.tga
		blendfunc blend
        rgbGen lightingDiffuse
    }
}

models/powerups/armor/energy_grn1
{
	deformVertexes wave 100 sin 2 0 0 0
	qer_editorimage textures/effects/envmaprail.tga
	qer_trans 0.3
	{
		map textures/effects/envmaprail.tga
		blendFunc GL_ONE GL_ONE
		tcGen environment
		tcmod rotate 30
		tcMod scroll 1 1
		rgbGen wave triangle -.3 1.3 0 .3
	}
}

models/powerups/armor/energy_gre1
{
	nopicmip
	{
		map models/powerups/armor/energy_gre1.tga 
		blendfunc add
		tcMod scroll 7.4 1.3
	}
}

models/powerups/armor/newgreen
{      
	nopicmip
      {
            map textures/sfx/specular.tga          
            tcGen environment
            rgbGen identity
	}  
     	{
		map models/powerups/armor/newgreen.tga
            blendfunc blend
		rgbGen identity
	}  
}

icons/key_gold
{
	nopicmip
	{
		map icons/key_gold.tga
		blendfunc blend
		alphaGen vertex
	}
}

icons/key_master
{
	nopicmip
	{
		map icons/key_master.tga
		blendfunc blend
		alphaGen vertex
	}
}

icons/key_silver
{
	nopicmip
	{
		map icons/key_silver.tga
		blendfunc blend
		alphaGen vertex
	}
}

models/powerups/keys/key_gold
{
	nopicmip
   {
      map models/powerups/keys/key_gold.tga
      rgbGen lightingDiffuse      
   }
   {
      map models/powerups/keys/envmap-y.tga
      blendFunc add      
      tcGen environment
      rgbGen lightingDiffuse
      tcMod scroll .05 .05
      tcmod scale 2 2
   }
   {
      map models/powerups/keys/envmap-y.tga
      blendFunc add
      tcGen environment
      tcMod scroll .05 .05
      tcmod scale 2 2
      rgbGen wave sin 0 1 0 .5
   }
   {
      map models/powerups/keys/key_gold.tga
      blendFunc blend
      rgbGen lightingDiffuse
   }
}

models/powerups/keys/key_gold_snake
{
	surfaceparm trans	
	cull none	
	qer_trans 0.5	
	nopicmip
	{
		map models/powerups/keys/key_gold_snake.tga
		blendFunc add	
		tcMod scroll -1 .5
		rgbGen wave sin 0 1 0 .5	
	}

}

models/powerups/keys/key_master
{
	nopicmip
	{
		map models/powerups/keys/key_master.tga
        rgbGen lightingDiffuse      
	}
	{
		map models/powerups/keys/envmap-r.tga
		blendFunc add	
		tcGen environment
		rgbGen vertex
		tcMod scroll .05 .05	
		tcmod scale 2 2	
	}	
	{
		map models/powerups/keys/envmap-r.tga
		blendFunc add	
		tcGen environment
		rgbGen vertex
		tcMod scroll .05 .05	
		tcmod scale 2 2
		rgbGen wave sin 0 1 0 .5		
	}
	{
		map models/powerups/keys/key_master.tga
		blendFunc blend
		rgbGen lightingDiffuse	
	}
}

models/powerups/keys/key_master_snake
{
	surfaceparm trans	
	cull none	
	nopicmip
	{
		map models/powerups/keys/key_master_snake.tga
		blendFunc add	
		tcMod scroll -1 .5
		rgbGen wave sin 0 1 0 .5	
	}

}

models/powerups/keys/key_silver
{
	nopicmip
   {
      map models/powerups/keys/key_silver.tga
      rgbGen lightingDiffuse      
   }
   {
      map models/powerups/keys/envmap-b.tga
      blendFunc add      
      tcGen environment
      rgbGen lightingDiffuse
      tcMod scroll .05 .05
      tcmod scale 2 2
   }
   {
      map models/powerups/keys/envmap-b.tga
      blendFunc add
      tcGen environment
      tcMod scroll .05 .05
      tcmod scale 2 2
      rgbGen wave sin 0 1 0 .5
   }
   {
      map models/powerups/keys/key_silver.tga
      blendFunc blend
      rgbGen lightingDiffuse
   }
}

models/powerups/keys/key_silver_snake
{
	surfaceparm trans	
	cull none	
	nopicmip
	{
		map models/powerups/keys/key_silver_snake.tga
		blendFunc add	
		tcMod scroll -1 .5
		rgbGen wave sin 0 1 0 .5	
	}
}

models/weapons2/grapple/f_grapple
{
	nopicmip
	cull disable
	sort additive
	{
		map models/weapons2/grapple/f_grapple.tga
		blendfunc add
	}
}

models/weapons2/grapple/grapple
{      
	nopicmip
    {
		map models/weapons2/grapple/grapple.tga
        blendfunc GL_ONE GL_ZERO
		rgbGen lightingDiffuse
	} 
    {
        map textures/sfx/specular.tga
        blendfunc add
        tcmod scroll .1 .1
        tcGen environment
        rgbGen lightingDiffuse
	}  
    {
		map models/weapons2/grapple/grapple.tga
        blendfunc blend
		rgbGen lightingDiffuse
	}   
	{
		map models/weapons2/grapple/grapple_brightglow.tga
		blendfunc add
		rgbGen wave triangle 1 0.5 0.5 1
	}      
}

models/weapons2/grapple/grapple_h
{
	nopicmip
    cull disable
	{
        map models/weapons2/grapple/grapple_h.tga
        alphaFunc GE128
		depthWrite
		rgbGen lightingDiffuse
    }
}

models/weapons2/grapple/grapple_j
{
	nopicmip
    cull disable
    {
        map models/weapons2/grapple/grapple_j.tga
        blendfunc GL_ONE GL_ZERO
        tcMod scroll 0.5 0
        rgbGen identity
    }
    {
        map textures/liquids/slimeteal.tga
        blendfunc add
        tcmod scale .7 .7
        tcMod turb 0 .4 0 .3
        tcMod scroll .7 -.4
        rgbGen identity
    }
    {
		map textures/effects/tinfx2c.tga
        tcGen environment
        blendfunc add              
        rgbGen lightingDiffuse
	}   
}

grapplingChain
{
	nopicmip
	cull disable
	{
		map gfx/misc/grapplingchain.tga
		alphaFunc GE128
		depthWrite
		rgbGen identity
	}
	{
		map $lightmap
		rgbGen identity
		blendfunc filter
		depthFunc equal
	}
}

icons/weap_hmg
{
	nopicmip
	{
		map icons/weap_hmg.tga
		blendfunc blend
		alphaGen vertex
	}
}

icons/iconw_grapple
{
	nopicmip
	{
		map icons/iconw_grapple.tga
		blendfunc blend
		alphaGen vertex
	}
}

icons/ammo_hmg
{
	nopicmip
	{
		map icons/ammo_hmg.tga
		blendfunc blend
		alphaGen vertex
	}
}

icons/ammo_pack
{
	nopicmip
	{
		map icons/ammo_pack.tga
		blendfunc blend
		alphaGen vertex
	}
}

icons/iconr_green
{
	nopicmip
	{
		map icons/iconr_green.tga
		blendfunc blend
		alphaGen vertex
	}
}

