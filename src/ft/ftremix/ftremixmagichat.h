#ifndef _FTREMIXMAGICHAT_H_
#define _FTREMIXMAGICHAT_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * '???' Magic Kirby Hat (Kirby/Kirby.asm kirby_magic_hat, clear_magic_hat_charge, restore_magic_hat_). A Kirby that holds the
 * magic hat gets a random power each time it uses its neutral special, keeps the charge of each hat that stores one
 * (Donkey Kong, Samus, Dark Samus, Mewtwo, Sheik, Peppy) and puts the Magician hat back when the special ends. The rules
 * behind it are in ftremixmagichatrules.h.
 */

#include <ft/fighter.h>

/**
 * Whether the port of fp holds the magic hat (KirbyHats.asm magic_hat_on). The flag lives in ftremixkirbypower.c.
 *
 * @param fp Kirby, J Kirby or Polygon Kirby struct.
 * @return TRUE while the magic hat is on.
 */
extern sb32 ftRemixKirbyMagicHatIsOn(FTStruct *fp);

/**
 * kirby_magic_hat (RAM 0x80151070 ground, 0x80150EE0 air): call it first in ftKirbySpecialNSetStatusSelect and
 * ftKirbySpecialAirNSetStatusSelect. With the magic hat on it stores the charge of the previous hat, picks a random power
 * (the previous one again when its charge is full), restores the charge saved for the new hat, sets
 * passive_vars.kirby.copy_id, swaps the hat model and sparkles over the head. It does nothing without the magic hat.
 *
 * @param fighter_gobj Kirby GObj.
 */
extern void ftRemixKirbyMagicHatRoll(GObj *fighter_gobj);

/**
 * restore_magic_hat_ (RAM 0x800E6F44): call it from ftMainSetStatus for every status change. A Kirby or J Kirby with the
 * magic hat on gets the Magician hat back when the new status is not a copied special.
 *
 * @param fighter_gobj Fighter GObj.
 * @param fp Fighter struct.
 * @param status_id Status the fighter changes to.
 */
extern void ftRemixKirbyMagicHatRestore(GObj *fighter_gobj, FTStruct *fp, s32 status_id);

/**
 * clear_magic_hat_charge (KirbyHats.asm kirby_hat_select_ 590-593): clears the stored charge of every hat for a port. Call it
 * when a Kirby spawns with the '???' hat value of the CSS Kirby Hat setting.
 *
 * @param player Port.
 */
extern void ftRemixKirbyMagicHatClearCharge(s32 player);

#endif

#endif
