#ifndef _ITREMIXBASKETBALL_H_
#define _ITREMIXBASKETBALL_H_

#include "it/itremix/itremix.h"

/* Spawns the Smashketball basketball (Remix item 0x40). Fighter and mode call sites use this signature. */
GObj* itRemixBasketballMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags);

/*
 * Basketball.asm vanilla hooks (REGION_REMIX only). Each routine checks the
 * item kind itself and is a no-op for anything that is not the basketball.
 */

/* Basketball.bounce_patch_ (four jal hooks in itMapCheckCollideAllRebound):
 * squashes the joint, offsets the translate, sets the bounce hitlag, plays the
 * bounce FGM, then runs the vanilla lbCommonReflect2D. */
void itRemixBasketballApplyBounce(GObj *item_gobj, Vec3f *angle, f32 x_scale, f32 y_scale, f32 x_offset, f32 y_offset);

/* Basketball multiply_player_hitlag_ / multiply_ball_hitlag_hurtbox_ /
 * add_ball_hitlag_hitbox_: TRUE when item_gobj is the basketball with the fire
 * state on (asm ITStruct 0x1D4). */
sb32 itRemixBasketballIsFire(GObj *item_gobj);

/* Basketball bounce_patch_ / prevent_bounce_fgm_: TRUE when item_gobj is the
 * basketball with the bounce squash applied (asm ITStruct 0x1E4). */
sb32 itRemixBasketballIsBounceScaled(GObj *item_gobj);

/* Smashketball.asm basketball_main_: the ball object in play (NULL when none), its position (the owner's item joint while it
 * is held) and one frame of basket or goal detection. */
GObj* itRemixBasketballFind(void);
void itRemixBasketballGetPosition(GObj *item_gobj, Vec3f *pos);
sb32 itRemixBasketballCheckHoop(GObj *item_gobj, f32 baskets_y, sb32 is_soccer, s32 *side);
sb32 itRemixBasketballIsGoldenBall(GObj *item_gobj);

/* Smashketball.asm camera_follow: TRUE while the ball respawns. Smashketball.asm prevent_despawn: TRUE when the item is the
 * ball of a Smashketball match, whose pickup wait must not count down. */
sb32 itRemixBasketballIsRespawning(GObj *item_gobj);
sb32 itRemixBasketballIsDespawnBlocked(GObj *item_gobj);

/* Basketball.asm prevent_fighter_enter_barrel_: TRUE while the ball of a Smashketball match is inside a barrel. */
sb32 itRemixBasketballIsBarrelOccupied(void);

extern const ITRemixItemDesc dITRemixBasketballItemDesc;

#endif
