#ifndef _SC1PTRAININGLEFTOVERRULES_H_
#define _SC1PTRAININGLEFTOVERRULES_H_

#include <stdint.h>

/*
 * Pure rules of the Remix Training leftovers (Training.asm update_frame_counts_ 2097-2152, update_actions_ 2156-2277, hold_to_exit_
 * 751-809, run_ hold bar 2020-2038, Item.asm extend_training_mode_hud_ 311-365). Ints only, compiled standalone by tests/remix. The
 * scene code reads the fighters and the controllers; these functions only decide.
 */

/** Action ids below this value use the shared action string table; from it on the character table (update_actions_ 2184, sltiu 0xDC). */
#define SC1PTRAININGLEFTOVERRULES_SHARED_ACTION_COUNT 0xDC

/** hold_to_exit_ NUM_FRAMES: 32 frames fill the bar and one more frame exits. */
#define SC1PTRAININGLEFTOVERRULES_HOLD_EXIT_FRAMES 33

/** run_ 2030-2036: the blue bar grows 2 per held frame up to 64 wide. */
#define SC1PTRAININGLEFTOVERRULES_HOLD_BAR_STEP 2
#define SC1PTRAININGLEFTOVERRULES_HOLD_BAR_MAX_WIDTH 64

/**
 * Frame counter of one fighter after one more game frame (update_frame_counts_). It counts frames outside hitlag and the first frame
 * of a hitlag, and restarts at 1 when the action changed.
 *
 * @param prev_action Action id seen on the previous frame.
 * @param action Action id of the fighter now.
 * @param prev_hitlag Hitlag tics seen on the previous frame.
 * @param hitlag Hitlag tics of the fighter now.
 * @param frame_count Counter before this frame.
 */
extern int32_t sc1PTrainingLeftoverRulesNextFrameCount(int32_t prev_action, int32_t action, int32_t prev_hitlag, int32_t hitlag,
                                                       int32_t frame_count);

/** Whether the action name comes from the shared table (action id below 0xDC) and not from the character table. */
extern int32_t sc1PTrainingLeftoverRulesIsSharedAction(int32_t action);

/** Index of an action in the character action string table (action id minus 0xDC). */
extern int32_t sc1PTrainingLeftoverRulesGetUniqueIndex(int32_t action);

/**
 * One frame of Hold to Exit (hold_to_exit_): the exit asks A held for 33 frames in a row.
 *
 * @param held_frames Frames A has been held so far.
 * @param is_a_held A is held on this frame.
 * @param next_held_frames Receives the count for the next frame: 0 when A is released or when the exit fires.
 * @return 1 when the exit must run on this frame.
 */
extern int32_t sc1PTrainingLeftoverRulesHoldExitStep(int32_t held_frames, int32_t is_a_held, int32_t *next_held_frames);

/**
 * Width of the Hold to Exit bar after one frame (run_ _check_A_held): it grows while A is held, stops at the maximum and is empty
 * once A is released.
 */
extern int32_t sc1PTrainingLeftoverRulesGetHoldBarWidth(int32_t width, int32_t is_a_held);

/**
 * Entry of the Remix item name table for a held item (Item.asm _fix_item_id_check and _get_custom_image): the Remix standard items
 * are the kinds first_kind to first_kind + count - 1; every other kind has no Remix name.
 *
 * @return Index in the table, or -1 when the held item is not one of the Remix standard items.
 */
extern int32_t sc1PTrainingLeftoverRulesGetHeldItemEntry(int32_t kind, int32_t first_kind, int32_t count);

#endif
