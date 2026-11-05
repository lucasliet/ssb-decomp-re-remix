#ifndef _FTREMIXNESSSHARED_H_
#define _FTREMIXNESSSHARED_H_

/*
 * Smash Remix patches of the Ness family (src/nessshared.asm, src/Lucas/Lucas.asm) that vanilla code calls
 * (REGION_REMIX only): J Ness and Lucas PK Fire and PK Thunder (own files, speeds, landing, multi-hit) and the Lucas PSI
 * Magnet. The values are in ftremixnessrules.h. Each helper keeps the vanilla value for every other fighter.
 *
 * Implemented in ftremixnessshared.c and lucas/ftremixlucas*.c, called from wp/wpness, it/itfighter/itnesspkfire.c,
 * ft/ftchar/ftness, ft/ftchar/ftkirby/ftkirbycopynessspecialn.c and (the model file helper) ef/efmanager.c.
 */

#include <ft/fighter.h>
#include <wp/weapon.h>
#include <it/item.h>

/** PK Fire launch values read from the Ness moves: whether the air launch is used and the two speeds. */
typedef struct FTRemixNessPKFireLaunch
{
    sb32 is_air;
    f32 speed_air;
    f32 speed_ground;

} FTRemixNessPKFireLaunch;

/** The fighter kind whose moves `fp` uses: the fighter itself, or the power a Kirby or J Kirby copied. */
s32 ftRemixNessSharedPowerKind(FTStruct *fp);

/** pkfire_horizontal, pkfire_*_speed_*_: adjusts the vanilla PK Fire launch values of `fp` (is_air set from fp->ga). */
void ftRemixNessSharedPKFireLaunch(FTStruct *fp, FTRemixNessPKFireLaunch *launch);

/** get_pkfire_pointer_: points the PK Fire weapon descriptor copy `desc` at the J Ness or Lucas PK Fire file. */
void ftRemixNessSharedPKFireWeaponDesc(FTStruct *fp, WPDesc *desc);

/** get_pkfire_pointer_: remembers in the PK Fire weapon the power kind of its owner (the asm projectile word 0x78). */
void ftRemixNessSharedPKFireSetKind(WPStruct *wp, FTStruct *fp);

/** get_pkfire_collision_pointer_: points the pillar item descriptor copy `desc` at the J Ness PK Fire file. */
void ftRemixNessSharedPKFireItemDesc(WPStruct *wp, ITDesc *desc);

/** get_pillar_playerstruct_: copies the power kind of the PK Fire weapon to the pillar item (the asm item word 0x354). */
void ftRemixNessSharedPKFireItemSetKind(ITStruct *ip, WPStruct *wp);

/** pkfire_pillar_gravity_: the pillar gravity and terminal velocity (J Ness has its own). */
void ftRemixNessSharedPKFirePillarPhysics(ITStruct *ip, f32 *gravity, f32 *tvel);

/** pkfire_explosion (Lucas.asm): a Lucas PK Fire hit makes a white sparkle at `pos`. Returns TRUE if it did. */
sb32 ftRemixNessSharedPKFireExplode(WPStruct *wp, Vec3f *pos);

/** up_special_ending_delay_: frames before PK Thunder ends once the head is gone. */
s32 ftRemixNessSharedPKThunderEndDelay(FTStruct *fp, s32 vanilla_delay);

/** up_special_landing_fsm_: air drift and landing lag after the aerial end of PK Thunder (arguments in/out). */
void ftRemixNessSharedPKThunderLanding(FTStruct *fp, f32 *drift, f32 *landing_lag);

/** get_pkthunder_special_struct1_: head descriptor copy for the controlled PK Thunder of `fp` (J Ness, Lucas files). */
void ftRemixNessSharedPKThunderHeadDesc(FTStruct *fp, WPDesc *desc);

/** get_pkthunder_special_struct2_: trail descriptor copy for the controlled PK Thunder of `fp`. */
void ftRemixNessSharedPKThunderTrailDesc(FTStruct *fp, WPDesc *desc);

/** get_pkthunder_reflect_struct1_: reflected head descriptor copy; `old_head_gobj` is the head that was reflected. */
void ftRemixNessSharedPKReflectHeadDesc(GObj *old_head_gobj, WPDesc *desc);

/** get_pkthunder_reflect_struct1_: the reflected head remembers the fighter that fired the PK Thunder it replaces. */
void ftRemixNessSharedPKReflectHeadSetParent(GObj *new_head_gobj, GObj *old_head_gobj);

/** get_pkthunder_reflect_struct2_: reflected trail descriptor copy; `old_gobj` is the head or previous trail. */
void ftRemixNessSharedPKReflectTrailDesc(GObj *old_gobj, s32 trail_id, WPDesc *desc);

/** pk_thunder_reflect_crash_fix_1_/2_: the Toggles.entry_pk_thunder_reflect_crash_fix value. */
sb32 ftRemixNessSharedPKReflectFixEnabled(void);

/** get_pkthunder_anim_struct_*: the loaded model file (Remix file 4) PK Thunder gfx of `kind` read. For efmanager.c. */
void **ftRemixNessSharedModelFile(s32 kind, void **vanilla_file);

/** usp_thunder_head_: the Lucas PK Thunder head takes part in the trail hit group. */
void ftRemixLucasUSPInitHead(WPStruct *head_wp);

/** usp_thunder_tail_: group id of the first trail, the head's one for Lucas, a new one otherwise. */
u32 ftRemixLucasUSPTrailGroupID(WPStruct *head_wp);

/** usp_multi_hit_: forgets the victims of the Lucas PK Thunder group every few frames. Call before the lifetime decrement. */
void ftRemixLucasUSPRefreshHits(GObj *weapon_gobj);

/** dsp_absorption_turnaround_prevent: TRUE if the PSI Magnet absorb keeps the facing of `fp`. */
sb32 ftRemixLucasDSPKeepsFacing(FTStruct *fp);

/** dsp_absorption_position_fix_: Lucas's own PSI Magnet absorb sphere, `vanilla_coll` for every other fighter. */
FTSpecialColl* ftRemixLucasDSPSpecialColl(FTStruct *fp, FTSpecialColl *vanilla_coll);

/**
 * The PSI Magnet absorb sphere of `fp`: Lucas's own, the Ness one of the main motion file for the others. J Ness and
 * N Lucas read it from their own copy of that file (file 0x0EE) so it does not depend on Ness being loaded.
 */
FTSpecialColl* ftRemixNessSharedPsiMagnetColl(FTStruct *fp);

#endif
