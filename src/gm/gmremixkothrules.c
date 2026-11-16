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

int gmRemixKothRulesWinnerTimeKept(int key, int king)
{
    return (key == king) ? 1 : 0;
}

/* Color.asm damage_icon.RED, BLUE, GREEN, YELLOW, GRAY. */
static const unsigned int dGMRemixKothRulesMeterColors[5] = { 0xFFA0A0FF, 0xB4B4FFFF, 0xA0FFA0FF, 0xE6E678FF, 0xDCDCDCFF };

/* KingOfTheHill.asm colors_bar / team_colors_bar, in the tag order. */
static const unsigned int dGMRemixKothRulesBarColors[5] = { 0xC96A6AFF, 0x7E7EC9FF, 0x4EAD4EFF, 0xB2B244FF, 0x888888FF };

unsigned int gmRemixKothRulesMeterColor(int tag)
{
    return dGMRemixKothRulesMeterColors[tag];
}

unsigned int gmRemixKothRulesBarColor(int tag)
{
    return dGMRemixKothRulesBarColors[tag];
}

void gmRemixKothRulesFlashStep(int *alpha, int *delta)
{
    *alpha += *delta;

    if (*alpha == 0)
    {
        *delta = GMREMIX_KOTHRULES_FLASH_STEP;
    }
    if (*alpha == GMREMIX_KOTHRULES_FLASH_MAX)
    {
        *delta = -GMREMIX_KOTHRULES_FLASH_STEP;
    }
}

int gmRemixKothRulesPickHill(int current, GMRemixKothRulesDrawLine draw, void *context)
{
    int repeats_left = GMREMIX_KOTHRULES_HILL_REPEATS;
    int invalid_left = GMREMIX_KOTHRULES_HILL_INVALID_MAX;

    for (;;)
    {
        int line_id = draw(context);

        if (line_id < 0)
        {
            if (invalid_left == 0)
            {
                return current;
            }
            invalid_left--;
            continue;
        }
        if (line_id != current)
        {
            return line_id;
        }
        if (repeats_left == 0)
        {
            return current;
        }
        repeats_left--;
    }
}

int gmRemixKothRulesHiddenTimeout(int is_hidden, int *hidden_frames, int hidden_max)
{
    if (is_hidden == 0)
    {
        *hidden_frames = 0;
        return 0;
    }
    if (*hidden_frames == hidden_max)
    {
        *hidden_frames = 0;
        return 1;
    }
    (*hidden_frames)++;

    return 0;
}

#endif
