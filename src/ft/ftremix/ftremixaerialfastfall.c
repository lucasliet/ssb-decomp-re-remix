/**
 * ftremixaerialfastfall.c - Fast fall during the five normal aerial attacks (AerialAttackFastFall.asm).
 *
 * The asm replaces the Proc Physics pointer of AttackAirN, F, B, Hi and Lw (ROM words 0xA5638..0xA5688 hold
 * 0x800D90E0, ftPhysicsApplyAirVelDrift). The port cannot patch the status table words, so ftMainSetStatus calls
 * ftRemixAerialFastFallSetPhysics after it loads the table pointer. The replacement reads
 * Toggles.entry_fast_fall_aerials on every frame.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "ft/ftphysics.h"
#include "gm/gmremixtoggle.h"
#include "ft/ftremix/ftremixhooks.h"

/**
 * Proc Physics of the normal aerial attack statuses. Fast fall while the toggle is on, the vanilla air drift otherwise.
 */
void ftRemixAerialFastFallPhysics(GObj *fighter_gobj)
{
    if (gmRemixToggle(fast_fall_aerials) != 0)
    {
        ftPhysicsApplyAirVelDriftFastFall(fighter_gobj);
        return;
    }
    ftPhysicsApplyAirVelDrift(fighter_gobj);
}

/**
 * Replaces the air drift of a common aerial attack status (AttackAirN to AttackAirLw) with the toggle-aware physics.
 * Other statuses and fighter-specific physics pointers are left unchanged.
 */
void ftRemixAerialFastFallSetPhysics(FTStruct *fp, s32 status_id)
{
    if ((status_id < nFTCommonStatusAttackAirN) || (status_id > nFTCommonStatusAttackAirLw))
    {
        return;
    }
    if (fp->proc_physics == ftPhysicsApplyAirVelDrift)
    {
        fp->proc_physics = ftRemixAerialFastFallPhysics;
    }
}

#endif
