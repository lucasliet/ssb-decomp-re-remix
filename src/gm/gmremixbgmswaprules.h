#ifndef _GMREMIXBGMSWAPRULES_H_
#define _GMREMIXBGMSWAPRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the two mid-match song swaps of the Remix: the Final Destination intro that gives way to the main song
 * (FD.asm music_fix_) and the 30-second Hurry Up song (BGM.asm thirty_second_music_swap). Separate from the engine so the
 * headless unit tests (tests/remix) can run them.
 */

/* Stages.id FINAL_DESTINATION, FINAL_DESTINATION_DL and FINAL_DESTINATION_TENT. */
#define GMREMIX_BGM_SWAP_STAGE_FD 0x10
#define GMREMIX_BGM_SWAP_STAGE_FD_DL 0x8F
#define GMREMIX_BGM_SWAP_STAGE_FD_TENT 0x90

/* FD.asm music_fix_: updates the intro plays before the main song starts (0x348). */
#define GMREMIX_BGM_SWAP_FD_INTRO_UPDATES 840

/* 1P Final Stage: SinglePlayerModes.STAGE_FLAG value, and the modes (singleplayer_mode_flag) that run it. */
#define GMREMIX_BGM_SWAP_1P_FINAL_STAGE 13
#define GMREMIX_BGM_SWAP_1P_MODE_STANDARD 0
#define GMREMIX_BGM_SWAP_1P_MODE_REMIX 4

/* BGM ids of the Hurry Up swap. MIDI.id.SMB2OVERWORLD, UNDERGROUND and UNDERGROUND_HURRY are the Remix ids of docs/remix/audio.md. */
#define GMREMIX_BGM_SWAP_MUSHROOM_KINGDOM 2
#define GMREMIX_BGM_SWAP_MUSHROOM_KINGDOM_FAST 3
#define GMREMIX_BGM_SWAP_SMB2_OVERWORLD 179
#define GMREMIX_BGM_SWAP_SMB2_BOSS 292
#define GMREMIX_BGM_SWAP_UNDERGROUND 315
#define GMREMIX_BGM_SWAP_UNDERGROUND_HURRY 316

/** TRUE for the three Final Destination stages (VS), whose file song is the Master Hand intro. */
extern int gmRemixBgmSwapRulesIsFinalDestination(int gkind);

/**
 * TRUE for the 1P Final Stage of the standard run and of Remix 1P (music_fix_ _final_destination path of the 1P game).
 *
 * @param mode Remix 1P mode (SC1PREMIX_MODE_*).
 * @param stage 1P progress (spgame_stage).
 */
extern int gmRemixBgmSwapRulesIs1PFinalStage(int mode, int stage);

/**
 * TRUE on the update the Final Destination intro ends: exactly 840 updates after the song started, and the stage song is
 * still the intro.
 *
 * @param update_count Updates since the scene started (dSYTaskmanUpdateCount).
 * @param timer_start Update count when the last song started.
 * @param stage_track Stage (default) song.
 */
extern int gmRemixBgmSwapRulesFdIntroEnds(unsigned int update_count, unsigned int timer_start, int stage_track);

/**
 * Hurry Up song for the song playing (thirty_second_music_swap): Mushroom Kingdom gives the Fast song, SMB2 Overworld the SMB2
 * Boss, Underground the Underground Hurry, on any stage. Nothing swaps when the stage song already is a Hurry Up song.
 *
 * @param current_track Song playing now (the item music while a Hammer or Star plays).
 * @param stage_track Stage (default) song.
 * @return The Hurry Up track, or GMREMIX_BGM_NO_TRACK when nothing swaps.
 */
extern int gmRemixBgmSwapRulesHurryTrack(int current_track, int stage_track);

#endif

#endif
