#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <mp/mpdef.h>
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixaimodes.h"
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

/**
 * Position, speed and fall numbers of a body, as ftComputerCheckDetectTarget keeps them on its stack (the asm reads the
 * slots 0x1AC to 0x198 for the CPU and 0x194 to 0x180 for the target).
 */
typedef struct FTRemixAiPredictionMotion
{
    f32 pos_x;
    f32 pos_y;
    f32 vel_x;
    f32 vel_y;
    f32 gravity;
    f32 max_fall;

} FTRemixAiPredictionMotion;

/** The motion numbers of a fighter. */
static void ftRemixAiPredictionGetFighterMotion(FTStruct *fp, FTRemixAiPredictionMotion *motion)
{
    motion->pos_x = fp->joints[nFTPartsJointTopN]->translate.vec.f.x;
    motion->pos_y = fp->joints[nFTPartsJointTopN]->translate.vec.f.y;
    motion->vel_x = fp->physics.vel_air.x;
    motion->vel_y = fp->physics.vel_air.y;
    motion->gravity = fp->attr->gravity;
    motion->max_fall = -fp->attr->tvel_base;
}

/**
 * The motion numbers of the attack target: the target fighter, or the ball in Smashketball. Smashketball.asm
 * ai_ball_attack_range (0x80133144) writes the ball into the target stack slots before fix_movement_prediction reads them.
 */
static void ftRemixAiPredictionGetTargetMotion(FTStruct *target, FTRemixAiPredictionMotion *motion)
{
    FTRemixAiBallTarget ball_target;

    if (ftRemixAiModesGetBallTarget(&ball_target) == FALSE)
    {
        ftRemixAiPredictionGetFighterMotion(target, motion);
        return;
    }
    motion->pos_x = ball_target.pos_x;
    motion->pos_y = ball_target.pos_y;
    motion->vel_x = ball_target.vel_x;
    motion->vel_y = ball_target.vel_y;
    motion->gravity = ball_target.gravity;
    motion->max_fall = ball_target.tvel_base;
}

/**
 * Predicted Y of a body after hit_frame frames: the current Y while grounded, the falling Y while airborne. The grounded
 * state and the floor under the body are always read from the fighter (asm 0x14C, 0xF0 and 0xEC of the fighter struct),
 * also when the motion numbers are the Smashketball ball.
 */
static f32 ftRemixAiPredictionBodyY(FTStruct *fp, const FTRemixAiPredictionMotion *motion, s32 hit_frame)
{
    if (fp->ga == nMPKineticsGround)
    {
        return motion->pos_y;
    }
    return ftRemixAiPredictionRulesFallY(motion->pos_y, motion->vel_y, motion->gravity, motion->max_fall,
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
 * flip with the facing direction; the asm of P18 does not multiply by lr (see the port notes). In Smashketball the
 * target numbers are the ball's, as for the lower levels.
 */
void ftRemixAiPredictAttackPosition(FTStruct *cpu, FTStruct *target, s32 hit_frame, f32 *predict_x, f32 *predict_y)
{
    FTRemixAiPredictionMotion cpu_motion;
    FTRemixAiPredictionMotion target_motion;
    f32 my_pred_x;
    f32 delta_x;

    if (cpu->level != FTREMIX_AI_PREDICTION_LEVEL_10)
    {
        return;
    }
    ftRemixAiPredictionGetFighterMotion(cpu, &cpu_motion);
    ftRemixAiPredictionGetTargetMotion(target, &target_motion);

    my_pred_x = ftRemixAiPredictionRulesMyX(cpu_motion.pos_x, cpu_motion.vel_x, ftRemixAiPredictionGroundFriction(cpu), hit_frame);
    delta_x = ftRemixAiPredictionRulesDeltaX(my_pred_x, target_motion.pos_x, target_motion.vel_x, hit_frame);

    *predict_x = delta_x * cpu->lr;
    *predict_y = ftRemixAiPredictionBodyY(target, &target_motion, hit_frame) - ftRemixAiPredictionBodyY(cpu, &cpu_motion, hit_frame);
}

#endif
