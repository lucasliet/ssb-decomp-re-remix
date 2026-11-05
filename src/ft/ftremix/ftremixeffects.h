#ifndef _FTREMIXEFFECTS_H_
#define _FTREMIXEFFECTS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Fighter effect patches that vanilla effect and capture code calls (REGION_REMIX only): the Dedede inhale wind and
 * captured timer, the Wolf and Slippy reflector, the Ness-family PK Thunder and PSI Magnet gfx files (J Ness, Lucas), the
 * Pikachu clone Thunder and Thunder Jolt gfx files and the Falcon punch announcement. The values are in
 * ftremixeffectsrules.h. Each helper keeps the vanilla answer for every other fighter.
 *
 * Implemented in ftremixeffects.c and ftremixdkcaptainshared.c, called from ef/efmanager.c and
 * ft/ftcommon/ftcommoncapturekirby.c. The `...Desc` helpers return `vanilla_desc` itself or a static copy of it that the
 * caller passes to efManagerMakeEffect right away.
 */

#include <ft/fighter.h>
#include <ef/efdef.h>

/* Dedede.asm dedede_gfx_: adds to the inhale wind transform the extra x offset Dedede has over the Kirby routine */
void ftRemixDededeInhaleWindOffset(LBTransform *xf, GObj *fighter_gobj);

/* Dedede.asm custom_initial_absorbed_timer_: breakout wait of `captured_fp` just inhaled (500 in the vanilla function) */
s32 ftRemixDededeCaptureWaitBreakout(FTStruct *captured_fp, s32 vanilla_wait);

/* Wolf.asm wolf_reflect_graphic: the reflector desc of Wolf and Slippy */
EFDesc* ftRemixReflectorDesc(FTStruct *fp, EFDesc *vanilla_desc);

/* Wolf.asm wolf_reflector_graphic_subroutine: animation joint `anim_id` of the reflector effect (Wolf and Slippy read their own file 7) */
AObjEvent32** ftRemixReflectorAnimJoint(GObj *effect_gobj, s32 anim_id, AObjEvent32 **vanilla_anim_joint);

/* nessshared.asm get_pkthunder_anim_struct_: PK Thunder head gfx desc of `fp` (Lucas also has its own offsets) */
EFDesc* ftRemixNessPKThunderWaveDesc(FTStruct *fp, EFDesc *vanilla_desc);

/* nessshared.asm get_pkthunder_anim_struct3_: PK Thunder trail gfx desc of `fp` */
EFDesc* ftRemixNessPKThunderTrailDesc(FTStruct *fp, EFDesc *vanilla_desc);

/* nessshared.asm get_pkthunder_anim_struct2_: reflected PK Thunder trail gfx desc; `head_gobj` is the reflected head */
EFDesc* ftRemixNessPKReflectTrailDesc(GObj *head_gobj, EFDesc *vanilla_desc);

/* Ness family: the PSI Magnet gfx desc reads the file 6 of J Ness, Lucas and N Lucas instead of the Ness globals */
EFDesc* ftRemixNessPsiMagnetDesc(FTStruct *fp, EFDesc *vanilla_desc);

/* Lucas.asm dsp_graphic_position_fix_: Lucas's PSI Magnet gfx x translation. Call before the Size initialize */
void ftRemixLucasPsiMagnetPlaceGfx(GObj *effect_gobj, FTStruct *fp);

/* Lucas.asm usp_graphic_position_fix_: TRUE if the PK Thunder head gfx of `fp` keeps its y rotation at 0 (Lucas) */
sb32 ftRemixLucasPKThunderGfxKeepsRotation(FTStruct *fp);

/* pikashared.asm get_thunder_anim_struct_: Thunder trail gfx desc of the fighter `owner_fkind` (E and J Pikachu own model file) */
EFDesc* ftRemixPikaThunderTrailDesc(s32 owner_fkind, EFDesc *vanilla_desc);

/* pikashared.asm get_thunder_jolt_anim_struct_: Thunder Jolt gfx desc of the ground Thunder Jolt `jolt_gobj` (J Pikachu own file 8) */
EFDesc* ftRemixPikaThunderJoltDesc(GObj *jolt_gobj, EFDesc *vanilla_desc);

/* Fighter kind of the owner of `weapon_gobj`, or -1 if it has none */
s32 ftRemixWeaponOwnerKind(GObj *weapon_gobj);

/* captainshared.asm get_punch_anim_struct_: plays the Falcon punch announcement for the power `fp` uses */
void ftRemixCaptainSharedPunchAnnounce(FTStruct *fp);

#endif

#endif
