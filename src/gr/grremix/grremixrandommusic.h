#ifndef _GRREMIXRANDOMMUSIC_H_
#define _GRREMIXRANDOMMUSIC_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Replaces the stage BGM with a random track of the Random Music list (BGM.asm random_music_). The list holds the
 * stages and menu tracks whose Random Music toggle is on. Preserves the last pick on a salty runback when Salty Runback
 * Preserves Song is on.
 *
 * @param default_bgm BGM id the stage would play without Random Music.
 * @param salty_runback TRUE when the match is a salty runback (GameEnd.is_salty_runback).
 * @return The BGM id to play; default_bgm when Random Music is off or no track is on.
 */
u32 grRemixRandomMusicPick(u32 default_bgm, sb32 salty_runback);

/**
 * Random track of the list for the pause D-pad Down (BGM.asm random_music_ with the D-pad bypass): ignores the Random Music
 * toggle and does not touch the salty runback pick.
 *
 * @param default_bgm BGM id returned when no track is on.
 * @return The BGM id to play.
 */
u32 grRemixRandomMusicPickForced(u32 default_bgm);

#endif

#endif
