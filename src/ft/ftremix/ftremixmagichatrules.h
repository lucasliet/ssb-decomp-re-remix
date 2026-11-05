#ifndef _FTREMIXMAGICHATRULES_H_
#define _FTREMIXMAGICHATRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the '???' Magic Kirby Hat (Kirby/Kirby.asm kirby_magic_hat, restore_magic_hat_, magic_kirby_hat_charge and
 * KirbyHats.asm spawn_with_table_). Only int, so the headless unit tests (tests/remix) can run them.
 */

/** KirbyHats.asm total_hats: the roll covers the table entries 1..41. */
#define FTREMIXMAGICHAT_ROLLS 41

/** Rows of the per-hat stored charge table (magic_kirby_hat_charge). */
#define FTREMIXMAGICHAT_CHARGE_SLOTS 6

/** Hat id of the Magician hat (restore_magic_hat_ t9 = 0x25). */
#define FTREMIXMAGICHAT_MAGICIAN_HAT 0x25

/**
 * restore_magic_hat_ (0x800E6F44): whether a status change puts the Magician hat back on a Kirby with the magic hat on.
 * Every status outside the copied specials does. Inside the vanilla copied special statuses only the Staring ones (0xFA,
 * 0xFB) and the Final Cutter ones (0x100-0x103) do. The Remix copied statuses (0x12F and up) never do.
 *
 * @param status_id Status Kirby changes to.
 * @return 1 to restore the Magician hat.
 */
extern int ftRemixMagicHatShouldRestore(int status_id);

/**
 * magic_kirby_hat_charge: row of the stored charge table of a hat.
 *
 * @param fkind Remix fighter id the hat copies.
 * @return Row 0..5 for Donkey Kong, Samus, Dark Samus, Mewtwo, Sheik and Peppy, -1 for the rest.
 */
extern int ftRemixMagicHatChargeSlot(int fkind);

/**
 * magic_kirby_hat_charge: charge value that counts as full for a row.
 *
 * @param slot Row from ftRemixMagicHatChargeSlot.
 * @return 10, 7, 7, 7, 6 or 5 by row order; 0 for an invalid row.
 */
extern int ftRemixMagicHatChargeMax(int slot);

/**
 * kirby_magic_hat: N64 offset of the word that holds the charge of a hat: 0x0AE8 for Donkey Kong, 0x0AE0 for the others.
 *
 * @param fkind Remix fighter id the hat copies.
 * @return FTStruct offset.
 */
extern int ftRemixMagicHatAmmoOffset(int fkind);

/**
 * KirbyHats.asm spawn_with_table_: Remix fighter id behind a roll.
 *
 * @param roll Table index 0..42 (the rolls are 1..FTREMIXMAGICHAT_ROLLS).
 * @return Remix fighter id; REMIX_ID_NONE for the magic entry (the roll must be repeated), REMIX_ID_KIRBY for an index out of
 * the table.
 */
extern int ftRemixMagicHatFighterFromRoll(int roll);

#endif

#endif
