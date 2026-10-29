#ifndef _FTREMIXAITABLES_H_
#define _FTREMIXAITABLES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/*
 * Attack list of a CPU (AI.asm add_attack_behaviour tables read from the Remix RAM image). The lists are built lazily, once per fkind.
 * Returns NULL when the vanilla registry list must be used: a vanilla fighter outside level 10 or without an lv10 entry, or a table that
 * could not be read (the reason is logged).
 */
FTComputerAttack *ftRemixAiGetAttackList(s32 fkind, s32 level);

#endif

#endif
