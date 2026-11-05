#ifndef _FTREMIXAIRECOVERYDRIFTRULES_H_
#define _FTREMIXAIRECOVERYDRIFTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the level 10 recovery drift (AI.asm _improve_recovery_drift and improved_recovery._find_closest_ledge), so the
 * headless unit tests (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_DRIFT_BLASTZONE_PADDING 200.0F /* AI.asm BLASTZONE_PADDING, 0x4348 */
#define FTREMIX_AI_DRIFT_BELOW_DISTANCE_SCALE 1.5F /* a ledge above the fighter looks 1.5 times farther in Y */
#define FTREMIX_AI_DRIFT_FACING_BIAS 200.0F        /* the fighter X is pushed this far toward its facing side */
#define FTREMIX_AI_DRIFT_LEDGE_PADDING 300.0F      /* AI.asm LEDGE_PADDING, 0x4396 */
#define FTREMIX_AI_DRIFT_TOO_HIGH_Y 1000.0F        /* inside the stage, higher than a ledge by this much: no drift */
#define FTREMIX_AI_DRIFT_USP_AWAY_X 1000.0F        /* USP from under the stage while not facing the ledge */
#define FTREMIX_AI_DRIFT_GROUND_PROBE_DROP 50.0F   /* a ledge with ground this far below it is skipped */

typedef struct FTRemixAiDriftState
{
    float x;                     /* fighter position */
    float y;
    int lr;                      /* facing direction */
    float left_x;                /* nearest line left edge ("left ledge") */
    float left_y;
    float right_x;               /* nearest line right edge ("right ledge") */
    float right_y;
    int left_grabbable;
    int right_grabbable;
    int special_flag_set;        /* byte 0x49 of the computer struct is not zero (it holds is_attempt_specialhi_recovery) */
    int is_fall_special;
    int resets_special_flag;     /* status is a damage status or CliffCatch */
    int is_slow_jumper;          /* Jigglypuff, Kirby and Dedede families (multi jump recoveries) */
    float grab_x;                /* attributes 0xAC (cliffcatch_coll.x) */
    float grab_y;                /* attributes 0xB0 (cliffcatch_coll.y) */
    float bounds_left;           /* gMPCollisionBounds.current */
    float bounds_right;

} FTRemixAiDriftState;

typedef struct FTRemixAiDriftResult
{
    int skipped;                 /* inner gap too high: the target is left as the walk set it */
    float target_x;
    float target_y;
    int clears_special_flag;     /* the byte 0x49 is cleared (damage status or CliffCatch) */

} FTRemixAiDriftResult;

/**
 * Gate of the drift patch: level 10, and either FallSpecial, or all jumps used, or recovering with no floor below.
 * (The first two do not need the Recover objective.)
 */
extern int ftRemixAiDriftGateOpens(int level_ten, int is_fall_special, int jumps_used, int jumps_max, int objective_is_recover,
                                   int floor_line_id);

/** TRUE when an edge lies inside the blastzone shrunk by FTREMIX_AI_DRIFT_BLASTZONE_PADDING (_bounds_check). */
extern int ftRemixAiDriftEdgeIsOnScreen(float edge_x, float edge_y, float top, float bottom, float right, float left);

/** Distance used to pick the closest ledge: a ledge above the fighter counts 1.5 times its Y distance. */
extern float ftRemixAiDriftLedgeDistance(float own_x, float own_y, float edge_x, float edge_y);

/** Picks the target for the drift from the two ledges, the fighter and its ledge grab box. */
extern void ftRemixAiDriftComputeTarget(const FTRemixAiDriftState *state, FTRemixAiDriftResult *result);

#endif

#endif
