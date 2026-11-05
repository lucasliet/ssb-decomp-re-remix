#ifndef _MNREMIXSTAGERANDOM_H_
#define _MNREMIXSTAGERANDOM_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Stage the RANDOM cell resolves to (Stages.asm swap_stage_ `_get_random_stage_id`): a random pick of the list of the Random
 * Stage Toggles that are on, narrowed to the tournament legal stages on the tournament layout (add_stage_to_random_list_).
 *
 * @param is_tournament TRUE when the screen opened with the tournament layout.
 * @return Stage id, or -1 when the list is empty.
 */
extern s32 mnRemixStageRandomPick(sb32 is_tournament);

/**
 * Stage of a match that starts with Stage Select off (Stages.asm swap_stage_._stage_select_off_begin): the same pick of the
 * Random Stage Toggles list as the RANDOM cell. The layout is read from the toggle because no stage select screen opened,
 * and the tournament layout also forces the hazard mode of the drawn stage.
 *
 * @return Stage id, or -1 when the list is empty.
 */
extern s32 mnRemixStageRandomPickStageSelectOff(void);

#endif

#endif
