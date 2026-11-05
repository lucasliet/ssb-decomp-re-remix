/*
 * ftremixairecoverydrift.c - level 10 recovery drift of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * _improve_recovery_drift, improved_recovery._find_closest_ledge and is_ledge_grabbed). Pure rules live in
 * ftremixairecoverydriftrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/map.h>
#include <mp/mpcollision.h>
#include <mp/mpdef.h>
#include <float.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixairecoverydrift.h"
#include "ft/ftremix/ftremixairecoverydriftrules.h"

#define FTREMIX_AI_DRIFT_DIRECTION_LEFT_EDGE 1
#define FTREMIX_AI_DRIFT_DIRECTION_RIGHT_EDGE (-1)

/** is_ledge_grabbed: TRUE when a fighter hangs on that line facing the direction the asm asks for. */
static sb32 ftRemixAiDriftIsLedgeGrabbed(s32 line_id, s32 direction)
{
    GObj *fighter_gobj;

    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        FTStruct *other_fp = ftGetStruct(fighter_gobj);

        if ((other_fp->is_cliff_hold != FALSE) && (other_fp->lr == direction) && (other_fp->coll_data.cliff_id == line_id))
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** TRUE when the edge of a floor line is a candidate: on screen and with no ground just below it. */
static sb32 ftRemixAiDriftEdgeIsCandidate(Vec3f *edge_pos)
{
    MPGroundData *ground = gMPCollisionGroundData;
    Vec3f probe;

    if (ftRemixAiDriftEdgeIsOnScreen(edge_pos->x, edge_pos->y, ground->map_bound_top, ground->map_bound_bottom,
                                     ground->map_bound_right, ground->map_bound_left) == 0)
    {
        return FALSE;
    }
    probe.x = edge_pos->x;
    probe.y = edge_pos->y - FTREMIX_AI_DRIFT_GROUND_PROBE_DROP;
    probe.z = 0.0F;

    return (func_ovl2_800F8FFC(&probe) == FALSE) ? TRUE : FALSE;
}

/** A floor line edge is grabbable when it carries the floor edge flag and nobody hangs on it already. */
static sb32 ftRemixAiDriftEdgeIsGrabbable(s32 line_id, s32 direction)
{
    if ((mpCollisionGetVertexFlagsLineID(line_id) & MAP_FLAG_FLOOREDGE) == 0)
    {
        return FALSE;
    }
    return (ftRemixAiDriftIsLedgeGrabbed(line_id, direction) == FALSE) ? TRUE : FALSE;
}

/**
 * _find_closest_ledge: the nearest edge over the floor lines, the left edges for direction 1 and the right edges for -1.
 * With no candidate the position stays (0, 0) and the ledge is not grabbable (the asm leaves the flag uninitialized).
 */
static void ftRemixAiDriftFindClosestLedge(FTStruct *fp, s32 direction, Vec2f *ledge_pos, sb32 *is_grabbable)
{
    u16 *line_ids = &gMPCollisionLineGroups[nMPLineKindFloor].line_id[0];
    s32 line_count = gMPCollisionLineGroups[nMPLineKindFloor].line_count;
    f32 min_distance = FLT_MAX;
    Vec3f edge_pos;
    f32 distance;
    s32 i;

    ledge_pos->x = 0.0F;
    ledge_pos->y = 0.0F;
    *is_grabbable = FALSE;

    for (i = 0; i < line_count; i++)
    {
        if (mpCollisionCheckExistLineID(line_ids[i]) == FALSE)
        {
            continue;
        }
        if (direction > 0)
        {
            mpCollisionGetFloorEdgeL(line_ids[i], &edge_pos);
        }
        else mpCollisionGetFloorEdgeR(line_ids[i], &edge_pos);

        if (ftRemixAiDriftEdgeIsCandidate(&edge_pos) == FALSE)
        {
            continue;
        }
        distance = ftRemixAiDriftLedgeDistance(fp->coll_data.p_translate->x, fp->coll_data.p_translate->y, edge_pos.x, edge_pos.y);

        if (distance >= min_distance)
        {
            continue;
        }
        min_distance = distance;
        ledge_pos->x = edge_pos.x;
        ledge_pos->y = edge_pos.y;
        *is_grabbable = ftRemixAiDriftEdgeIsGrabbable(line_ids[i], direction);
    }
}

/** The asm tests the whole byte 0x49 of the computer struct (eight flags, the last is is_attempt_specialhi_recovery). */
static sb32 ftRemixAiDriftFlagsByteIsSet(FTComputer *com)
{
    return com->is_within_vertical_bounds || com->ftcom_flags_0x49_b1 || com->ftcom_flags_0x49_b2 || com->ftcom_flags_0x49_b3 ||
           com->is_counterattack || com->is_shield_item_weapon || com->is_opponent_ra || com->is_attempt_specialhi_recovery;
}

static void ftRemixAiDriftClearFlagsByte(FTComputer *com)
{
    com->is_within_vertical_bounds = FALSE;
    com->ftcom_flags_0x49_b1 = FALSE;
    com->ftcom_flags_0x49_b2 = FALSE;
    com->ftcom_flags_0x49_b3 = FALSE;
    com->is_counterattack = FALSE;
    com->is_shield_item_weapon = FALSE;
    com->is_opponent_ra = FALSE;
    com->is_attempt_specialhi_recovery = FALSE;
}

/** Jigglypuff, Kirby and Dedede families always aim at the outside of the ledge (their later jumps are too low). */
static sb32 ftRemixAiDriftIsSlowJumper(s32 fkind)
{
    switch (fkind)
    {
    case REMIX_ID_JIGGLY:
    case REMIX_ID_JPUFF:
    case REMIX_ID_EPUFF:
    case REMIX_ID_NJIGGLY:
    case REMIX_ID_KIRBY:
    case REMIX_ID_JKIRBY:
    case REMIX_ID_NKIRBY:
    case REMIX_ID_DEDEDE:
    case REMIX_ID_NDEDEDE:
        return TRUE;
    }
    return FALSE;
}

static sb32 ftRemixAiDriftResetsSpecialFlag(s32 status_id)
{
    if ((status_id >= nFTCommonStatusDamageHi1) && (status_id <= nFTCommonStatusDamageFlyRoll))
    {
        return TRUE;
    }
    return (status_id > nFTCommonStatusDamageFlyRoll) && (status_id == nFTCommonStatusCliffCatch);
}

static void ftRemixAiDriftReadState(FTStruct *fp, FTRemixAiDriftState *state, sb32 left_grabbable, sb32 right_grabbable)
{
    FTComputer *com = &fp->computer;

    state->x = fp->coll_data.p_translate->x;
    state->y = fp->coll_data.p_translate->y;
    state->lr = fp->lr;
    state->left_x = com->cliff_left_pos.x;
    state->left_y = com->cliff_left_pos.y;
    state->right_x = com->cliff_right_pos.x;
    state->right_y = com->cliff_right_pos.y;
    state->left_grabbable = left_grabbable;
    state->right_grabbable = right_grabbable;
    state->special_flag_set = ftRemixAiDriftFlagsByteIsSet(com);
    state->is_fall_special = (fp->status_id == nFTCommonStatusFallSpecial) ? 1 : 0;
    state->resets_special_flag = ftRemixAiDriftResetsSpecialFlag(fp->status_id);
    state->is_slow_jumper = ftRemixAiDriftIsSlowJumper(fp->fkind);
    state->grab_x = fp->attr->cliffcatch_coll.x;
    state->grab_y = fp->attr->cliffcatch_coll.y;
    state->bounds_left = gMPCollisionBounds.current.left;
    state->bounds_right = gMPCollisionBounds.current.right;
}

void ftRemixAiRecoveryDrift(FTStruct *fp)
{
    FTComputer *com = &fp->computer;
    FTRemixAiDriftState state;
    FTRemixAiDriftResult result;
    sb32 left_grabbable;
    sb32 right_grabbable;

    if (ftRemixAiDriftGateOpens(ftRemixAiIsLevelTen(fp->level), fp->status_id == nFTCommonStatusFallSpecial, fp->jumps_used,
                                fp->attr->jumps_max, com->objective == nFTComputerObjectiveRecover,
                                fp->coll_data.floor_line_id) == 0)
    {
        return;
    }
    ftRemixAiDriftFindClosestLedge(fp, FTREMIX_AI_DRIFT_DIRECTION_LEFT_EDGE, &com->cliff_left_pos, &left_grabbable);
    ftRemixAiDriftFindClosestLedge(fp, FTREMIX_AI_DRIFT_DIRECTION_RIGHT_EDGE, &com->cliff_right_pos, &right_grabbable);

    ftRemixAiDriftReadState(fp, &state, left_grabbable, right_grabbable);
    ftRemixAiDriftComputeTarget(&state, &result);

    if (result.skipped != 0)
    {
        return;
    }
    if (result.clears_special_flag != 0)
    {
        ftRemixAiDriftClearFlagsByte(com);
    }
    com->target_pos.x = result.target_x;
    com->target_pos.y = result.target_y;
}

#endif
