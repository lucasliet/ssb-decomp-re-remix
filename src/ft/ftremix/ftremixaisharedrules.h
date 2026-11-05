#ifndef _FTREMIXAISHAREDRULES_H_
#define _FTREMIXAISHAREDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the CPU hooks that the Remix <family>shared.asm files add to the vanilla CPU (dkshared, jigglypuffkirbyshared,
 * nessshared, yoshishared), so the headless unit tests (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_SHARED_INPUT_NSP 0x09 /* AI.asm AI.ROUTINE.NSP: neutral special, smash toward the target */

/* Marina.asm Action.Cargo to CargoTurn: the carry loop of her Cargo (0xEB, CargoWalk1-3 0xEC-0xEE, CargoTurn 0xEF). */
#define FTREMIX_AI_SHARED_MARINA_CARGO_FIRST 0xEB
#define FTREMIX_AI_SHARED_MARINA_CARGO_LAST 0xEF

/* Dedede.asm Action.NSP_IDLE_GROUND, NSP_FALL (inhaled victim held) and USP_MOVE (jumping up special). */
#define FTREMIX_AI_SHARED_DEDEDE_NSP_IDLE_GROUND 0xEC
#define FTREMIX_AI_SHARED_DEDEDE_NSP_FALL 0xF4
#define FTREMIX_AI_SHARED_DEDEDE_USP_MOVE 0xE5

/* Kirby.asm Action.DEDEDE_NSP_IDLE_GROUND and DEDEDE_NSP_FALL: Kirby wearing the Dedede hat holding an inhaled victim. */
#define FTREMIX_AI_SHARED_KIRBY_DEDEDE_NSP_IDLE_GROUND 0x170
#define FTREMIX_AI_SHARED_KIRBY_DEDEDE_NSP_FALL 0x178

/* Hat ids from 0x20 up belong to the Dedede hat (jigglypuffkirbyshared.asm kirby_cpu_inhale_3, `sltiu at, hat, 0x20`). */
#define FTREMIX_AI_SHARED_DEDEDE_HAT_MIN 0x20

#define FTREMIX_AI_SHARED_MARINA_USP_JUMPS_USED 6 /* yoshishared.asm yoshi_cpu_fix_1: Marina has used all six jumps */

/* nessshared.asm improved_pk_thunder_: aim offset = MULTIPLIER * atan2(ledge_dy + LEDGE_Y_OFFSET, |ledge_dx|), used while the
 * hold status is at most MAX_TIME tics old. */
#define FTREMIX_AI_SHARED_PKTHUNDER_MULTIPLIER 512.0F  /* lui 0x4400 */
#define FTREMIX_AI_SHARED_PKTHUNDER_LEDGE_Y_OFFSET 500.0F /* lui 0x43FA */
#define FTREMIX_AI_SHARED_PKTHUNDER_MAX_TIME 0x3B

typedef enum FTRemixAiSharedInhaleOutcome
{
    FTREMIX_AI_SHARED_INHALE_SKIP,       /* no inhale logic for this fighter or status */
    FTREMIX_AI_SHARED_INHALE_KIRBY_BLOCK, /* run the vanilla Kirby inhale block (swallow or spit by Kirby statuses) */
    FTREMIX_AI_SHARED_INHALE_PRESS_NSP   /* press B to spit the held victim */

} FTRemixAiSharedInhaleOutcome;

/** TRUE for the Marina and N Marina Cargo carry statuses (dkshared.asm cpu_fix_2: a CPU must not evade while it carries). */
extern int ftRemixAiSharedIsMarinaCargoStatus(int status_id);

/**
 * kirby_cpu_inhale_3 decision. Kirby and J Kirby (is_kirby_kind) run the vanilla Kirby block unless their hat id is a Dedede hat
 * (compared unsigned, as the asm does), in which case they press B in the Dedede inhaled statuses. Dedede presses B in his own
 * inhaled statuses. Anything else is skipped.
 */
extern int ftRemixAiSharedInhaleOutcome(int is_kirby_kind, int is_dedede, int hat_id, int status_id);

/** multi_jump_recovery_fix_: Dedede in USP_MOVE leaves the walk objective at once instead of steering. */
extern int ftRemixAiSharedWalkExitsInDededeUsp(int status_id);

/** yoshi_cpu_fix_1: Marina recovers with her up special only after all six jumps are used. */
extern int ftRemixAiSharedMarinaMayUseUsp(int jumps_used);

/** improved_pk_thunder_: TRUE while the PK Thunder hold is young enough (status tics <= MAX_TIME) to aim at the ledge. */
extern int ftRemixAiSharedPkThunderAimsAtLedge(unsigned int status_total_tics);

/**
 * improved_pk_thunder_: the value that replaces the vanilla 100 unit offset; the aim target Y is the own Y minus this value.
 * It is MULTIPLIER * atan2(ledge_y - own_y + LEDGE_Y_OFFSET, |ledge_x - own_x|).
 */
extern float ftRemixAiSharedPkThunderAimOffset(float own_x, float own_y, float ledge_x, float ledge_y);

#endif

#endif
