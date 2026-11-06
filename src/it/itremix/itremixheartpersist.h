#ifndef _ITREMIXHEARTPERSIST_H_
#define _ITREMIXHEARTPERSIST_H_

#include "it/itremix/itremix.h"

/**
 * SinglePlayerModes.asm heart_persist (RAM 0x8016F5BC): hearts of the All-Star Rest Area never fade or despawn.
 *
 * @param item_gobj Item object.
 * @return TRUE when item_gobj is a heart lying in the Rest Area of an All-Star run, whose pickup wait must not count down.
 */
sb32 itRemixHeartIsDespawnBlocked(GObj *item_gobj);

#endif
