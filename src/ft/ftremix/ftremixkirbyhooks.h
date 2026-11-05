#ifndef _FTREMIXKIRBYHOOKS_H_
#define _FTREMIXKIRBYHOOKS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Kirby hooks of Smash Remix that vanilla code calls (REGION_REMIX only), see ftremixhooks.h for the other fighters:
 * spawn with a CSS Kirby Hat (KirbyHats.asm, Size.asm 358-381), the copy begin routines of Slippy and Dragon King
 * (Kirby.asm kirby_ground_nsp / kirby_air_nsp) and the Dark Samus and Mewtwo branches of the copied Samus charge shot.
 */

#include <ft/fighter.h>
#include "ft/ftremix/ftremix.h"

#define FTREMIX_KIRBY_DSAMUS_HAT_ID 0x13
#define FTREMIX_KIRBY_MEWTWO_BALL_JOINT 0x1A

/**
 * Spawn power and hat of a Kirby or J Kirby (KirbyHats.asm kirby_hat_select_ 0x800D7E08 and kirby_load_with_hat_fix_
 * 0x800D7E54): the CSS Kirby Hat of the port replaces the power of the spawn description, then the hat is the one of the
 * power. Other fighters are left alone.
 *
 * @param fighter_gobj Fighter being initialised.
 * @param spawn_copy_id Copy id of the spawn description.
 */
void ftRemixKirbyHatSpawnInit(GObj *fighter_gobj, s32 spawn_copy_id);

/**
 * Per port power flags set by every fighter spawn (Size.asm initialize_base_multiplier_table_, 358-381): the magic hat starts
 * on for the magic power or the '???' hat, and the power is kept when hit or taunting as soon as a hat is chosen.
 *
 * @param fp Fighter being initialised.
 * @param spawn_copy_kind Copy id of the spawn description.
 */
void ftRemixKirbyPowerInitFighter(FTStruct *fp, s32 spawn_copy_kind);

/**
 * Begin routine Kirby runs for a copied Slippy or Dragon King neutral special (SlippyNSP.kirby_*_begin_initial,
 * DragonKingNSP.kirby_*_begin_initial_).
 *
 * @param addr Remix RAM address of the routine.
 * @return The port routine, or NULL when addr is not one of the four.
 */
FTRemixProcFn ftRemixFindKirbyCopyBeginProc(u32 addr);

/**
 * DsamusSpecial.asm initial_variable_override_kirby (0x80157788): a Kirby wearing the Dark Samus hat only auto-shoots the
 * aerial start at full ammo.
 *
 * @param fp Kirby or J Kirby.
 * @param vanilla_is_release Value the vanilla routine sets.
 * @return The release flag of the aerial start.
 */
sb32 ftRemixKirbyDSamusNSPAirStartIsRelease(FTStruct *fp, sb32 vanilla_is_release);

/**
 * DsamusSpecial.asm remove_shoot_flag_aerial_transition_kirby (0x801570F8): leaving the ground keeps the buffered shoot flag
 * of the Dark Samus hat instead of forcing it.
 *
 * @param fp Kirby or J Kirby.
 * @return TRUE when the release flag must be left alone.
 */
sb32 ftRemixKirbyDSamusNSPKeepsReleaseOnAirSwitch(FTStruct *fp);

/**
 * DsamusSpecial.asm check_aerial_charge_kirby (0x80156F88): below full ammo and with no buffered shot, the aerial start of
 * the Dark Samus hat keeps charging instead of shooting.
 *
 * @param fighter_gobj Kirby or J Kirby.
 * @return TRUE when the aerial charge status was entered.
 */
sb32 ftRemixKirbyDSamusNSPCheckAerialCharge(GObj *fighter_gobj);

/**
 * MewtwoSpecial.asm ball_position_fix_ (0x8015D35C), Kirby branch: the Shadow Ball of a Kirby or J Kirby sits at the grab
 * joint of the Mewtwo power.
 *
 * @param fp Fighter whose charge shot position is wanted.
 * @param pos Receives the position when the fighter is Kirby or J Kirby.
 * @return TRUE when pos was written.
 */
sb32 ftRemixKirbyMewtwoNGetBallPosition(FTStruct *fp, Vec3f *pos);

/**
 * MewtwoSpecial.asm kirby_shoot_velocity_patch_ (0x801573E0): shooting the Shadow Ball with the Mewtwo power recoils with a
 * base of 10 + 5 * charge level and a y step of -5.
 *
 * @param fp Kirby or J Kirby.
 * @param recoil_base In: vanilla base. Out: base for the Mewtwo power.
 * @param recoil_y_step In: vanilla y step. Out: step for the Mewtwo power.
 */
void ftRemixKirbyMewtwoNGetShootRecoil(FTStruct *fp, f32 *recoil_base, f32 *recoil_y_step);

#endif

#endif
