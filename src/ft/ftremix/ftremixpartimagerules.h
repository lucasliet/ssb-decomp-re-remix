#ifndef _FTREMIXPARTIMAGERULES_H_
#define _FTREMIXPARTIMAGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix model part image swaps (Costumes.asm check_for_custom_costume_, select_, get_random_legal_costume_,
 * Shield.asm alter_yoshi_egg_color_), so the headless unit tests (tests/remix) can run them. Only int and unsigned int.
 *
 * A fighter model has costume ids 0 to num_costumes (the vanilla ones), then `skipped` ids that no player can pick (Remix
 * keeps them for models with holes in their costume list), then the extra costumes. The extra arrays of the part tables are
 * indexed by the extra costume index: costume - (num_costumes + skipped + 1).
 */

#define FTREMIX_PARTIMAGE_CHARACTER_COUNT 97
#define FTREMIX_PARTIMAGE_NO_EXTRA (-1)
#define FTREMIX_PARTIMAGE_PART_ENTRY_SIZE 8
#define FTREMIX_PARTIMAGE_IMAGE_INFO_SIZE 0x10
#define FTREMIX_PARTIMAGE_PRIM_COLOR_SIZE 8

#define FTREMIX_PARTIMAGE_SHIELD_VANILLA 16
#define FTREMIX_PARTIMAGE_SHIELD_COSTUME 17
#define FTREMIX_PARTIMAGE_EGG_PALETTE_COUNT 16

/** First costume id that selects an extra costume (Costumes.asm check_for_custom_costume_, t5). */
extern int ftRemixPartImageRulesFirstExtraCostume(int num_costumes, int skipped);

/**
 * Extra costume index of a costume id, or FTREMIX_PARTIMAGE_NO_EXTRA for a vanilla costume, a skipped id, an id past the last
 * extra costume or a character with no extra costumes (extra_count 0).
 */
extern int ftRemixPartImageRulesExtraIndex(int costume, int num_costumes, int skipped, int extra_count);

/** Highest costume id a player can reach (Costumes.asm select_, t4). */
extern int ftRemixPartImageRulesMaxCostume(int num_costumes, int skipped, int extra_count);

/** Costume after a C right press (select_ _right): jumps over the skipped ids and wraps to 0 after the last extra costume. */
extern int ftRemixPartImageRulesNextCostume(int costume, int num_costumes, int skipped, int extra_count);

/** Costume after a C left press (select_ _left): jumps over the skipped ids and wraps to the last extra costume. */
extern int ftRemixPartImageRulesPrevCostume(int costume, int num_costumes, int skipped, int extra_count);

/** TRUE when a Random pick may land on `costume`: not one of the skipped ids (get_random_legal_costume_). */
extern int ftRemixPartImageRulesIsLegalRandomCostume(int costume, int num_costumes, int skipped);

/** TRUE when the images array of the part is the special part one (special part index above 0, else the normal images). */
extern int ftRemixPartImageRulesUsesSpecialArray(int special_part_index);

/** Prim color word of the extra costume: the low poly word when the model is low poly and it is set, else the high poly word. */
extern unsigned int ftRemixPartImageRulesPickPrimColor(unsigned int high_poly, unsigned int low_poly, int is_low_poly);

/** Image index (position in the part's image list) of the MObj of an accessory part: start, or start + 1 for the second MObj. */
extern int ftRemixPartImageRulesAccessoryImageIndex(int start_index, int is_first_mobj);

/**
 * Palette index of a Yoshi egg (Shield.asm alter_yoshi_egg_color_ _set_palette_*): 0 for Vanilla, the Character.costume_shield_color
 * index of the costume for Costume, and Character.yoshi_egg_color.table for every other option, which no Remix file fills (0).
 * An index outside the 16 palettes of the table falls back to 0.
 */
extern int ftRemixPartImageRulesEggPaletteIndex(int shield_option, int costume_color_index);

#endif

#endif
