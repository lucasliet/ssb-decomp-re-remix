#ifndef _GRREMIXSAFFRON_H_
#define _GRREMIXSAFFRON_H_

/*
 * Saffron City Pokemon Rate setting of the Remix build (Hazards.asm saffron_city_pokemon_rate_init,
 * saffron_city_pokemon_rate). gryamabuki.c calls these under #ifdef REGION_REMIX. The setting only applies on the VS and
 * Training screens.
 */

#include <ssb_types.h>

/** Pokemon wait of the sleep to wait transition; base_wait is the vanilla rand(1000) + 1000. */
u16 grRemixSaffronSleepMonsterWait(s32 base_wait);

/** Pokemon wait of a closing door; base_wait is the vanilla rand(1000) + 1000. */
u16 grRemixSaffronClosedMonsterWait(s32 base_wait);

/** Door wait of a closing door; base_wait is the vanilla 1000. */
u16 grRemixSaffronClosedGateWait(s32 base_wait);

#endif
