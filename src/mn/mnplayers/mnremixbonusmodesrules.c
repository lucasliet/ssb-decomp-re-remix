/**
 * mnremixbonusmodesrules.c - state-free rules of the Remix 1P modes on the Bonus 2 character select, see mnremixbonusmodesrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixbonusmodesrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

int mnRemixBonusModesHeaderOffset(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_BONUS3:
        return MNREMIX_BONUSMODES_HEADER_BONUS3;

    case SC1PREMIX_MODE_MULTIMAN:
        return MNREMIX_BONUSMODES_HEADER_MULTIMAN;

    case SC1PREMIX_MODE_CRUEL:
        return MNREMIX_BONUSMODES_HEADER_CRUEL;

    case SC1PREMIX_MODE_HRC:
        return MNREMIX_BONUSMODES_HEADER_HRC;

    default:
        return 0;
    }
}

int mnRemixBonusModesCounterOffset(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
        return MNREMIX_BONUSMODES_COUNTER_KOS;

    case SC1PREMIX_MODE_HRC:
        return MNREMIX_BONUSMODES_COUNTER_FEET;

    default:
        return 0;
    }
}

int mnRemixBonusModesRecordDigits(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
        return MNREMIX_BONUSMODES_KO_DIGITS;

    case SC1PREMIX_MODE_HRC:
        return MNREMIX_BONUSMODES_FEET_DIGITS;

    default:
        return MNREMIX_BONUSMODES_VANILLA_DIGITS;
    }
}

int mnRemixBonusModesRulesShowsTime(int mode, int is_random, int is_complete)
{
    if (mode == SC1PREMIX_MODE_BONUS3)
    {
        return 1;
    }
    if (mode != SC1PREMIX_MODE_STANDARD)
    {
        return 0;
    }
    if (is_random != 0)
    {
        return 0;
    }
    return (is_complete != 0) ? 1 : 0;
}

int mnRemixBonusModesRulesShowsTotal(int mode, int is_all_complete)
{
    if (mnRemixBonusModesTotalDigits(mode) != 0)
    {
        return 1;
    }
    return (is_all_complete != 0) ? 1 : 0;
}

int mnRemixBonusModesTotalDigits(int mode)
{
    switch (mode)
    {
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
        return MNREMIX_BONUSMODES_KO_TOTAL_DIGITS;

    case SC1PREMIX_MODE_HRC:
        return MNREMIX_BONUSMODES_FEET_TOTAL_DIGITS;

    default:
        return 0;
    }
}

unsigned int mnRemixBonusModesSumRecords(const unsigned int *records, int count)
{
    unsigned int total = 0;
    int index;

    for (index = 0; index < count; index++)
    {
        total += (records[index] == MNREMIX_BONUSMODES_RECORD_FILLER) ? 0 : records[index];
    }
    return total;
}

int mnRemixBonusModesToggleStep(int mode, int direction)
{
    int next = mode + direction;

    if ((next < 0) || (next >= MNREMIX_BONUSMODES_MODE_COUNT))
    {
        return mode;
    }
    return next;
}

int mnRemixBonusModesToggleArrowExists(int mode, int is_right)
{
    if (is_right != 0)
    {
        return (mode < (MNREMIX_BONUSMODES_MODE_COUNT - 1)) ? 1 : 0;
    }
    return (mode > 0) ? 1 : 0;
}

int mnRemixBonusModesToggleBlinkNext(int timer)
{
    return ((timer + 1) >= MNREMIX_BONUSMODES_BLINK_PERIOD) ? 0 : (timer + 1);
}

int mnRemixBonusModesToggleBlinkIsOn(int timer)
{
    return (timer < MNREMIX_BONUSMODES_BLINK_ON_FRAMES) ? 1 : 0;
}

int mnRemixBonusModesToggleArrowHit(int pos_x, int pos_y, int arrow_x, int arrow_y, int width, int height, int is_right)
{
    int left_pad = (is_right != 0) ? MNREMIX_BONUSMODES_ARROW_INNER_PAD : MNREMIX_BONUSMODES_ARROW_OUTER_PAD;
    int right_pad = (is_right != 0) ? MNREMIX_BONUSMODES_ARROW_OUTER_PAD : MNREMIX_BONUSMODES_ARROW_INNER_PAD;

    if ((pos_x < (arrow_x - left_pad)) || (pos_x > (arrow_x + width + right_pad)))
    {
        return 0;
    }
    return ((pos_y >= arrow_y) && (pos_y <= (arrow_y + height))) ? 1 : 0;
}

int mnRemixBonusModesHasDecimal(int mode)
{
    return (mode == SC1PREMIX_MODE_HRC) ? 1 : 0;
}

#endif
