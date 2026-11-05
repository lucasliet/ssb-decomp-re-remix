/**
 * sc1premixending.c - own ending of the Remix 1P and All-Star runs, see sc1premixending.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/scene.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "sc/sc1pmode/sc1premixending.h"
#include "sc/sc1pmode/sc1premixendingrules.h"
#include "sc/sc1pmode/sc1premixmode.h"

#define SC1PREMIX_ENDING_SONG_ENTRY_SIZE 2

static u32 sc1PRemixEndingReadSong(s32 fkind)
{
    return remix_ram_image_read_half(REMIX_SYM_Character__remix_1p_end_bgm__table + (fkind * SC1PREMIX_ENDING_SONG_ENTRY_SIZE));
}

u32 sc1PRemixEndingGetCreditsSong(u32 standard_song)
{
    if (sc1PRemixEndingUsesFighterSong(sc1PRemixModeGet()) == 0)
    {
        return standard_song;
    }
    return sc1PRemixEndingPickSong(sc1PRemixEndingReadSong(gSCManagerSceneData.fkind), standard_song);
}

#endif
