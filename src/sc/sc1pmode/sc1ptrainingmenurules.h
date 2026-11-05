#ifndef _SC1PTRAININGMENURULES_H_
#define _SC1PTRAININGMENURULES_H_

#include <stdint.h>

/*
 * Pure navigation rules of the Remix Training Z menu (Menu.asm update_ 772-1380, get_num_entries_, Joypad.asm check_stick_ 239-282).
 * Ints only, compiled standalone by tests/remix. The scene code reads the controllers and draws; these functions only decide where
 * the selection goes and what a value becomes.
 */

/* Training.asm info: max_per_page 12, row height trunc(14 * 0.921875) = 12, menu at (68, 50). */
#define SC1PTRAININGMENURULES_ROWS_PER_PAGE 12
#define SC1PTRAININGMENURULES_ROW_HEIGHT 12

/* Joypad.DEADZONE and the check_stick_ repeat: one step on the first frame, then one every 4 frames from the 28th. */
#define SC1PTRAININGMENURULES_STICK_DEADZONE 30
#define SC1PTRAININGMENURULES_STICK_REPEAT_DELAY 28
#define SC1PTRAININGMENURULES_STICK_REPEAT_INTERVAL 4

/** Direction argument of SC1PTrainingMenuRulesStepValue. */
#define SC1PTRAININGMENURULES_STEP_LEFT (-1)
#define SC1PTRAININGMENURULES_STEP_RIGHT 1

/**
 * Joypad.check_stick_: whether a stick direction held for `frames_held` frames counts as a press this frame.
 *
 * @param frames_held Frames the direction has been held, 0 when it is not.
 * @return 1 on the first frame and on every 4th frame from the 28th on, 0 otherwise.
 */
extern int32_t sc1PTrainingMenuRulesIsStickStep(int32_t frames_held);

/** Page (0 based) a selection is on. */
extern int32_t sc1PTrainingMenuRulesGetPage(int32_t selection);

/** Row (0 based) of a selection on its page. */
extern int32_t sc1PTrainingMenuRulesGetRow(int32_t selection);

/**
 * Down: next entry of the page, the first row after the last one; the last entry of the whole list goes to the top of its own page
 * (the first entry when the list is a single page).
 *
 * @param selection Current selection.
 * @param entry_count Entries in the menu.
 */
extern int32_t sc1PTrainingMenuRulesMoveDown(int32_t selection, int32_t entry_count);

/** Up: previous entry of the page; the first row goes to the last row of its page, or the last entry of the list on a short page. */
extern int32_t sc1PTrainingMenuRulesMoveUp(int32_t selection, int32_t entry_count);

/** R: the same row on the next page (the first page after the last), kept inside the list. */
extern int32_t sc1PTrainingMenuRulesNextPage(int32_t selection, int32_t entry_count);

/** Z: the same row on the previous page (the last page before the first), kept inside the list. A single page keeps the selection. */
extern int32_t sc1PTrainingMenuRulesPrevPage(int32_t selection, int32_t entry_count);

/**
 * Left or right on an entry: one step inside [min, max] that wraps around.
 *
 * @param direction SC1PTRAININGMENURULES_STEP_LEFT or SC1PTRAININGMENURULES_STEP_RIGHT.
 */
extern int32_t sc1PTrainingMenuRulesStepValue(int32_t value, int32_t min, int32_t max, int32_t direction);

/** Costume entry after the character changed: the value, lowered to `max` when the new character has fewer costumes. */
extern int32_t sc1PTrainingMenuRulesClampValue(int32_t value, int32_t max);

#endif
