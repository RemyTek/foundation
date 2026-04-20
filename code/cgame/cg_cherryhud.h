#ifndef CG_CHERRYHUD_H
#define CG_CHERRYHUD_H

#include "cg_local.h"

#ifdef __cplusplus
extern "C" {
#endif


qboolean CG_CHUDLoadConfigWithLexer(const char* filename);
void CG_CHUDLoadConfig(void);
void CG_CHUDRoutine(void);


void CG_CHUDToggleScoreboard(void);


#ifdef __cplusplus
}
#endif

#endif
