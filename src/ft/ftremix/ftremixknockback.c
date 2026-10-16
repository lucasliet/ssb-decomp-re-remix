/**
 * ftremixknockback.c - Smash Remix Knockback.asm: randomized knockback angles per action.
 *
 * randomize_angles_ (ftManagerMakeFighter), get_angle_hitbox_ (ftMainParseMotionEvent) and
 * get_angle_throw_ (ftCommonThrownReleaseThrownUpdateStats) call into this file.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/utils.h>

#define FTREMIX_KNOCKBACK_PLAYERS 4
#define FTREMIX_KNOCKBACK_ACTIONS 0x200
#define FTREMIX_KNOCKBACK_SAKURAI_ANGLE 361

#define FTREMIX_KNOCKBACK_CHANCE_GOOD 60
#define FTREMIX_KNOCKBACK_CHANCE_F_YOU 15
#define FTREMIX_KNOCKBACK_CHANCE_RANDOM 15

/* REMIX-TODO(toggles): the CSS Knockback menu (css/Knockback.asm) writes this word per port.
 * Remix default is 0 ("Default"), which keeps the vanilla angles. 1 ("Random") uses the table. */
static s32 sFTRemixKnockbackState[FTREMIX_KNOCKBACK_PLAYERS];

/* Scene the angles were last rolled for, per port (asm screen_table). */
static u8 sFTRemixKnockbackScreen[FTREMIX_KNOCKBACK_PLAYERS];

/* Knockback angle per port and action id (asm table_p1..table_p4). */
static u16 sFTRemixKnockbackAngles[FTREMIX_KNOCKBACK_PLAYERS][FTREMIX_KNOCKBACK_ACTIONS];

/**
 * Returns TRUE when the scene changed since the last roll for this port (asm randomize_angles_).
 * The scene is stored on every call. Re-entering the same scene (training reset, salty runback)
 * keeps the angles already rolled.
 */
static sb32 ftRemixKnockbackIsNewScene(s32 player)
{
    u8 current_scene = gSCManagerSceneData.scene_curr;
    u8 previous_scene = sFTRemixKnockbackScreen[player];

    sFTRemixKnockbackScreen[player] = current_scene;
    return current_scene != previous_scene;
}

/**
 * Rolls one knockback angle: 60% good angle (45-135), 15% semi spike or spike (180-360),
 * 15% fully random (0-360), otherwise the Sakurai angle (361).
 */
static u16 ftRemixKnockbackRollAngle(void)
{
    s32 roll = syUtilsRandIntRange(100);

    if (roll < FTREMIX_KNOCKBACK_CHANCE_GOOD)
    {
        return 45 + syUtilsRandIntRange(91);
    }
    if (roll < FTREMIX_KNOCKBACK_CHANCE_GOOD + FTREMIX_KNOCKBACK_CHANCE_F_YOU)
    {
        return 180 + syUtilsRandIntRange(181);
    }
    if (roll < FTREMIX_KNOCKBACK_CHANCE_GOOD + FTREMIX_KNOCKBACK_CHANCE_F_YOU + FTREMIX_KNOCKBACK_CHANCE_RANDOM)
    {
        return syUtilsRandIntRange(361);
    }
    return FTREMIX_KNOCKBACK_SAKURAI_ANGLE;
}

/**
 * Re-rolls the knockback angle of every action for the fighter's port when the scene changed.
 * Called from ftManagerMakeFighter right after the port is stored in the fighter struct.
 */
void ftRemixKnockbackRandomizeAngles(FTStruct *fp)
{
    s32 action_id;
    s32 player = fp->player;

    if (player >= FTREMIX_KNOCKBACK_PLAYERS)
    {
        return;
    }
    if (!ftRemixKnockbackIsNewScene(player))
    {
        return;
    }
    for (action_id = 0; action_id < FTREMIX_KNOCKBACK_ACTIONS; action_id++)
    {
        sFTRemixKnockbackAngles[player][action_id] = ftRemixKnockbackRollAngle();
    }
}

/**
 * Returns the knockback angle for the fighter's current action when the randomizer is on.
 * Returns vanilla_angle otherwise, including for out-of-range port or action ids.
 */
s32 ftRemixKnockbackAngle(FTStruct *fp, s32 vanilla_angle)
{
    s32 player = fp->player;
    s32 action_id = fp->status_id;

    if (player >= FTREMIX_KNOCKBACK_PLAYERS || sFTRemixKnockbackState[player] == 0)
    {
        return vanilla_angle;
    }
    if (action_id < 0 || action_id >= FTREMIX_KNOCKBACK_ACTIONS)
    {
        return vanilla_angle;
    }
    return sFTRemixKnockbackAngles[player][action_id];
}

#endif
