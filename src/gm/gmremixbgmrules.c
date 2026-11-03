/**
 * gmremixbgmrules.c - pure BGM rules of the Remix, see gmremixbgmrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixbgmrules.h"

#define TIME_TWISTER_BACKGROUNDS 4

static const int sTimeTwisterTracks[TIME_TWISTER_BACKGROUNDS] = { 318, 359, 360, 317 };

int gmRemixBgmRulesIsProtected(int bgm_id)
{
    return (bgm_id == GMREMIX_BGM_HAMMER) || (bgm_id == GMREMIX_BGM_INVINCIBLE);
}

/** First slot from first_slot on that holds a track, else the default track. */
static int firstTrackFrom(int first_slot, const int *alternates, int default_track)
{
    int slot;

    for (slot = first_slot; slot < GMREMIX_BGM_ALT_SLOTS; slot++)
    {
        if (alternates[slot] != GMREMIX_BGM_NO_TRACK)
        {
            return alternates[slot];
        }
        if (slot == 1)
        {
            break;
        }
    }
    return default_track;
}

int gmRemixBgmRulesNextAlternate(int current_track, int default_track, const int *alternates)
{
    if (current_track == default_track)
    {
        /* occasional, else rare; rare 2 is only reached from rare (the asm "safety check" stops at slot 1) */
        return firstTrackFrom(0, alternates, default_track);
    }
    if (current_track == alternates[2])
    {
        return default_track;
    }
    if (current_track == alternates[1])
    {
        return (alternates[2] == GMREMIX_BGM_NO_TRACK) ? default_track : alternates[2];
    }
    if (current_track == alternates[0])
    {
        return (alternates[1] == GMREMIX_BGM_NO_TRACK) ? default_track : alternates[1];
    }
    return default_track;
}

int gmRemixBgmRulesTimeTwisterChanges(int stage_track)
{
    return stage_track == GMREMIX_BGM_TIME_TWISTER;
}

int gmRemixBgmRulesTimeTwisterTrack(int bg_index)
{
    if ((bg_index < 0) || (bg_index >= TIME_TWISTER_BACKGROUNDS))
    {
        return GMREMIX_BGM_NO_TRACK;
    }
    return sTimeTwisterTracks[bg_index];
}

#endif
