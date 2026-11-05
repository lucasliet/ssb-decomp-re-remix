#ifndef _FTREMIXSLIPPYHIPSRULES_H_
#define _FTREMIXSLIPPYHIPSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the Slippy hip Y translation tables (Slippy.asm:374 hips_y_translation_, SlippyHipTranslation.asm): one float per
 * action in hip_translation_table (0xF6 entries) and one per menu action (status ids with bit 0x10000, 0x10 entries) in
 * hip_translation_menu_table. Only int, so the headless unit tests (tests/remix) can run it.
 */

#define FTREMIX_SLIPPY_HIP_MENU_FLAG 0x10000
#define FTREMIX_SLIPPY_HIP_TABLE_SIZE 0xF6
#define FTREMIX_SLIPPY_HIP_MENU_TABLE_SIZE 0x10
#define FTREMIX_SLIPPY_HIP_ENTRY_SIZE 4

/**
 * RAM address of the multiplier word of an action.
 *
 * @param status_id Status id of the fighter; menu actions carry FTREMIX_SLIPPY_HIP_MENU_FLAG.
 * @param table_address RAM address of hip_translation_table.
 * @param menu_table_address RAM address of hip_translation_menu_table.
 * @param word_address Receives the address when the action is inside its table.
 * @return 1 when the action has an entry, 0 when its index is past the end of the table (the multiplier is then 1.0).
 */
extern int ftRemixSlippyHipFindWordAddress(unsigned int status_id, unsigned int table_address, unsigned int menu_table_address, unsigned int *word_address);

#endif

#endif
