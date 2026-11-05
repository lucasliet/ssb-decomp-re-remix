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

/** Bottom file of the picture of a character without its own (File.SINGLEPLAYER_VICTORY_IMAGE_BOTTOM). */
#define MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE 0x92D

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
 * Bottom file of the victory picture of a Remix character (SinglePlayer.asm replace_victory_image_ _custom and the
 * set_ending_image calls); the top file is the next one and both use MN_REMIX_VICTORY_IMAGE_OFFSET.
 *
 * @param character_id Character.id, MN_REMIX_GALLERY_VANILLA_CHARACTER_COUNT or above.
 * @return The bottom file id, or 0 when character_id uses the vanilla picture table.
 */
extern int mnRemixGalleryRulesGetVictoryFileId(int character_id);

#endif

#endif
