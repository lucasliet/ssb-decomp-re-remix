/**
 * grremixyoshiclouds.c - Yoshi's Island cloud hazards and platform movement gate. See grremixyoshiclouds.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtoggle.h"
#include "grremixyoshiclouds.h"
#include "grremixgating.h"
#include "grremixhazards.h"
#include "grremixids.h"
#include "grremixplatformrules.h"
#include "grremixstage.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
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

sb32 grRemixYosterDisableCloudClipping(void)
{
    s32 i;

    if (grRemixHazardsAreOn())
    {
        return FALSE;
    }
    for (i = 0; i < ARRAY_COUNT(sGRRemixYosterCloudLineIDs); i++)
    {
        DObj *cloud_dobj = gMPCollisionYakumonoDObjs->dobjs[sGRRemixYosterCloudLineIDs[i]];

        cloud_dobj->user_data.s = GRREMIX_YOSTER_CLIPPING_DISABLED;
    }
    return TRUE;
}

u8 grRemixYosterCloudMatrixKind(void)
{
    if (grRemixYosterCloudScaleIsOn())
    {
        return nGCMatrixKindTraRotRpyRSca;
    }
    return nGCMatrixKindTra;
}

/** Stages.stage_hazard_table entry of a stage; ids past the table have neither hazards nor movement. */
static s32 grRemixStageHazardType(s32 gkind)
{
    if ((gkind < 0) || (gkind > GRREMIX_KIND_MAX))
    {
        return 0;
    }
    return (s32)remix_ram_image_read_byte(REMIX_SYM_Stages__stage_hazard_table + gkind);
}

sb32 grRemixPlatformMovementIsOn(void)
{
    s32 gkind = gSCManagerBattleState->gkind;

    return grRemixPlatformRulesMovementIsOn(gkind, grRemixStageHazardType(gkind), grRemixHazardToggleIsOn(), grRemixMovementToggleIsOn());
}

#endif
