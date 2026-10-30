/*
 * ftremixaiprevent.c - PREVENT_ATTACK routines of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm
 * PREVENT_ATTACK.ROUTINE, registered per fighter in the <Fighter>/AI/Attacks.asm files). Pure rules live in
 * ftremixaipreventrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/mpdef.h>
#include <port_log.h>
#include "remix/remix_roster_data.h"
#include "ft/ftremix/ftremixaiprevent.h"
#include "ft/ftremix/ftremixaipreventrules.h"
#include "ft/ftremix/ftremixfields.h"

#define FTREMIX_AI_PREVENT_LOG_FKIND_MAX 64
#define FTREMIX_AI_PREVENT_N64_CONKER_GRENADE 0x0ADC

static u64 sFtRemixAiPreventLoggedFkinds;

/**
 * Logs the first veto of each fighter kind, so a match shows which Remix routine fired without flooding the log.
 * Kinds >= FTREMIX_AI_PREVENT_LOG_FKIND_MAX are not logged.
 */
static void ftRemixAiPreventLogFirstVeto(FTStruct *fp, s32 input_kind)
{
    u64 kind_bit;

    if ((fp->fkind < 0) || (fp->fkind >= FTREMIX_AI_PREVENT_LOG_FKIND_MAX))
    {
        return;
    }
    kind_bit = (u64)1 << fp->fkind;
    if ((sFtRemixAiPreventLoggedFkinds & kind_bit) != 0)
    {
        return;
    }
    sFtRemixAiPreventLoggedFkinds |= kind_bit;
    port_log("ftRemixAiPreventAttack: fkind 0x%X level %d vetoed input 0x%X (first veto of this fkind)\n",
             fp->fkind, fp->level, input_kind);
}

/** Fox, Falco and Slippy Up Special: vetoed at CPU level 10. */
static sb32 ftRemixAiPreventFoxUsp(FTStruct *fp, s32 input_kind)
{
    return ftRemixAiPreventRulesFoxUspVeto(input_kind, fp->level);
}

/** Falco NSP is vetoed while airborne; an unvetoed Up Special falls through to the Fox USP rule, as in the asm. */
static sb32 ftRemixAiPreventFalcoNsp(FTStruct *fp, s32 input_kind)
{
    if (ftRemixAiPreventRulesFalcoNspVeto(input_kind, fp->ga != nMPKineticsGround))
    {
        return TRUE;
    }
    return ftRemixAiPreventFoxUsp(fp, input_kind);
}

/** Conker grenade: a Down Special is vetoed while a grenade is already out (N64 word 0x0ADC, passive_vars). */
static sb32 ftRemixAiPreventConkerGrenade(FTStruct *fp, s32 input_kind)
{
    s32 grenade_word = *ftRemixFighterWord(fp, FTREMIX_AI_PREVENT_N64_CONKER_GRENADE);

    return ftRemixAiPreventRulesConkerGrenadeVeto(input_kind, grenade_word);
}

/**
 * Crash DSP and USP on a soft platform (Crash/Crash.asm:716, PREVENT_ATTACK.ROUTINE.CRASH). The asm reads the surface
 * halfword at N64 0x00F6 and tests 0x4000, which is MAP_VERTEX_COLL_PASS in the port's coll_data.floor_flags.
 * REMIX-VERIFY: that N64 offset is inferred from the flag value, not checked against the N64 FTStruct layout.
 */
static sb32 ftRemixAiPreventCrashSoftPlatform(FTStruct *fp, s32 input_kind)
{
    sb32 is_soft_platform = (fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS) != 0;

    return ftRemixAiPreventRulesCrashSoftPlatformVeto(input_kind, is_soft_platform);
}

/**
 * Dispatches the PREVENT_ATTACK routine that the fighter kind's Attacks.asm registers. Duplicate registrations were
 * resolved by include order: Falco keeps FALCO_NSP, DSamus keeps SONIC_DSP.
 */
static sb32 ftRemixAiPreventRoutine(FTStruct *fp, s32 input_kind)
{
    switch (fp->fkind)
    {
    case REMIX_ID_FALCO:
        return ftRemixAiPreventFalcoNsp(fp, input_kind);
    case REMIX_ID_FOX:
    case REMIX_ID_JFOX:
    case REMIX_ID_SLIPPY:
        return ftRemixAiPreventFoxUsp(fp, input_kind);
    case REMIX_ID_BOWSER:
        return ftRemixAiPreventRulesIsDspOrUsp(input_kind);
    case REMIX_ID_GBOWSER:
        return ftRemixAiPreventRulesIsInput(input_kind, FTREMIX_AI_PREVENT_INPUT_DSPG);
    case REMIX_ID_WOLF:
    case REMIX_ID_DEDEDE:
        return ftRemixAiPreventRulesIsInput(input_kind, FTREMIX_AI_PREVENT_INPUT_USPG);
    case REMIX_ID_SONIC:
    case REMIX_ID_DSAMUS:
        return ftRemixAiPreventRulesIsInput(input_kind, FTREMIX_AI_PREVENT_INPUT_DSPG);
    case REMIX_ID_WARIO:
        return ftRemixAiPreventRulesIsWarioRisky(input_kind);
    case REMIX_ID_MARINA:
        return ftRemixAiPreventRulesIsInput(input_kind, FTREMIX_AI_PREVENT_INPUT_NSPG);
    case REMIX_ID_CONKER:
        return ftRemixAiPreventConkerGrenade(fp, input_kind);
    case REMIX_ID_CRASH:
        return ftRemixAiPreventCrashSoftPlatform(fp, input_kind);
    /*
     * REMIX-TODO(AI): Banjo, DrLuigi, DrMario, Marth, Peach, Piano and Roy use the vanilla MARIO routine (0x801334EC) and
     * Ganondorf and DragonKing the vanilla YOSHI_FALCON routine (0x80133510). Their code is not in the smashremix source
     * and needs the ROM disassembly before porting. Mewtwo, Lanky, NFalco, NMarina, Sheik, NSheik and NGanondorf use
     * NONE, which never vetoes.
     */
    default:
        return FALSE;
    }
}

/**
 * PREVENT_ATTACK (AI.asm): TRUE vetoes the candidate attack input_kind before it is committed. The first veto of each
 * fighter kind is logged once through port_log.
 */
sb32 ftRemixAiPreventAttack(FTStruct *fp, s32 input_kind)
{
    sb32 is_vetoed = ftRemixAiPreventRoutine(fp, input_kind);

    if (is_vetoed)
    {
        ftRemixAiPreventLogFirstVeto(fp, input_kind);
    }
    return is_vetoed;
}

#endif
