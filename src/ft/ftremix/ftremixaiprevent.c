/*
 * ftremixaiprevent.c - PREVENT_ATTACK routines of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * PREVENT_ATTACK.ROUTINE, registered per fighter in the <Fighter>/AI/Attacks.asm files). Pure rules live in
 * ftremixaipreventrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/mpdef.h>
#include <port_log.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixaiprevent.h"
#include "ft/ftremix/ftremixaipreventrules.h"
#include "ft/ftremix/ftremixfields.h"

#define FTREMIX_AI_PREVENT_LOG_FKIND_MAX 64
#define FTREMIX_AI_PREVENT_N64_CONKER_GRENADE 0x0ADC
#define FTREMIX_AI_PREVENT_TABLE_ENTRY_BYTES 4

/* Fighter that registers a Remix routine in Character.ai_attack_prevent; its clones copy the entry of their parent. */
typedef struct FTRemixAiPreventOwner
{
    s32 fkind;
    s32 routine_kind;
} FTRemixAiPreventOwner;

static const FTRemixAiPreventOwner sFtRemixAiPreventOwners[] =
{
    { REMIX_ID_FOX, FTREMIX_AI_PREVENT_ROUTINE_FOX_USP },
    { REMIX_ID_FALCO, FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP },
    { REMIX_ID_BOWSER, FTREMIX_AI_PREVENT_ROUTINE_BOWSER_USP_DSP },
    { REMIX_ID_GBOWSER, FTREMIX_AI_PREVENT_ROUTINE_GBOWSER },
    { REMIX_ID_WOLF, FTREMIX_AI_PREVENT_ROUTINE_USP },
    { REMIX_ID_SONIC, FTREMIX_AI_PREVENT_ROUTINE_SONIC_DSP },
    { REMIX_ID_WARIO, FTREMIX_AI_PREVENT_ROUTINE_WARIO },
    { REMIX_ID_MARINA, FTREMIX_AI_PREVENT_ROUTINE_MARINA_NSP },
    { REMIX_ID_CONKER, FTREMIX_AI_PREVENT_ROUTINE_CONKER_GRENADE },
    { REMIX_ID_CRASH, FTREMIX_AI_PREVENT_ROUTINE_CRASH }
};

static u64 sFtRemixAiPreventLoggedCandidateFkinds;

/** Routine address that Character.ai_attack_prevent holds for a fighter kind (0 outside the loaded image). */
static u32 ftRemixAiPreventGetTableEntry(s32 fkind)
{
    return remix_ram_image_read_word(REMIX_SYM_Character__ai_attack_prevent__table +
                                     (fkind * FTREMIX_AI_PREVENT_TABLE_ENTRY_BYTES));
}

/**
 * Identifies a Remix routine address. The routines are Remix code with no port symbol, so the address is matched with the
 * entry of the fighter that registers it (clones copy their parent's entry in Character.asm add_to_table).
 */
static s32 ftRemixAiPreventRemixRoutineKind(u32 routine_addr)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sFtRemixAiPreventOwners); i++)
    {
        if (ftRemixAiPreventGetTableEntry(sFtRemixAiPreventOwners[i].fkind) == routine_addr)
        {
            return sFtRemixAiPreventOwners[i].routine_kind;
        }
    }
    return FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN;
}

/** Routine kind of a fighter kind, from its Character.ai_attack_prevent entry; an unknown routine counts as NONE. */
static s32 ftRemixAiPreventGetRoutineKind(s32 fkind)
{
    u32 routine_addr = ftRemixAiPreventGetTableEntry(fkind);
    s32 routine_kind = ftRemixAiPreventRulesVanillaKind(routine_addr);

    if (routine_kind != FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN)
    {
        return routine_kind;
    }
    return ftRemixAiPreventRemixRoutineKind(routine_addr);
}

/** Logs the first non-allow verdict of each fighter kind, once. Kinds >= FTREMIX_AI_PREVENT_LOG_FKIND_MAX are not logged. */
static void ftRemixAiPreventLogFirstVerdict(FTStruct *fp, s32 input_kind, s32 verdict)
{
    u64 kind_bit;

    if ((fp->fkind < 0) || (fp->fkind >= FTREMIX_AI_PREVENT_LOG_FKIND_MAX))
    {
        return;
    }
    kind_bit = (u64)1 << fp->fkind;
    if ((sFtRemixAiPreventLoggedCandidateFkinds & kind_bit) != 0)
    {
        return;
    }
    sFtRemixAiPreventLoggedCandidateFkinds |= kind_bit;
    port_log("ftRemixAiPreventCandidate: fkind 0x%X level %d input 0x%X verdict %d (first non-allow verdict of this fkind)\n",
             fp->fkind, fp->level, input_kind, verdict);
}

s32 ftRemixAiPreventCandidate(FTStruct *fp, s32 input_kind)
{
    FTRemixAiPreventContext context;
    s32 verdict;

    context.input_kind = input_kind;
    context.cpu_level = fp->level;
    context.is_airborne = (fp->ga != nMPKineticsGround) ? 1 : 0;
    context.is_soft_platform = ((fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS) != 0) ? 1 : 0;
    context.grenade_word = *ftRemixFighterWord(fp, FTREMIX_AI_PREVENT_N64_CONKER_GRENADE);

    verdict = ftRemixAiPreventRulesVerdict(ftRemixAiPreventGetRoutineKind(fp->fkind), &context);

    if (verdict != FTREMIX_AI_PREVENT_VERDICT_ALLOW)
    {
        ftRemixAiPreventLogFirstVerdict(fp, input_kind, verdict);
    }
    return verdict;
}

#endif
