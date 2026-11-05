/**
 * ftremixthrown.c - bowser.asm thrown_mario_bros_fix_: the Bowsers keep the ThrownMarioBros status when the throw animation
 * ends. Pure rule in ftremixthrownrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixthrownrules.h"

/** TRUE when the thrown fighter must not switch status at the end of the throw animation. */
sb32 ftRemixThrownKeepsStatusAtAnimEnd(s32 fkind)
{
    return ftRemixThrownRulesKeepsStatusAtAnimEnd(fkind);
}

#endif
