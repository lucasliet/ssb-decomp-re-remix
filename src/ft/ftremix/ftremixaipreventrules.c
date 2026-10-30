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

#endif
