#ifndef _FTREMIXAIDIRULES_H_
#define _FTREMIXAIDIRULES_H_

#include <stdint.h>

/*
 * Pure rules of the Remix CPU DI (AI.asm cpu_di_ 6533-6812 and the DI_* input routines 2039-2145): ints only, compiled standalone by
 * tests/remix. The caller reads the fighter and the Training menu, and sets the CPU script from the descriptor built here.
 */

#define FTREMIXAIDI_PARAM_COUNT 4
#define FTREMIXAIDI_PARAM_TYPE 0
#define FTREMIXAIDI_PARAM_STRENGTH 1
#define FTREMIXAIDI_PARAM_DIRECTION 2
#define FTREMIXAIDI_PARAM_ADVANCED 3

#define FTREMIXAIDI_DIRECTION_LEFT 0
#define FTREMIXAIDI_DIRECTION_RIGHT 1
#define FTREMIXAIDI_DIRECTION_UP 2
#define FTREMIXAIDI_DIRECTION_DOWN 3

#define FTREMIXAIDI_STRENGTH_HIGH 0
#define FTREMIXAIDI_STRENGTH_MEDIUM 1
#define FTREMIXAIDI_STRENGTH_LOW 2

/** Longest DI script: the slide DI (eight stick commands plus the end byte). */
#define FTREMIXAIDI_SCRIPT_SIZE 20

/** Training menu entries read by the DI (Training.asm entry_di_*), as the menu stores them. */
#define FTREMIXAIDI_MENU_TYPE_NONE 0
#define FTREMIXAIDI_MENU_TYPE_RANDOM 1
#define FTREMIXAIDI_MENU_DIRECTION_RANDOM 0
#define FTREMIXAIDI_MENU_DIRECTION_AWAY 5

typedef struct FTRemixAiDiIn
{
    int32_t is_training;                /* Training match: the menu entries below decide */
    int32_t menu_type;                  /* 0 none, 1 random, 2 smash, 3 slide */
    int32_t menu_strength;              /* 0 random, 1 high, 2 medium, 3 low */
    int32_t menu_direction;             /* 0 random, 1 left, 2 right, 3 up, 4 down, 5 away, 6 toward */
    int32_t menu_first_hit;             /* Start on Hit */
    int32_t hit_count;                  /* combo hits taken (combo_count_foe) */
    int32_t is_in_hitlag;               /* hitlag_tics != 0 (the asm reads it as "hitstun") */
    int32_t status_frame;               /* status_total_tics */
    int32_t is_damage_status;           /* status id between DamageHi1 and DamageFlyRoll */
    int32_t cpu_level;
    int32_t is_facing_right;
    int32_t uses_advanced_ai;           /* Remix 1P, or Improved AI outside the classic 1P game (non-training only) */

} FTRemixAiDiIn;

typedef struct FTRemixAiDiOut
{
    int32_t has_routine;                /* TRUE: replace the objective with the DI script */
    int32_t is_slide;
    int32_t strength;                   /* FTREMIXAIDI_STRENGTH_* */
    int32_t direction;                  /* FTREMIXAIDI_DIRECTION_* */
    int8_t params[FTREMIXAIDI_PARAM_COUNT]; /* cpu_di_parameters to store for the port */

} FTRemixAiDiOut;

/** Returns a number from 0 to range - 1. The game's RNG in the port, a fake in tests. */
typedef int32_t (*FTRemixAiDiRandomFn)(int32_t range);

/**
 * One call of cpu_di_ for one CPU fighter.
 *
 * @param in Fighter and menu state.
 * @param saved_params cpu_di_parameters of the port, -1 meaning not set.
 * @param random_below RNG, called only where the asm calls Global.get_random_int_.
 * @param out Decision and the parameters to store back.
 */
extern void ftRemixAiDiDecide(const FTRemixAiDiIn *in, const int8_t *saved_params, FTRemixAiDiRandomFn random_below, FTRemixAiDiOut *out);

/**
 * Builds the CPU input script of a DI routine (the di_input macro). The bytes are the format the CPU parser reads: B0 yy sets the
 * stick y, A<wait> xx sets the stick x and waits, FF ends.
 *
 * @param is_slide Slide DI (stick wiggle) instead of smash DI.
 * @param strength FTREMIXAIDI_STRENGTH_*, which sets the wait of every stick x command (1, 3 or 5 frames).
 * @param direction FTREMIXAIDI_DIRECTION_*.
 * @param out_script Buffer of FTREMIXAIDI_SCRIPT_SIZE bytes.
 * @return Length of the script in bytes.
 */
extern int32_t ftRemixAiDiBuildScript(int32_t is_slide, int32_t strength, int32_t direction, uint8_t *out_script);

#endif
