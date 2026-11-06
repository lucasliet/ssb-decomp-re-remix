#ifndef _GMREMIXMENUMUSICRULES_H_
#define _GMREMIXMENUMUSICRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the "itsatrap" easter egg of Menu Music (Toggles.asm play_menu_music_ _sm64_trap_check and Toggles.itsatrap),
 * separate from the audio calls so the headless unit tests (tests/remix) can run them.
 */

/** Counter value while the trap variant of FILESELECT_SM64 is active (Toggles.itsatrap, -1). */
#define GMREMIX_MENU_MUSIC_TRAP_ACTIVE (-1)

/** Presses on FILESELECT_SM64 in a row that switch it to the trap variant. */
#define GMREMIX_MENU_MUSIC_TRAP_PRESSES 4

/**
 * Counter after a press that lands on FILESELECT_SM64: it counts the presses (the one after the trap restarts at 1) and the
 * press that reaches GMREMIX_MENU_MUSIC_TRAP_PRESSES activates the trap.
 *
 * @param count Counter before the press: 0 to 3, or GMREMIX_MENU_MUSIC_TRAP_ACTIVE.
 * @return The counter after the press.
 */
extern int gmRemixMenuMusicRulesCountTrapPress(int count);

/** TRUE when the counter says FILESELECT_SM64 plays its trap variant. */
extern int gmRemixMenuMusicRulesTrapIsActive(int count);

#endif

#endif
