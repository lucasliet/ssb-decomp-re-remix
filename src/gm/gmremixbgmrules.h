#ifndef _GMREMIXBGMRULES_H_
#define _GMREMIXBGMRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure BGM rules of the Remix (BGM.asm, Pause.asm set_alternate_music_, Hazards.asm time_twister_*), separate from the
 * audio calls so the headless unit tests (tests/remix) can run them.
 */

/** BGM ids of the item music that the pause track change and the Time Twister swap never replace (BGM.special). */
#define GMREMIX_BGM_HAMMER 45
#define GMREMIX_BGM_INVINCIBLE 46

/** Id of the Time Twister song (MIDI.id.CRASH3); the BGM change only runs on a stage that plays it. */
#define GMREMIX_BGM_TIME_TWISTER 317

/** An entry of the alternate music table with no track (Stages.alternate_music_table uses 0xFFFF). */
#define GMREMIX_BGM_NO_TRACK (-1)

#define GMREMIX_BGM_ALT_SLOTS 3

/** TRUE for the Hammer and Star item music, which a track change leaves alone. */
extern int gmRemixBgmRulesIsProtected(int bgm_id);

/**
 * Track the D-pad Right of the pause plays next (Pause.asm set_alternate_music_ _stage_track): the default track steps to
 * the occasional, then rare, then rare 2 track of the stage, skipping empty slots, and the last one (or any other track,
 * for example a random pick) returns to the default.
 *
 * @param current_track Track the match plays now.
 * @param default_track Default track of the stage (before the alternate roll).
 * @param alternates Occasional, rare and rare 2 tracks, GMREMIX_BGM_NO_TRACK for an empty slot.
 * @return The track to play.
 */
extern int gmRemixBgmRulesNextAlternate(int current_track, int default_track, const int *alternates);

/** TRUE when the Time Twister swaps the song with the background (the stage plays the Time Twister song). */
extern int gmRemixBgmRulesTimeTwisterChanges(int stage_track);

/**
 * Song the Time Twister starts for a background (Hazards.asm time_twister_bgms: N. Sanity Beach, Snow Go, Future Frenzy,
 * Time Twister).
 *
 * @return The BGM id, or GMREMIX_BGM_NO_TRACK when bg_index is outside 0..3.
 */
extern int gmRemixBgmRulesTimeTwisterTrack(int bg_index);

#endif

#endif
