/*
 * ftremixailongrange.c - LONG_RANGE and custom far-range command of the Remix AI (AI.asm LONG_RANGE scope, the table
 * of Character.ai_long_range and Character.nsp_shoot_custom_move). Pure rules live in ftremixailongrangerules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <port_log.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixailongrange.h"
#include "ft/ftremix/ftremixailongrangerules.h"

#define FTREMIX_AI_LONG_RANGE_ENTRY_BYTES 4
#define FTREMIX_AI_LONG_RANGE_LOG_KIND_MAX 64

static u64 sFtRemixAiLongRangeLoggedKinds;

/**
 * Logs the first unknown routine of each fighter kind, so a bad table entry shows once and does not flood the log every
 * frame. Kinds >= FTREMIX_AI_LONG_RANGE_LOG_KIND_MAX are not logged.
 */
static void ftRemixAiLongRangeLogUnknownRoutine(s32 fkind, u32 routine_addr)
{
    u64 kind_bit;

    if ((fkind < 0) || (fkind >= FTREMIX_AI_LONG_RANGE_LOG_KIND_MAX))
    {
        return;
    }
    kind_bit = (u64)1 << fkind;
    if ((sFtRemixAiLongRangeLoggedKinds & kind_bit) != 0)
    {
        return;
    }
    sFtRemixAiLongRangeLoggedKinds |= kind_bit;
    port_log("ftRemixAiLongRangeSwitchKind: fkind 0x%X has long range routine 0x%08X, expected NSP_SHOOT 0x80138D24, "
             "PHYSICAL_SHOOT 0x80138CD4 or NONE 0x80138ECC; treated as NONE\n",
             fkind, routine_addr);
}

/** Maps the port kind of a LONG_RANGE routine to the far-range switch case that runs the same routine. */
static s32 ftRemixAiLongRangeCaseOfKind(s32 routine_kind)
{
    switch (routine_kind)
    {
    case FTREMIX_AI_LONG_RANGE_KIND_NSP_SHOOT:
        return nFTKindMario;
    case FTREMIX_AI_LONG_RANGE_KIND_PHYSICAL_SHOOT:
        return nFTKindLink;
    default:
        return FTREMIX_AI_LONG_RANGE_NO_CASE;
    }
}

/**
 * Far-range switch case of fkind. Vanilla fighters keep the vanilla switch; Remix fighters read their routine from
 * Character.ai_long_range.table, and polygon variants never use the far-range move.
 */
s32 ftRemixAiLongRangeSwitchKind(s32 fkind)
{
    u32 routine_addr;
    s32 routine_kind;

    if (!ftRemixAiLongRangeRulesUsesTable(fkind))
    {
        return fkind;
    }
    if (ftRemixAiLongRangeRulesIsPolygon(fkind))
    {
        return FTREMIX_AI_LONG_RANGE_NO_CASE;
    }
    routine_addr = remix_ram_image_read_word(REMIX_SYM_Character__ai_long_range__table +
                                             (fkind * FTREMIX_AI_LONG_RANGE_ENTRY_BYTES));
    routine_kind = ftRemixAiLongRangeRulesRoutineKind(routine_addr);
    if (routine_kind == FTREMIX_AI_LONG_RANGE_KIND_UNKNOWN)
    {
        ftRemixAiLongRangeLogUnknownRoutine(fkind, routine_addr);
        return FTREMIX_AI_LONG_RANGE_NO_CASE;
    }
    return ftRemixAiLongRangeCaseOfKind(routine_kind);
}

/**
 * Far-range command of fp: the table entry of fp->fkind (Character.nsp_shoot_custom_move.table, indexed by the fighter id
 * at N64 0x08) replaces default_command when it is not 0.
 */
s32 ftRemixAiLongRangeCommand(FTStruct *fp, s32 default_command)
{
    u32 table_command;

    table_command = remix_ram_image_read_word(REMIX_SYM_Character__nsp_shoot_custom_move__table +
                                              (fp->fkind * FTREMIX_AI_LONG_RANGE_ENTRY_BYTES));

    return ftRemixAiLongRangeRulesCustomCommand((s32)table_command, default_command);
}

/**
 * Issues the far-range command of fp with the custom move override. The default matches the vanilla branch on is_delay.
 */
void ftRemixAiLongRangeSetCommand(FTStruct *fp, sb32 is_delay)
{
    s32 default_command = nFTComputerInputStickSmashAutoXButtonB;

    if (is_delay != FALSE)
    {
        default_command = nFTComputerInputStickTiltAutoXNYD5SmashAutoXButtonB;
    }
    ftComputerSetCommandWaitLong(fp, ftRemixAiLongRangeCommand(fp, default_command));
}

#endif
