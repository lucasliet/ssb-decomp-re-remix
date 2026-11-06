/**
 * sc1premixmusicrules.c - music rules of the Remix 1P modes, see sc1premixmusicrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixids.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixmusicrules.h"

typedef struct SC1PRemixMusicDuoSong
{
    int gkind;
    int bgm;

} SC1PRemixMusicDuoSong;

static const SC1PRemixMusicDuoSong sSC1PRemixMusicDuoSongs[] =
{
    { nGRRemixStage_EDO, SC1PREMIX_BGM_ALOOF_SOLDIER },
    { nGRRemixStage_ONETT, SC1PREMIX_BGM_DANGEROUS_FOE },
    { nGRRemixStage_NPC, SC1PREMIX_BGM_DANGEROUS_FOE },
    { nGRRemixStage_POKEMON_STADIUM_2, SC1PREMIX_BGM_PIKA_CUP },
    { nGRRemixStage_GYM_LEADER_CASTLE, SC1PREMIX_BGM_PIKA_CUP },
    { nGRRemixStage_SAFFRON_DL, SC1PREMIX_BGM_PIKA_CUP },
    { nGRRemixStage_SPIRALM, SC1PREMIX_BGM_WIZPIG },
    { nGRRemixStage_MADMM, SC1PREMIX_BGM_WIZPIG },
    { nGRRemixStage_FROSTY, SC1PREMIX_BGM_WIZPIG },
    { nGRRemixStage_WINDY, SC1PREMIX_BGM_WIZPIG },
    { nGRRemixStage_MT_DEDEDE, SC1PREMIX_BGM_BATTLE_AMONG_FRIENDS },
    { nGRRemixStage_DREAM_LAND_SR, SC1PREMIX_BGM_BATTLE_AMONG_FRIENDS },
};

static int sc1PRemixMusicIsSonicStage(int gkind)
{
    return (gkind == nGRRemixStage_CASINO) || (gkind == nGRRemixStage_GHZ) || (gkind == nGRRemixStage_MMADNESS);
}

static int sc1PRemixMusicDuoSong(int gkind)
{
    unsigned int i;

    for (i = 0; i < (sizeof(sSC1PRemixMusicDuoSongs) / sizeof(sSC1PRemixMusicDuoSongs[0])); i++)
    {
        if (sSC1PRemixMusicDuoSongs[i].gkind == gkind)
        {
            return sSC1PRemixMusicDuoSongs[i].bgm;
        }
    }
    return SC1PREMIX_MUSIC_KEEP_STAGE;
}

int sc1PRemixModePlaysMusicAtOnce(int mode, int progress)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
    case SC1PREMIX_MODE_HRC:
        return 1;

    case SC1PREMIX_MODE_REMIX_1P:
        return (progress == SC1PREMIX_MUSIC_STAGE_MINI_BOSS) ? 1 : 0;

    default:
        return 0;
    }
}

int sc1PRemixMusicRollCount(int mode, int progress, int gkind)
{
    if (mode == SC1PREMIX_MODE_MULTIMAN)
    {
        return SC1PREMIX_MUSIC_MULTIMAN_ROLLS;
    }
    if ((mode == SC1PREMIX_MODE_REMIX_1P) && (progress == SC1PREMIX_MUSIC_STAGE_MINI_BOSS) && (sc1PRemixMusicIsSonicStage(gkind) != 0))
    {
        return SC1PREMIX_MUSIC_SONIC_ROLLS;
    }
    return 0;
}

static int sc1PRemixMusicPickSonic(int roll)
{
    switch (roll)
    {
    case 0:
        return SC1PREMIX_BGM_LIVE_AND_LEARN;

    case 2:
        return SC1PREMIX_BGM_EVERYTHING;

    default:
        return SC1PREMIX_BGM_OPEN_YOUR_HEART;
    }
}

static int sc1PRemixMusicPickRemix1P(int progress, int gkind, int roll)
{
    switch (progress)
    {
    case SC1PREMIX_MUSIC_STAGE_FINAL:
        return SC1PREMIX_BGM_ULTIMATE_BOWSER;

    case SC1PREMIX_MUSIC_STAGE_MINI_BOSS:
        return (sc1PRemixMusicIsSonicStage(gkind) != 0) ? sc1PRemixMusicPickSonic(roll) : SC1PREMIX_MUSIC_KEEP_STAGE;

    case SC1PREMIX_MUSIC_STAGE_DUO:
        return sc1PRemixMusicDuoSong(gkind);

    default:
        return SC1PREMIX_MUSIC_KEEP_STAGE;
    }
}

int sc1PRemixMusicPick(int mode, int progress, int gkind, int roll)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_MULTIMAN:
        return (roll == 0) ? SC1PREMIX_BGM_MULTIMAN_2 : SC1PREMIX_BGM_MULTIMAN;

    case SC1PREMIX_MODE_CRUEL:
        return SC1PREMIX_BGM_CRUEL;

    case SC1PREMIX_MODE_HRC:
        return SC1PREMIX_BGM_TARGETS;

    case SC1PREMIX_MODE_REMIX_1P:
        return sc1PRemixMusicPickRemix1P(progress, gkind, roll);

    default:
        return SC1PREMIX_MUSIC_KEEP_STAGE;
    }
}

#endif
