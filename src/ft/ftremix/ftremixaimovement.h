#ifndef _FTREMIXAIMOVEMENT_H_
#define _FTREMIXAIMOVEMENT_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Level 10 movement hooks of the Remix AI (AI.asm prevent_fastfall, detect_offstage_opponents, clamp_target_pos_to_stage,
 * shorthop_followup_attack, jump_in_neutral). */

/** prevent_fastfall: TRUE when the fastfall input may be pressed. Level 10 also clears the "already tried" flag. */
sb32 ftRemixAiMovementFastfallPermitted(FTStruct *fp);

/** shorthop_followup_attack: the input for a grounded CPU whose target is above. Level 10 only, else vanilla_input. */
s32 ftRemixAiMovementShorthopInput(FTStruct *fp, s32 vanilla_input);

/** jump_in_neutral: the neutral walk input. Level 10 rolls for a jump and sets the Attack objective when it jumps. */
s32 ftRemixAiMovementNeutralInput(FTStruct *fp);

/** detect_offstage_opponents: TRUE when the CPU may target an opponent who is offstage (level 10). */
sb32 ftRemixAiMovementTargetsOffstage(FTStruct *fp);

/** clamp_target_pos_to_stage: clamps the chosen target position in place (level 10 only). */
void ftRemixAiMovementClampTarget(FTStruct *fp, Vec2f *target);

#endif

#endif
