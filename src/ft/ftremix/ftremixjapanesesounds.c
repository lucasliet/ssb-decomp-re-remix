/**
 * ftremixjapanesesounds.c - Smash Remix Japanese.asm japanese_sfx_*: the Japanese Sounds toggle picks the J sound
 * effect ids of hits, beam swords and projectile hits.
 *
 * The toggle is 0 for the default (J fighters only), 1 for always J and 2 for never J. The J ids are the U ids plus
 * the difference listed in the asm (FGM.hit.J_* and FGM.item.J_BEAM_SWORD_LIGHT).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <sys/objdef.h>
#include <it/itdef.h>
#include "gm/gmremixtoggle.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_JSFX_ALWAYS 1
#define FTREMIX_JSFX_NEVER 2

/* FGM.item.BEAM_SWORD_HEAVY: the beam sword ids are the range [HEAVY, LIGHT] */
#define FTREMIX_JSFX_BEAM_SWORD_HEAVY 0x43
#define FTREMIX_JSFX_BEAM_SWORD_LIGHT 0x45

/* J_BEAM_SWORD_LIGHT (0x323) - BEAM_SWORD_LIGHT (0x45), the same for the three beam sword ids */
#define FTREMIX_JSFX_BEAM_SWORD_DIFF 0x2DE

typedef struct FTRemixJSfxDiff
{
    u16 fgm_id;
    u16 diff;
} FTRemixJSfxDiff;

/* J_ minus U hit ids (FGM.hit.J_PUNCH_S - FGM.hit.PUNCH_S and so on) */
static const FTRemixJSfxDiff sFTRemixJSfxHitDiffs[] =
{
    { 0x28, 0x6B }, /* PUNCH_S */
    { 0x26, 0x6C }, /* PUNCH_M */
    { 0x25, 0x6C }, /* PUNCH_L */
    { 0x22, 0x6E }, /* KICK_S */
    { 0x20, 0x6F }, /* KICK_M */
    { 0x1F, 0x6F }, /* KICK_L */
};

/**
 * Returns TRUE when the fighter uses the J sound of the Japanese Sounds toggle. fp may be NULL (no fighter): the
 * default toggle value then uses the U sound. The default value follows the sound type of the fighter, as in
 * Character.asm sound_type (REMIX_SOUND_J in the roster).
 */
static sb32 ftRemixJSfxIsJ(FTStruct *fp)
{
    switch (gmRemixToggle(japanese_sounds))
    {
        case FTREMIX_JSFX_ALWAYS:
            return TRUE;

        case FTREMIX_JSFX_NEVER:
            return FALSE;

        default:
            break;
    }
    if ((fp == NULL) || (fp->fkind < 0) || (fp->fkind >= ARRAY_COUNT(kRemixRoster)))
    {
        return FALSE;
    }
    return kRemixRoster[fp->fkind].sound_type == REMIX_SOUND_J;
}

/** Returns the J minus U difference of a hit sound id, 0 when the id is not one of the six hit sounds. */
static s32 ftRemixJSfxHitDiff(u16 fgm_id)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sFTRemixJSfxHitDiffs); i++)
    {
        if (sFTRemixJSfxHitDiffs[i].fgm_id == fgm_id)
        {
            return sFTRemixJSfxHitDiffs[i].diff;
        }
    }
    return 0;
}

/** Returns the J minus U difference of a beam sword id, 0 when the id is not a beam sword. */
static s32 ftRemixJSfxBeamSwordDiff(u16 fgm_id)
{
    if ((fgm_id >= FTREMIX_JSFX_BEAM_SWORD_HEAVY) && (fgm_id <= FTREMIX_JSFX_BEAM_SWORD_LIGHT))
    {
        return FTREMIX_JSFX_BEAM_SWORD_DIFF;
    }
    return 0;
}

/**
 * Returns the fgm id of a PlayFGM motion event of fp (japanese_sfx_, 0x800DF86C in ftMainParseMotionEvent).
 * The value is the 16 bit fgm id of the event, as the asm masks it before the call.
 */
u16 ftRemixJapaneseMotionSoundFgm(FTStruct *fp, u32 value)
{
    u16 fgm_id = (u16) (value & 0xFFFF);
    s32 diff;

    if (fgm_id < FTREMIX_JSFX_BEAM_SWORD_HEAVY)
    {
        diff = ftRemixJSfxHitDiff(fgm_id);
    }
    else diff = ftRemixJSfxBeamSwordDiff(fgm_id);

    if ((diff != 0) && ftRemixJSfxIsJ(fp))
    {
        return fgm_id + diff;
    }
    return fgm_id;
}

/**
 * Returns the fgm id of a projectile hit of a fighter owner (japanese_sfx_projectile_, 0x800E359C in
 * ftMainUpdateDamageStatWeapon). owner_gobj is the owner of the weapon, NULL when it has none; the
 * projectile hit sounds only cover the six hit ids, as in the asm.
 */
u16 ftRemixJapaneseProjectileSoundFgm(GObj *owner_gobj, u16 fgm_id)
{
    FTStruct *owner_fp = NULL;
    s32 diff = ftRemixJSfxHitDiff(fgm_id);

    if ((owner_gobj != NULL) && (owner_gobj->link_id == nGCCommonLinkIDFighter))
    {
        owner_fp = ftGetStruct(owner_gobj);
    }
    if ((diff != 0) && ftRemixJSfxIsJ(owner_fp))
    {
        return fgm_id + diff;
    }
    return fgm_id;
}

/**
 * Returns the fgm id of the hit sound of an item at spawn (japanese_sfx_link_bomb_collide_, 0x8016E548 in
 * itManagerMakeItem, where the asm stores ITStruct.attack_coll.fgm_id; Link's bomb is the case the Remix label names).
 *
 * DEVIATION: the asm reads the owner from stack slot 0x20(sp) of itManagerMakeItem, which the function never writes
 * (the slot holds whatever the caller chain left there, and is only used if it equals a player struct). The port takes
 * the fighter that spawns the item (ITEM_FLAG_PARENT_FIGHTER) as the owner and no owner otherwise. Toggle Always and
 * Never behave as in the asm.
 *
 * @param parent_gobj Parent GObj given to itManagerMakeItem.
 * @param flags Spawn flags given to itManagerMakeItem.
 * @param fgm_id Hit sound id of the item attributes.
 */
u16 ftRemixJapaneseItemSoundFgm(GObj *parent_gobj, u32 flags, u16 fgm_id)
{
    GObj *owner_gobj = NULL;

    if ((flags & ITEM_MASK_PARENT) == ITEM_FLAG_PARENT_FIGHTER)
    {
        owner_gobj = parent_gobj;
    }
    return ftRemixJapaneseProjectileSoundFgm(owner_gobj, fgm_id);
}

#endif
