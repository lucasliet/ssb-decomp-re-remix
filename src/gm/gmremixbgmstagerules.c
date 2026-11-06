/**
 * gmremixbgmstagerules.c - pure rules of the stage song pick of the Remix, see gmremixbgmstagerules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixbgmrules.h"
#include "gm/gmremixbgmstagerules.h"

int gmRemixBgmStageRulesCButtonSlot(unsigned int held_buttons)
{
    if (held_buttons & GMREMIX_BGM_STAGE_C_UP)
    {
        return GMREMIX_BGM_STAGE_SLOT_DEFAULT;
    }
    if (held_buttons & GMREMIX_BGM_STAGE_C_LEFT)
    {
        return GMREMIX_BGM_STAGE_SLOT_OCCASIONAL;
    }
    if (held_buttons & GMREMIX_BGM_STAGE_C_RIGHT)
    {
        return GMREMIX_BGM_STAGE_SLOT_RARE;
    }
    if (held_buttons & GMREMIX_BGM_STAGE_C_DOWN)
    {
        return GMREMIX_BGM_STAGE_SLOT_RARE_2;
    }
    return GMREMIX_BGM_STAGE_SLOT_NONE;
}

int gmRemixBgmStageRulesRollSlot(int roll)
{
    if (roll < GMREMIX_BGM_STAGE_CHANCE_MAIN)
    {
        return GMREMIX_BGM_STAGE_SLOT_DEFAULT;
    }
    if (roll < GMREMIX_BGM_STAGE_CHANCE_MAIN + GMREMIX_BGM_STAGE_CHANCE_OCCASIONAL)
    {
        return GMREMIX_BGM_STAGE_SLOT_OCCASIONAL;
    }
    if (roll < GMREMIX_BGM_STAGE_CHANCE_MAIN + GMREMIX_BGM_STAGE_CHANCE_OCCASIONAL + GMREMIX_BGM_STAGE_CHANCE_RARE)
    {
        return GMREMIX_BGM_STAGE_SLOT_RARE;
    }
    return GMREMIX_BGM_STAGE_SLOT_RARE_2;
}

int gmRemixBgmStageRulesNeedsRoll(int forced_slot, int preserved_track)
{
    return (forced_slot == GMREMIX_BGM_STAGE_SLOT_NONE) && (preserved_track == GMREMIX_BGM_NO_TRACK);
}

/** Track of a slot, or the main song when the slot is empty. */
static int trackOfSlotOrDefault(int slot, int default_track, const int *alternates)
{
    return (alternates[slot] == GMREMIX_BGM_NO_TRACK) ? default_track : alternates[slot];
}

GMRemixBgmStagePick gmRemixBgmStageRulesPick(int default_track, const int *alternates, int forced_slot, int preserved_track,
                                             int roll)
{
    GMRemixBgmStagePick pick;
    int slot;

    pick.track = default_track;
    pick.random_disabled = 0;

    if (forced_slot == GMREMIX_BGM_STAGE_SLOT_DEFAULT)
    {
        pick.random_disabled = 1;
        return pick;
    }
    if (forced_slot != GMREMIX_BGM_STAGE_SLOT_NONE)
    {
        pick.track = trackOfSlotOrDefault(forced_slot, default_track, alternates);
        pick.random_disabled = (alternates[forced_slot] != GMREMIX_BGM_NO_TRACK);
        return pick;
    }
    if (preserved_track != GMREMIX_BGM_NO_TRACK)
    {
        pick.track = preserved_track;
        return pick;
    }
    slot = gmRemixBgmStageRulesRollSlot(roll);

    if (slot != GMREMIX_BGM_STAGE_SLOT_DEFAULT)
    {
        pick.track = trackOfSlotOrDefault(slot, default_track, alternates);
    }
    return pick;
}

int gmRemixBgmStageRulesDemoTrack(int file_track)
{
    return (file_track == GMREMIX_BGM_STAGE_FD_INTRO) ? GMREMIX_BGM_STAGE_FD_MAIN : file_track;
}

#endif
