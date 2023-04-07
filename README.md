# foundation

Unofficial Quake III Arena gamecode patch

# current state




https://user-images.githubusercontent.com/290740/230672040-74bcfa4d-9400-4be6-a102-5333569f0030.mp4


# What is done:

 * new toolchain used (optimized q3lcc and q3asm)
 * upstream security fixes
 * floatfix
 * fixed vote system
 * fixed spawn system
 * fixed in-game crosshair proportions
 * fixed UI mouse sensitivity for high-resolution

https://user-images.githubusercontent.com/290740/230671995-3eab0fa8-96c3-4aa6-9936-d2acaa8c26f8.mp4


 * fixed server browser + faster scanning
 * new demo UI (subfolders,filtering,sorting)
 * updated serverinfo UI
 * map rotation system
 * unlagged weapons
 * improved prediction
 * damage-based hitsounds
 * colored skins
 * high-quality proportional font renderer
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

# Documentation

See /docs/

# Compilation and installation

Look in /build/
