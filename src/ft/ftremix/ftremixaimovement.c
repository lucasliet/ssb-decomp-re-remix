#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <mp/mpdef.h>
#include <mp/map.h>
#include <sys/utils.h>
#include "ft/ftremix/ftremixaimovement.h"
#include "ft/ftremix/ftremixaimovementrules.h"

/** The FTStruct target of the CPU, or NULL when there is none. */
static FTStruct *ftRemixAiMovementTargetFighter(FTStruct *fp)
{
    return fp->computer.target_user;
}

/** Floor line of the target, 0 when there is no target (the rule ignores it then). */
static s32 ftRemixAiMovementTargetFloorLineId(FTStruct *target_fp)
{
    if (target_fp == NULL)
    {
        return 0;
    }
    return target_fp->coll_data.floor_line_id;
}

/**
 * prevent_fastfall. The asm clears the "already tried fastfall" flag before the checks, so a CPU that was denied can
 * try again on a later frame.
 */
sb32 ftRemixAiMovementFastfallPermitted(FTStruct *fp)
{
    FTStruct *target_fp;

    if (fp->level != FTREMIX_AI_MOVEMENT_LEVEL_10)
    {
        return TRUE;
    }
    fp->computer.ftcom_flags_0x4A_b0 = FALSE;
    target_fp = ftRemixAiMovementTargetFighter(fp);

    return ftRemixAiMovementRulesFastfallAllowed(fp->jumps_used, fp->attr->jumps_max, target_fp != NULL,
                                                 ftRemixAiMovementTargetFloorLineId(target_fp), fp->coll_data.floor_dist);
}

/**
 * shorthop_followup_attack. The gap is the target Y plus its air Y velocity (one frame ahead) minus the CPU Y, as the
 * asm computes it.
 */
s32 ftRemixAiMovementShorthopInput(FTStruct *fp, s32 vanilla_input)
{
    FTStruct *target_fp;
    f32 diff;

    if (fp->level != FTREMIX_AI_MOVEMENT_LEVEL_10)
    {
        return vanilla_input;
    }
    target_fp = ftRemixAiMovementTargetFighter(fp);
    if (target_fp == NULL)
    {
        return vanilla_input;
    }
    diff = (fp->computer.target_pos.y + target_fp->physics.vel_air.y) - fp->joints[nFTPartsJointTopN]->translate.vec.f.y;

    return ftRemixAiMovementRulesShorthopInput(diff, vanilla_input);
}

/**
 * jump_in_neutral. Level 10 rolls the port RNG on every call in this state (0..199). Only a jump roll sets the Attack
 * objective, as the asm does in its _modified path.
 */
s32 ftRemixAiMovementNeutralInput(FTStruct *fp)
{
    s32 input_kind;

    if (fp->level != FTREMIX_AI_MOVEMENT_LEVEL_10)
    {
        return nFTComputerInputMoveAuto;
    }
    input_kind = ftRemixAiMovementRulesJumpInNeutralInput(syUtilsRandIntRange(FTREMIX_AI_MOVEMENT_JUMP_IN_NEUTRAL_ROLL_MAX));

    if (input_kind != FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK)
    {
        fp->computer.objective = nFTComputerObjectiveAttack;
    }
    return input_kind;
}

/** detect_offstage_opponents: the on-stage test is skipped for level 10 only. */
sb32 ftRemixAiMovementTargetsOffstage(FTStruct *fp)
{
    return ftRemixAiMovementRulesOffstageTargetAllowed(fp->level);
}

/**
 * clamp_target_pos_to_stage. The X bounds are the current map bounds (MPAllBounds.current, left and right). The Y clamp
 * is the asm one, applied after the X clamp.
 */
void ftRemixAiMovementClampTarget(FTStruct *fp, Vec2f *target)
{
    if (fp->level != FTREMIX_AI_MOVEMENT_LEVEL_10)
    {
        return;
    }
    target->x = ftRemixAiMovementRulesClampTargetX(target->x, gMPCollisionBounds.current.left, gMPCollisionBounds.current.right);
    target->y = ftRemixAiMovementRulesClampTargetY(target->y);
}

#endif
