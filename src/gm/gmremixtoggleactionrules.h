#ifndef _GMREMIXTOGGLEACTIONRULES_H_
#define _GMREMIXTOGGLEACTIONRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * State-free rules of the A-press routines of the Remix settings (Toggles.asm), kept apart so the headless unit tests (tests/remix)
 * can run them. The port menu runs the same routine when the value of such a setting changes.
 */

/** Highest Punish Failed Z-Cancel value, RANDOM (Toggles.asm entry_punish_on_failed_z_cancel). */
#define GMREMIX_PUNISH_MODE_RANDOM 11

/** Menu Music value of the Blast Corps track, the only one that makes the OFF value play a sound (Toggles.asm punish_fgm_). */
#define GMREMIX_PUNISH_MENU_MUSIC_BLASTCORPS 8

/** Volume the A press of BGM Volume and SFX Volume sets before the setting scales it (Toggles.asm update_bgm_volume). */
#define GMREMIX_VOLUME_ACTION_LEVEL 0x7800

/** Test sound of the A press of SFX Volume (Toggles.asm update_fgm_volume). */
#define GMREMIX_VOLUME_ACTION_TEST_FGM 0x020

/**
 * Sound the A press of Punish Failed Z-Cancel plays (Toggles.asm punish_fgm_).
 *
 * @param punish_mode Punish Failed Z-Cancel value, 0 (OFF) to GMREMIX_PUNISH_MODE_RANDOM.
 * @param menu_music Menu Music value.
 * @return The FGM id, or 0 when the press is silent.
 */
extern int gmRemixToggleActionGetPunishFgm(int punish_mode, int menu_music);

/** Range of the DEFAULT roll of the Menu Music A press: 85 values for the 64 track plus one each for MELEE, MENU 2 and BRAWL (Toggles.asm _random_menu_song_smash). */
#define GMREMIX_AUDITION_DEFAULT_ROLL_RANGE 88

/** Rolls below this value keep the 64 track in the DEFAULT roll of the Menu Music A press (BGM.alt_menu_music_.CHANCE_64). */
#define GMREMIX_AUDITION_CHANCE_64 85

/** Menu Music value of the 64 track (Toggles.asm string_table_menu_music). */
#define GMREMIX_AUDITION_VALUE_64 1

/** Menu Music value of MELEE, the first of the three Smash tracks the DEFAULT roll can pick besides the 64 track. */
#define GMREMIX_AUDITION_VALUE_MELEE 2

/** Nothing remembered: no Random Music preview has played since the port menu opened. */
#define GMREMIX_PREVIEW_IDLE (-2)

/** What closing the port menu does after Random Music previews. */
#define GMREMIX_PREVIEW_RESTORE_NOTHING 0
#define GMREMIX_PREVIEW_RESTORE_PLAY 1
#define GMREMIX_PREVIEW_RESTORE_STOP 2

/**
 * Menu Music value the DEFAULT value plays on an A press (Toggles.asm play_menu_music_._random_menu_song_smash).
 *
 * @param roll Random number from 0 to GMREMIX_AUDITION_DEFAULT_ROLL_RANGE - 1.
 * @return GMREMIX_AUDITION_VALUE_64 for rolls below GMREMIX_AUDITION_CHANCE_64, otherwise 2 (MELEE), 3 (MENU 2) or 4 (BRAWL).
 */
extern int gmRemixToggleActionGetAuditionDefaultValue(int roll);

/**
 * Value the toggle-all rows write to every toggle after them (Toggles.asm toggle_all_): the opposite of the first one.
 *
 * @param first_value Value of the first toggle after the title, 0 or 1.
 * @return The value for all of them.
 */
extern int gmRemixToggleActionGetToggleAllValue(int first_value);

/**
 * BGM id to put back when the port menu closes after Random Music previews. The asm sets reset_menu_music and restarts the menu
 * music on leaving the settings screen; the port menu also opens during a match, so it puts back the track playing before the
 * first preview.
 *
 * @param remembered_bgm Track remembered by an earlier preview, or GMREMIX_PREVIEW_IDLE.
 * @param playing_bgm BGM id playing now, -1 when none.
 * @return The track to remember: the one already remembered, else the one playing now.
 */
extern int gmRemixToggleActionGetPreviewRemembered(int remembered_bgm, int playing_bgm);

/**
 * Step that closing the port menu takes after Random Music previews.
 *
 * @param remembered_bgm Track remembered by the previews, -1 when none was playing, or GMREMIX_PREVIEW_IDLE.
 * @return GMREMIX_PREVIEW_RESTORE_NOTHING when nothing was previewed, _PLAY when a track was playing before, _STOP when none was.
 */
extern int gmRemixToggleActionGetPreviewRestoreStep(int remembered_bgm);

#endif

#endif
