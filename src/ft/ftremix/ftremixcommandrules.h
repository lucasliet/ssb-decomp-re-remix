#ifndef _FTREMIXCOMMANDRULES_H_
#define _FTREMIXCOMMANDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the animation translation multiplier (Command.asm translation_multiplier_ apply_ground_z_, apply_ground_x_,
 * apply_air_z_, apply_air_y_, apply_air_x_). Float only, so the headless unit tests (tests/remix) can run it.
 */

/** Command.asm SUPER_MULT (0x3F190000): applied when the Size multiplier is above 1. */
#define FTREMIXCOMMAND_SUPER_MULT 0.59765625F

/** Command.asm POISON_MULT (0x3FCC0000): applied when the Size multiplier is below 1. */
#define FTREMIXCOMMAND_POISON_MULT 1.59375F

/**
 * Factor the five apply_* patches multiply the animation translation by, on top of the command 0xD3 value, for the Size
 * multiplier of the player. Giant fighters travel 0.5977 of it, tiny ones 1.59375.
 *
 * @param size_multiplier Size multiplier of the player (1.0 for normal size).
 * @return 1.0, FTREMIXCOMMAND_SUPER_MULT or FTREMIXCOMMAND_POISON_MULT.
 */
extern float ftRemixCommandGetSizeFactor(float size_multiplier);

#endif

#endif
