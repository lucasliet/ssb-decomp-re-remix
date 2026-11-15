#ifndef _GMREMIXMENUMUSIC_H_
#define _GMREMIXMENUMUSIC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Starts the menu track of the Menu Music toggle (BGM.asm alt_menu_music_) in place of the vanilla menu BGM.
 * Menu Music OFF stops the BGM; RANDOM, RANDOM CLASSICS and the track values pick from the menu list; DEFAULT keeps
 * default_bgm, with the title screen chance roll (85 percent default, 5 percent each of MELEE, MENU 2 and BRAWL).
 *
 * @param default_bgm BGM id the vanilla screen plays (nSYAudioBGMModeSelect or nSYAudioBGMBattleSelect).
 * @param from_title_screen TRUE only on the title screen call, the only one that rolls the DEFAULT chance.
 */
void gmRemixMenuMusicPlay(u32 default_bgm, sb32 from_title_screen);

/**
 * Stops the BGM when a menu screen is left, except for the menu tracks and the RANDOM values (BGM.asm _prevent_stop).
 * Call it in place of syAudioStopBGMAll on the CSS back paths.
 */
void gmRemixMenuMusicStop(void);

/**
 * A press on Menu Music (Toggles.asm play_menu_music_): plays the chosen menu track now and always restarts it; OFF stops the
 * music. The port menu calls it after the value changes. Four presses in a row that land on FILESELECT_SM64 switch it to
 * MAIN_TRAP_SM64 (Toggles.itsatrap), which the menu screens then play until a press lands on another track.
 */
void gmRemixMenuMusicAudition(void);

/**
 * Leaving the Remix settings (Toggles.asm _exit_sub_menu, Play Music half): on a menu screen, Play Music OFF stops the menu
 * track and Play Music ON with nothing playing restarts it, as play_menu_music_ with v0 = 0 does. The port menu calls it when
 * it closes, after the Random Music preview is restored.
 */
void gmRemixMenuMusicSettingsClosed(void);

/**
 * An A press on BGM Volume (Toggles.asm update_bgm_volume): on a menu screen with Play Music ON and nothing playing, restarts
 * the menu music.
 */
void gmRemixMenuMusicVolumeApplied(void);

#endif

#endif
