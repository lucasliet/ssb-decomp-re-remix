#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <mp/mpdef.h>
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixaiprediction.h"
#include "ft/ftremix/ftremixaipredictionrules.h"

/** Ground friction of the body times its character traction, 0 while airborne (AI.asm predict_my_x, _get_friction). */
static f32 ftRemixAiPredictionGroundFriction(FTStruct *fp)
{
    if (fp->ga != nMPKineticsGround)
    {
        return 0.0F;
    }
    return grRemixSurfaceFriction(fp->coll_data.floor_flags & MAP_VERTEX_MAT_MASK) * fp->attr->traction;
}

/** Predicted Y of a body after hit_frame frames: the current Y while grounded, the falling Y while airborne. */
static f32 ftRemixAiPredictionBodyY(FTStruct *fp, s32 hit_frame)
{
    f32 current_y = fp->joints[nFTPartsJointTopN]->translate.vec.f.y;

    if (fp->ga == nMPKineticsGround)
    {
        return current_y;
    }
    return ftRemixAiPredictionRulesFallY(current_y, fp->physics.vel_air.y, fp->attr->gravity, -fp->attr->tvel_base,
                                         fp->coll_data.floor_dist, fp->coll_data.floor_line_id >= 0, hit_frame);
}

/** The attack view range base of a CPU, see ftRemixAiPredictionRulesAttackViewBase. */
f32 ftRemixAiPredictionAttackViewBase(FTStruct *fp)
{
    return ftRemixAiPredictionRulesAttackViewBase(fp->level);
}

/**
 * Candidate filter of fix_movement_prediction: a running CPU (Dash, Run, RunBrake) only considers the attacks that can
 * come out of a run. Other levels and other states consider every candidate.
 */
sb32 ftRemixAiPredictionCandidateAllowed(FTStruct *fp, s32 input_kind)
{
    s32 is_running;

    if (fp->level != FTREMIX_AI_PREDICTION_LEVEL_10)
    {
        return TRUE;
    }
    is_running = (fp->status_id == nFTCommonStatusDash) || (fp->status_id == nFTCommonStatusRun) ||
                 (fp->status_id == nFTCommonStatusRunBrake);

    return ftRemixAiPredictionRulesRunningCandidateAllowed(is_running, input_kind);
}

/**
 * Predicted offsets of the candidate. The X offset keeps the vanilla facing factor (`* lr`), because the detect boxes
 * flip with the facing direction; the asm of P18 does not multiply by lr (see the port notes).
 */
void ftRemixAiPredictAttackPosition(FTStruct *cpu, FTStruct *target, s32 hit_frame, f32 *predict_x, f32 *predict_y)
{
    f32 my_pred_x;
    f32 delta_x;
    f32 cpu_x = cpu->joints[nFTPartsJointTopN]->translate.vec.f.x;
    f32 target_x = target->joints[nFTPartsJointTopN]->translate.vec.f.x;

    if (cpu->level != FTREMIX_AI_PREDICTION_LEVEL_10)
    {
        return;
    }
    my_pred_x = ftRemixAiPredictionRulesMyX(cpu_x, cpu->physics.vel_air.x, ftRemixAiPredictionGroundFriction(cpu), hit_frame);
    delta_x = ftRemixAiPredictionRulesDeltaX(my_pred_x, target_x, target->physics.vel_air.x, hit_frame);

    *predict_x = delta_x * cpu->lr;
    *predict_y = ftRemixAiPredictionBodyY(target, hit_frame) - ftRemixAiPredictionBodyY(cpu, hit_frame);
}

#endif
