/**
 * grremixyoshiclouds.c - Yoshi's Island cloud hazards and platform movement gate. See grremixyoshiclouds.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

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

/* Hazards.type bits of Stages.asm stage_hazard_table (HAZARDS = 0x01, MOVEMENT = 0x02). */
#define GRREMIX_HAZARD_TYPE_NONE 0x00
#define GRREMIX_HAZARD_TYPE_HAZARDS 0x01
#define GRREMIX_HAZARD_TYPE_MOVEMENT 0x02
#define GRREMIX_HAZARD_TYPE_BOTH 0x03

/* Stages.asm stage_hazard_table, indexed by stage id. Stage ids past the end of this table are NONE (asm fill). */
static const u8 sGRRemixStageHazardTypes[] = {
    [nGRRemixStage_PEACHS_CASTLE] = GRREMIX_HAZARD_TYPE_BOTH,
    [nGRRemixStage_SECTOR_Z] = GRREMIX_HAZARD_TYPE_HAZARDS,
    [nGRRemixStage_CONGO_JUNGLE] = GRREMIX_HAZARD_TYPE_BOTH,
    [nGRRemixStage_PLANET_ZEBES] = GRREMIX_HAZARD_TYPE_BOTH,
    [nGRRemixStage_HYRULE_CASTLE] = GRREMIX_HAZARD_TYPE_HAZARDS,
    [nGRRemixStage_YOSHIS_ISLAND] = GRREMIX_HAZARD_TYPE_HAZARDS,
    [nGRRemixStage_DREAM_LAND] = GRREMIX_HAZARD_TYPE_HAZARDS,
    [nGRRemixStage_SAFFRON_CITY] = GRREMIX_HAZARD_TYPE_BOTH,
    [nGRRemixStage_MUSHROOM_KINGDOM] = GRREMIX_HAZARD_TYPE_HAZARDS,
    [nGRRemixStage_DREAM_LAND_BETA_1] = GRREMIX_HAZARD_TYPE_NONE,
    [nGRRemixStage_DREAM_LAND_BETA_2] = GRREMIX_HAZARD_TYPE_MOVEMENT,
};

/** TRUE when the stage's Hazards.type has the MOVEMENT bit (the inverse of the asm dont_freeze_stage flag). */
static sb32 grRemixStageHasMovement(s32 gkind)
{
    if ((gkind < 0) || (gkind >= ARRAY_COUNT(sGRRemixStageHazardTypes)))
    {
        return FALSE;
    }
    return ((sGRRemixStageHazardTypes[gkind] & GRREMIX_HAZARD_TYPE_MOVEMENT) != 0) ? TRUE : FALSE;
}

/**
 * Platform movement gate of a ground display (Hazards.asm platform_movement_). A stage without the MOVEMENT bit always
 * moves its platforms. Onett and the Tal Tal stages follow the hazard toggle, all other stages the movement toggle.
 */
sb32 grRemixPlatformMovementIsOn(void)
{
    s32 gkind = gSCManagerBattleState->gkind;

    if (grRemixStageHasMovement(gkind) == FALSE)
    {
        return TRUE;
    }
    if (grRemixStageMovesAsHazard(gkind))
    {
        return grRemixHazardToggleIsOn();
    }
    return grRemixMovementToggleIsOn();
}

#endif
