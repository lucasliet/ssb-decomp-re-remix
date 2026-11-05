/**
 * grremixtrainingbackground.c - parent stage of the Training wallpaper. See grremixtrainingbackground.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixstage.h"
#include "grremixtrainingbackground.h"

s32 grRemixTrainingBackgroundGkind(s32 gkind)
{
    if ((gkind < 0) || (gkind > GRREMIX_KIND_MAX))
    {
        return gkind;
    }
    return grRemixGetBackgroundStage(gkind);
}

#endif
