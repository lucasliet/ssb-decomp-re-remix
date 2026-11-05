#ifndef _GRREMIXJUNGLEBARRELS_H_
#define _GRREMIXJUNGLEBARRELS_H_

/*
 * Barrels of the Remix stages (Hazards.asm congo_jungle_barrel_, add_barrel_, btp_bomb_or_barrel_setup). Congo Jungle, its
 * Remix clones and Congo Falls make the normal barrel only with the hazards on; Smashketball gets two static barrels pointing
 * down; Banjo's Board the Platforms gets 12 barrels that follow animation tracks of its stage file.
 */

#include <ssb_types.h>
#include <sys/objdef.h>

/** Body of grJungleMakeGround on the Remix build: decides and makes the barrels of the current stage. */
void grRemixJungleBarrelsMakeGround(void);

/** btp_bomb_or_barrel_setup (_banjo): makes the 12 barrels of Banjo's Board the Platforms; no-op on other stages. */
void grRemixJungleBarrelsMakeBanjo(void);

/**
 * check_basketball_barrel_collision_: the first barrel of the stage the ball overlaps.
 *
 * @param item_gobj Basketball object.
 * @return The barrel object, or NULL when the ball touches none.
 */
GObj* grRemixJungleBarrelsFindForBasketball(GObj *item_gobj);

#endif
