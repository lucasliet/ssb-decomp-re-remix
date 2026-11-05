#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaipreventrules.h"

/** TRUE when the candidate input is the given input id. */
int ftRemixAiPreventRulesIsInput(int input_kind, int vetoed_input)
{
    return input_kind == vetoed_input;
}

/** TRUE when the candidate input is the Down Special or the Up Special (Bowser). */
int ftRemixAiPreventRulesIsDspOrUsp(int input_kind)
{
    return (input_kind == FTREMIX_AI_PREVENT_INPUT_DSPG) || (input_kind == FTREMIX_AI_PREVENT_INPUT_USPG);
}

/** TRUE when the candidate input is one of the Wario neutral special towards inputs or the Down Special. */
int ftRemixAiPreventRulesIsWarioRisky(int input_kind)
{
    if (input_kind == FTREMIX_AI_PREVENT_INPUT_DSPG)
    {
        return 1;
    }
    if (input_kind == FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS)
    {
        return 1;
    }
    return input_kind == FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS_JUMP;
}

/** Falco NSP is vetoed only while airborne; the grounded NSP is safe. */
int ftRemixAiPreventRulesFalcoNspVeto(int input_kind, int is_airborne)
{
    if (input_kind != FTREMIX_AI_PREVENT_INPUT_NSPG)
    {
        return 0;
    }
    return is_airborne != 0;
}

/** Fox, Falco and Slippy Up Special is vetoed at CPU level 10 only. */
int ftRemixAiPreventRulesFoxUspVeto(int input_kind, int cpu_level)
{
    if (input_kind != FTREMIX_AI_PREVENT_INPUT_USPG)
    {
        return 0;
    }
    return cpu_level >= FTREMIX_AI_PREVENT_CPU_LEVEL_LVL10;
}

/** Conker grenade is vetoed for the Down Special routines while a grenade is already out. */
int ftRemixAiPreventRulesConkerGrenadeVeto(int input_kind, int grenade_word)
{
    int is_grenade_input;

    is_grenade_input = (input_kind == FTREMIX_AI_PREVENT_INPUT_DSPG) || (input_kind == FTREMIX_AI_PREVENT_INPUT_SMASH_DSP);
    return is_grenade_input && (grenade_word != 0);
}

/** Crash DSP and USP are vetoed on a soft platform; the other inputs are never vetoed by this rule. */
int ftRemixAiPreventRulesCrashSoftPlatformVeto(int input_kind, int is_soft_platform)
{
    int is_crash_input;

    is_crash_input = (input_kind == FTREMIX_AI_PREVENT_INPUT_DSPG) || (input_kind == FTREMIX_AI_PREVENT_INPUT_USPG);
    return is_crash_input && (is_soft_platform != 0);
}

/** Table entries that hold vanilla code: the NONE branch, the three vanilla routines and the debug skip. */
int ftRemixAiPreventRulesVanillaKind(unsigned int routine_addr)
{
    switch (routine_addr)
    {
    case FTREMIX_AI_PREVENT_ADDR_NONE:
        return FTREMIX_AI_PREVENT_ROUTINE_NONE;
    case FTREMIX_AI_PREVENT_ADDR_MARIO:
        return FTREMIX_AI_PREVENT_ROUTINE_MARIO;
    case FTREMIX_AI_PREVENT_ADDR_KIRBY:
        return FTREMIX_AI_PREVENT_ROUTINE_KIRBY;
    case FTREMIX_AI_PREVENT_ADDR_YOSHI_FALCON:
        return FTREMIX_AI_PREVENT_ROUTINE_YOSHI_FALCON;
    case FTREMIX_AI_PREVENT_ADDR_SKIP_ATTACK:
        return FTREMIX_AI_PREVENT_ROUTINE_SKIP_ATTACK;
    default:
        return FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN;
    }
}

/** Maps a veto of the asm (t2 = 1) to the cliff check verdict. */
static int ftRemixAiPreventRulesCliffVerdict(int is_vetoed)
{
    return (is_vetoed != 0) ? FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK : FTREMIX_AI_PREVENT_VERDICT_ALLOW;
}

/** Maps a jump to SKIP_ATTACK to the skip verdict. */
static int ftRemixAiPreventRulesSkipVerdict(int is_skipped)
{
    return (is_skipped != 0) ? FTREMIX_AI_PREVENT_VERDICT_SKIP : FTREMIX_AI_PREVENT_VERDICT_ALLOW;
}

/** Falco: the NSP check, whose Up Special case jumps to the Fox Up Special routine. */
static int ftRemixAiPreventRulesFalcoVerdict(const FTRemixAiPreventContext *context)
{
    if (context->input_kind == FTREMIX_AI_PREVENT_INPUT_USPG)
    {
        return ftRemixAiPreventRulesSkipVerdict(ftRemixAiPreventRulesFoxUspVeto(context->input_kind, context->cpu_level));
    }
    return ftRemixAiPreventRulesCliffVerdict(ftRemixAiPreventRulesFalcoNspVeto(context->input_kind, context->is_airborne));
}

/** The vanilla routines (MARIO 0x801334EC, KIRBY 0x80133500, YOSHI_FALCON 0x80133510) only set t2. */
static int ftRemixAiPreventRulesVanillaVerdict(int routine_kind, int input_kind)
{
    switch (routine_kind)
    {
    case FTREMIX_AI_PREVENT_ROUTINE_MARIO:
        return ftRemixAiPreventRulesCliffVerdict(input_kind == FTREMIX_AI_PREVENT_INPUT_USPG);
    case FTREMIX_AI_PREVENT_ROUTINE_KIRBY:
        return ftRemixAiPreventRulesCliffVerdict(ftRemixAiPreventRulesIsDspOrUsp(input_kind));
    case FTREMIX_AI_PREVENT_ROUTINE_YOSHI_FALCON:
        return ftRemixAiPreventRulesCliffVerdict(input_kind == FTREMIX_AI_PREVENT_INPUT_DSPG);
    default:
        return FTREMIX_AI_PREVENT_VERDICT_ALLOW;
    }
}

int ftRemixAiPreventRulesVerdict(int routine_kind, const FTRemixAiPreventContext *context)
{
    int input_kind = context->input_kind;

    switch (routine_kind)
    {
    case FTREMIX_AI_PREVENT_ROUTINE_SKIP_ATTACK:
        return FTREMIX_AI_PREVENT_VERDICT_SKIP;
    case FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP:
        return ftRemixAiPreventRulesFalcoVerdict(context);
    case FTREMIX_AI_PREVENT_ROUTINE_FOX_USP:
        return ftRemixAiPreventRulesSkipVerdict(ftRemixAiPreventRulesFoxUspVeto(input_kind, context->cpu_level));
    case FTREMIX_AI_PREVENT_ROUTINE_BOWSER_USP_DSP:
        return ftRemixAiPreventRulesCliffVerdict(ftRemixAiPreventRulesIsDspOrUsp(input_kind));
    case FTREMIX_AI_PREVENT_ROUTINE_GBOWSER:
    case FTREMIX_AI_PREVENT_ROUTINE_SONIC_DSP:
        return ftRemixAiPreventRulesCliffVerdict(input_kind == FTREMIX_AI_PREVENT_INPUT_DSPG);
    case FTREMIX_AI_PREVENT_ROUTINE_USP:
        return ftRemixAiPreventRulesCliffVerdict(input_kind == FTREMIX_AI_PREVENT_INPUT_USPG);
    case FTREMIX_AI_PREVENT_ROUTINE_WARIO:
        return ftRemixAiPreventRulesCliffVerdict(ftRemixAiPreventRulesIsWarioRisky(input_kind));
    case FTREMIX_AI_PREVENT_ROUTINE_MARINA_NSP:
        return ftRemixAiPreventRulesCliffVerdict(input_kind == FTREMIX_AI_PREVENT_INPUT_NSPG);
    case FTREMIX_AI_PREVENT_ROUTINE_CONKER_GRENADE:
        return ftRemixAiPreventRulesSkipVerdict(ftRemixAiPreventRulesConkerGrenadeVeto(input_kind, context->grenade_word));
    case FTREMIX_AI_PREVENT_ROUTINE_CRASH:
        return ftRemixAiPreventRulesCliffVerdict(
            ftRemixAiPreventRulesCrashSoftPlatformVeto(input_kind, context->is_soft_platform));
    default:
        return ftRemixAiPreventRulesVanillaVerdict(routine_kind, input_kind);
    }
}

#endif
