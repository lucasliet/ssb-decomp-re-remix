#ifndef _GRJUNGLE_H_
#define _GRJUNGLE_H_

#include <ssb_types.h>
#include <sys/objdef.h>
#include <gr/grdef.h>

extern void grJungleTaruCannAddAnimOffset(GObj *ground_gobj, intptr_t offset);
extern void grJungleTaruCannAddAnimFill(GObj *ground_gobj);
extern void grJungleTaruCannAddAnimShoot(GObj *ground_gobj);
extern void grJungleTaruCannUpdateMove(GObj *ground_gobj);
extern void grJungleTaruCannUpdateRotate(GObj *ground_gobj);
extern void grJungleTaruCannProcUpdate(GObj *ground_gobj);
extern void grJungleMakeTaruCann(void);
extern GObj* grJungleMakeGround(void);
extern sb32 grJungleTaruCannCheckGetDamageKind(GObj *ground_gobj, GObj *fighter_gobj, s32 *kind);
extern void grJungleTaruCannGetPosition(Vec3f *pos);
extern f32 grJungleTaruCannGetRotate(void);

#ifdef REGION_REMIX
/* Hazards.asm add_barrel_, barrel_angle_fix_ and Basketball.asm check_basketball_barrel_collision_ */
extern GObj* grJungleRemixMakeTaruCannAt(void *map_head, f32 x, f32 y, f32 rotate_z, AObjEvent32 **anim_joints);
extern sb32 grJungleTaruCannCheckBasketball(GObj *ground_gobj, GObj *item_gobj);
extern f32 grJungleTaruCannGetRotateOf(GObj *tarucann_gobj);
#endif

#endif
