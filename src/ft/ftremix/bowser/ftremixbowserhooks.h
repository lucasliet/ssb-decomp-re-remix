#ifndef _FTREMIXBOWSERHOOKS_H_
#define _FTREMIXBOWSERHOOKS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Bowser hooks of Smash Remix that vanilla code calls (REGION_REMIX only): the fire breath ammo of Bowser and of Kirby
 * with the Bowser power, and the bottom blast zone of Giga Bowser in the Remix 1P mode.
 */

#include <ft/fighter.h>

/**
 * SinglePlayerModes.asm limbo_clear (0x800D7A4C), Bowser half: Bowser spawns with full fire breath ammo. The Kirby half of
 * the asm writes an ammo that kirby_hat_select_ clears right after (`sw r0, 0x0AE0`), so Kirby with the Bowser power
 * starts empty and recharges.
 *
 * @param fp Fighter being initialised.
 */
void ftRemixBowserAmmoInitFighter(FTStruct *fp);

/**
 * jigglypuffkirbyshared.asm kirby_blast_fix_1 (0x800D79B0): a KO'd Bowser has full fire breath ammo again.
 *
 * @param fp Fighter whose weapons are destroyed.
 */
void ftRemixBowserAmmoRefillOnKO(FTStruct *fp);

/**
 * bowser.asm bowser_nsp_recharge (0x800E23E0), called once per frame after the map proc: Bowser and Kirby with the Bowser
 * power regain 2 ammo every 30 frames while not breathing fire.
 *
 * @param fp Fighter being updated.
 */
void ftRemixBowserAmmoRecharge(FTStruct *fp);

/**
 * gbowser.asm _remix_1p_prevent_bottom_blast_zone_KO (0x8013CD64): the CPU Giga Bowser of the Remix 1P mode is not KO'd at
 * the bottom team blast zone; below -6500 while falling he is thrown back up and toward the stage center.
 *
 * @param fighter_gobj Fighter below the bottom team blast zone.
 * @param pos Top joint translation of the fighter.
 * @return TRUE when the KO must be skipped.
 */
sb32 ftRemixGBowserSavedFromBottomBlastZone(GObj *fighter_gobj, Vec3f *pos);

#endif

#endif
