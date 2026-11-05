#ifndef _FTREMIXCOMMAND_H_
#define _FTREMIXCOMMAND_H_

/*
 * Smash Remix custom moveset commands 0xD0-0xFF (src/command.asm) and the
 * per-player override state they keep. REGION_REMIX only.
 *
 * Remix hooks the three motion script runners: the full runner
 * (ftMainUpdateMotionEventsAll, load_command_) runs every command, while the
 * two fast-forward runners (load_command_2_) skip the ones that must not
 * repeat (y velocity, fast fall, sounds, env color, direction switch).
 */

#include <ft/fighter.h>

/**
 * Runs the command at `ms->p_script` if it is a Remix custom command
 * (opcode byte >= 0xD0) and advances the script past it. `is_forward`
 * selects the fast-forward table. Returns FALSE for vanilla commands, which
 * the caller dispatches as usual.
 */
sb32 ftRemixCommandCheckRun(GObj *fighter_gobj, FTStruct *fp, FTMotionScript *ms, sb32 is_forward);

/** change_action_: resets the translation multiplier, env color override and the Training action frame count. */
void ftRemixCommandResetStatus(FTStruct *fp);

/** end_hitbox_: clears every hitbox's direction and FGM override. */
void ftRemixCommandResetAttackAll(FTStruct *fp);

/** create_hitbox_: clears hitbox `attack_id`'s direction and FGM override. */
void ftRemixCommandResetAttack(s32 player, s32 attack_id);

/** apply_direction_: forces `this_fp->damage_lr` when the hitbox has a direction override. */
void ftRemixCommandApplyDamageDirection(FTStruct *this_fp, FTStruct *attacker_fp, FTAttackColl *attack_coll);

/** apply_fgm_: plays the hit sound of `attack_coll`, honoring its FGM override. */
void ftRemixCommandPlayHitFGM(FTStruct *fp, FTAttackColl *attack_coll, u16 fgm_id, f32 pos_x);

/** Topjoint translation multiplier set by command 0xD3 (1.0 by default), times the Size factor of the apply_* patches (0.5977 giant, 1.59375 tiny). */
f32 ftRemixCommandGetTranslationMultiplier(FTStruct *fp);

/** Env color forced by command 0xD9 for `player`, or 0 for none. */
u32 ftRemixCommandGetEnvColor(s32 player);

#endif
