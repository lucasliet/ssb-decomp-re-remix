/**
 * ftremixyoshisharedrules.c - pure rules of yoshishared.asm and JYoshi.asm, see ftremixyoshisharedrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixyoshisharedrules.h"
#include "remix/remix_roster_data.h"

/* Bowser.asm:378, Dedede.asm:644, NBowser, NDedede and Piano.asm:410: the DK row, copied by the Remix authors */
static const float sFTRemixYoshiEggDonkeyRow[FTREMIX_YOSHI_EGG_ROW_FLOATS] =
{
    3.5F, 0.0F, 230.0F, 0.0F, 245.0F, 245.0F, 245.0F
};

/* gbowser.asm:364: 0x40B66666 0 0x43C80000 0 0x43AF0000 x3 */
static const float sFTRemixYoshiEggGigaBowserRow[FTREMIX_YOSHI_EGG_ROW_FLOATS] =
{
    5.7F, 0.0F, 400.0F, 0.0F, 350.0F, 350.0F, 350.0F
};

int ftRemixYoshiSharedRulesHasEggShield(int fkind)
{
    return (fkind == REMIX_ID_YOSHI) || (fkind == REMIX_ID_JYOSHI);
}

int ftRemixYoshiSharedRulesIsGrabSwallower(int fkind)
{
    return (fkind == REMIX_ID_YOSHI) || (fkind == REMIX_ID_NYOSHI) || (fkind == REMIX_ID_JYOSHI);
}

int ftRemixYoshiSharedRulesIsThrowSwallower(int fkind)
{
    return ftRemixYoshiSharedRulesIsGrabSwallower(fkind) || (fkind == REMIX_ID_PIANO);
}

int ftRemixYoshiSharedRulesIsJapaneseEggLayer(int capture_fkind, int copy_fkind)
{
    if ((capture_fkind == REMIX_ID_KIRBY) || (capture_fkind == REMIX_ID_JKIRBY))
    {
        return copy_fkind == REMIX_ID_JYOSHI;
    }
    return capture_fkind == REMIX_ID_JYOSHI;
}

int ftRemixYoshiSharedRulesEggEscapeWaitMax(int is_japanese_layer, int vanilla_wait)
{
    return is_japanese_layer ? FTREMIX_YOSHI_J_EGG_ESCAPE_WAIT_MAX : vanilla_wait;
}

int ftRemixYoshiSharedRulesEggBreakoutInputsMin(int is_japanese_layer, int vanilla_inputs)
{
    return is_japanese_layer ? FTREMIX_YOSHI_J_EGG_BREAKOUT_INPUTS_MIN : vanilla_inputs;
}

/** Copies the FTREMIX_YOSHI_EGG_ROW_FLOATS floats of `source` to `row`. */
static void ftRemixYoshiSharedRulesCopyEggRow(const float *source, float *row)
{
    int index;

    for (index = 0; index < FTREMIX_YOSHI_EGG_ROW_FLOATS; index++)
    {
        row[index] = source[index];
    }
}

int ftRemixYoshiSharedRulesGetEggRow(int fkind, float *row)
{
    switch (fkind)
    {
    case REMIX_ID_BOWSER:
    case REMIX_ID_NBOWSER:
    case REMIX_ID_DEDEDE:
    case REMIX_ID_NDEDEDE:
    case REMIX_ID_PIANO:
        ftRemixYoshiSharedRulesCopyEggRow(sFTRemixYoshiEggDonkeyRow, row);
        return 1;

    case REMIX_ID_GBOWSER:
        ftRemixYoshiSharedRulesCopyEggRow(sFTRemixYoshiEggGigaBowserRow, row);
        return 1;

    default:
        return 0;
    }
}

#endif
