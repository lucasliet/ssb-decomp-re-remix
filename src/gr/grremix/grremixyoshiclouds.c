/**
 * grremixyoshiclouds.c - Yoshi's Island cloud hazards and platform movement gate. See grremixyoshiclouds.h.
 */

#ifdef PORT

#include "gm/gmremixtoggle.h"
#include "grremixyoshiclouds.h"
#include "grremixgating.h"
#include "grremixhazards.h"
#include "grremixids.h"
#include <gr/ground.h>
#include <mp/map.h>
#include <sc/scene.h>
#include <sc/scmanager.h>

/* Yakumono line ids of the three clouds, the same values as dGRYosterCloudLineIDs in gryoster.c. */
static const u8 sGRRemixYosterCloudLineIDs[] = { 0x1, 0x2, 0x3 };

/* Value the asm stores in the cloud DObj field at 0x84 (DObj.user_data) to turn its clipping off (Hazards.asm a0 = 4). */
#define GRREMIX_YOSTER_CLIPPING_DISABLED 4

/** Toggles.entry_yi_clouds ("Yoshi's Island Cloud Anims", default off). */
static sb32 grRemixYosterCloudScaleIsOn(void)
{
    return (gmRemixToggle(yi_clouds) != 0) ? TRUE : FALSE;
}

/** TRUE on Onett and the Tal Tal stages, whose moving platform is a hazard and follows the hazard toggle (asm platform_movement_). */
static sb32 grRemixStageMovesAsHazard(s32 gkind)
{
    switch (gkind)
    {
    case nGRRemixStage_ONETT:
    case nGRRemixStage_TALTAL:
    case nGRRemixStage_TALTAL_REMIX:
        return TRUE;

    default:
        return FALSE;
    }
}

void grRemixYosterDisableCloudClipping(void)
{
    s32 i;

    if (grRemixHazardsAreOn())
    {
        return;
    }
    for (i = 0; i < ARRAY_COUNT(sGRRemixYosterCloudLineIDs); i++)
    {
        DObj *cloud_dobj = gMPCollisionYakumonoDObjs->dobjs[sGRRemixYosterCloudLineIDs[i]];

        cloud_dobj->user_data.s = GRREMIX_YOSTER_CLIPPING_DISABLED;
    }
}

u8 grRemixYosterCloudMatrixKind(void)
{
    if (grRemixYosterCloudScaleIsOn())
    {
        return nGCMatrixKindTraRotRpyRSca;
    }
    return nGCMatrixKindTra;
}

/**
 * REMIX-TODO(Stages.stage_hazard_table): the asm skips the platform movement when the stage's Hazards.type has no
 * MOVEMENT bit (dont_freeze_stage). Until that table is ported every stage is treated as freezable, which only differs
 * from the asm when a movement toggle is off.
 */
sb32 grRemixPlatformMovementIsOn(void)
{
    s32 gkind = gSCManagerBattleState->gkind;

    if (grRemixStageMovesAsHazard(gkind))
    {
        return grRemixHazardToggleIsOn();
    }
    return grRemixMovementToggleIsOn();
}

#endif
