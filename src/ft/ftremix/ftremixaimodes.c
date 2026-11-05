/*
 * ftremixaimodes.c - CPU hooks of the Remix game modes (.toolchain/pkgs/smashremix-src/src/Smashketball.asm and
 * KingOfTheHill.asm). Pure rules live in ftremixaimodesrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixkoth.h>
#include <gm/gmremixkothmodel.h>
#include <gm/gmremixsmashketball.h>
#include <it/item.h>
#include "ft/ftremix/ftremixaimodes.h"
#include "ft/ftremix/ftremixaimodesrules.h"
#include "it/itremix/itremixbasketball.h"

/** The ball in play during a Smashketball match, NULL otherwise. */
static GObj* ftRemixAiModesFindBall(void)
{
    if (gmRemixSmashketballIsActive() == FALSE)
    {
        return NULL;
    }
    return itRemixBasketballFind();
}

sb32 ftRemixAiModesSeekBall(FTStruct *fp, f32 this_pos_x, f32 this_pos_y, f32 *distance)
{
    GObj *ball_gobj = ftRemixAiModesFindBall();
    Vec3f ball_pos;
    f32 target_x;

    if (ball_gobj == NULL)
    {
        return FALSE;
    }
    itRemixBasketballGetPosition(ball_gobj, &ball_pos);

    target_x = ball_pos.x + ftRemixAiModesBallSeekOffset(fp->team, gmRemixSmashketballGetHomeTeam());

    fp->computer.target_pos.x = target_x;
    fp->computer.target_pos.y = ball_pos.y;

    *distance = ftRemixAiModesSquaredDistance(this_pos_x, this_pos_y, target_x, ball_pos.y);
    return TRUE;
}

sb32 ftRemixAiModesGetBallTarget(FTRemixAiBallTarget *ball_target)
{
    GObj *ball_gobj = ftRemixAiModesFindBall();
    ITStruct *ip;
    Vec3f ball_pos;

    if (ball_gobj == NULL)
    {
        return FALSE;
    }
    ip = itGetStruct(ball_gobj);
    itRemixBasketballGetPosition(ball_gobj, &ball_pos);

    ball_target->hurtbox_width = FTREMIX_AI_MODES_BALL_HURTBOX_SIZE;
    ball_target->hurtbox_height = FTREMIX_AI_MODES_BALL_HURTBOX_SIZE;
    ball_target->pos_x = ball_pos.x;
    ball_target->pos_y = ball_pos.y;
    ball_target->vel_x = ip->physics.vel_air.x;
    ball_target->vel_y = ip->physics.vel_air.y;
    ball_target->tvel_base = FTREMIX_AI_MODES_BALL_FALL_SPEED;
    ball_target->gravity = FTREMIX_AI_MODES_BALL_GRAVITY;
    return TRUE;
}

sb32 ftRemixAiModesSeekHill(FTStruct *fp)
{
    FTComputer *com = &fp->computer;
    f32 marker_x;
    f32 marker_y;

    if (gmRemixKothIsActive() == FALSE)
    {
        return FALSE;
    }
    if (gmRemixKothModelGetMarkerPosition(&marker_x, &marker_y) == FALSE)
    {
        com->target_line_id = -1;
        return FALSE;
    }
    com->target_pos.x = marker_x;
    com->target_pos.y = ftRemixAiModesHillTargetY(marker_y);
    com->target_line_id = gmRemixKothGetHillLine();
    return TRUE;
}

#endif
