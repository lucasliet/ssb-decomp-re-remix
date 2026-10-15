#ifndef _ITREMIXSUPERMUSHROOM_H_
#define _ITREMIXSUPERMUSHROOM_H_

#include "it/itremix/itremix.h"

/* Item.SuperMushroom.id and Item.PoisonMushroom.id (Item.asm add_item 0x2E/0x2F). Both items
 * share one effect state (asm SuperMushroom.asm player_shrooms and spawning_shrooms), so the
 * spawn-velocity, setup, movement, ground and touch routines below are the shared implementation
 * for the two mushrooms. */
#define ITREMIX_SUPERMUSHROOM_KIND 0x2E
#define ITREMIX_SUPERMUSHROOM_POISON_KIND 0x2F

/* Spawns Super Mushroom (Remix item 0x2E). Fighter call sites use this signature. */
GObj* itRemixSuperMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Spawn velocity of spawn_custom_item_based_on_star_: horizontal speed 16 + 2 per mushroom spawned
 * this frame (asm spawning_shrooms, shared by both items and reset by the shared movement routine). */
void itRemixSuperMushroomGetSpawnVel(Vec3f *pos, Vec3f *vel);

/* Tail of spawn_custom_item_based_on_star_ after itManagerMakeItem: fighter interact mask, 0x20
 * active delay, grounded flag and render XObj. Shared by both mushrooms. */
void itRemixSuperMushroomSetupItem(GObj *item_gobj);

/* movement_routine_no_bounce_ (ITDesc word 0x14 of both mushrooms): Star update with gravity
 * skipped while grounded; also resets spawning_shrooms. */
sb32 itRemixSuperMushroomMovementProcUpdate(GObj *item_gobj);

/* ground_collision_no_bounce_ (ITDesc word 0x18 of both mushrooms): Star ground collision without
 * the floor bounce. */
sb32 itRemixSuperMushroomGroundProcMap(GObj *item_gobj);

/* collide_mushroom_ (PLAYER_COLLISION of both mushrooms; asm PoisonMushroom.asm reuses it):
 * registers the per-port effect routine and dispatches on ip->kind. */
void itRemixSuperMushroomProcTouch(FTStruct *fp, ITStruct *ip, GObj *item_gobj);

/* Frees every active mushroom routine of both kinds and forgets the per-port owners. Meant for
 * the scene-change hook (see notes, Vanilla patches). */
void itRemixSuperMushroomClearStates(void);

/* TRUE while a mushroom effect of either kind runs on the player (asm player_shrooms[port] != 0). */
sb32 itRemixSuperMushroomIsActive(s32 player);

/* Destroys the player's mushroom effect routine without restoring its size (Lightning _zap_player). */
void itRemixSuperMushroomDestroyForPlayer(s32 player);

extern const ITRemixItemDesc dITRemixSuperMushroomItemDesc;

#endif
