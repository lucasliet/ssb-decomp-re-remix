#ifndef _FTREMIXAIPREVENTRULES_H_
#define _FTREMIXAIPREVENTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure veto rules of the Remix PREVENT_ATTACK routines (AI.asm PREVENT_ATTACK.ROUTINE scope), so the headless unit tests
 * (tests/remix) can run them without game state. Numbers in, numbers out. A non-zero result vetoes the candidate attack. */

#define FTREMIX_AI_PREVENT_INPUT_NSPG 0x09
#define FTREMIX_AI_PREVENT_INPUT_USPG 0x0D
#define FTREMIX_AI_PREVENT_INPUT_DSPG 0x1B
#define FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS 0x4D
#define FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS_JUMP 0x4E
#define FTREMIX_AI_PREVENT_INPUT_SMASH_DSP 0x41
#define FTREMIX_AI_PREVENT_CPU_LEVEL_LVL10 10

/** TRUE when the candidate input is the given input id. */
extern int ftRemixAiPreventRulesIsInput(int input_kind, int vetoed_input);

/** TRUE when the candidate input is the Down Special or the Up Special (Bowser). */
extern int ftRemixAiPreventRulesIsDspOrUsp(int input_kind);

/** TRUE when the candidate input is one of the Wario neutral special towards inputs or the Down Special. */
extern int ftRemixAiPreventRulesIsWarioRisky(int input_kind);

/**
 * Falco neutral special: vetoed only while airborne (ga != nMPKineticsGround), the grounded NSP is safe. The Up Special
 * goes to ftRemixAiPreventRulesFoxUspVeto instead, as in the asm (FALCO_NSP jumps to FOX_USP).
 */
extern int ftRemixAiPreventRulesFalcoNspVeto(int input_kind, int is_airborne);

/** Fox, Falco and Slippy Up Special: vetoed only at CPU level 10 (the offensive Up Special is not used at lv10). */
extern int ftRemixAiPreventRulesFoxUspVeto(int input_kind, int cpu_level);

/**
 * Conker grenade: the Down Special or its SMASH_DSP routine is vetoed while a grenade is already out. The asm tests the
 * grenade word with `beqz` (zero means available), so any non-zero word is a grenade in use.
 */
extern int ftRemixAiPreventRulesConkerGrenadeVeto(int input_kind, int grenade_word);

/**
 * Crash: the grounded Down Special and Up Special are vetoed while Crash stands on a soft platform (the asm reads the
 * surface flag 0x4000 of the fighter's floor flags).
 */
extern int ftRemixAiPreventRulesCrashSoftPlatformVeto(int input_kind, int is_soft_platform);

#endif

#endif
