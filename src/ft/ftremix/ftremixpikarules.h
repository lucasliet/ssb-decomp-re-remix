#ifndef _FTREMIXPIKARULES_H_
#define _FTREMIXPIKARULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Pikachu clone patches (src/pikashared.asm, src/JPika/JPika.asm) so the headless unit tests
 * (tests/remix) can run them without game state. The engine side is ftremixpikashared.c.
 *
 * "Power kind" is the fighter kind whose moves are used: the fighter itself, or the copied power of a Kirby.
 */

/* thunderjolt_duration_: JPIKA_DURATION 0x78 (the US Pikachu keeps its 100 frames) */
#define FTREMIX_PIKA_THUNDERJOLT_LIFETIME_JPIKA 120

/* get_thunder_special_struct_1/2: E Pikachu and J Pikachu read the Thunder head and trail attributes of their own main file. */
int ftRemixPikaRulesThunderUsesOwnFile(int fkind);

/* get_thunder_jolt_special_struct_1/2: J Pikachu reads the Thunder Jolt attributes of its own file 6 (the asm covers only
 * J Pikachu). E Pikachu's file 6 is the vanilla Pikachu one (0x0F4, Remix define_character), so reading it too is
 * content-equivalent and does not depend on the Pikachu globals being loaded. */
int ftRemixPikaRulesThunderJoltUsesOwnFile(int power_kind);

/* thunderjolt_duration_: lifetime of the aerial Thunder Jolt when it is fired. */
int ftRemixPikaRulesThunderJoltLifetime(int power_kind, int vanilla_lifetime);

#endif

#endif
