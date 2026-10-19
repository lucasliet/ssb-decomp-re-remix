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

#endif

#endif
