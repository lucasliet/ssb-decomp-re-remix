#ifndef _FTREMIXBOWSERAMMORULES_H_
#define _FTREMIXBOWSERAMMORULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the fire breath ammo of Bowser, and of Kirby with the Bowser power (src/Bowser/bowser.asm bowser_nsp_recharge
 * 0x800E23E0, src/SinglePlayerModes.asm limbo_clear 0x800D7A4C, src/jigglypuffkirbyshared.asm kirby_blast_fix_1 0x800D79B0),
 * so the headless unit tests (tests/remix) can run them without game state.
 *
 * Both fighters keep a 32 bit scratch word (N64 FTStruct 0x0ADC for Bowser, 0x0AE0 for Kirby): the upper halfword is the
 * recharge timer and the lower halfword is the ammo, the one the fire breath reads. */

#define FTREMIX_BOWSER_AMMO_MAX 0x14
#define FTREMIX_BOWSER_AMMO_RECHARGE_STEP 2
#define FTREMIX_BOWSER_AMMO_RECHARGE_FRAMES 0x1E
#define FTREMIX_BOWSER_AMMO_HALFWORD_MASK 0x0000FFFFu

/**
 * Ammo of a fighter that just spawned: full ammo and a restarted recharge timer.
 *
 * @return The scratch word.
 */
extern unsigned int ftRemixBowserAmmoSpawnWord(void);

/**
 * Scratch word after Bowser's KO refills the ammo; the recharge timer keeps counting.
 *
 * @param word Scratch word.
 * @return The word with the ammo set to FTREMIX_BOWSER_AMMO_MAX.
 */
extern unsigned int ftRemixBowserAmmoRefillWord(unsigned int word);

/**
 * One frame of the recharge: the timer counts up and every FTREMIX_BOWSER_AMMO_RECHARGE_FRAMES frames restarts, giving
 * FTREMIX_BOWSER_AMMO_RECHARGE_STEP ammo unless the fire breath is running or the ammo is exactly FTREMIX_BOWSER_AMMO_MAX
 * (the asm tests for equality, so an ammo that went past the maximum by an odd count keeps growing).
 *
 * @param word Scratch word.
 * @param is_firing Whether the fighter is in a fire breath status.
 * @return The updated scratch word.
 */
extern unsigned int ftRemixBowserAmmoRechargeWord(unsigned int word, int is_firing);

#endif

#endif
