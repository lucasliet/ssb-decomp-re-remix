#ifndef _SC1PREMIX1PRULES_H_
#define _SC1PREMIX1PRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Random picks of the Remix 1P path (SinglePlayerModes.asm remix_1p_randomization, duo_select): which fighter fills a slot, on which
 * of its three stages, which Kirby powers and which final boss. The random source is passed in so the headless unit tests
 * (tests/remix) can drive it.
 */

#define SC1PREMIX_1P_STAGE_OPTIONS 3
#define SC1PREMIX_1P_BOSS_MAD_PIANO 0
#define SC1PREMIX_1P_BOSS_METAL_BROS 1
#define SC1PREMIX_1P_BOSS_SUPER_SONIC 2
#define SC1PREMIX_1P_SONIC_STAGE_GHZ 0
#define SC1PREMIX_1P_SONIC_STAGE_CASINO 1

/** Random source: returns a number in [0, range). */
typedef int (*SC1PRemix1PRandom)(int range);

/**
 * Picks a fighter of the pool that no slot has taken yet and marks it as taken.
 *
 * @param rand Random source.
 * @param is_used Per-fighter flags of the pool, updated.
 * @param count Fighters in the pool.
 * @return Index of the fighter, or -1 when every fighter is taken.
 */
extern int sc1PRemix1PPickFighter(SC1PRemix1PRandom rand, int *is_used, int count);

/**
 * Picks one of the three stage options of a fighter that the path has not used yet and marks it as used. The vanilla loop retries
 * until an option is free and never ends when all three are used; here that case returns -1 so the caller picks another fighter.
 *
 * @param rand Random source.
 * @param options Stage ids of the three options.
 * @param is_stage_used Flags per stage id, updated.
 * @param stage_count Entries of is_stage_used.
 * @return Stage id, or -1 when no option is free.
 */
extern int sc1PRemix1PPickStage(SC1PRemix1PRandom rand, const int options[SC1PREMIX_1P_STAGE_OPTIONS], int *is_stage_used, int stage_count);

/**
 * Picks distinct entries of a pool (the Kirby powers and hats).
 *
 * @param rand Random source.
 * @param count Entries in the pool.
 * @param picks Receives the pool indices.
 * @param picks_count How many to pick, at most count.
 */
extern void sc1PRemix1PPickDistinct(SC1PRemix1PRandom rand, int count, int *picks, int picks_count);

/**
 * Picks the final boss of the path.
 *
 * @param rand Random source.
 * @return SC1PREMIX_1P_BOSS_*.
 */
extern int sc1PRemix1PPickBoss(SC1PRemix1PRandom rand);

/**
 * Picks the stage of Super Sonic's fight: Green Hill Zone twice as often as Casino Night Zone.
 *
 * @param rand Random source.
 * @return SC1PREMIX_1P_SONIC_STAGE_*.
 */
extern int sc1PRemix1PPickSonicStage(SC1PRemix1PRandom rand);

/** Fighters of the All-Star run: the 18 Remix fighters and the 12 vanilla ones. */
#define SC1PREMIX_ALLSTAR_FIGHTER_COUNT 30

/**
 * Orders the All-Star run (SinglePlayerModes.asm allstar_randomization): every fighter of the pool once, each on one of its three
 * stages that no earlier slot took. The asm retries the stage forever when the three are taken; here the slot goes to another fighter
 * and, when no fighter that is left has a free stage, to a random one on its first stage, which is the only place a stage repeats.
 *
 * @param rand Random source.
 * @param stage_options Stage ids of the three options of each fighter of the pool.
 * @param stage_count Entries of is_stage_used.
 * @param is_stage_used Flags per stage id, updated.
 * @param order Receives the pool index of the fighter of each slot (SC1PREMIX_ALLSTAR_FIGHTER_COUNT entries).
 * @param stages Receives the stage id of each slot.
 */
extern void sc1PRemixAllstarPickOrder(SC1PRemix1PRandom rand, const int stage_options[SC1PREMIX_ALLSTAR_FIGHTER_COUNT][SC1PREMIX_1P_STAGE_OPTIONS],
                                      int stage_count, int *is_stage_used, int *order, int *stages);

#endif

#endif
