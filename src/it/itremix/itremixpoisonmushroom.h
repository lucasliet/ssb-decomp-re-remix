#ifndef _ITREMIXPOISONMUSHROOM_H_
#define _ITREMIXPOISONMUSHROOM_H_

#include "it/itremix/itremix.h"

/* Spawns Poison Mushroom (Remix item 0x2F). Fighter call sites use this signature. */
GObj* itRemixPoisonMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Poison-side clear for the scene-end aggregator. The asm has no PoisonMushroom clear:
 * Item.asm clear_active_custom_items_ resets the shared mushroom tables once through
 * SuperMushroom.clear_active_mushrooms_ (ported as itRemixSuperMushroomClearStates, which this
 * forwards to). The separate Poison.clear_poison_ of src/Poison.asm clears the unrelated
 * poisoned_players status table and has no port yet. */
void itRemixPoisonMushroomClearActive(void);

extern const ITRemixItemDesc dITRemixPoisonMushroomItemDesc;

#endif
