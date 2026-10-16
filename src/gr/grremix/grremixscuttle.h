#ifndef _GRREMIXSCUTTLE_H_
#define _GRREMIXSCUTTLE_H_

/*
 * Scuttle Town setup (Hazards.asm scuttle_town_setup_, scuttle_town_bg_fade_): a full screen fade that switches the
 * night and day backgrounds when the stage palette changes, and the Scuttle Minion factory.
 */

#include <ssb_types.h>

#ifdef PORT

/**
 * Creates the Scuttle Town background fade (skipped in training mode unless the training stage background is shown)
 * and, when hazards are on, the Scuttle Minion factory. The caller is the Scuttle Town dispatch in
 * grremixhazards_scuttle.c.
 */
void grRemixScuttleTownSetup(void);

#endif

#endif
