#ifndef _ITREMIXFRANKLINBADGE_H_
#define _ITREMIXFRANKLINBADGE_H_

#include "it/itremix/itremix.h"

/* Spawns Franklin Badge (Remix item 0x33). Fighter call sites use this signature. */
GObj* itRemixFranklinBadgeMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Custom reflect action of the badge hitbox (reflect_initial_). Not yet wired to the reflect system. */
void itRemixFranklinBadgeReflectInitial(GObj *fighter_gobj);

/* TRUE while the player's badge countdown runs (used by the immunity hooks in notes). */
sb32 itRemixFranklinBadgeIsActive(FTStruct *fp);

/* TRUE when a status change must keep the reflect flag (keep_on_action_change_ hook in notes). */
sb32 itRemixFranklinBadgeKeepsReflect(FTStruct *fp);

/* TRUE when the badge absorbs the hitbox of item `ip` held by `fp` (grant_item_immunity_ hook in notes). */
sb32 itRemixFranklinBadgeIsItemImmune(FTStruct *fp, ITStruct *ip);

/* Drops every active badge at scene end (clear_active_franklin_badges_). */
void itRemixFranklinBadgeClearAll(void);

extern const ITRemixItemDesc dITRemixFranklinBadgeItemDesc;

#endif
