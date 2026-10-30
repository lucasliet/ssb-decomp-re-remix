/*
 * ftremixaisize.c - size hook of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm size_multiplier_check_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixaisize.h"

/**
 * size_multiplier_check_ (AI.asm): the CPU detect ranges of a Remix body size are scaled by the size multiplier of the
 * player. Giant DK keeps the vanilla path, so it returns 1.0F here and its own 1.4F factor is applied by the caller.
 */
f32 ftRemixAiSizeDetectScale(FTStruct *fp)
{
    if (fp->fkind == nFTKindGDonkey)
    {
        return 1.0F;
    }
    return ftRemixSizeGetMultiplier(fp->player);
}

#endif
