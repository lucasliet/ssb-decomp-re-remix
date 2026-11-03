#ifndef _SC1PTRAININGRULES_H_
#define _SC1PTRAININGRULES_H_

#include <stdint.h>

/*
 * Pure rules of the Remix Training custom menu (Training.asm shield_break_mode_, spam_practice_, spam_practice_timer_): ints only,
 * compiled standalone by tests/remix. The scene code reads the fighters and writes the controller; these functions only decide.
 */

#define SC1PTRAININGRULES_SHIELD_BREAK_OFF 0
#define SC1PTRAININGRULES_SHIELD_BREAK_ON 1
#define SC1PTRAININGRULES_SHIELD_BREAK_INFINITE 2

/** Per-port state of the shield break CPU (Training.asm player_shield_status). */
#define SC1PTRAININGRULES_STATUS_SHIELD 0
#define SC1PTRAININGRULES_STATUS_STUN 1
#define SC1PTRAININGRULES_STATUS_OOS_FRAME_1 2
#define SC1PTRAININGRULES_STATUS_OOS_FRAME_2 3

#define SC1PTRAININGRULES_OOS_COUNT 12
#define SC1PTRAININGRULES_OOS_DSP 6
#define SC1PTRAININGRULES_OOS_NAIR 7

/** Shield health the CPU is held at (Training.asm 0x37, the unstale maximum). */
#define SC1PTRAININGRULES_MAX_SHIELD 0x37

/** Button masks of the N64 pad that the OOS tables use. */
#define SC1PTRAININGRULES_BUTTON_A 0x8000
#define SC1PTRAININGRULES_BUTTON_B 0x4000
#define SC1PTRAININGRULES_BUTTON_Z 0x2000

/** What the shield break CPU does to the controller this frame. */
typedef enum SC1PTrainingRulesShieldAction
{
    SC1PTRAININGRULES_ACTION_NONE,           /* controller untouched */
    SC1PTRAININGRULES_ACTION_FORCE_SHIELD,   /* hold Z */
    SC1PTRAININGRULES_ACTION_OOS_INPUT,      /* press the OOS button and stick, release Z */
    SC1PTRAININGRULES_ACTION_END_OOS         /* release the OOS button and stick, hold Z */

} SC1PTrainingRulesShieldAction;

typedef struct SC1PTrainingRulesShieldIn
{
    int32_t mode;                     /* SC1PTRAININGRULES_SHIELD_BREAK_* */
    int32_t status;                   /* SC1PTRAININGRULES_STATUS_* of this port */
    int32_t oos_option;               /* 0 to SC1PTRAININGRULES_OOS_COUNT - 1 */
    int32_t is_in_shield_stun;        /* current status is the shield stun status */
    int32_t is_shield_stun_ending;    /* shield stun frames left minus one is not above zero */
    int32_t is_in_shield_jumpsquat;   /* current status is the shield jumpsquat status */
    int32_t is_last_jumpsquat_frame;  /* status frame equals the whole jumpsquat length */

} SC1PTrainingRulesShieldIn;

typedef struct SC1PTrainingRulesShieldOut
{
    int32_t new_status;
    int32_t max_shield;       /* TRUE: set the shield health to SC1PTRAININGRULES_MAX_SHIELD */
    int32_t action;           /* SC1PTrainingRulesShieldAction */
    int32_t button_mask;      /* OOS_INPUT: pressed and held; END_OOS: released */
    int32_t stick_x;
    int32_t stick_y;

} SC1PTrainingRulesShieldOut;

/**
 * One frame of the shield break CPU (Training.asm shield_break_mode_ 2308-2451).
 *
 * @param in Fighter state read by the caller.
 * @param out Result: next status, whether to refill the shield, and the controller change.
 */
extern void sc1PTrainingRulesShieldStep(const SC1PTrainingRulesShieldIn *in, SC1PTrainingRulesShieldOut *out);

/**
 * Button mask and stick of an OOS option (Training.asm oos_inputs_frame_1 and oos_inputs_frame_2).
 *
 * @param option OOS option, clamped to the table.
 * @param is_second_frame FALSE for the first table, TRUE for the second.
 * @param out_stick_x Stick x of the entry.
 * @param out_stick_y Stick y of the entry.
 * @return Button mask of the entry.
 */
extern int32_t sc1PTrainingRulesGetOosInput(int32_t option, int32_t is_second_frame, int32_t *out_stick_x, int32_t *out_stick_y);

/**
 * Spam timer value to load when the timer ran out (Training.asm spam_practice_ _get_manual_interval and random interval). The timer is
 * one byte, so an interval above 255 wraps as it does in the asm.
 *
 * @param is_random_interval Spam Interval Random.
 * @param interval Spam Interval entry, 1 to 999.
 * @param random_below_100 Random number from 0 to 99, used by the random interval.
 */
extern int32_t sc1PTrainingRulesSpamReloadTimer(int32_t is_random_interval, int32_t interval, int32_t random_below_100);

/**
 * Next spam timer value of one fighter for one frame (spam_practice_timer_): only a CPU outside hitlag counts down, and not below zero.
 */
extern int32_t sc1PTrainingRulesSpamTickTimer(int32_t timer, int32_t is_cpu, int32_t is_in_hitlag);

/** "D-pad Controls" entry values (Training.asm entry_dpad_menu): 0 on, 1 reset only, 2 off. */
#define SC1PTRAININGRULES_DPAD_ON 0
#define SC1PTRAININGRULES_DPAD_RESET_ONLY 1
#define SC1PTRAININGRULES_DPAD_OFF 2

/** Whether the D-pad legend lines of the toggles (Toggle Pause, Frame Advance, Model Display; group 0x19) show: only with D-pad Controls on. */
extern int32_t sc1PTrainingRulesIsDpadToggleLegendShown(int32_t dpad_setting);

/** Whether the Quick Reset line of the D-pad legend (group 0x1A) shows: with D-pad Controls on or reset only. */
extern int32_t sc1PTrainingRulesIsDpadResetLegendShown(int32_t dpad_setting);

#endif
