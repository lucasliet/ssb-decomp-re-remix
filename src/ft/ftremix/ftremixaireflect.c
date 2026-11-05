/*
 * ftremixaireflect.c - CPU reflect and absorb hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/Reflect.asm, scope
 * AI, and AI.asm AERIAL_CHASE). The Character.fighter_reflect and Character.ai_aerial_chase tables are read from the Remix
 * RAM image. Pure rules live in ftremixaireflectrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <ft/ftchar/ftfox/ftfox.h>
#include <ft/ftchar/ftness/ftness.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixaishared.h"
#include "ft/ftremix/ftremixailongrangerules.h"
#include "ft/ftremix/ftremixaireflect.h"
#include "ft/ftremix/ftremixaireflectrules.h"
#include "ft/ftremix/ftremixfields.h"

#define FTREMIX_AI_REFLECT_TABLE_SIZE 97
#define FTREMIX_AI_REFLECT_ENTRY_BYTES 4
#define FTREMIX_AI_REFLECT_CHASE_ROUTINE 0x80133A14u
#define FTREMIX_AI_REFLECT_MARINA_CHARGE_WORD 0x0ADC

/** Character.fighter_reflect entry of fkind (one byte per fighter id); 0 for an id outside the table. */
static sb32 ftRemixAiReflectIsTableReflector(s32 fkind)
{
    if ((fkind < 0) || (fkind >= FTREMIX_AI_REFLECT_TABLE_SIZE))
    {
        return FALSE;
    }
    return (remix_ram_image_read_byte(REMIX_SYM_Character__fighter_reflect__table + fkind) != 0) ? TRUE : FALSE;
}

/**
 * Character.ai_aerial_chase entry of fp (the copied power for Kirby): CHASE makes the NSP candidate yield to a reflector.
 * Polygon fighters are always NORMAL (AERIAL_CHASE.extend_polygon_check_).
 */
static sb32 ftRemixAiReflectIsOwnChase(FTStruct *fp)
{
    s32 fkind = ftRemixAiCopiedPowerKind(fp);

    if ((fkind < 0) || (fkind >= FTREMIX_AI_REFLECT_TABLE_SIZE) || ftRemixAiLongRangeRulesIsPolygon(fkind))
    {
        return FALSE;
    }
    return (remix_ram_image_read_word(REMIX_SYM_Character__ai_aerial_chase__table + (fkind * FTREMIX_AI_REFLECT_ENTRY_BYTES)) ==
            FTREMIX_AI_REFLECT_CHASE_ROUTINE) ? TRUE : FALSE;
}

/** Marina cannot start Clanpot while she holds an item or her charge is at the maximum. */
static sb32 ftRemixAiReflectMarinaCannotAbsorb(FTStruct *fp)
{
    if (fp->item_gobj != NULL)
    {
        return TRUE;
    }
    return (*ftRemixFighterWord(fp, FTREMIX_AI_REFLECT_MARINA_CHARGE_WORD) == FTREMIX_AI_REFLECT_MARINA_CHARGE_MAX) ? TRUE : FALSE;
}

/** TRUE when the status of fp is inside the special scope of the reflect/absorb kind. */
static sb32 ftRemixAiReflectIsInScope(FTStruct *fp, s32 kind)
{
    switch (kind)
    {
    case FTREMIX_AI_REFLECT_KIND_FOX:
        return ((fp->status_id >= nFTFoxStatusSpecialLwScopeStart) && (fp->status_id <= nFTFoxStatusSpecialLwScopeEnd)) ? TRUE : FALSE;

    case FTREMIX_AI_REFLECT_KIND_NESS:
        return ((fp->status_id >= nFTNessStatusSpecialLwScopeStart) && (fp->status_id <= nFTNessStatusSpecialLwScopeEnd)) ? TRUE : FALSE;

    default:
        return ftRemixAiReflectRulesIsAbsorbAction(kind, fp->status_id) ? TRUE : FALSE;
    }
}

f32 ftRemixAiReflectDetectionFrames(FTStruct *fp)
{
    return ftRemixAiReflectRulesDetectionFrames(fp->fkind);
}

sb32 ftRemixAiReflectProjectileRaisesFlag(FTStruct *fp, WPStruct *wp)
{
    return ftRemixAiReflectRulesProjectileRaisesFlag(fp->fkind, wp->kind, ftRemixAiReflectMarinaCannotAbsorb(fp)) ? TRUE : FALSE;
}

sb32 ftRemixAiReflectItemRaisesFlag(FTStruct *fp)
{
    return ftRemixAiReflectRulesItemRaisesFlag(fp->fkind, ftRemixAiReflectIsTableReflector(fp->fkind),
                                               ftRemixAiReflectMarinaCannotAbsorb(fp)) ? TRUE : FALSE;
}

sb32 ftRemixAiReflectMaintainInput(FTStruct *fp)
{
    s32 kind = ftRemixAiReflectRulesMaintainKind(fp->fkind);

    if ((kind == FTREMIX_AI_REFLECT_KIND_NONE) || (ftRemixAiReflectIsInScope(fp, kind) == FALSE))
    {
        return FALSE;
    }
    ftComputerSetCommandWaitShort(fp, nFTComputerInputStickNButtonBRelease);
    return TRUE;
}

void ftRemixAiReflectApplyInput(FTStruct *fp)
{
    s32 kind = ftRemixAiReflectRulesApplyKind(fp->fkind);

    if ((kind == FTREMIX_AI_REFLECT_KIND_NONE) || (ftRemixAiReflectIsInScope(fp, kind) != FALSE))
    {
        return;
    }
    ftComputerSetCommandImmediate(fp, nFTComputerInputStickNXSmashLwButtonBReleaseBHold);
}

sb32 ftRemixAiReflectSkipsNspCandidate(FTStruct *fp, FTStruct *target_fp)
{
    return ftRemixAiReflectRulesSkipsNspCandidate(ftRemixAiReflectIsTableReflector(target_fp->fkind),
                                                  ftRemixAiReflectIsOwnChase(fp)) ? TRUE : FALSE;
}

#endif
