#ifndef _FTREMIXAIPOSTCOMMON_H_
#define _FTREMIXAIPOSTCOMMON_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Helpers shared by the per-fighter routines of the Remix AI post process (Character.cpu_post_process table). */

/**
 * Tells if the CPU is about to run the given input script: the command pointer is compared with the script of the id,
 * as the asm compares ft_com->p_command with the AI.command_table entry.
 *
 * @param fp CPU fighter.
 * @param input_kind Vanilla input id or Remix routine id.
 * @return TRUE when the current command is that script.
 */
sb32 ftRemixAiPostCommandIs(FTStruct *fp, s32 input_kind);

/**
 * Fighter struct of the CPU's current target.
 *
 * @param fp CPU fighter.
 * @return The target fighter, or NULL when the CPU has no target object.
 */
FTStruct *ftRemixAiPostGetTarget(FTStruct *fp);

/**
 * Tells if a floor line lies below the fighter (the asm compares the clipping below, FTStruct 0xEC, with -1).
 *
 * @param fp CPU fighter.
 * @return TRUE when the fighter is above ground.
 */
sb32 ftRemixAiPostHasFloorBelow(FTStruct *fp);

/**
 * Writes the CPU stick, as the asm does with `sb` on FTStruct 0x1C8 and 0x1C9.
 *
 * @param fp CPU fighter.
 * @param stick_x Stick X, -80 to 80.
 * @param stick_y Stick Y, -80 to 80.
 */
void ftRemixAiPostSetStick(FTStruct *fp, s32 stick_x, s32 stick_y);

/**
 * Replaces the current command with the empty NULL routine, so the inputs the post process wrote are not overridden
 * (AI.ROUTINE.NULL through 0x80132758).
 *
 * @param fp CPU fighter.
 */
void ftRemixAiPostSetNullCommand(FTStruct *fp);

/**
 * Distance between the fighter and its target, computed as the asm does (sqrt of the squared X and Y differences of the
 * location vectors).
 *
 * @param fp CPU fighter.
 * @param target_fp Target fighter.
 * @return Distance in world units.
 */
f32 ftRemixAiPostDistanceToTarget(FTStruct *fp, FTStruct *target_fp);

/**
 * Steers a USP that dives (Fox, Falco and Pikachu): with ground below, clears the command and holds the stick down.
 *
 * @param fp CPU fighter.
 */
void ftRemixAiPostDiveDown(FTStruct *fp);

/**
 * ground_check of Falco, Wario and Wolf: casts a probe 2000 units toward the target and tells if ground lies below it.
 * Without ground below the fighter the move is always safe.
 *
 * @param fp CPU fighter.
 * @return TRUE when the move is safe, FALSE when it would end off stage.
 */
sb32 ftRemixAiPostIsLandingSafe(FTStruct *fp);

#endif

#endif
