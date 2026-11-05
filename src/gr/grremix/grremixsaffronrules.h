#ifndef _GRREMIXSAFFRONRULES_H_
#define _GRREMIXSAFFRONRULES_H_

#include <stdint.h>

/*
 * Pure rules of the Saffron City Pokemon Rate setting (Hazards.asm saffron_city_pokemon_rate_init and
 * saffron_city_pokemon_rate). Ints only, so the headless tests can run them.
 */

#define GRREMIX_SAFFRON_RATE_DEFAULT 0
#define GRREMIX_SAFFRON_RATE_SUPER 1
#define GRREMIX_SAFFRON_RATE_HYPER 2
#define GRREMIX_SAFFRON_RATE_QUICK_ATTACK 3

/**
 * Pokemon wait stored when the stage leaves its sleep status (saffron_city_pokemon_rate_init): Super and Hyper halve the
 * wait, Quick Attack uses a single tic.
 *
 * @param rate Toggles.entry_saffron_poke_rate.
 * @param base_wait The vanilla wait, rand(1000) + 1000.
 * @return Wait in tics.
 */
int32_t grRemixSaffronRateSleepMonsterWait(int32_t rate, int32_t base_wait);

/**
 * Pokemon wait stored when the door closes again (saffron_city_pokemon_rate): Super and Hyper halve the wait, Quick Attack
 * uses 15 tics.
 *
 * @param rate Toggles.entry_saffron_poke_rate.
 * @param base_wait The vanilla wait, rand(1000) + 1000.
 * @return Wait in tics.
 */
int32_t grRemixSaffronRateClosedMonsterWait(int32_t rate, int32_t base_wait);

/**
 * Door wait stored when the door closes again (saffron_city_pokemon_rate): Super halves it, Hyper and Quick Attack open
 * the door after a single tic.
 *
 * @param rate Toggles.entry_saffron_poke_rate.
 * @param base_wait The vanilla door wait, 1000.
 * @return Wait in tics.
 */
int32_t grRemixSaffronRateClosedGateWait(int32_t rate, int32_t base_wait);

#endif
