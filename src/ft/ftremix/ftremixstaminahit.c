/**
 * ftremixstaminahit.c - Smash Remix Stamina.asm action_hit_crash_prevent (0x800E825C), see ftremixstaminahit.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixstaminahit.h"
#include "gm/gmremixstamina.h"
#include <sc/scmanager.h>
#include <sc/sctypes.h>

void ftRemixStaminaClearDeadWaitOnHit(FTStruct *fp)
{
    if (gSCManagerSceneData.scene_curr != nSCKindVSBattle)
    {
        return;
    }
    if ((gmRemixStaminaIsActive() == FALSE) || (gmRemixStaminaIsDefeated(fp->player) == FALSE))
    {
        return;
    }
    fp->status_vars.common.dead.wait = 0;
}

#endif
