/**
 * ftremixleftovers.c - Smash Remix leftover vanilla patches: Piano's tongue and held item visibility
 * (Piano/Piano.asm extra_part_visibility_, called from ftMainSetStatus) and Marina's items that stay on the floor
 * (Item.asm prevent_item_despawn_, called from itMapCheckDestroyDropped). Each helper is a no-op for every other
 * fighter or item.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixhooks.h"
#include "it/item.h"
#include "it/itremix/itremix.h"
#include <sys/objman.h>

enum
{
    nFTRemixPianoLeftoverActionLightItemPickup = 0x064,     /* Action.LightItemPickup, Action.asm line 105 */
    nFTRemixPianoLeftoverActionHammerLanding = 0x097,       /* Action.HammerLanding, Action.asm line 156 */
    nFTRemixPianoLeftoverActionGrab = 0x0A6,                /* Action.Grab, Action.asm line 171 */
    nFTRemixPianoLeftoverActionGrabPull = 0x0A7,            /* Action.GrabPull, Action.asm line 172 */
    nFTRemixPianoLeftoverTongueJointFirst = 0x0E,           /* Piano.asm extra_part_visibility_ loop, first part */
    nFTRemixPianoLeftoverTongueJointLast = 0x10             /* Piano.asm extra_part_visibility_ loop, last part */
};

extern const ITRemixItemDesc dITRemixTurnipItemDesc;
extern const ITRemixItemDesc dITRemixGemItemDesc;
extern const ITRemixItemDesc dITRemixShurikenItemDesc;
extern const ITRemixItemDesc dITRemixBoomerangItemDesc;
extern const ITRemixItemDesc dITRemixClanBombItemDesc;

/* Piano.asm extra_part_visibility_: TRUE for the actions that keep the tongue and held item visible. */
static sb32 ftRemixPianoLeftoverPartsAreShown(s32 action)
{
    if ((action == nFTRemixPianoLeftoverActionGrab) || (action == nFTRemixPianoLeftoverActionGrabPull))
    {
        return TRUE;
    }
    return (action >= nFTRemixPianoLeftoverActionLightItemPickup) && (action <= nFTRemixPianoLeftoverActionHammerLanding);
}

/* Piano.asm extra_part_visibility_: the joints 0x0E to 0x10 are hidden and their display lists are removed, so
 * vanilla reverts them through is_modelpart_modify on the next action change. */
static void ftRemixPianoLeftoverHideTongueParts(FTStruct *fp)
{
    s32 joint_id;

    for (joint_id = nFTRemixPianoLeftoverTongueJointFirst; joint_id <= nFTRemixPianoLeftoverTongueJointLast; joint_id++)
    {
        fp->modelpart_status[joint_id - nFTPartsJointCommonStart].modelpart_id_curr = -1;
        gcRemoveMObjAll(fp->joints[joint_id]);
        fp->joints[joint_id]->dl = NULL;
    }
}

/**
 * Hides Piano's tongue and held item outside the grab and item pickup actions. Called in ftMainSetStatus right after
 * is_item_show is set to TRUE, the vanilla value that this helper overrides for Piano.
 * @param fp Fighter whose status was just set.
 */
void ftRemixPianoExtraPartVisibility(FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_PIANO)
    {
        return;
    }
    if (ftRemixPianoLeftoverPartsAreShown(fp->status_id))
    {
        return;
    }
    fp->is_item_show = FALSE;
    ftRemixPianoLeftoverHideTongueParts(fp);
    fp->is_modelpart_modify = TRUE;
}

/**
 * TRUE for the Marina items that skip the random despawn check when they land (Item.asm prevent_item_despawn_).
 * @param item_gobj Item being checked in itMapCheckDestroyDropped.
 */
sb32 ftRemixLeftoverItemSkipsDespawn(GObj *item_gobj)
{
    s32 kind = itGetStruct(item_gobj)->kind;

    return (kind == dITRemixTurnipItemDesc.kind) || (kind == dITRemixGemItemDesc.kind) ||
           (kind == dITRemixShurikenItemDesc.kind) || (kind == dITRemixBoomerangItemDesc.kind) ||
           (kind == dITRemixClanBombItemDesc.kind);
}

#endif
