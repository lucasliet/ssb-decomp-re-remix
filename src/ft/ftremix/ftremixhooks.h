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

/* PeachFloat and Peach ledge patches, see docs/remix/notes/49_PEACH.md */
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
 * jumpsquat_patch_ (0x8014D964), throw_f_patch_ (0x8014E5C0), see docs/remix/notes/3F_MARINA_NSP.md */
sb32 ftRemixMarinaCargoShakeCheckInterrupt(GObj *fighter_gobj);
s32 ftRemixMarinaCargoJumpStatusId(FTStruct *fp);
f32 ftRemixMarinaCargoAnimSpeed(FTStruct *fp);
s32 ftRemixMarinaCargoThrowFDamage(FTStruct *fp);
void ftRemixMarinaCargoThrowFInitGemSpawned(FTStruct *fp);

/* Marina absorb patches: ftMainSearchHitItem (0x800E5A10) and ftMainSearchHitWeapon (0x800E5330),
 * see docs/remix/notes/3F_MARINA_DSP.md */
void ftRemixMarinaDSPAbsorbItemCharge(FTStruct *fp, ITStruct *ip);
void ftRemixMarinaDSPAbsorbProjectileCharge(FTStruct *fp, WPStruct *wp);

/* Reflect.custom_reflect_table routines (ftremixreflect.c): MarinaDSP.absorb_initial_ and DededeNSP.absorb_initial_ */
void ftRemixMarinaDSPAbsorbInitial(GObj *fighter_gobj);
void ftRemixDededeNSPAbsorbInitial(GObj *fighter_gobj);

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
void **ftRemixLinkSharedBombFile(s32 fkind, void **vanilla_file);

/* DK and Captain clones, ftremixdkcaptainshared.c (dkshared.asm, captainshared.asm): the cargo, item and
 * damage checks treat JDK, Marina and NMarina as Donkey Kong; the Falcon-family clones use their own
 * kick and punch files and the Falcon hand bone */
sb32 ftRemixDKSharedIsCargoClone(s32 fkind);
sb32 ftRemixCaptainSharedUsesFalconHandBone(s32 fkind);
s32 ftRemixFalconDiveOffsetRow(s32 fkind);
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

/* Samus clones, ftremixsamusshared2.c (samusshared.asm get_grapple_anim_gfx_struct): the grapple glow descriptor
 * efManagerSamusGrappleBeamGlowMakeEffect spawns; Dark Samus gets its own file, other fighters the vanilla one */
struct EFDesc *ftRemixSamusSharedGrappleBeamDesc(GObj *fighter_gobj, struct EFDesc *vanilla_desc);

/* J Kirby grounded_script, ftremix/jkirby/ftremixjkirby.c (Kirby.asm clear_marth_flag_) */
void ftRemixJKirbyGroundedScript(FTStruct *fp);

/* Dragon King drain flash fix, ftremix/dragonking/ftremixdragonkingdrainflash.c (DragonKingSpecial.asm drain_flash_fix_):
 * TRUE when the damage digits of fighter_gobj keep their colour instead of the white flash */
sb32 ftRemixDragonKingDrainFlashSuppressed(GObj *fighter_gobj);

/* Piano command grab and absorb, ftremixpianoshared.c (PianoSpecial.asm command_grab_action_fix_ 0x8014C8FC and
 * absorb_behaviour_ 0x800E33AC): the action the captured fighter enters, the absorb healing and the Mad Piano bonus ammo */
s32 ftRemixCommandGrabCaptureAction(GObj *grabber_gobj);
s32 ftRemixAbsorbHealAmount(FTStruct *fp, s32 damage);
void ftRemixAbsorbPianoBonusAmmo(FTStruct *fp, s32 damage);

/* Piano capture_action_fix_, ftremixpianoshared.c: custom_capture_dk_interrupt of Mad Piano, keyed by the grabber */
s32 ftRemixPianoCaptureDKInterrupt(FTStruct *grabber_fp);

/* Kirby clone power action swap, ftremix/kirby/ftremixkirbyclone.c (Kirby.asm clone_power_action_swap_ 0x800E6F34),
 * applied at the top of ftMainSetStatus */
s32 ftRemixKirbyCloneActionSwap(GObj *fighter_gobj, s32 action_id);

/* Body size, ftremix/ftremixsize.c (Size.asm phase a): per-player multiplier (1.0F by default), the top joint and ECB
 * hook, the camera zoom factor and the value scale used for the camera offset and the ECB display */
f32 ftRemixSizeGetMultiplier(s32 player);
void ftRemixSizeSetMultiplier(s32 player, f32 multiplier);
void ftRemixSizeInitFighter(FTStruct *fp);
#define FTREMIX_SIZE_STATE_NORMAL 0
#define FTREMIX_SIZE_STATE_GIANT 1
#define FTREMIX_SIZE_STATE_TINY 2
#define FTREMIX_SIZE_GIANT_MULTIPLIER 2.25F
#define FTREMIX_SIZE_TINY_MULTIPLIER 0.5F
s32 ftRemixSizeGetMatchState(s32 player);
void ftRemixSizeSetMatchState(s32 player, s32 state);
f32 ftRemixSizeGetModeMultiplier(s32 player);
void ftRemixSizeApplyTopJointAndEcb(FTStruct *fp);
f32 ftRemixSizeCameraZoomFactor(FTStruct *fp);
f32 ftRemixSizeScaleValue(FTStruct *fp, f32 value);

/* Size phase (b): 1 + coefficient * (size - 1) for the attribute coefficients of Size.asm */
f32 ftRemixSizeAttrMultiplier(s32 player, f32 coefficient);
#define FTREMIX_SIZE_COEF_WALKING_SPEED 0.45F
#define FTREMIX_SIZE_COEF_BRAKE_FORCE -0.04712F
#define FTREMIX_SIZE_COEF_DASHING_SPEED 0.4F
#define FTREMIX_SIZE_COEF_DASH_DECEL 0.45172F
#define FTREMIX_SIZE_COEF_RUNNING_SPEED 0.36664F
#define FTREMIX_SIZE_COEF_JUMPING_HEIGHT 0.3234F
#define FTREMIX_SIZE_COEF_BASE_JUMPING_HEIGHT 0.0727F
#define FTREMIX_SIZE_COEF_SECOND_JUMPING_HEIGHT 0.0395604F
#define FTREMIX_SIZE_COEF_WEIGHT -0.24F

/* Casino Night side bumper bounce, 0x80141B88 in ftCommonWallDamageSetStatus (gr/grremix/grremixcasino.c) */
f32 grRemixCasinoWallBounceScale(FTStruct *fp);

/* Size phase (b) weight, ftremix/ftremixsizeweight.c (Size.asm adjust_player_weight_1..8): the attribute weight
 * a knockback formula reads, scaled by the size of the fighter */
f32 ftRemixSizeWeight(FTStruct *fp, f32 vanilla_weight);

/* Size phase (b) movement, ftremix/ftremixsizemove.c (Size.asm adjust_player_*_ and adjust_jumping_height_multiplier_*):
 * walk, brake, dash, run, jump squat and jump height values scaled by the size of the fighter */
f32 ftRemixSizeWalkingSpeed(FTStruct *fp, f32 vanilla_speed);
f32 ftRemixSizeBrakeForce(FTStruct *fp, f32 vanilla_force);
f32 ftRemixSizeDashDecel(FTStruct *fp, f32 vanilla_decel);
f32 ftRemixSizeInitialDashSpeed(FTStruct *fp, f32 vanilla_speed);
f32 ftRemixSizeRunSpeed(FTStruct *fp, f32 vanilla_speed);
f32 ftRemixSizeJumpSquatLength(FTStruct *fp, f32 vanilla_length);
f32 ftRemixSizeJumpHeightMul(FTStruct *fp, f32 vanilla_mul);
f32 ftRemixSizeJumpHeightBase(FTStruct *fp, f32 vanilla_base);
f32 ftRemixSizeAerialJumpHeight(FTStruct *fp, f32 vanilla_height);

/* Size phase (c) effects, ftremix/ftremixsizefx2.c (Size.asm link.adjust_usp_projectile_size_ and adjust_usp_gfx_size_):
 * the Up Special weapon and effect scaled by the owner multiplier, set at spawn */
void ftRemixSizeScaleLinkWeapon(FTStruct *fp, GObj *weapon_gobj);
void ftRemixSizeScaleLinkEffect(FTStruct *fp, GObj *effect_gobj);

/* Size phase (b) hitbox and damage, ftremix/ftremixsizebody.c (Size.asm fix_size_, fix_damage_, passive_armor_):
 * the hitbox size, the hitbox damage and the passive knockback resist scaled by the size of the fighter */
s32 ftRemixSizeScaleHitboxSize(FTStruct *fp, s32 size);
s32 ftRemixSizeScaleDamage(FTStruct *fp, s32 damage);
f32 ftRemixSizePassiveArmor(FTStruct *fp, f32 passive_resist);

/* Size phase (c) entry, ftremix/ftremixsizeentry.c (Size.asm entry animation_ and arwing update_routine_): the entry
 * animation translation scaled before ftCommonAppearProcPhysics reads it, and the Arwing entry wing scaled per frame */
void ftRemixSizeEntryAnimationScale(FTStruct *fp);
void ftRemixSizeEntryArwingBind(GObj *effect_gobj, GObj *fighter_gobj);
void ftRemixSizeEntryArwingUpdate(GObj *effect_gobj);

/* Size phase (c): effect GFX scaled by the owner size, ftremix/ftremixsizefx.c (Size.asm yoshi_egg, ground_gfx,
 * kirby nsp suck, falcon punch). The Yoshi shield is 1.5F in Remix, times the owner multiplier. */
#define FTREMIX_SIZE_YOSHI_SHIELD_SCALE 1.5F
void ftRemixSizeScaleYoshiShield(GObj *effect_gobj, FTStruct *fp);
void ftRemixSizeScaleEntryEgg(GObj *egg_gobj, FTStruct *fp);
void ftRemixSizeAttachGroundGfx(GObj *effect_gobj, GObj *fighter_gobj);
void ftRemixSizeGroundGfxProcUpdate(GObj *effect_gobj);
void ftRemixSizeKirbySuckTransform(LBTransform *xf, GObj *fighter_gobj);
void ftRemixSizeFalconPunchProcDisplay(GObj *effect_gobj);

/* Knockback.asm (knockback angle randomizer), ftremixknockback.c: randomize_angles_ (ftManagerMakeFighter),
 * get_angle_hitbox_ (ftMainParseMotionEvent) and get_angle_throw_ (ftCommonThrownReleaseThrownUpdateStats) */
void ftRemixKnockbackRandomizeAngles(FTStruct *fp);
s32 ftRemixKnockbackAngle(FTStruct *fp, s32 vanilla_angle);

/* Hitbox.asm force_grab_immunity_ (0x800E6064), ftremixhitbox.c: TRUE when the victim is not grabbable */
sb32 ftRemixForceGrabImmunity(FTStruct *fp);

/* Surface.asm get_friction_ (0x800D8BDC, 0x800E21DC), gr/grremix/grremixsurface.c: ground friction of a material id */
f32 grRemixSurfaceFriction(u32 material);

/* Surface.asm fighter_apply_conveyor_surface (0x800E23D8) and item_apply_conveyor_surface (0x8016F8E4), gr/grremix/grremixsurface.c:
 * TRUE and the push speed when the material under a grounded object is a conveyor that is active */
sb32 grRemixSurfaceConveyorPush(u32 material, f32 *speed);

/* Blastzone.asm skip_bottom_blast_sfx (0x800E234C), gr/grremix/grremixblastzone.c: TRUE when the bottom altitude sound is skipped */
sb32 grRemixBlastZoneSkipsBottomSfx(void);

/* Camera.asm override_camera_type_ (0x8010DAA4), gr/grremix/grremixcamera.c: camera routine of the default branch */
s32 grRemixCameraDefaultStatus(s32 gkind);

/* Camera.asm camera_adjust_ (0x800E7270), gr/grremix/grremixcamera.c: camera zoom range set by ftMainSetStatus */
f32 grRemixCameraAdjustZoomRange(void);

/* Spawn.asm load_respawn_point_ (0x800FC8B0), gr/grremix/grremixspawn.c: TRUE when the respawn position was overridden */
sb32 grRemixSpawnRespawnPointOverride(Vec3f *pos);

/* Damage.asm damage types 7..0xC (add_damage_type), ftremix/ftremixdamage.c: the ids the on-hit jump tables route here */
#define FTREMIX_DAMAGE_SHADOW 0x7
#define FTREMIX_DAMAGE_STUN 0x8
#define FTREMIX_DAMAGE_LASER 0x9
#define FTREMIX_DAMAGE_DEKU_STUN 0xA
#define FTREMIX_DAMAGE_BURY 0xB
#define FTREMIX_DAMAGE_FLASHBANG 0xC
sb32 ftRemixDamageIsNewType(s32 element);
sb32 ftRemixDamageSetColAnim(GObj *fighter_gobj, s32 element, s32 damage_level);
void ftRemixDamageMakeShadowEffect(Vec3f *pos, s32 damage);

/* Poison.asm poison_hook and the acid writes, ftremix/ftremixpoison.c (called from ftMainProcUpdateInterrupt) */
void ftRemixPoisonUpdate(GObj *fighter_gobj);
void ftRemixPoisonSet(s32 port, s32 duration, s32 damage, s32 type);
void ftRemixPoisonAdvanceTimer(void);
void ftRemixPoisonClear(void);

/* F7 leftovers, ftremix/ftremixleftovers.c: Piano extra_part_visibility_ (ftMainSetStatus) and Marina
 * prevent_item_despawn_ (itMapCheckDestroyDropped, it/itmap.c) */
void ftRemixPianoExtraPartVisibility(FTStruct *fp);
sb32 ftRemixLeftoverItemSkipsDespawn(GObj *item_gobj);


/* Kirby copy table of every Remix fighter (ftremixkirbycopy.c), a drop-in for the FTKirbyCopy table of the Kirby file */
FTKirbyCopy* ftRemixKirbyGetCopyTable(void);

/* Kirby power flags (ftremixkirbypower.c, KirbyHats.asm): magic hat, power loss on hit or taunt */
void ftRemixKirbyPowerSetTauntLosesPower(s32 player, sb32 taunt_loses_power);
void ftRemixKirbyPowerReset(void);
void ftRemixKirbyPowerStolen(FTStruct *fp);
s32 ftRemixKirbyStolenPowerID(FTStruct *victim_fp, s32 copy_id);
void ftRemixKirbyUpdateMagicHat(FTStruct *fp, s32 copy_id);
sb32 ftRemixKirbyLoseCopyPrevented(FTStruct *fp);

/* Kirby hats (ftremixkirbyhats.c): the model part of hat modelpart_id on the hat joint, extended past hat 0xE */
FTModelPart* ftRemixKirbyHatModelPart(FTStruct *fp, s32 joint_id, s32 modelpart_id, FTModelPartDesc *desc, s32 detail_index);
void ftRemixKirbyHatsClearCache(void);

/* DI toggle, ftremixdi.c (DI.asm, apply_di_modifiers 0x801408DC): the smash DI range multiplier of
 * ftCommonDamageCommonProcLagUpdate */
f32 ftRemixDamageSmashDIRangeMul(f32 vanilla_mul);

/* Move Staling, ftremix/ftremixstalemoves.c (StaleMoves.asm): ftParamGetStale in the custom modes and the table of the
 * vanilla loop in the strict and wait-for-it modes */
sb32 ftRemixStaleMovesIsCustom(void);
f32 ftRemixStaleMovesGetMultiplier(s32 player, s32 attack_id, u16 motion_count);
const f32 *ftRemixStaleMovesTable(const f32 *vanilla_table);

/* Jab Locking (JabLock.asm), ftremix/ftremixjablock.c: the lock bool at N64 0x0B28 and its hooks in DownBounce */
void ftRemixJabLockClear(FTStruct *fp);
void ftRemixJabLockDamageStatus(FTStruct *fp, s32 *status_id_set);
sb32 ftRemixJabLockInterruptDownStand(GObj *fighter_gobj);

/* Tripping (Tripping.asm), ftremix/ftremixtripping.c: the trip roll in the Turn, Dash and TurnRun status set, the turn
 * state reset that keeps a trip's hitstun, and the trip recovery of ftCommonDamageCommonProcUpdate */
s32 ftRemixTrippingEnterStatus(FTStruct *fp, s32 status_id);
sb32 ftRemixTrippingAllowsTurnStateReset(FTStruct *fp);
sb32 ftRemixTrippingFinishDamage(GObj *fighter_gobj);

/* Z-Cancel settings (ZCancel.asm, Practice.asm flash_on_z_cancel_), ftremix/ftremixzcancel.c: the landing of an aerial
 * attack, called by ftCommonAttackAirProcMap */
void ftRemixZCancelAttackAirLanding(GObj *fighter_gobj);

/* Air Dodging setting (Toggles.asm entry_air_dodge), ftremix/ftremixairdodge.c (AirDodge.asm): the jump check of
 * ftCommonJumpAerialCheckInterruptCommon (TRUE when the dodge or air dash started) and the air dash flag reset of the
 * grounded script (mpCommonSetFighterLandingParams) and of the initial script (ftManagerInitFighter) */
sb32 ftRemixAirDodgeCheckInterrupt(GObj *fighter_gobj);
void ftRemixAirDodgeResetAirDash(FTStruct *fp);

/* Footstool Jumping setting (Toggles.asm entry_footstool), ftremix/ftremixfootstool.c (FootStool.asm): the aerial jump
 * sites of ftCommonJumpAerialCheckInterruptCommon (TRUE when a footstool jump started, the caller then skips its jump)
 * and the jump reset of mpCommonSetFighterAir */
sb32 ftRemixFootStoolTryAerialJump(GObj *fighter_gobj, s32 jump_type);
sb32 ftRemixFootStoolPreservesJumps(FTStruct *fp);

/* Edge C-Jumping (Toggles.asm entry_ledge_jump), ftremix/ftremixledgejump.c (LedgeJump.asm): the C button check of
 * ftCommonCliffWaitProcInterrupt (TRUE when a C button was tapped, the caller skips the fall check) and the jump of the
 * default case of the Quick and Slow cliff proc updates */
sb32 ftRemixLedgeJumpCheckCButtons(GObj *fighter_gobj);
void ftRemixLedgeJumpCliffMotionDefault(GObj *fighter_gobj);

/* Ledge Trumping (Toggles.asm entry_ledge_trump), ftremix/ftremixledgetrump.c (LedgeTrump.asm ledge_check_, 0x800DE66C):
 * TRUE when the search for the ledge holder goes on, called by mpCommonRunFighterSpecialCollisions */
sb32 ftRemixLedgeTrumpSkipsHolder(GObj *holder_gobj);

/* Game Speed setting (Toggles.asm entry_game_speed), ftremix/ftremixspeed.c (Speed.asm handle_rate_ and
 * handle_rate_training): the gcRunAll frames of ifCommonBattleGoUpdateInterface, the Training variant (returns the new
 * Training.skip_advance; training_speed_is_default is FALSE when the Training speed multiplier overrides the toggle), and
 * the additional_advance flag read by PoisonDmg */
void ftRemixSpeedRunFrames(void);
sb32 ftRemixSpeedTrainingRunFrames(sb32 training_speed_is_default, sb32 skip_advance);
sb32 ftRemixSpeedIsAdditionalAdvance(void);

/* Single Button Mode setting (Toggles.asm entry_single_button_mode), ftremix/ftremixsinglebutton.c (SingleButtonMode.asm):
 * the input filter of single_button_mode for a gate (FTREMIX_SBM_GATE_*) and the CPU A/B and Z/L press hooks */
#define FTREMIX_SBM_GATE_OPEN 0
#define FTREMIX_SBM_GATE_TRAINING_MENU 1
#define FTREMIX_SBM_GATE_CLOSED 2
void ftRemixSingleButtonModeFilter(u16 *held, u16 *pressed, s32 gate);
u16 ftRemixSingleButtonCpuAOrBPress(s32 mode, u16 cpu_button);
u16 ftRemixSingleButtonCpuZOrLPress(s32 mode, u16 cpu_button);

/* Rage setting (Toggles.asm entry_rage), ftremix/ftremixrage.c (Rage.asm): the attacker damage of a hit, the weapon owner
 * of a projectile hit, the knockback scale of ftParamGetCommonKnockback and the smoke of the physics map */
void ftRemixRageSaveAttackerPercent(FTStruct *attacker_fp);
void ftRemixRageSaveProjectileOwnerPercent(WPStruct *wp);
f32 ftRemixRageKnockbackScale(f32 knockback, s32 knockback_weight);
void ftRemixRageSmokeEffect(FTStruct *fp);

/* Wall Teching setting (Toggles.asm entry_wall_teching), ftremix/ftremixwalltech.c (WallTeching.asm): called by
 * ftCommonWallDamageSetStatus, TRUE when the wall damage became a tech and the caller must skip its wall damage status */
sb32 ftRemixWallTechTryStart(GObj *fighter_gobj);

/* Spot Dodging setting (Toggles.asm entry_spot_dodge), ftremix/ftremixspotdodge.c (SpotDodge.asm): called by
 * ftCommonGuardPassCheckInterruptGuard, TRUE when a spot dodge started; the pass input is vanilla_pass when the setting is off */
sb32 ftRemixSpotDodgeCheckInterrupt(GObj *fighter_gobj);
sb32 ftRemixSpotDodgePassInput(FTStruct *fp, sb32 vanilla_pass);

/* Perfect Shielding, ftremix/ftremixperfectshield.c (PerfectShield.asm): the shield stun skip of ftMainProcParams
 * (0x800E64D8), and the projectile and item reflect in the shield branch of wpProcessProcHitCollisions (0x80166C7C)
 * and itProcessProcHitCollisions (0x801711BC) */
sb32 ftRemixPerfectShieldHandlesGuardSetOff(GObj *fighter_gobj);
sb32 ftRemixPerfectShieldWeapon(GObj *weapon_gobj);
sb32 ftRemixPerfectShieldItem(GObj *item_gobj);

/* Japanese.asm settings, ftremix/ftremixhitlag.c, ftremixshieldstun.c, ftremixjstunsleep.c and ftremixjapanesesounds.c:
 * the Hitlag formula of ftParamGetHitLag (vanilla_add is the vanilla constant of the formula), the Shield Stun frames of
 * ftCommonGuardSetOffSetStatus, the Fura Fura and Fura Sleep breakout waits, and the fgm ids of PlayFGM motion events
 * (ftMainParseMotionEvent), projectile hits (ftMainUpdateDamageStatWeapon) */
s32 ftRemixHitlagTics(s32 damage, f32 hitlag_mul, f32 vanilla_add);
f32 ftRemixShieldStunFrames(FTStruct *fp, f32 vanilla_frames);
s32 ftRemixFuraFuraBreakoutWait(s32 breakout_wait);
s32 ftRemixFuraSleepBreakoutWait(s32 breakout_wait);
u16 ftRemixJapaneseMotionSoundFgm(FTStruct *fp, u32 value);
u16 ftRemixJapaneseProjectileSoundFgm(GObj *owner_gobj, u16 fgm_id);

/* Charge Smashes setting (Toggles.asm entry_charged_smashes), ftremix/ftremixchargesmash.c (ChargeSmashAttacks.asm): the
 * logic run on action change (ftMainSetStatus), the wait hold of ftAnimEndSetWait, and the shake offset of ftDisplayMainProcDisplay
 * and func_ovl0_800C9F70 */
void ftRemixChargeSmashLogic(GObj *fighter_gobj);
sb32 ftRemixChargeSmashHoldsAnimEnd(GObj *fighter_gobj);
Vec2f ftRemixChargeSmashShufflePosition(FTStruct *fp);

/* Parrying (BETA) setting (Toggles.asm entry_parry), ftremix/ftremixparry.c (Parry.asm): the parry check before the shield
 * check of ftMainSearchHitFighter, ftMainSearchHitWeapon and ftMainSearchHitItem (TRUE when the hit was parried, the caller
 * skips it), and the extra attacker hitlag of ftMainProcParams */
sb32 ftRemixParryDirectHit(FTStruct *victim_fp, FTStruct *attacker_fp);
sb32 ftRemixParryWeaponHit(FTStruct *fp, WPStruct *wp, sb32 *is_damage_detect);
sb32 ftRemixParryItemHit(FTStruct *fp, ITStruct *ip, sb32 *is_damage_detect);
void ftRemixParryAddAttackerHitlag(FTStruct *fp);

/* Screenshake setting (Toggles.asm entry_screenshake), ef/efremixscreenshake.c (Accessibility.asm screenshake_toggle,
 * 0x80100964 in efManagerQuakeMakeEffect): the severity the quake effect switches on */
s32 efRemixQuakeSeverity(s32 severity);

/* Pause settings (Toggles.asm entry_hold_to_pause, entry_camera_mode), ftremix/ftremixpause.c (Pause.asm): the START check
 * of ifCommonBattleGoUpdateInterface (TRUE when the pause starts; vanilla is the tap check when the setting is off) and the
 * scene camera keep check of ifCommonBattlePauseUpdateInterface */
sb32 ftRemixPauseStartRequested(s32 player, u16 button_tap, u16 button_hold);
sb32 ftRemixPauseKeepsSceneCamera(void);

/* Item.asm item_drop (All Items R Drop (Aerial)), ftremix/ftremixitemdrop.c: TRUE when the aerial attack throws the held item,
 * FALSE when R drops it (setting on, R held) */
sb32 ftRemixAerialItemIsThrowType(FTStruct *fp);

/* BlastZone Warp (BETA) setting (Toggles.asm entry_blastzone_warp), ftremix/ftremixblastzone.c (Blastzone.asm): called by
 * ftCommonDeadCheckInterruptCommon before each KO call of the standard map bounds (TRUE when the warp replaces the KO and
 * the caller returns TRUE) */
sb32 ftRemixBlastZoneWarpFromRight(GObj *fighter_gobj, Vec3f *pos);
sb32 ftRemixBlastZoneWarpFromLeft(GObj *fighter_gobj, Vec3f *pos);
sb32 ftRemixBlastZoneWarpFromTop(GObj *fighter_gobj, Vec3f *pos);
sb32 ftRemixBlastZoneWarpFromBottom(GObj *fighter_gobj, Vec3f *pos);

/* BlastZone GFX setting (Toggles.asm entry_blastzone_gfx), ef/efremixblastzone.c (Accessibility.asm blastzone_explode_gfx):
 * the OFF skip of efManagerDeadExplodeMakeEffect and the REDUCED scales of its effect DObj and star particles */
sb32 efRemixBlastZoneGfxIsOff(void);
void efRemixBlastZoneScaleEffect(DObj *dobj);
void efRemixBlastZoneScaleParticle(LBTransform *xf);

/* Item Containers setting (Toggles.asm entry_item_containers), it/itremix/itremixcontainers.c (Item.asm item_containers_*):
 * the item mask update of itManagerSetupContainerDrops, the keep-or-explode choice of a hit container (its spawn result
 * in, TRUE when it is kept) and the crate sound of itBoxCommonCheckSpawnItems */
void itRemixContainersApplyToggle(void);
sb32 itRemixContainerKeeps(sb32 is_spawned);
s32 itRemixContainerSmashFgm(void);

/* Pokemon item picker (Toggles.asm entry_pokemon_*), it/itremix/itremixpokemonpicker.c: whos_that_pokemon replaces the Mew
 * check and roll of itMainMakeMonster (the caller keeps the vanilla vel setup and tail), whos_that_pokemon_setup replaces
 * the body of itManagerInitMonsterVars */
GObj* itRemixPokemonMakeMonster(GObj *item_gobj, Vec3f *vel);
void itRemixPokemonResetPicker(void);

/* Pokemon voices (Toggles.asm entry_pokemon_voices), it/itremix/itremixpokemonvoices.c: pokemon_variant_sfx, the FGM id of the
 * ten Pokemon voice sites (Item.asm 0x8017E2D4 .. 0x8017C66C) */
u16 itRemixPokemonVoiceFgm(u16 fgm_id);

/* Camera Mode setting (Camera.asm entry_camera_mode), gm/gmremixcameramode.c: gmRemixCameraSetupDefault runs in the default
 * branch of gmCameraMakeDefaultCamera (set_initial_coordinates_), gmRemixCameraApplyFixedRun after the status routine is
 * chosen (get_custom_camera_), gmRemixCameraSetStatusIsFrozen in gmCameraSetStatus (fixed_camera_always_),
 * gmRemixCameraFreezeUpdate replaces the vector add of the four camera updates (frozen_camera_), gmRemixCameraIsFixed skips
 * gcPlayCamAnim (fixed_camera_boss_), gmRemixCameraCinematicFocus is the countdown focus (cinematic_entry_), and the pause
 * checks (prevent_fixed_camera_pause_bonus_, pause_disable_control_flag_) */
sb32 gmRemixCameraIsFixed(void);
sb32 gmRemixCameraIsScene(void);
void gmRemixCameraSetupDefault(CObj *cobj);
void gmRemixCameraApplyFixedRun(s32 gkind);
sb32 gmRemixCameraSetStatusIsFrozen(void);
sb32 gmRemixCameraFreezeUpdate(CObj *cobj);
s32 gmRemixCameraCinematicFocus(s32 random_focus);
sb32 gmRemixCameraPauseBonusZoomAllowed(void);
sb32 gmRemixCameraPauseControlEnabled(void);

/* Disable HUD setting (Camera.asm entry_disable_hud), gm/gmremixcamerapause.c: disable_hud_ after the GO announcement and
 * unpause_camera_ when the pause ends (the magnifying glass flag it returns) */
sb32 gmRemixCameraHudHiddenOnGo(void);
sb32 gmRemixCameraRestorePauseHud(void);

/* Combo meter settings (Toggles.asm entry_improved_combo_meter, entry_tech_chase_combo_meter), ftremix/ftremixcombo.c
 * (Combo.asm improve_meter_, 0x800E19CC): TRUE when the combo count of the fighter must not reset this frame, in place of
 * the is_hitstun check of ftMainUpdate */
sb32 ftRemixComboKeepsCount(FTStruct *fp);

/* Combo meter (Toggles.asm entry_combo_meter, entry_1v1_combo_meter_swap), if/ifremixcombometer.c (ComboMeter.asm): setup_
 * (called from syTaskmanLoadScene before the scene starts) and hide_for_score_sprite_ (ifCommonPlayerScoreMakeEffect) */
void ifRemixComboMeterSetup(void);
void ifRemixComboMeterHideForScore(s32 port);

/* Special Zoom (BETA) setting (Toggles.asm entry_special_zoom), gm/gmremixspecialzoom.c (SpecialZoom.asm): the hit zoom
 * of ftCommonDamageInitDamageVars, the zoom focus of gmCameraUpdatePlayerZoom, the wallpaper of gmCameraMakeWallpaperCamera
 * and the slow motion of gcRunAll (TRUE when the object update of the game update is skipped) */
void ftRemixSpecialZoomFinalKnockback(FTStruct *fp, s32 damage_player_num);
void ftRemixSpecialZoomCameraOffset(Vec3f *pos);
void ftRemixSpecialZoomMakeWallpaper(void);
sb32 ftRemixSpecialZoomSkipsObjectsUpdate(void);

/* Special Zoom helpers, gm/gmremixspecialzoomko.c (KO prediction, decisive stock) and gmremixspecialzoomattack.c (attacker
 * special moves) and gmremixspecialzoomwallpaper.c (background file, wallpaper camera and background object) */
sb32 ftRemixSpecialZoomIsDecisiveStock(FTStruct *fp);
s32 ftRemixSpecialZoomCountPlayersInMatch(void);
sb32 ftRemixSpecialZoomCheckKO(FTStruct *fp);
sb32 ftRemixSpecialZoomIsAttackerSpecialMove(GObj *attacker_gobj);
void ftRemixSpecialZoomWallpaperLoad(void);
void ftRemixSpecialZoomWallpaperCreateCamera(void);
void ftRemixSpecialZoomWallpaperShow(void);
void ftRemixSpecialZoomWallpaperPlayAnim(void);
void ftRemixSpecialZoomWallpaperHide(void);

/* Special Zoom helper, if/ifcommon.c: the teams in play minus one (sIFCommonBattlePlace, the
 * RAM 0x801317F4 gBattlePlacement of SpecialZoom.asm) */
s32 ifRemixBattlePlace(void);

/* Special Model Display and Model Display, ft/ftremix/ftremixhitboxdisplay.c (Hitbox.asm hitbox_mode_, Costumes.asm
 * force_hi_lo_poly_): the forced poly detail of a fighter, and the fighter display mode from the Special Model toggle */
u8 ftRemixModelDisplayDetail(u8 detail);
void ftRemixHitboxDisplayApply(FTStruct *fp);

/* Damage.asm stun_damage_action_ (0x8014158C) and stun_electric_action_ (0x801412E8), ftremix/ftremixdamagestun.c:
 * TRUE when the stun, deku stun, flashbang or bury action started (ftCommonDamageGotoDamageStatus), and TRUE when the
 * damage type takes the electric status path (ftCommonDamageInitDamageVars) */
sb32 ftRemixDamageGotoStunAction(GObj *fighter_gobj);
sb32 ftRemixDamageUsesElectricStatus(s32 element);

#endif
