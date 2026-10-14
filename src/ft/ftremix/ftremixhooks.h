#ifndef _FTREMIXHOOKS_H_
#define _FTREMIXHOOKS_H_

/*
 * Ported Remix routines that vanilla code calls (REGION_REMIX only).
 *
 * Smash Remix patches vanilla routines (OS.patch_start / jal hooks) to add
 * per-fighter behaviour. The port inserts these calls at the same vanilla
 * sites under #ifdef REGION_REMIX; each routine checks the fighter kind
 * itself and is a no-op for other fighters.
 */

#include <ft/fighter.h>

/* Character.grounded_script / Character.on_hit tables, ftremixchartables.c */
void ftRemixRunGroundedScript(FTStruct *fp);
void ftRemixRunOnHit(FTStruct *fp);

/* Bowser patches on Yoshi's specials, ftremixchartables.c */
void ftRemixBowserUSPLedgeGrabPatch(FTStruct *fp);
sb32 ftRemixBowserDSPSkipsStars(FTStruct *fp);

/* GoemonDSP.item_hurtbox_patch_ in itProcessSearchHitFighter, ftremixchartables.c */
sb32 ftRemixGoemonDSPIgnoresCatchStatus(FTStruct *fp);

/* MarthDSP counter, 0x800EA248 in ftParamUpdateDamage */
s32 ftRemixMarthDSPCounterDamage(FTStruct *fp, s32 damage);

/* SonicUSP.action_check_patch_, 0x80150F10 in ftCommonSpecialAirCheckInterruptCommon */
sb32 ftRemixSonicUSPIsUpSpecialUsed(FTStruct *fp);

/* MewtwoNSP ball_position_fix_ (0x8015D35C) and shoot_velocity_patch_ (0x8015D878) */
sb32 ftRemixMewtwoSpecialNGetBallPosition(FTStruct *fp, Vec3f *pos);
void ftRemixMewtwoSpecialNGetShootRecoil(FTStruct *fp, f32 *recoil_base, f32 *recoil_y_step);

/* DededeNSP inhale_*_ / *_transition_ patches in ftkirbyspecialn.c, applied in ftMainSetStatus */
s32 ftRemixDededeMapKirbyInhaleStatus(FTStruct *fp, s32 status_id);

/* PeachFloat and Peach ledge patches, see build/remixasm/notes/49_PEACH.md */
sb32 ftRemixPeachFloatHoldsHeight(FTStruct *fp);
sb32 ftRemixPeachJumpAerialCheckOrFloat(GObj *fighter_gobj);
sb32 ftRemixPeachFallOverride(GObj *fighter_gobj);
sb32 ftRemixPeachLightThrowCheckItemTypeThrow(FTStruct *fp);
void ftRemixPeachFloatAutoCancelAerial(FTStruct *fp);
void ftRemixPeachEndFloatOnHit(FTStruct *fp);
s32 ftRemixPeachFallSpecialStatus(FTStruct *fp, s32 status_id);
void ftRemixPeachLandingSetLedgeFlag(FTStruct *fp);
void ftRemixPeachGroundedScriptRestoreFloat(FTStruct *fp);
void ftRemixPeachCliffRestoreFloat(FTStruct *fp);

/* MarinaCargo patches: shake_check_ (0x8014DFB4), jump_patch_ (0x8014DB28), air_patch_ (0x8014DACC),
 * jumpsquat_patch_ (0x8014D964), throw_f_patch_ (0x8014E5C0), see build/remixasm/notes/3F_MARINA_NSP.md */
sb32 ftRemixMarinaCargoShakeCheckInterrupt(GObj *fighter_gobj);
s32 ftRemixMarinaCargoJumpStatusId(FTStruct *fp);
f32 ftRemixMarinaCargoAnimSpeed(FTStruct *fp);
s32 ftRemixMarinaCargoThrowFDamage(FTStruct *fp);
void ftRemixMarinaCargoThrowFInitGemSpawned(FTStruct *fp);

/* Marina absorb patches: ftMainSearchHitItem (0x800E5A10) and ftMainSearchHitWeapon (0x800E5330),
 * see build/remixasm/notes/3F_MARINA_DSP.md */
void ftRemixMarinaDSPAbsorbItemCharge(FTStruct *fp, ITStruct *ip);
void ftRemixMarinaDSPAbsorbProjectileCharge(FTStruct *fp, WPStruct *wp);

/* YoungLink, ftremixyounglink.c: each helper returns or does nothing unless the owner is Young Link */
f32 ftRemixYoungLinkUpSpecialAirVelY(FTStruct *fp, f32 vanilla_vel_y);
f32 ftRemixYoungLinkUpSpecialLandingLag(FTStruct *fp, f32 vanilla_lag);
f32 ftRemixYoungLinkBoomerangTiltVel(FTStruct *fp, f32 vanilla_vel);
f32 ftRemixYoungLinkBoomerangSmashVel(FTStruct *fp, f32 vanilla_vel);
void ftRemixYoungLinkBombSetOwner(GObj *item_gobj, FTStruct *owner_fp);

/* Conker, ftremixconkerdsp.c: the active grenade slot (N64 0x0ADC) that the grenade item sets and clears */
void ftRemixConkerDSPSetActiveGrenade(FTStruct *fp, GObj *grenade_gobj);

/* Dedede, ftremixdedededsp.c: the minion slots (N64 0x0ADC/0x0AE0 field, 0x0B20 held) that the minion items update */
void ftRemixDededeDSPAddFieldMinion(GObj *owner_gobj, GObj *minion_gobj);
void ftRemixDededeDSPFreeMinion(GObj *owner_gobj, GObj *minion_gobj);

/* Mad Piano, ftremixpianodsp.c: Ness's absorb enters Piano's own absorb status */
sb32 ftRemixPianoDSPAbsorbRedirect(FTStruct *fp, GObj *fighter_gobj);

/* Lanky, ftremixlankyhooks.c: damage status replacement and the 1P fighter stance bonus */
void ftRemixLankyUpSpecialDamageStatus(FTStruct *fp, s32 *status_id_set, s32 *status_id_var);
sb32 ftRemixLankyIsStanceBonus(s32 fkind, s32 end_status_id);

/* Super Sonic, ftremixssonicusp.c: replaces the proc_interrupt call in ftMainProcUpdateInterrupt */
void ftRemixSSonicUSPInterruptPatch(GObj *fighter_gobj, void (*proc_interrupt)(GObj *));

/* Ebisumaru, ftremixebiusp.c: the up special becomes usable again after a hit */
void ftRemixEbiUSPRefresh(FTStruct *fp);

/* Dark Samus, ftremixdsamushooks.c: each helper keeps the vanilla value unless the fighter is Dark Samus */
void ftRemixDSamusDSPGroundPhysics(FTStruct *fp, f32 *vel, f32 *friction);
f32 ftRemixDSamusDSPAirAccel(FTStruct *fp, f32 vanilla_accel);
f32 ftRemixDSamusDSPGroundJumpVelY(FTStruct *fp, f32 vanilla_vel_y);
s32 ftRemixDSamusDSPAirStatus(FTStruct *fp, s32 vanilla_status_id);
sb32 ftRemixDSamusNSPAirStartIsRelease(FTStruct *fp, sb32 vanilla_is_release);
sb32 ftRemixDSamusNSPKeepsReleaseOnAirSwitch(FTStruct *fp);
sb32 ftRemixDSamusNSPCheckAerialCharge(GObj *fighter_gobj);
const wpSamusChargeShotAttributes *ftRemixDSamusChargeShotAttributes(WPStruct *wp, const wpSamusChargeShotAttributes *vanilla_attrs);

/* Samus clones, ftremixsamusshared.c: the file a Charge Shot or Bomb loads its model from */
void **ftRemixSamusSharedChargeShotFile(FTStruct *fp, void **vanilla_file);
void **ftRemixSamusSharedBombFile(FTStruct *fp, void **vanilla_file);

/* Samus clones, ftremixsamusshared2.c: the Samus branches of fully_charged_check_, kirby_power_change_,
 * kirby_power_check_flash_ and throw_ball_graphic(_2) */
s32 ftRemixSamusSharedChargeSparkleFkind(s32 fkind);
s32 ftRemixSamusSharedKirbyCopyPowerFkind(s32 copy_id);
void ftRemixSamusSharedKirbyCopyChargeFlash(GObj *fighter_gobj);
sb32 ftRemixSamusSharedHasGrappleBeam(s32 fkind);

/* Link clones, ftremixlinkshared.c (linkshared.asm): Link family checks and the J/E Link up special values */
sb32 ftRemixLinkSharedIsLinkFamily(FTStruct *fp);
sb32 ftRemixLinkSharedHasSwordTrail(FTStruct *fp);
f32 ftRemixLinkSharedUpSpecialAirVelY(FTStruct *fp, f32 vanilla_vel_y);
f32 ftRemixLinkSharedUpSpecialFallDrift(FTStruct *fp, f32 vanilla_drift);
void ftRemixLinkSharedUpSpecialWeapon(FTStruct *fp, WPDesc *desc);
void ftRemixLinkSharedBoomerangWeapon(FTStruct *fp, WPDesc *desc);

/* DK and Captain clones, ftremixdkcaptainshared.c (dkshared.asm, captainshared.asm): the cargo, item and
 * damage checks treat JDK, Marina and NMarina as Donkey Kong; the Falcon-family clones use their own
 * kick and punch files and the Falcon hand bone */
sb32 ftRemixDKSharedIsCargoClone(s32 fkind);
sb32 ftRemixCaptainSharedUsesFalconHandBone(s32 fkind);
EFDesc* ftRemixCaptainSharedKickDesc(FTStruct *fp, EFDesc *vanilla_desc);
EFDesc* ftRemixCaptainSharedPunchDesc(FTStruct *fp, EFDesc *vanilla_desc);

/* Mario fireball clones, wp/wpmario/wpmarioremixfireball.c (fireball.asm): fkind -> fireball index, projectile file,
 * capsule palette, capsule/book hit and bounce effects, banjo egg bounce threshold and the Piano book spawn */
s32 wpRemixMarioFireballIndexForFkind(s32 fkind);
void **wpRemixMarioFireballFile(s32 file_fkind);
s32 wpRemixMarioFireballPaletteId(FTStruct *fp, s32 palette_base, s32 palette_range);
f32 wpRemixMarioFireballMinVel(WPStruct *wp, f32 vanilla_min_vel);
sb32 wpRemixMarioFireballHitEffect(GObj *weapon_gobj);
sb32 wpRemixMarioFireballBounceEffect(GObj *weapon_gobj);
GObj* wpRemixPianoBookMake(GObj *fighter_gobj, Vec3f *pos);

/* Entry script of every synth fighter, ftremixentry.c: installed as the descriptor's entry_make_effect */
void ftRemixEntryMakeEffect(FTStruct *fp);

/* captainshared.asm get_entry_file_ptr_ and clown_car_animation1/2: the file and the three animation joint offsets
 * efManagerCaptainEntryCarMakeEffect reads; the arguments hold the vanilla values on entry */
void ftRemixEntryGetCarAnim(void **car_file, intptr_t *o_car_anim, intptr_t *o_wheel_anim_l, intptr_t *o_wheel_anim_r);

/* Wolf.asm wolfen_entry_2/_3: the two files and the three animation joint offsets efManagerFoxEntryArwingMakeEffect
 * reads; the arguments hold the vanilla values on entry */
void ftRemixEntryGetArwingAnim(void **wing_file, intptr_t *o_wing_anim, void **body_file, intptr_t *o_body_anim_r, intptr_t *o_body_anim_l);

/* Yoshi clones, ftremixyoshishared.c (yoshishared.asm): the egg and star descriptors point at the clone's own file */
WPDesc *ftRemixYoshiEggWeaponDesc(FTStruct *fp, WPDesc *vanilla_desc);
WPDesc *ftRemixYoshiStarWeaponDesc(FTStruct *fp, WPDesc *vanilla_desc);

/* Jigglypuff and Kirby clones, ftremixpuffkirbyshared.c (jigglypuffkirbyshared.asm, pipe_turn of marioshared.asm):
 * Dedede jump multiplier, pipe turn table, Kirby copy ammo and J Kirby cutter descriptor */
f32 ftRemixDededeJumpAerialMultiplier(s32 jumps_used);
sb32 ftRemixIsPipeTurnFkind(s32 fkind);
void ftRemixKirbyCopyAmmo(FTStruct *fp);
WPDesc *ftRemixKirbyCutterWeaponDesc(FTStruct *fp, WPDesc *vanilla_desc);
s32 ftRemixKirbyStoneHealthMax(FTStruct *fp, s32 vanilla_max);
s32 ftRemixKirbyStoneHealthMid(FTStruct *fp, s32 vanilla_mid);
s32 ftRemixKirbyStoneReleaseDuration(FTStruct *fp, s32 vanilla_duration);

#endif
