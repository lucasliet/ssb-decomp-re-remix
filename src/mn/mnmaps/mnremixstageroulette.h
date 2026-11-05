#ifndef _MNREMIXSTAGEROULETTE_H_
#define _MNREMIXSTAGEROULETTE_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Random Stage Roulette of the Remix stage select (Stages.asm random_stage_roulette, a_, roulette_cursor_timer), as a pure
 * timer so the headless unit tests (tests/remix) can run it. C-down over RANDOM starts it; the cursor then jumps to a random
 * cell of the page every MNREMIX_ROULETTE_DELAY frames until B, L or C-down stops it or A or Start picks the hovered stage. The
 * cell is drawn by mnremixstagegridrules (mnRemixStageGridRulesRouletteRerolls); the slot mask functions at the end are kept for the
 * unit tests of the earlier per-page draw. */

/* random_stage_roulette.DELAY: frames between two cursor jumps. */
#define MNREMIX_ROULETTE_DELAY 8

/* The slots of a page that can be hovered, as the bit mask the picking functions take (bit n is slot n). */
#define MNREMIX_ROULETTE_SLOT_MAX 32

/* roulette_cursor_timer: 0 idle, 1 to DELAY running, -1 once A or Start was pressed. */
typedef struct MNRemixRoulette
{
    int timer;

} MNRemixRoulette;

/** Idles the roulette (screen open). */
extern void mnRemixRouletteReset(MNRemixRoulette *roulette);

/** C-down over RANDOM: starts the roulette with the timer at DELAY so the first jump happens on the same frame. */
extern void mnRemixRouletteStart(MNRemixRoulette *roulette);

/** TRUE while the roulette runs, which includes the frame after A or Start was pressed. */
extern int mnRemixRouletteIsActive(const MNRemixRoulette *roulette);

/** B, L or C-down: ends the roulette on the hovered stage. */
extern void mnRemixRouletteStop(MNRemixRoulette *roulette);

/** A or Start while the roulette runs: the press is swallowed and the stage is chosen on the next frame (a_ sets the timer to -1). */
extern void mnRemixRouletteMarkSelect(MNRemixRoulette *roulette);

/**
 * TRUE once on the frame after A or Start was swallowed: the timer goes back to 0 and the caller treats the press as made now.
 */
extern int mnRemixRouletteTakeSelect(MNRemixRoulette *roulette);

/**
 * Advances a running roulette by one frame.
 *
 * @return TRUE on the frames the cursor must jump: the first one, then every MNREMIX_ROULETTE_DELAY frames.
 */
extern int mnRemixRouletteTick(MNRemixRoulette *roulette);

/** Number of slots set in the mask. */
extern int mnRemixRouletteCountSlots(unsigned int slot_mask);

/**
 * The nth hoverable slot.
 *
 * @param slot_mask Bit n set when slot n can be hovered.
 * @param index Which of the set slots, 0 based, below mnRemixRouletteCountSlots.
 * @return The slot number, or -1 when index is outside the set slots.
 */
extern int mnRemixRouletteNthSlot(unsigned int slot_mask, int index);

#endif

#endif
