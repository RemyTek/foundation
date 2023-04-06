# foundation

Unofficial Quake III Arena gamecode patch

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
 * CPMA fullbright skins
 * linux qvm build support
 * cg_crosshairColor support (seta cg_crosshairColor "0x00ff00ff" //for green)
 * high-quality proportional font renderer
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
