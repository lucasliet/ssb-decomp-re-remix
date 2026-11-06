#ifndef _MNREMIXGALLERYRULES_H_
#define _MNREMIXGALLERYRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix Gallery (Gallery.asm id_table) and of the Remix victory pictures (SinglePlayer.asm
 * custom_victory_file_table), separate from the scene code so the headless unit tests (tests/remix) can run them.
 */

/** Number of pictures of the Gallery (Gallery.asm id_table.SIZE). */
#define MN_REMIX_GALLERY_ENTRY_COUNT 40

/** Character.id values below this one use the vanilla Congratulations picture table (replace_victory_image_ sltiu 0x000C). */
#define MN_REMIX_GALLERY_VANILLA_CHARACTER_COUNT 12

/** Offset inside both halves of a custom victory picture (SinglePlayer.asm VICTORY_OFFSET_1). */
#define MN_REMIX_VICTORY_IMAGE_OFFSET 0x20718

/** Bottom file of the Remix splash image shown after the N64 logo, Character.id NONE (File.SPLASH_IMAGE_BOTTOM). */
#define MN_REMIX_SPLASH_IMAGE_FILE 0x95E

/** Bottom file of the picture of a character without its own (File.SINGLEPLAYER_VICTORY_IMAGE_BOTTOM). */
#define MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE 0x92D

/** Frames of the fade between two pictures (Gallery.asm FADE_LENGTH_NORMAL, FADE_LENGTH_QUICK, FADE_LENGTH_IDLE). */
#define MN_REMIX_GALLERY_FADE_NORMAL 45
#define MN_REMIX_GALLERY_FADE_QUICK 4
#define MN_REMIX_GALLERY_FADE_IDLE 100

/** Frames the idle mode shows a picture before the next one (Gallery.asm IDLE_TIME, 150 seconds). */
#define MN_REMIX_GALLERY_IDLE_FRAMES (150 * 60)

/** Idle modes (Gallery.asm status.idle): off, pictures in a shuffled order, random songs with their pictures. */
#define MN_REMIX_GALLERY_IDLE_OFF 0
#define MN_REMIX_GALLERY_IDLE_SHUFFLE 1
#define MN_REMIX_GALLERY_IDLE_RANDOM_SONG 2

/**
 * Character.id shown by a Gallery page (Gallery.asm id_table).
 *
 * @param index Page index, 0 to MN_REMIX_GALLERY_ENTRY_COUNT - 1.
 * @return The Character.id, or -1 when index is out of range.
 */
extern int mnRemixGalleryRulesGetCharacterId(int index);

/**
 * Page reached by moving through the Gallery (Gallery.asm _cycle_image): it wraps at both ends.
 *
 * @param index Current page index.
 * @param step Pages to move, negative to go back.
 * @return The new page index.
 */
extern int mnRemixGalleryRulesStepIndex(int index, int step);

/**
 * Idle mode that START moves to (Gallery.asm _check_start_press): off and the random song mode go to the shuffled pictures, the
 * shuffled pictures go to the random song mode.
 *
 * @param idle_mode Current idle mode.
 * @return The new idle mode.
 */
extern int mnRemixGalleryRulesNextIdleMode(int idle_mode);

/**
 * Position of the next picture in the shuffled idle order (Gallery.asm initial_ _idle_mode_1): it wraps after the last page.
 *
 * @param idle_index Current position, -1 right after START.
 * @return The next position, 0 to MN_REMIX_GALLERY_ENTRY_COUNT - 1.
 */
extern int mnRemixGalleryRulesNextIdleIndex(int idle_index);

/**
 * Fills the idle order with every page once in a random order (Gallery.asm populate_idle_table_): each page takes a random free
 * position.
 *
 * @param order Output, MN_REMIX_GALLERY_ENTRY_COUNT entries.
 * @param random_range Returns a random number from 0 to range - 1.
 */
extern void mnRemixGalleryRulesBuildIdleOrder(int *order, int (*random_range)(int range));

/**
 * Bottom file of the victory picture of a Remix character (SinglePlayer.asm replace_victory_image_ _custom and the
 * set_ending_image calls); the top file is the next one and both use MN_REMIX_VICTORY_IMAGE_OFFSET.
 *
 * @param character_id Character.id, MN_REMIX_GALLERY_VANILLA_CHARACTER_COUNT or above.
 * @return The bottom file id, or 0 when character_id uses the vanilla picture table.
 */
extern int mnRemixGalleryRulesGetVictoryFileId(int character_id);

#endif

#endif
