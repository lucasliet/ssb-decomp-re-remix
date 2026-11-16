#ifndef _SC1PTRAININGFRAMERULES_H_
#define _SC1PTRAININGFRAMERULES_H_

#include <stdint.h>

/*
 * Pure rules of the Training D-pad shortcuts and of the frame they leave (Training.asm advance_frame_ 1043-1300). Ints only,
 * compiled standalone by tests/remix. The scene code reads the controllers and runs the frame; these functions only decide.
 */

/** D-pad Controls entry values (Training.asm entry_dpad_menu). */
#define SC1PTRAININGFRAMERULES_DPAD_ON 0
#define SC1PTRAININGFRAMERULES_DPAD_RESET_ONLY 1
#define SC1PTRAININGFRAMERULES_DPAD_OFF 2

/** Actions sc1PTrainingFrameRulesReadDpad asks the scene for. */
#define SC1PTRAININGFRAMERULES_ACTION_RESET 0x1
#define SC1PTRAININGFRAMERULES_ACTION_MODEL_DISPLAY 0x2

/** What the scene does with the frame (the three return addresses of advance_frame_, plus the forced advance). */
typedef enum SC1PTrainingFrameKind
{
    nSC1PTrainingFrameKindRun,     /* the frame runs after Speed.handle_rate_training (0x8018DA48) */
    nSC1PTrainingFrameKindAdvance, /* D-pad right: exactly one frame runs, the Game Speed toggle is not asked */
    nSC1PTrainingFrameKindSkip,    /* lag tic of the vanilla Speed option: only the camera runs (0x8018DA58) */
    nSC1PTrainingFrameKindFrozen   /* frozen: neither the objects nor the camera run (0x8018DA50) */

} SC1PTrainingFrameKind;

/** advance_frame_ freeze, du_pressed and dr_pressed. The two presses stay latched until a frame that is not a lag tic uses them. */
typedef struct SC1PTrainingFrameState
{
    int32_t is_frozen;
    int32_t is_up_latched;
    int32_t is_right_latched;

} SC1PTrainingFrameState;

/** Not frozen, nothing latched. */
extern void sc1PTrainingFrameRulesInit(SC1PTrainingFrameState *state);

/**
 * Reads the D-pad of one frame (_check_dl to _update_model_display). Left asks for the quick reset unless the option is Off and
 * ends the read. Up, right and down are only read with the option On and no press latched; any other option clears the freeze.
 * Right wins over up and down pressed on the same frame.
 *
 * @param state Freeze and latches, updated.
 * @param dpad_option D-pad Controls value, SC1PTRAININGFRAMERULES_DPAD_*.
 * @param is_left_tap D-pad left pressed this frame on any controller.
 * @param is_right_repeat D-pad right pressed or repeating (Joypad.TURBO) on any controller.
 * @param is_up_tap D-pad up pressed this frame on any controller.
 * @param is_down_tap D-pad down pressed this frame on any controller.
 * @return SC1PTRAININGFRAMERULES_ACTION_* bits.
 */
extern int32_t sc1PTrainingFrameRulesReadDpad(SC1PTrainingFrameState *state, int32_t dpad_option, int32_t is_left_tap,
                                              int32_t is_right_repeat, int32_t is_up_tap, int32_t is_down_tap);

/**
 * Decides the frame (_skip_input to _end). A lag tic keeps the latches for the next frame. Otherwise a latched up flips the
 * freeze, a latched right freezes and forces one frame, and both latches are cleared.
 *
 * @param state Freeze and latches, updated.
 * @param is_lag_skip sc1PTrainingModeCheckLagTic of the frame.
 */
extern SC1PTrainingFrameKind sc1PTrainingFrameRulesResolve(SC1PTrainingFrameState *state, int32_t is_lag_skip);

#endif
