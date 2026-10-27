/**
 * ftremixsize.c - Smash Remix body size subsystem, phase (a) of Size.asm.
 *
 * Holds the per-player size multiplier and applies it to the fighter top joint,
 * the ECB (map collision diamond), the ledge grab box, the camera zoom and the
 * camera offset. The multiplier defaults to 1.0F, so nothing changes until a
 * source (Super Mushroom, Poison Mushroom...) calls ftRemixSizeSetMultiplier.
 * Physics (walk speed, weight, jump height) and cosmetic GFX are later phases.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmdef.h>
#include <gm/gmremixpanel.h>
#include <port_log.h>
#include <sc/scene.h>
#include <sc/scmanager.h>

/* Size.asm multiplier_table: current size multiplier of each player. */
static f32 sFTRemixSizeMultiplier[GMCOMMON_PLAYERS_MAX] = { 1.0F, 1.0F, 1.0F, 1.0F };

/* Size.asm base_multiplier_table: the body size of each player, the value its top joint is scaled from. */
static f32 sFTRemixSizeBaseMultiplier[GMCOMMON_PLAYERS_MAX] = { 1.0F, 1.0F, 1.0F, 1.0F };

/* Size.asm match_state_table: GIANT or TINY forces the body size through the whole match. */
static s32 sFTRemixSizeMatchState[GMCOMMON_PLAYERS_MAX];

/* Size state a mode forces on a port (the Remix 1P giant and tiny stages) in place of the CSS panel, FTREMIX_SIZE_STATE_UNFORCED when none. */
static s32 sFTRemixSizeForcedState[GMCOMMON_PLAYERS_MAX] = { FTREMIX_SIZE_STATE_UNFORCED, FTREMIX_SIZE_STATE_UNFORCED,
                                                              FTREMIX_SIZE_STATE_UNFORCED, FTREMIX_SIZE_STATE_UNFORCED };

/** TRUE when player is a valid index into the per-player tables, logs the offending value otherwise. */
static sb32 ftRemixSizePlayerIsValid(s32 player)
{
    if ((player < 0) || (player >= GMCOMMON_PLAYERS_MAX))
    {
        port_log("ftRemixSize: player index %d is outside 0..%d\n", player, GMCOMMON_PLAYERS_MAX - 1);
        return FALSE;
    }
    return TRUE;
}

/** Current size multiplier of player, 1.0F for an invalid index. */
f32 ftRemixSizeGetMultiplier(s32 player)
{
    if (!ftRemixSizePlayerIsValid(player))
    {
        return 1.0F;
    }
    return sFTRemixSizeMultiplier[player];
}

/** Sets the size multiplier of player. Ignored for an invalid index. */
void ftRemixSizeSetMultiplier(s32 player, f32 multiplier)
{
    if (!ftRemixSizePlayerIsValid(player))
    {
        return;
    }
    sFTRemixSizeMultiplier[player] = multiplier;
}

/** Match size state (FTREMIX_SIZE_STATE_*) of player, NORMAL for an invalid index. */
s32 ftRemixSizeGetMatchState(s32 player)
{
    if (!ftRemixSizePlayerIsValid(player))
    {
        return FTREMIX_SIZE_STATE_NORMAL;
    }
    return sFTRemixSizeMatchState[player];
}

/** Sets the match size state of player (the CSS size menu writes it). Ignored for an invalid index or state. */
void ftRemixSizeSetMatchState(s32 player, s32 state)
{
    if (!ftRemixSizePlayerIsValid(player))
    {
        return;
    }
    if ((state < FTREMIX_SIZE_STATE_NORMAL) || (state > FTREMIX_SIZE_STATE_TINY))
    {
        port_log("ftRemixSize: match state %d is outside 0..%d\n", state, FTREMIX_SIZE_STATE_TINY);
        return;
    }
    sFTRemixSizeMatchState[player] = state;
}

/** Forces the size state of player for the fighters created from now on, FTREMIX_SIZE_STATE_UNFORCED gives the port back to the CSS panel. */
void ftRemixSizeForceMatchState(s32 player, s32 state)
{
    if (!ftRemixSizePlayerIsValid(player))
    {
        return;
    }
    if ((state < FTREMIX_SIZE_STATE_UNFORCED) || (state > FTREMIX_SIZE_STATE_TINY))
    {
        port_log("ftRemixSize: forced state %d is outside %d..%d\n", state, FTREMIX_SIZE_STATE_UNFORCED, FTREMIX_SIZE_STATE_TINY);
        return;
    }
    sFTRemixSizeForcedState[player] = state;
}

/** Multiplier of the match size state of player: 2.25F GIANT, 0.5F TINY, 1.0F NORMAL. */
f32 ftRemixSizeGetModeMultiplier(s32 player)
{
    switch (ftRemixSizeGetMatchState(player))
    {
        case FTREMIX_SIZE_STATE_GIANT:
            return FTREMIX_SIZE_GIANT_MULTIPLIER;

        case FTREMIX_SIZE_STATE_TINY:
            return FTREMIX_SIZE_TINY_MULTIPLIER;

        default:
            return 1.0F;
    }
}

/**
 * Records the body size of a new fighter and sets its multiplier: 1.0F, or the size of its match state outside the
 * How to Play and Auto-Demo screens (Size.asm initialize_base_multiplier_table_).
 * Called from ftManagerInitFighter.
 */
void ftRemixSizeInitFighter(FTStruct *fp)
{
    if (!ftRemixSizePlayerIsValid(fp->player))
    {
        return;
    }
    ftRemixSizeSetMatchState(fp->player, (sFTRemixSizeForcedState[fp->player] != FTREMIX_SIZE_STATE_UNFORCED) ? sFTRemixSizeForcedState[fp->player]
                                                                                                           : gmRemixPanel(SIZE, fp->player));
    sFTRemixSizeBaseMultiplier[fp->player] = fp->attr->size;
    sFTRemixSizeMultiplier[fp->player] = 1.0F;

    if ((gSCManagerSceneData.scene_curr == nSCKindExplain) || (gSCManagerSceneData.scene_curr == nSCKindAutoDemo))
    {
        return;
    }
    sFTRemixSizeMultiplier[fp->player] = ftRemixSizeGetModeMultiplier(fp->player);
}

/** Scales the top joint by base size times multiplier. */
static void ftRemixSizeApplyTopJointScale(FTStruct *fp, f32 multiplier)
{
    DObj *topn_joint = fp->joints[nFTPartsJointTopN];
    f32 scale = sFTRemixSizeBaseMultiplier[fp->player] * multiplier;

    topn_joint->scale.vec.f.x = scale;
    topn_joint->scale.vec.f.y = scale;
    topn_joint->scale.vec.f.z = scale;
}

/** Scales the ECB diamond and the ledge grab box from the attributes. */
static void ftRemixSizeApplyEcb(FTStruct *fp, f32 multiplier)
{
    FTAttributes *attr = fp->attr;

    fp->coll_data.map_coll.top = attr->map_coll.top * multiplier;
    fp->coll_data.map_coll.center = attr->map_coll.center * multiplier;
    fp->coll_data.map_coll.bottom = attr->map_coll.bottom * multiplier;
    fp->coll_data.map_coll.width = attr->map_coll.width * multiplier;
    fp->coll_data.cliffcatch_coll.x = attr->cliffcatch_coll.x * multiplier;
    fp->coll_data.cliffcatch_coll.y = attr->cliffcatch_coll.y * multiplier;
}

/**
 * Size.asm adjust_top_joint_and_ecb_ (RAM 0x800E2068): applied in ftMainProcPhysicsMap
 * right after the TopN position is copied to the previous position.
 */
void ftRemixSizeApplyTopJointAndEcb(FTStruct *fp)
{
    f32 multiplier;

    if (!ftRemixSizePlayerIsValid(fp->player))
    {
        return;
    }
    multiplier = ftRemixSizeGetMultiplier(fp->player);
    ftRemixSizeApplyTopJointScale(fp, multiplier);
    ftRemixSizeApplyEcb(fp, multiplier);
}

/**
 * Camera zoom factor of a fighter: 1 + (multiplier - 1) / 2, so half of the size change
 * reaches the zoom (Size.asm adjust_camera_zoom_, RAM 0x8010BB9C).
 */
f32 ftRemixSizeCameraZoomFactor(FTStruct *fp)
{
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);

    return 1.0F + ((multiplier - 1.0F) * 0.5F);
}

/**
 * Scales a value that belongs to the fighter's body (camera offset, ECB display box) by the
 * size multiplier of the fighter. Returns value unchanged while the multiplier is 1.0F.
 */
f32 ftRemixSizeScaleValue(FTStruct *fp, f32 value)
{
    return value * ftRemixSizeGetMultiplier(fp->player);
}

/**
 * Size.asm attribute scaling: x = 1 + coefficient * (size - 1). The coefficients are the Size.asm constants
 * (walking_speed_multiplier ... weight_multiplier), see the FTREMIX_SIZE_COEF_* defines in ftremixhooks.h.
 */
f32 ftRemixSizeAttrMultiplier(s32 player, f32 coefficient)
{
    return 1.0F + (coefficient * (ftRemixSizeGetMultiplier(player) - 1.0F));
}

#endif
