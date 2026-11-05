#ifndef _FTREMIXAIRECOVERYLOGICRULES_H_
#define _FTREMIXAIRECOVERYLOGICRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Character.recovery_logic routines of the Remix AI (AI.asm custom_recovery_logic, and the
 * <family>shared.asm / <Fighter>.asm routines it calls after the walk step), so the headless unit tests (tests/remix) can run
 * them without game state. Numbers in, numbers out. The hook (ftremixairecoverylogic.c) fills the state from the fighter and
 * applies the result. */

/* AI.asm AI.ROUTINE ids that the routines issue. */
#define FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_JUMP 0x04
#define FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_FTILT 0x05
#define FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP 0x1B
#define FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS 0x42
#define FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET 0x49
#define FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NULL 0x4C

#define FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE (-1)

/* CPU button bits (cp.button_inputs). */
#define FTREMIX_AI_RECOVERY_LOGIC_BUTTON_B 0x4000
#define FTREMIX_AI_RECOVERY_LOGIC_BUTTON_C_UP 0x0008
#define FTREMIX_AI_RECOVERY_LOGIC_BUTTONS_C_MASK 0xFFF0

/* Maximum CPU stick value (0x50 in the asm). */
#define FTREMIX_AI_RECOVERY_LOGIC_STICK_MAX 80

/* One routine per Character.recovery_logic entry that the shared files and fighters register. */
typedef enum FTRemixAiRecoveryLogicKind
{
    FTREMIX_AI_RECOVERY_LOGIC_KIND_NONE,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO,   /* marioshared: Mario, Luigi, Dr. Mario and their J versions */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_PIKACHU, /* pikashared: Pikachu, EPika, JPika */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF,    /* jigglypuffkirbyshared: Jigglypuff, JPuff, EPuff */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI,   /* yoshishared and nessshared (same body): Yoshi, JYoshi, Ness, JNess, Lucas */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_CAPTAIN, /* captainshared: Falcon, JFalcon, Dragon King */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_LINK,    /* linkshared: Link, JLink, ELink */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX,     /* foxshared: Fox, JFox */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_WARIO,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_MARTH,   /* Marth.asm: Marth, Roy */
    FTREMIX_AI_RECOVERY_LOGIC_KIND_CRASH,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_FALCO,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_SHEIK,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_MARINA,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_SONIC,
    FTREMIX_AI_RECOVERY_LOGIC_KIND_PEACH

} FTRemixAiRecoveryLogicKind;

/* What the routines read from the fighter (N64 offsets of the asm in the comments). */
typedef struct FTRemixAiRecoveryLogicState
{
    int level_ten;             /* cpu level (0x13) is 10 */
    int action_id;             /* status id (0x24) */
    unsigned int status_frame; /* status_total_tics (0x1C) */
    int lr;                    /* facing direction (0x44) */
    int jumps_used;            /* 0x148 */
    int jumps_max;             /* attributes 0x64 */
    int has_item;              /* held item (0x84C) is set */
    int jump_command_active;   /* the CPU command script is the JUMP routine (0x1D4) */
    int word_adc;              /* passive word 0 (0x0ADC): Mario tornado used, Sheik aerial DSP used */
    int minions_full;          /* Dedede has both thrown minions out (0x0ADC and 0x0AE0) */
    int frame_is_odd;          /* low bit of the screen frame count */
    unsigned int buttons;      /* cp.button_inputs (0x1C6) */
    float x;                   /* position (0x78 pointer) */
    float y;
    float vel_x;               /* 0x48 */
    float vel_y;               /* 0x4C */
    float air_speed_max_x;     /* attributes 0x50 */
    float air_friction;        /* attributes 0x54 */
    float cliff_catch_y;       /* attributes 0xB0, used by the asm as both the ledge grab X and Y */
    float left_x;              /* nearest left ledge (0x1CC + 0x4C) */
    float left_y;
    float right_x;             /* nearest right ledge (0x1CC + 0x54) */
    float right_y;
    float target_x;            /* computer target (0x1CC + 0x60) */
    float target_y;

} FTRemixAiRecoveryLogicState;

/* What a routine asks the hook to do, in the asm order: store the target, set stick and buttons, run the command. */
typedef struct FTRemixAiRecoveryLogicResult
{
    int command;      /* routine id for ftComputerSetCommandImmediate, or FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE */
    int sets_target;
    float target_x;
    float target_y;
    int sets_stick_x;
    int stick_x;
    int sets_stick_y;
    int stick_y;
    int sets_buttons;
    unsigned int buttons;
    int roll_modulus; /* > 0: the command and target apply only when a draw in [0, roll_modulus) is 0 */

} FTRemixAiRecoveryLogicResult;

/** Clears a result to "do nothing". */
extern void ftRemixAiRecoveryLogicClearResult(FTRemixAiRecoveryLogicResult *result);

/** The nearest ledge by horizontal distance (the left one on a tie); writes its position. */
extern void ftRemixAiRecoveryLogicNearestLedge(const FTRemixAiRecoveryLogicState *state, float *ledge_x, float *ledge_y);

/** Stores the ledge as the target and asks for a routine (the `swc1 ... 0x60/0x64` pairs followed by jal 0x80132758). */
extern void ftRemixAiRecoveryLogicTargetLedge(FTRemixAiRecoveryLogicResult *result, int command, float ledge_x, float ledge_y);

/** Holds the stick up and toward the side of x_diff under the NULL routine, so the inputs are not overridden (_hold_up). */
extern void ftRemixAiRecoveryLogicHoldUpToward(FTRemixAiRecoveryLogicResult *result, float x_diff);

/** TRUE when the fighter has used all its jumps. */
extern int ftRemixAiRecoveryLogicHasNoJumpsLeft(const FTRemixAiRecoveryLogicState *state);

/** Runs the routine of a kind. The result is cleared first. */
extern void ftRemixAiRecoveryLogicRun(int kind, const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);

/* Families registered in the shared files (ftremixairecoverylogicsharedrules.c). */
extern void ftRemixAiRecoveryLogicMario(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicPikachu(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicPuff(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicYoshi(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicCaptain(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicLink(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicFox(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);

/* Single fighters (ftremixairecoverylogicfighterrules.c). */
extern void ftRemixAiRecoveryLogicBanjo(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicWario(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicMarth(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicCrash(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicDedede(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicFalco(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicSheik(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicMarina(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicSonic(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);
extern void ftRemixAiRecoveryLogicPeach(const FTRemixAiRecoveryLogicState *state, FTRemixAiRecoveryLogicResult *result);

#endif

#endif
