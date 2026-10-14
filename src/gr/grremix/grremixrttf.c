/**
 * grremixrttf.c - Race to the Finish bumper offset (Hazards.asm rttf_bumpers_). See grremixrttf.h.
 */

#ifdef PORT

#include "grremixrttf.h"
#include "grremixids.h"
#include <reloc_data.h>
#include <sc/scene.h>

s32 grRemixBonus3GetBumpersAnimJointOffset(void)
{
    if (gSCManagerBattleState->gkind == nGRRemixStage_REMIX_RTTF)
    {
        return GRREMIX_RTTF_BUMPERS_ANIM_JOINT_OFFSET;
    }

    return llGRBonus3MapBumpersAnimJoint;
}

#endif
