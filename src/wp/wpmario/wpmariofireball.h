#ifndef _WPMARIOFIREBALL_H_
#define _WPMARIOFIREBALL_H_

#include <ssb_types.h>
#include <sys/objdef.h>
#include <wp/wpdef.h>

extern sb32 wpMarioFireballProcUpdate(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcMap(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcHit(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcHop(GObj *weapon_gobj);
extern sb32 wpMarioFireballProcReflector(GObj *weapon_gobj);
extern GObj* wpMarioFireballMakeWeapon(GObj *fighter_gobj, Vec3f *pos, s32 index);
extern GObj* wpMarioFireballMakeWeaponAtAngle(GObj *fighter_gobj, Vec3f *pos, s32 index, f32 angle);

#ifdef REGION_REMIX
/* Index into dWPMarioFireballWeaponAttributes; the order follows the struct order in fireball.asm (ids 2..6). */
#define WP_MARIO_FIREBALL_INDEX_MARIO    0
#define WP_MARIO_FIREBALL_INDEX_LUIGI    1
#define WP_MARIO_FIREBALL_INDEX_CAPSULE  2
#define WP_MARIO_FIREBALL_INDEX_JMARIO   3
#define WP_MARIO_FIREBALL_INDEX_JLUIGI   4
#define WP_MARIO_FIREBALL_INDEX_BOOK     5
#define WP_MARIO_FIREBALL_INDEX_DRL      6
#endif

#endif
