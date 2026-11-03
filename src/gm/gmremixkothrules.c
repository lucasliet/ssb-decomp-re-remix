/**
 * gmremixkothrules.c - pure king rules of King of the Hill, see gmremixkothrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixkothrules.h"

int gmRemixKothRulesCandidate(const GMRemixKothRulesFighter *fighters, int count, int *eligible)
{
    int candidate = GMREMIX_KOTHRULES_NO_KING;
    int i;

    for (i = 0; i < count; i++)
    {
        eligible[i] = 0;

        if (fighters[i].is_on_hill == 0)
        {
            continue;
        }
        if (fighters[i].holds_enemy != 0)
        {
            candidate = GMREMIX_KOTHRULES_CONTESTED;
            continue;
        }
        eligible[i] = 1;

        if (candidate == GMREMIX_KOTHRULES_NO_KING)
        {
            candidate = fighters[i].key;
        }
        else if (candidate != fighters[i].key)
        {
            candidate = GMREMIX_KOTHRULES_CONTESTED;
        }
    }
    return candidate;
}

int gmRemixKothRulesSettle(int candidate, int *buffer, int buffer_frames)
{
    if (candidate != GMREMIX_KOTHRULES_CONTESTED)
    {
        if (*buffer != 0)
        {
            (*buffer)--;
            return GMREMIX_KOTHRULES_CONTESTED;
        }
        return candidate;
    }
    *buffer = buffer_frames;

    return candidate;
}

void gmRemixKothRulesMeterY(int dragon_king_hud, int is_dragon_king_stage, float *frame_y, float *bar_y)
{
    int at_top = (dragon_king_hud == GMREMIX_KOTHRULES_DK_HUD_ALWAYS) ||
                 ((dragon_king_hud != GMREMIX_KOTHRULES_DK_HUD_OFF) && (is_dragon_king_stage != 0));

    *frame_y = at_top ? 8.0F : 180.0F;
    *bar_y = at_top ? 16.0F : 188.0F;
}

float gmRemixKothRulesMeterX(int port)
{
    return 40.0F + (70.0F * (float)port);
}

#endif
