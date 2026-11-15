#ifndef _GMREMIXMENUMUSICRULES_H_
#define _GMREMIXMENUMUSICRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of Menu Music: the "itsatrap" easter egg (Toggles.asm play_menu_music_ _sm64_trap_check and Toggles.itsatrap),
 * the RANDOM step of BGM.asm alt_menu_music_ and the Play Music resets of Toggles.asm _exit_sub_menu and update_bgm_volume,
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

/** Menu Music value of the FILESELECT_SM64 track (Toggles.asm string_table_menu_music). */
#define GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64 7

/**
 * Counter after an A press on Menu Music (Toggles.asm play_menu_music_ _check_bgm): a raw value other than FILESELECT_SM64,
 * RANDOM and RANDOM CLASSICS included, clears the counter before the random value is rolled, so only direct presses on
 * FILESELECT_SM64 build it up.
 *
 * @param raw_value Menu Music value of the toggle.
 * @param resolved_value Value the press resolved to (the rolled track for RANDOM, RANDOM CLASSICS and DEFAULT).
 * @param count Counter before the press.
 * @return The counter after the press.
 */
extern int gmRemixMenuMusicRulesCountAuditionPress(int raw_value, int resolved_value, int count);

/** What a RANDOM value does on a menu screen (BGM.asm alt_menu_music_ 636-647). */
#define GMREMIX_MENU_MUSIC_RANDOM_KEEP 0
#define GMREMIX_MENU_MUSIC_RANDOM_SCREEN_TRACK 1
#define GMREMIX_MENU_MUSIC_RANDOM_REROLL 2

/** BGM ids BGM.asm tests there: menu.CHARACTER_SELECT, menu.DATA and menu.MAIN. */
#define GMREMIX_MENU_MUSIC_RULES_BGM_CHARACTER_SELECT 10
#define GMREMIX_MENU_MUSIC_RULES_BGM_DATA 43
#define GMREMIX_MENU_MUSIC_RULES_BGM_MAIN 44

/**
 * Picks what a RANDOM, RANDOM CLASSICS or RANDOM ALL value does with the track playing: a 64 track follows the vanilla logic
 * and becomes the screen's own 64 track, the Data theme forces a new roll, any other track keeps playing and silence rolls.
 *
 * @param current_bgm BGM id playing now, -1 when none.
 * @return GMREMIX_MENU_MUSIC_RANDOM_KEEP, GMREMIX_MENU_MUSIC_RANDOM_SCREEN_TRACK or GMREMIX_MENU_MUSIC_RANDOM_REROLL.
 */
extern int gmRemixMenuMusicRulesRandomStep(int current_bgm);

/**
 * Checks whether leaving the settings restarts the menu music (Toggles.asm _exit_sub_menu, Play Music half): it does when
 * Play Music is OFF (the restart then stops the track) or when nothing plays.
 *
 * @param play_music Play Music toggle value.
 * @param current_bgm BGM id playing now, -1 when none.
 * @return TRUE when the menu music is reset.
 */
extern int gmRemixMenuMusicRulesSettingsCloseResets(int play_music, int current_bgm);

/**
 * Checks whether an A press on BGM Volume restarts the menu music (Toggles.asm update_bgm_volume): Play Music ON and nothing
 * playing.
 *
 * @param play_music Play Music toggle value.
 * @param current_bgm BGM id playing now, -1 when none.
 * @return TRUE when the menu music is reset.
 */
extern int gmRemixMenuMusicRulesVolumeResets(int play_music, int current_bgm);

#endif

#endif
