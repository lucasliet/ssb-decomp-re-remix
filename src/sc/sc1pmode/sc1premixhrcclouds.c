/**
 * sc1premixhrcclouds.c - background clouds of the Home-Run Contest, see sc1premixhrcclouds.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/obj.h>
#include <sys/objtypes.h>
#include "sc/sc1pmode/sc1premixhrc.h"
#include "sc/sc1pmode/sc1premixhrcclouds.h"
#include "sc/sc1pmode/sc1premixhrcrules.h"

static float sSC1PRemixHrcCloudX;
static float sSC1PRemixHrcCloudSpeed;

void sc1PRemixHrcCloudsReset(void)
{
    sSC1PRemixHrcCloudX = SC1PREMIX_HRC_CLOUD_START;
    sSC1PRemixHrcCloudSpeed = 0.0F;
}

void sc1PRemixHrcCloudsSetSpeed(float sandbag_dx, sb32 is_past_start)
{
    sSC1PRemixHrcCloudSpeed = sc1PRemixHrcCloudSpeed(sandbag_dx, (is_past_start != FALSE) ? 1 : 0);
}

void sc1PRemixHrcCloudsApply(SObj *wallpaper_sobj)
{
    if (sc1PRemixHrcIsActive() == FALSE)
    {
        return;
    }
    sSC1PRemixHrcCloudX = sc1PRemixHrcCloudStep(sSC1PRemixHrcCloudX, sSC1PRemixHrcCloudSpeed);

    wallpaper_sobj->pos.x = sSC1PRemixHrcCloudX;
    wallpaper_sobj->sprite.scalex = SC1PREMIX_HRC_CLOUD_SCALE_X;
}

#endif
