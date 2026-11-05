#ifndef _FTREMIXINITIALARMOR_H_
#define _FTREMIXINITIALARMOR_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/**
 * Passive armor of the Remix fighters whose Character.initial_script gives them one (MLuigi.asm and gbowser.asm initial_script_,
 * based on the Giant Donkey Kong script at 0x800D7DD4): Metal Luigi resists 30 and Giga Bowser 55 knockback, and both clear
 * the first passive variable word.
 *
 * @param fp Fighter being initialised, after its passive armor was reset to zero.
 */
void ftRemixInitialScriptArmor(FTStruct *fp);

#endif

#endif
