#ifndef _SC1PREMIXALLSTARRESTRULES_H_
#define _SC1PREMIXALLSTARRESTRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Layout of the series icons of the All-Star Rest Area (SinglePlayerModes.asm rest_area_setup, _loop_icons), kept apart so the
 * headless unit tests (tests/remix) run it. One small icon per fighter the run still has to face, ten to a row.
 */

#define SC1PREMIX_ALLSTAR_REST_ICONS_PER_ROW 10

/**
 * Number of icons the Rest Area shows: the fighters from the next one to the last of the order.
 *
 * @param fighter_count Fighters of the order.
 * @param progress Fighters already faced.
 * @return The icon count, 0 when the run is over.
 */
extern int sc1PRemixAllstarRestIconCount(int fighter_count, int progress);

/**
 * Screen position of an icon: 9 units apart in a row starting at x 30, rows 11 units apart starting at y 30, or y 180 with the Dragon
 * King HUD setting "always" (1).
 *
 * @param index Icon number, 0 for the next fighter.
 * @param dragon_king_hud Toggles.entry_dragon_king_hud.
 * @param x Receives the left edge.
 * @param y Receives the top edge.
 */
extern void sc1PRemixAllstarRestIconPosition(int index, int dragon_king_hud, float *x, float *y);

#endif

#endif
