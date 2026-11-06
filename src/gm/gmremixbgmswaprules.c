/**
 * gmremixbgmswaprules.c - pure rules of the Final Destination intro and Hurry Up song swaps, see gmremixbgmswaprules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixbgmrules.h"
#include "gm/gmremixbgmstagerules.h"
#include "gm/gmremixbgmswaprules.h"

int gmRemixBgmSwapRulesIsFinalDestination(int gkind)
{
    return (gkind == GMREMIX_BGM_SWAP_STAGE_FD) || (gkind == GMREMIX_BGM_SWAP_STAGE_FD_DL) ||
           (gkind == GMREMIX_BGM_SWAP_STAGE_FD_TENT);
}

int gmRemixBgmSwapRulesIs1PFinalStage(int mode, int stage)
{
    if ((mode != GMREMIX_BGM_SWAP_1P_MODE_STANDARD) && (mode != GMREMIX_BGM_SWAP_1P_MODE_REMIX))
    {
        return 0;
    }
    return stage == GMREMIX_BGM_SWAP_1P_FINAL_STAGE;
}

int gmRemixBgmSwapRulesFdIntroEnds(unsigned int update_count, unsigned int timer_start, int stage_track)
{
    if ((update_count - timer_start) != GMREMIX_BGM_SWAP_FD_INTRO_UPDATES)
    {
        return 0;
    }
    return stage_track == GMREMIX_BGM_STAGE_FD_INTRO;
}

/** Hurry Up song of a playing song, GMREMIX_BGM_NO_TRACK when the song has none. */
static int hurryTrackOf(int track)
{
    switch (track)
    {
    case GMREMIX_BGM_SWAP_MUSHROOM_KINGDOM:
        return GMREMIX_BGM_SWAP_MUSHROOM_KINGDOM_FAST;

    case GMREMIX_BGM_SWAP_SMB2_OVERWORLD:
        return GMREMIX_BGM_SWAP_SMB2_BOSS;

    case GMREMIX_BGM_SWAP_UNDERGROUND:
        return GMREMIX_BGM_SWAP_UNDERGROUND_HURRY;

    default:
        return GMREMIX_BGM_NO_TRACK;
    }
}

int gmRemixBgmSwapRulesHurryTrack(int current_track, int stage_track)
{
    int hurry_track = hurryTrackOf(current_track);

    if (hurry_track == GMREMIX_BGM_NO_TRACK)
    {
        return GMREMIX_BGM_NO_TRACK;
    }
    if ((stage_track == GMREMIX_BGM_SWAP_MUSHROOM_KINGDOM_FAST) || (stage_track == GMREMIX_BGM_SWAP_SMB2_BOSS) ||
        (stage_track == GMREMIX_BGM_SWAP_UNDERGROUND_HURRY))
    {
        return GMREMIX_BGM_NO_TRACK;
    }
    return hurry_track;
}

#endif
