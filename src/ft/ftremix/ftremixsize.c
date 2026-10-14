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
#include <port_log.h>

/* Size.asm multiplier_table: current size multiplier of each player. */
static f32 sFTRemixSizeMultiplier[GMCOMMON_PLAYERS_MAX] = { 1.0F, 1.0F, 1.0F, 1.0F };

/* Size.asm base_multiplier_table: the body size of each player, the value its top joint is scaled from. */
static f32 sFTRemixSizeBaseMultiplier[GMCOMMON_PLAYERS_MAX] = { 1.0F, 1.0F, 1.0F, 1.0F };

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

/**
 * Records the body size of a new fighter and resets its multiplier to 1.0F.
 * Called from ftManagerInitFighter, where Size.asm initialize_base_multiplier_table_ runs.
 */
void ftRemixSizeInitFighter(FTStruct *fp)
{
    if (!ftRemixSizePlayerIsValid(fp->player))
    {
        return;
    }
    sFTRemixSizeBaseMultiplier[fp->player] = fp->attr->size;
    sFTRemixSizeMultiplier[fp->player] = 1.0F;
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

#endif
