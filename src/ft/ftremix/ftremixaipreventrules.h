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

/* Vanilla routine addresses a Character.ai_attack_prevent entry can hold (AI.asm PREVENT_ATTACK.ROUTINE, vanilla part). */
#define FTREMIX_AI_PREVENT_ADDR_NONE 0x80133520u
#define FTREMIX_AI_PREVENT_ADDR_MARIO 0x801334ECu
#define FTREMIX_AI_PREVENT_ADDR_KIRBY 0x80133500u
#define FTREMIX_AI_PREVENT_ADDR_YOSHI_FALCON 0x80133510u
#define FTREMIX_AI_PREVENT_ADDR_SKIP_ATTACK 0x80133A14u

/* What a routine does to a candidate attack: nothing, set is_attempt_cliffcatch (the ledge probe of the picker may drop the
 * candidate), or drop the candidate (jump to 0x80133A14). */
#define FTREMIX_AI_PREVENT_VERDICT_ALLOW 0
#define FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK 1
#define FTREMIX_AI_PREVENT_VERDICT_SKIP 2

/* Routine kinds (one per PREVENT_ATTACK.ROUTINE label that a fighter registers). */
typedef enum FTRemixAiPreventRoutineKind
{
    FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN = -1,
    FTREMIX_AI_PREVENT_ROUTINE_NONE,
    FTREMIX_AI_PREVENT_ROUTINE_MARIO,
    FTREMIX_AI_PREVENT_ROUTINE_KIRBY,
    FTREMIX_AI_PREVENT_ROUTINE_YOSHI_FALCON,
    FTREMIX_AI_PREVENT_ROUTINE_SKIP_ATTACK,
    FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP,
    FTREMIX_AI_PREVENT_ROUTINE_FOX_USP,
    FTREMIX_AI_PREVENT_ROUTINE_BOWSER_USP_DSP,
    FTREMIX_AI_PREVENT_ROUTINE_GBOWSER,
    FTREMIX_AI_PREVENT_ROUTINE_USP,
    FTREMIX_AI_PREVENT_ROUTINE_SONIC_DSP,
    FTREMIX_AI_PREVENT_ROUTINE_WARIO,
    FTREMIX_AI_PREVENT_ROUTINE_MARINA_NSP,
    FTREMIX_AI_PREVENT_ROUTINE_CONKER_GRENADE,
    FTREMIX_AI_PREVENT_ROUTINE_CRASH
} FTRemixAiPreventRoutineKind;

/* Fighter state the routines read (the registers t1 = input, s0 = fighter of the asm). */
typedef struct FTRemixAiPreventContext
{
    int input_kind;
    int cpu_level;
    int is_airborne;
    int is_soft_platform;
    int grenade_word;
} FTRemixAiPreventContext;

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

/**
 * Routine kind of a vanilla address held by the table, or FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN for Remix code (the caller
 * resolves it through the fighters that register the Remix routines).
 */
extern int ftRemixAiPreventRulesVanillaKind(unsigned int routine_addr);

/**
 * Effect of a routine on a candidate attack (FTREMIX_AI_PREVENT_VERDICT_*). A routine that makes the asm set t2 = 1
 * returns CLIFF_CHECK, one that jumps to SKIP_ATTACK (0x80133A14) returns SKIP. The vanilla MARIO, KIRBY and
 * YOSHI_FALCON routines only set t2. Unknown and NONE kinds allow.
 */
extern int ftRemixAiPreventRulesVerdict(int routine_kind, const FTRemixAiPreventContext *context);

#endif

#endif
