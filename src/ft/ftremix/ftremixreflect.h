#ifndef _FTREMIXREFLECT_H_
#define _FTREMIXREFLECT_H_

/*
 * Smash Remix Reflect.asm: reflect hitboxes of type CUSTOM (halfword 2 of the hitbox == 3) carry a custom id in
 * halfword 0 that selects the routine run when the fighter reflects: 0 Franklin Badge, 1 Marina absorb, 2 Dedede absorb.
 * FTSpecialColl.kind holds both halfwords: (custom id << 16) | reflect type.
 */

#include <ssb_types.h>
#include <ft/fttypes.h>
#include <it/ittypes.h>
#include <wp/wptypes.h>

#define FTREMIX_REFLECT_TYPE_CUSTOM 3

typedef enum FTRemixReflectResult
{
    nFTRemixReflectOriginal,    /* run the vanilla reflect routine of the projectile or item */
    nFTRemixReflectHandled,     /* the custom reflect routine took over; the object lives on */
    nFTRemixReflectDestroy      /* the custom reflect routine took over; destroy the object */

} FTRemixReflectResult;

/** TRUE when the fighter's reflect hitbox is of type CUSTOM. */
sb32 ftRemixReflectIsCustom(FTStruct *fp);

/** override_reflectability_: whether the projectile can be reflected by this fighter's hitbox. */
sb32 ftRemixReflectCanReflectWeapon(FTStruct *fp, WPStruct *wp);

/** extend_reflect_types: runs the custom reflect routine of the fighter's hitbox (the fighter reflected something). */
void ftRemixReflectRunCustom(GObj *fighter_gobj);

/** override_projectile_reflect_routine_: custom absorb of a reflected projectile. */
FTRemixReflectResult ftRemixReflectWeapon(GObj *weapon_gobj);

/** override_item_reflect_routine_: custom absorb of a reflected item. */
FTRemixReflectResult ftRemixReflectItem(GObj *item_gobj);

/** projectile_direction_fix_custom_ / item_direction_fix_custom_: custom reflect hitboxes simply invert the horizontal speed. */
sb32 ftRemixReflectInvertsDirection(FTStruct *fp);

#endif
