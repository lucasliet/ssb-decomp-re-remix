/**
 * gmremixspecialzoomko.c - Smash Remix SpecialZoom.asm: the KO prediction and the decisive stock check that decide the
 * Special Zoom (BETA) setting of a hit (check_ko, CheckDecisiveStock, CheckPlayersInMatch).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/map.h>

extern f32 atan2f(f32 y, f32 x);
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "ft/ftremix/ftremixhooks.h"

#define FTREMIX_SPECIAL_ZOOM_KB_STEP 1.7F

/* Count of live teammates of fp (vanilla ifCommonBattleUpdateScoreStocks, first loop; the copy in SpecialZoom.asm
 * CheckDecisiveStock is the same loop) */
static s32 ftRemixSpecialZoomCountTeammates(FTStruct *fp)
{
    s32 teammates_remain = 0;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gSCManagerBattleState->players); i++)
    {
        SCPlayerData *player = &gSCManagerBattleState->players[i];
        s32 current_team;

        if (player->pkind == nFTPlayerKindNot)
        {
            continue;
        }
        current_team = gSCManagerBattleState->is_team_battle ? player->team : i;

        if ((current_team == fp->team) && (player->stock_count != -1))
        {
            teammates_remain++;
        }
    }
    return teammates_remain;
}

/**
 * SpecialZoom.asm CheckDecisiveStock: TRUE when fp is the only live fighter of its team, the match has two teams left
 * and fp has no extra stocks left.
 */
sb32 ftRemixSpecialZoomIsDecisiveStock(FTStruct *fp)
{
    if (ftRemixSpecialZoomCountTeammates(fp) != 1)
    {
        return FALSE;
    }
    if (ifRemixBattlePlace() != 1)
    {
        return FALSE;
    }
    return fp->stock_count == 0;
}

/**
 * SpecialZoom.asm CheckPlayersInMatch: number of players still in the match. The time rule counts every fighter, the
 * other rules count the fighters with stocks left.
 */
s32 ftRemixSpecialZoomCountPlayersInMatch(void)
{
    s32 count = 0;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(gSCManagerBattleState->players); i++)
    {
        SCPlayerData *player = &gSCManagerBattleState->players[i];

        if (player->fighter_gobj == NULL)
        {
            continue;
        }
        if (gSCManagerBattleState->game_rules == SCBATTLE_GAMERULE_TIME)
        {
            count++;
        }
        else if (player->stock_count >= 0)
        {
            count++;
        }
    }
    return count;
}

/**
 * Knockback gravity of one frame of the KO simulation: the accumulator adds the gravity and is clamped to the terminal
 * velocity of the attributes (SpecialZoom.asm check_ko, the clamp keeps the accumulator below tvel_base).
 */
static f32 ftRemixSpecialZoomAddGravity(FTAttributes *attr, f32 gravity_sum)
{
    f32 gravity_next = gravity_sum + attr->gravity;

    if (gravity_next >= attr->tvel_base)
    {
        return attr->tvel_base;
    }
    return gravity_next;
}

/**
 * TRUE when the fighter's final position is outside the map blast zones: top (the highest point of the trajectory), left,
 * right, and bottom only when the fighter has no ground below (floor_line_id of -1).
 */
static sb32 ftRemixSpecialZoomIsOutsideBlastZone(FTStruct *fp, f32 final_x, f32 final_y, f32 max_y)
{
    MPGroundData *ground = gMPCollisionGroundData;

    if (ground->map_bound_top <= max_y)
    {
        return TRUE;
    }
    if (final_x <= ground->map_bound_left)
    {
        return TRUE;
    }
    if (ground->map_bound_right <= final_x)
    {
        return TRUE;
    }
    if (fp->coll_data.floor_line_id != -1)
    {
        return FALSE;
    }
    return final_y <= ground->map_bound_bottom;
}

/**
 * SpecialZoom.asm check_ko: simulates the knockback trajectory of fp for its hitstun tics (at least one step) and returns
 * TRUE when the fighter ends up in a blast zone.
 */
sb32 ftRemixSpecialZoomCheckKO(FTStruct *fp)
{
    Vec3f *pos = fp->coll_data.p_translate;
    f32 vel_x = fp->physics.vel_damage_air.x;
    f32 vel_y = fp->physics.vel_damage_air.y;
    f32 angle = atan2f(fp->physics.vel_damage_air.y, fp->physics.vel_damage_air.x);
    f32 x = pos->x;
    f32 y = pos->y;
    f32 max_y = pos->y;
    f32 gravity_sum = 0.0F;
    s32 tics = fp->status_vars.common.damage.hitstun_tics;

    do
    {
        vel_x -= FTREMIX_SPECIAL_ZOOM_KB_STEP * __cosf(angle);
        x += vel_x;

        vel_y -= FTREMIX_SPECIAL_ZOOM_KB_STEP * __sinf(angle);
        y += vel_y;

        gravity_sum = ftRemixSpecialZoomAddGravity(fp->attr, gravity_sum);
        y -= gravity_sum;

        if (max_y < y)
        {
            max_y = y;
        }
        tics--;
    } while (tics > 0);

    return ftRemixSpecialZoomIsOutsideBlastZone(fp, x, y, max_y);
}

#endif
