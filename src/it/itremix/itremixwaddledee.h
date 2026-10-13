#ifndef _ITREMIXWADDLEDEE_H_
#define _ITREMIXWADDLEDEE_H_

#ifdef PORT

#include "it/itremix/itremix.h"

/* Item vars shared by every Dedede minion (Waddle Dee, Waddle Doo, Gordo): the shared minion routines read them. */
typedef struct ITRemixWaddleDeeVars
{
    u32 bombhei_smoke_delay_reserved;   /* itBombHeiWalkInitVars writes smoke_delay at item_vars offset 0 */
    s32 throw_unknown;                  /* N64 ITStruct 0x354, set to 4 by throw_initial_; never read */
    GObj *player_gobj;                  /* N64 0x1C4, original owner; restored as owner on landing */
    f32 max_speed;                      /* N64 0x1C8 */
    f32 rotation_direction;             /* N64 0x1CC */
    s32 hitbox_refresh_timer;           /* N64 0x1D0 */
    s32 is_hitbox_collided;             /* N64 0x1D4 */
} ITRemixWaddleDeeVars;

_Static_assert(sizeof(ITRemixWaddleDeeVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixWaddleDeeVars must fit in ITStruct.item_vars");

/* Waddle Dee (Item.WaddleDee.id 0x46) descriptor, read by the Remix item dispatch. */
extern const ITRemixItemDesc dITRemixWaddleDeeItemDesc;

/* Spawns a Waddle Dee for `parent_gobj`. Waddle Doo's descriptor must not use it as SPAWN_ITEM: it would spawn kind 0x46. Use itRemixWaddleDeeSpawnMinion with Doo's ITDesc. */
extern GObj* itRemixWaddleDeeStageSetting(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/* Spawns a minion for `parent_gobj` from `item_desc`. The caller's descriptor is copied and its p_file is set from the owner. */
extern GObj* itRemixWaddleDeeSpawnMinion(GObj *parent_gobj, const ITDesc *item_desc, Vec3f *pos, Vec3f *vel);

/* Shared minion states, used by the Waddle Doo and Gordo ports. */
extern sb32 itRemixWaddleDeeMinionNullMain(GObj *item_gobj);
extern sb32 itRemixWaddleDeeWaddleDeeAerialMain(GObj *item_gobj);
extern sb32 itRemixWaddleDeeWaddleDeeCollision(GObj *item_gobj);
extern sb32 itRemixWaddleDeeMinionHurtboxCollision(GObj *item_gobj);
extern sb32 itRemixWaddleDeeMinionHitboxCollision(GObj *item_gobj);
extern sb32 itRemixWaddleDeeWaddleDeeLandingMain(GObj *item_gobj);
extern sb32 itRemixWaddleDeeWaddleDeeRestingCollision(GObj *item_gobj);
extern sb32 itRemixWaddleDeeMinionWalkMain(GObj *item_gobj);
extern sb32 itRemixWaddleDeeWaddleGroundCollision(GObj *item_gobj);

/* Shared walk setup and owner bookkeeping (Dedede's minion slots). */
extern void itRemixWaddleDeeMinionWalkInitial(GObj *item_gobj);
extern void itRemixWaddleDeeMinionFree(GObj *item_gobj);
extern sb32 itRemixWaddleDeeMinionBlastZone(GObj *item_gobj);
extern void itRemixWaddleDeeRemoveMinionCheck(GObj *item_gobj);

#endif

#endif
