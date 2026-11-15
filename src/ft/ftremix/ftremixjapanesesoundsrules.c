/**
 * ftremixjapanesesoundsrules.c - pure rules of the Japanese Sounds toggle, see ftremixjapanesesoundsrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixjapanesesoundsrules.h"

#define FTREMIXJSFX_HIT_ROW_PUNCH 0
#define FTREMIXJSFX_HIT_ROW_KICK 1
#define FTREMIXJSFX_HIT_ROW_COUNT 2
#define FTREMIXJSFX_HIT_LEVEL_COUNT 3

/* Character.asm sound_type_J rows Punch and Kick: FGM.hit.J_PUNCH_S/M/L and J_KICK_S/M/L (FGM.asm:1880-1885) */
static const int sFTRemixJSfxHitRows[FTREMIXJSFX_HIT_ROW_COUNT][FTREMIXJSFX_HIT_LEVEL_COUNT] =
{
    { 0x93, 0x92, 0x91 },
    { 0x90, 0x8F, 0x8E }
};

int ftRemixJapaneseSoundsUseJ(int toggle_value, int is_j_fighter)
{
    switch (toggle_value)
    {
        case FTREMIXJSFX_TOGGLE_ALWAYS:
            return 1;

        case FTREMIXJSFX_TOGGLE_NEVER:
            return 0;

        default:
            return is_j_fighter ? 1 : 0;
    }
}

int ftRemixJapaneseHitCollisionFgmRule(int use_j, int fgm_kind, int fgm_level, int fgm_id)
{
    if (!use_j)
    {
        return fgm_id;
    }
    if ((fgm_kind != FTREMIXJSFX_HIT_ROW_PUNCH) && (fgm_kind != FTREMIXJSFX_HIT_ROW_KICK))
    {
        return fgm_id;
    }
    if ((fgm_level < 0) || (fgm_level >= FTREMIXJSFX_HIT_LEVEL_COUNT))
    {
        return fgm_id;
    }
    return sFTRemixJSfxHitRows[fgm_kind][fgm_level];
}

#endif
