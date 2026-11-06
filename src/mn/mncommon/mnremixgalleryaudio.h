#ifndef _MNREMIXGALLERYAUDIO_H_
#define _MNREMIXGALLERYAUDIO_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Music and sound tables of the Remix Gallery (Gallery.asm bgm_table, bgm_to_gallery_table, fgm_drum_kit), separate from the scene
 * code so the headless unit tests (tests/remix) can run them. BGM ids are the Remix music ids of docs/remix/audio.md.
 */

/** BGM played on a picture while no song of its list was picked (MIDI.id.GALLERY). */
#define MN_REMIX_GALLERY_BGM_GALLERY 281

/** Number of drum kit entries the Z button cycles through (Gallery.asm DRUM_KIT_ENTRY_COUNT). */
#define MN_REMIX_GALLERY_DRUM_KIT_COUNT 18

/**
 * Number of songs the A button cycles through on a picture (the first halfword of each Gallery.asm bgm_ array).
 *
 * @param page Page index, 0 to MN_REMIX_GALLERY_ENTRY_COUNT - 1.
 * @return The song count, or 0 when page is out of range.
 */
extern int mnRemixGalleryAudioGetSongCount(int page);

/**
 * Song of a picture.
 *
 * @param page Page index, 0 to MN_REMIX_GALLERY_ENTRY_COUNT - 1.
 * @param song_number 1 to the song count; the number 0 means "no song picked" and plays MN_REMIX_GALLERY_BGM_GALLERY.
 * @return The BGM id, or -1 when page or song_number is out of range.
 */
extern int mnRemixGalleryAudioGetSong(int page, int song_number);

/**
 * Song number the A button moves to (Gallery.asm _a_press_normal): it goes up by one and wraps from the last song to the first.
 *
 * @param page Page index.
 * @param song_number Current song number, 0 when none was picked.
 * @return The next song number, 1 to the song count.
 */
extern int mnRemixGalleryAudioNextSongNumber(int page, int song_number);

/**
 * Picture that matches a BGM (Gallery.asm bgm_to_gallery_table), used when the idle mode plays a random song.
 *
 * @param bgm_id Remix BGM id.
 * @return The page index; the Remix picture for tracks without a character.
 */
extern int mnRemixGalleryAudioGetPageForBgm(int bgm_id);

/**
 * Sound of the drum kit that L or R play (Gallery.asm fgm_drum_kit).
 *
 * @param kit_index 0 to MN_REMIX_GALLERY_DRUM_KIT_COUNT - 1.
 * @param is_right Non-zero for the R button sound, zero for L.
 * @return The FGM id, or -1 when kit_index is out of range.
 */
extern int mnRemixGalleryAudioGetDrumFgm(int kit_index, int is_right);

/**
 * Drum kit entry the Z button moves to (Gallery.asm _check_z_press): it wraps after the last one.
 *
 * @param kit_index Current entry.
 * @return The next entry.
 */
extern int mnRemixGalleryAudioNextDrumKit(int kit_index);

#endif

#endif
