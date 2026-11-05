#ifndef _MNREMIXCSSHIGHSCORES_H_
#define _MNREMIXCSSHIGHSCORES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * 1P record switch of the character selects (CharacterSelectDebugMenu.asm check_high_scores_enabled, CharacterSelect.asm
 * set_high_scores_disabled_visibility_): the panel options, the Gameplay and Pokemon toggles and the Single Player Enemy option decide
 * whether the records of the 1P menu modes are kept. The result goes to the switch the record writers read (sc1premixhighscores.h)
 * and, while it is off, the select shows "High Scores Disabled".
 */

/**
 * Recomputes the switch and the "High Scores Disabled" text; call every frame on the 1P, Bonus and All-Star selects.
 *
 * @param mode MNRemixPanelMenuMode of the screen.
 * @param item_values Value of each panel item (MNREMIX_PANELMENU_ITEM_COUNT entries) for the human port.
 * @return TRUE while the high scores stay on.
 */
extern sb32 mnRemixCssHighScoresUpdate(s32 mode, const s32 *item_values);

#endif

#endif
