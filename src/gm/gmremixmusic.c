/**
 * gmremixmusic.c - master BGM and FGM volume of the Remix settings (BGM.asm master_bgm_volume, FGM.asm master_fgm_volume).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixmusic.h"

u32 gmRemixMasterVolume(u32 volume, s32 level)
{
    f32 scale;

    if (level <= 0)
    {
        return 0;
    }
    scale = (f32)level / 10.0F;
    return (u32)((f32)(s32)volume * scale);
}

#endif
