/**
 * ftremixjab.c - Smash Remix jab 3 and rapid jab tables (src/Character.asm :1233-1380, tables :2063-2144) read from the RAM
 * image, plus the Mewtwo (Mewtwo.asm rapid_jab_patch_) and Slippy (Slippy.asm unique_jab_loop_) rapid jab patches.
 * See ftremixjab.h and ftremixjabrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixjab.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

_Static_assert(nFTCommonStatusSpecialStart == FTREMIX_JAB_SPECIAL_STATUS_START, "Remix action 0xDC");

#define FTREMIX_JAB_TABLE_ENTRY_SIZE 4

/** Word of the fighter's row in one of the Character.asm jab tables. */
static u32 ftRemixJabReadRow(u32 table, s32 fkind)
{
    return remix_ram_image_read_word(table + (fkind * FTREMIX_JAB_TABLE_ENTRY_SIZE));
}

static void ftRemixJabReadVanillaRows(u32 table, u32 *vanilla_words)
{
    s32 index;

    for (index = 0; index < FTREMIX_RAPID_JAB_STYLE_COUNT; index++)
    {
        vanilla_words[index] = ftRemixJabReadRow(table, dFTRemixJabRapidStyleFkinds[index]);
    }
}

static u32 ftRemixJabGetRapidPhaseTable(s32 phase)
{
    switch (phase)
    {
    case nFTRemixJabRapidPhaseBegin:
        return REMIX_SYM_Character__rapid_jab_begin_action__table;

    case nFTRemixJabRapidPhaseLoop:
        return REMIX_SYM_Character__rapid_jab_loop_action__table;

    default:
        return REMIX_SYM_Character__rapid_jab_ending_action__table;
    }
}

sb32 ftRemixJabIsRemixKind(s32 fkind)
{
    return fkind >= nFTKindEnumCount;
}

sb32 ftRemixJab3IsEnabled(s32 fkind)
{
    if (ftRemixJabRulesIsJab3Gate(ftRemixJabReadRow(REMIX_SYM_Character__jab_3__table, fkind)) == FALSE)
    {
        return FALSE;
    }
    return ftRemixJab3GetStatus(fkind) != FTREMIX_JAB_STATUS_NONE;
}

void ftRemixJabSetFollowupFrames(FTStruct *fp)
{
    u32 timer_word;

    if (ftRemixJabIsRemixKind(fp->fkind) == FALSE)
    {
        return;
    }
    timer_word = ftRemixJabReadRow(REMIX_SYM_Character__jab_3_timer__table, fp->fkind);

    switch (ftRemixJabRulesFollowupKind(timer_word))
    {
    case nFTRemixJabFollowupDefault:
        fp->attack1_followup_frames = FTCOMMON_ATTACK1_FOLLOWUP_FRAMES_DEFAULT;
        break;

    case nFTRemixJabFollowupLong:
        fp->attack1_followup_frames = FTREMIX_JAB_3_LONG_FOLLOWUP_FRAMES;
        break;

    default:
        break;
    }
}

s32 ftRemixJab3GetStatus(s32 fkind)
{
    u32 action_word;

    if (ftRemixJabIsRemixKind(fkind) == FALSE)
    {
        return FTREMIX_JAB_STATUS_NONE;
    }
    action_word = ftRemixJabReadRow(REMIX_SYM_Character__jab_3_action__table, fkind);

    return ftRemixJabRulesJab3Status(fkind, action_word);
}

sb32 ftRemixJabStartsRapidJabAtJab2(s32 fkind)
{
    return ftRemixJabRulesStartsRapidJabAtJab2(fkind);
}

sb32 ftRemixJabRapidJabRestartsJab1(s32 fkind)
{
    return ftRemixJabRulesRapidJabRestartsJab1(fkind);
}

sb32 ftRemixJabRapidGateIsOpen(s32 fkind)
{
    return ftRemixJabRulesIsRapidGate(ftRemixJabReadRow(REMIX_SYM_Character__rapid_jab__table, fkind));
}

s32 ftRemixJabRapidStatus(s32 fkind, s32 phase)
{
    u32 vanilla_words[FTREMIX_RAPID_JAB_STYLE_COUNT];
    u32 table;
    s32 status_id;

    if (ftRemixJabIsRemixKind(fkind) == FALSE)
    {
        return FTREMIX_JAB_RAPID_VANILLA;
    }
    table = ftRemixJabGetRapidPhaseTable(phase);
    ftRemixJabReadVanillaRows(table, vanilla_words);

    status_id = ftRemixJabRulesRapidStatus(phase, fkind, ftRemixJabReadRow(table, fkind), vanilla_words);

    return (status_id == FTREMIX_JAB_STATUS_NONE) ? FTREMIX_JAB_RAPID_UNRESOLVED : status_id;
}

sb32 ftRemixJabRapidInputQuery(s32 fkind, s32 *inputs_min, s32 *status_id)
{
    u32 vanilla_words[FTREMIX_RAPID_JAB_STYLE_COUNT];
    s32 style;

    ftRemixJabReadVanillaRows(REMIX_SYM_Character__rapid_jab_unknown__table, vanilla_words);

    style = ftRemixJabRulesRapidStyle(ftRemixJabReadRow(REMIX_SYM_Character__rapid_jab_unknown__table, fkind), vanilla_words);

    if (style == nFTRemixJabRapidStyleNone)
    {
        return FALSE;
    }
    *inputs_min = ftRemixJabRulesRapidInputsMin(style);
    *status_id = (style == nFTRemixJabRapidStyleCaptain) ? nFTCaptainStatusAttack13 : nFTCommonStatusAttack12;

    return TRUE;
}

#endif
