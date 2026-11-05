/**
 * grremixbarrelexit.c - exit of a fighter shot from a barrel on the Remix stages, see grremixbarrelexit.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixbarrelexit.h"
#include "gr/grremix/grremixids.h"
#include <ft/fighter.h>
#include <sc/scene.h>
#include <reloc_data.h>

/*
 * Hazards.asm congo_falls_hazard _congo_falls_hitbox_moveset: the words of the asm as they are. HITBOX(0, 0, 0, 0, 0, 0, 250, 1, 1,
 * 6, 10, 0, 0, 50, 50, 2, 361, 0, 1) is the comment of the line; the words encode shield damage 10, base knockback 60, fixed
 * knockback 0 and scaling 150 instead of the values of that comment, and the words are what runs.
 */
static const u32 sGRRemixBarrelExitFallsMoveset[] =
{
    0x0C0000C0,
    0x01F40000,
    0x00000000,
    0x5A496003,
    0x0A201E00,
    0x04000032, /* WAIT(50) */
    0x18000000, /* END_HITBOXES */
    0x00000000, /* END */
};

intptr_t grRemixBarrelExitGetMapHeader(void)
{
    if (gSCManagerBattleState->gkind == nGRRemixStage_BTP_BANJO)
    {
        return 0;
    }
    return llGRJungleMapMapHeader;
}

void grRemixBarrelExitSetHitbox(FTStruct *fp)
{
    if (gSCManagerBattleState->gkind != nGRRemixStage_FALLS)
    {
        return;
    }
    fp->motion_scripts[0][0].p_script = (u32*) sGRRemixBarrelExitFallsMoveset;
}

#endif
