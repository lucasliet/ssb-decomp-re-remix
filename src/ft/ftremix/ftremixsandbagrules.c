/**
 * ftremixsandbagrules.c - pure rules of the Sandbag landing patches, see ftremixsandbagrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixsandbagrules.h"

#define FTREMIXSANDBAG_PI 3.14159012F               /* 0x40490FD0 */
#define FTREMIXSANDBAG_HEAD_LIMIT 0.628318548F      /* 0x3F20D97C, 0.2pi */
#define FTREMIXSANDBAG_FEET_LIMIT 2.51327419F       /* 0x4020D97C, 0.8pi */

int ftRemixSandbagGetLandingStatus(float rotation_x)
{
    float distance_from_upside_down = rotation_x + FTREMIXSANDBAG_PI;

    if (distance_from_upside_down < 0.0F)
    {
        distance_from_upside_down = -distance_from_upside_down;
    }
    if (distance_from_upside_down <= FTREMIXSANDBAG_HEAD_LIMIT)
    {
        return FTREMIXSANDBAG_STATUS_HEADSTAND;
    }
    if (FTREMIXSANDBAG_FEET_LIMIT <= distance_from_upside_down)
    {
        return FTREMIXSANDBAG_STATUS_IDLE;
    }
    return FTREMIXSANDBAG_STATUS_NONE;
}

int ftRemixSandbagGetStandWait(int stand_wait, int is_knockback_x_zero)
{
    return (is_knockback_x_zero != 0) ? 1 : stand_wait;
}

int ftRemixSandbagResetsDamageMul(int status_id)
{
    return (status_id == FTREMIXSANDBAG_STATUS_IDLE) || (status_id == FTREMIXSANDBAG_STATUS_HEADSTAND);
}

#endif
