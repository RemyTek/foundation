# foundation

Unofficial Quake III Arena gamecode patch

# Current state:

## Got FB skins working with team-based gametypes:

https://user-images.githubusercontent.com/290740/230755343-e4a1347e-c6bc-4b59-83f9-8d8c2620ce44.mp4

## Added instagib support:

https://user-images.githubusercontent.com/290740/230708646-ef9bd016-2284-4ca0-9efb-65e1457297a7.mp4

## Disabled screen shake when taking damage:

https://user-images.githubusercontent.com/290740/230672040-74bcfa4d-9400-4be6-a102-5333569f0030.mp4


# What is done:

 * new toolchain used (optimized q3lcc and q3asm)
 * upstream security fixes
 * floatfix
 * fixed vote system
 * fixed spawn system
 * fixed in-game crosshair proportions
 * fixed UI mouse sensitivity for high-resolution
 * fixed server browser + faster scanning
 * new demo UI (subfolders,filtering,sorting)
 * updated serverinfo UI
 * map rotation system
 * unlagged weapons
 * improved prediction
 * damage-based hitsounds
 * colored skins
 * high-quality proportional font renderer
 * Kr3m's custom ctf_unified.shader for popping decals at any picmip setting.
 * CPMA-style fullbright skins
 * linux qvm build support
 * cg_crosshairColor support (seta cg_crosshairColor "0x00ff00ff" //for green)
 * cg_kickScale support with a default value of 0 to disable screen shake when taking damage.

# TODO:

 * bugfixes
 * add projectile delag code
 * rework crosshair colors to support color string names and q3 color codes (e.g. "Green" or "2").
 * add ruleset and crontab support similar to excessiveplus, edawn, etc.
 * add more gametypes
 * add fullbright skin support for team games.

# Documentation

See /docs/

# Compilation and installation

Look in /build/
