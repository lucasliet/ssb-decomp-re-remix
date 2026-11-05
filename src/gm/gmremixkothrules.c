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

static const int dGMRemixKothRulesFfaTags[4] = { GMREMIX_KOTHRULES_TAG_RED, GMREMIX_KOTHRULES_TAG_BLUE, GMREMIX_KOTHRULES_TAG_YELLOW, GMREMIX_KOTHRULES_TAG_GREEN };
static const int dGMRemixKothRulesTeamTags[4] = { GMREMIX_KOTHRULES_TAG_RED, GMREMIX_KOTHRULES_TAG_BLUE, GMREMIX_KOTHRULES_TAG_GREEN, GMREMIX_KOTHRULES_TAG_YELLOW };

/* Color.asm tag.RED, BLUE, GREEN, YELLOW, GRAY. */
static const unsigned int dGMRemixKothRulesTagColors[5] = { 0xED3636FF, 0x4E4EE9FF, 0x4EB94EFF, 0xFFDF1AFF, 0xACACACFF };

#define GMREMIX_KOTHRULES_CONTESTED_GRAY 0xAA

int gmRemixKothRulesTag(int key, int is_team_battle, int is_cpu)
{
    if (is_team_battle != 0)
    {
        return dGMRemixKothRulesTeamTags[key];
    }
    return (is_cpu != 0) ? GMREMIX_KOTHRULES_TAG_GRAY : dGMRemixKothRulesFfaTags[key];
}

unsigned int gmRemixKothRulesTagColor(int tag)
{
    return dGMRemixKothRulesTagColors[tag];
}

int gmRemixKothRulesContestedGray(int buffer)
{
    return GMREMIX_KOTHRULES_CONTESTED_GRAY - (2 * buffer);
}

#endif
