/*
 * ftremixairecovery.c - recovery hooks of the Remix AI (.toolchain/pkgs/smashremix-src/src/AI.asm recovery_fix_).
 * Pure rules live in ftremixairecoveryrules.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <gm/gmremixtoggle.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "ft/ftremix/ftremixairecovery.h"
#include "ft/ftremix/ftremixairecoveryrules.h"

/**
 * recovery_fix_ (AI.asm): the vanilla up-B limit is skipped for CPUs at level 10 or with the improved AI, and never for
 * vanilla 1P/RTTF. Classic mode is the 1P game type here, see the deviation note in docs/remix/notes/f13_ai_port.md.
 */
sb32 ftRemixAiRecoveryAllowsUpBRepeat(FTStruct *fp)
{
    int is_classic_mode = (gSCManagerBattleState != NULL) && (gSCManagerBattleState->game_type == nSCBattleGameType1PGame);

    return ftRemixAiRecoveryUpBRepeatAllowed(fp->level, is_classic_mode, gmRemixToggle(improved_ai));
}

#endif
