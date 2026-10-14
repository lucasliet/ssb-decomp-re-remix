#ifndef _ITREMIX_CACODEMON_H_
#define _ITREMIX_CACODEMON_H_

/*
 * Smash Remix Cacodemon item (id 0x3F), ported from src/items/Cacodemon.asm and the
 * cacodemon_* routines of src/Hazards.asm. The Spawned Fear hazard spawns it
 * (grremixcacodemonfactory.c). The descriptor is dITRemixCacodemonItemDesc in itremixcacodemon.c.
 */

#include "it/itremix/itremix.h"

#define ITREMIX_CACODEMON_KIND 0x3F /* Item.Cacodemon.id */

typedef struct ITRemixCacodemonVars
{
    u16 spawn_timer;                /* asm 0x1C8: counts up to the loop start frame (0x3C) */
    u16 loop_timer;                 /* asm 0x1CA: frame of the attack loop, restarts at 0x20C */
    GObj *target_gobj;              /* asm 0x1C0: fighter the next Thunderball aims at */
    f32 target_x_diff;              /* asm 0x1C4: |target x - Cacodemon x| of target_gobj */
} ITRemixCacodemonVars;

_Static_assert(sizeof(ITRemixCacodemonVars) <= sizeof(((ITStruct *) 0)->item_vars), "ITRemixCacodemonVars must fit in item_vars");

#define ITREMIX_CACODEMON_VARS(ip) ((ITRemixCacodemonVars *) &(ip)->item_vars)

/** Sets the Cacodemon file top: the base its item attributes and display lists are read from. */
void itRemixCacodemonSetFileTop(void *file_top);

/** SPAWN_ITEM of the Cacodemon (cacodemon_stage_setting_). */
GObj* itRemixCacodemonMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

#endif
