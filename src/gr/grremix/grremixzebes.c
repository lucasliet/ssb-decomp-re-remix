/**
 * grremixzebes.c - Remix lava level tables of the Zebes-based stages. See grremixzebes.h.
 */

#ifdef PORT

#include "grremixzebes.h"
#include "grremixids.h"

/* Hazards.asm norfair_lava_levels */
static const float sGRRemixNorfairLevels[GRREMIX_ZEBES_ACID_LEVELS_NUM] =
{
    200.0F, -600.0F, -100.0F, -2400.0F, -2400.0F, -600.0F, -100.0F, 200.0F,
    -2400.0F, -600.0F, -100.0F, 200.0F, -600.0F, -100.0F, 200.0F, -2400.0F
};

/* Hazards.asm norfair_remix_lava_levels */
static const float sGRRemixNorfairRemixLevels[GRREMIX_ZEBES_ACID_LEVELS_NUM] =
{
    -100.0F, -2400.0F, 200.0F, -2400.0F, -600.0F, 1200.0F, -2400.0F, 200.0F,
    -600.0F, -2400.0F, 1200.0F, -100.0F, -2400.0F, 200.0F, -600.0F, -2400.0F
};

/* Hazards.asm zebes_dl_lava_levels */
static const float sGRRemixZebesDLLevels[GRREMIX_ZEBES_ACID_LEVELS_NUM] =
{
    200.0F, -600.0F, -100.0F, -2400.0F, -2400.0F, -600.0F, -100.0F, 1100.0F,
    -2400.0F, -600.0F, -100.0F, 900.0F, -600.0F, -100.0F, 300.0F, -2400.0F
};

/* Hazards.asm zebes_o_lava_levels */
static const float sGRRemixZebesOLevels[GRREMIX_ZEBES_ACID_LEVELS_NUM] =
{
    -300.0F, -600.0F, -2400.0F, -2400.0F, -2400.0F, -600.0F, -100.0F, -300.0F,
    -2400.0F, -600.0F, -600.0F, -300.0F, -600.0F, -2400.0F, -300.0F, -2400.0F
};

const float *grRemixGetAltAcidLevels(int32_t gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_NORFAIR:
        return sGRRemixNorfairLevels;

    case nGRRemixStage_NORFAIR_REMIX:
        return sGRRemixNorfairRemixLevels;

    case nGRRemixStage_ZEBES_DL:
        return sGRRemixZebesDLLevels;

    case nGRRemixStage_ZEBES_O:
        return sGRRemixZebesOLevels;

    default:
        return NULL;
    }
}

const float *grRemixZebesSelectAcidLevels(int32_t gkind, const float *vanilla_levels)
{
    const float *levels = grRemixGetAltAcidLevels(gkind);

    return (levels != NULL) ? levels : vanilla_levels;
}

#endif
