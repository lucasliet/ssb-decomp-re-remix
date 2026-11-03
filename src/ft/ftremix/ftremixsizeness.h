#ifndef _FTREMIXSIZENESS_H_
#define _FTREMIXSIZENESS_H_

/*
 * Smash Remix body size for Ness's DSP (Psychic Magnet) and USP (PK Thunder wave) gfx (REGION_REMIX only).
 * Implemented in ftremixsizeness.c, called from efmanager.c.
 */

#include <ft/fighter.h>

void ftRemixSizeNessDSPInitialize(GObj *effect_gobj);
void ftRemixSizeNessDSPProcUpdate(GObj *effect_gobj);
void ftRemixSizeNessUSPInitialize(GObj *effect_gobj);
void ftRemixSizeNessUSPProcUpdate(GObj *effect_gobj);

#endif
