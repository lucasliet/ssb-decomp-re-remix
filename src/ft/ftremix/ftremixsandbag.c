/**
 * ftremixsandbag.c - Sandbag landing patches, see ftremixsandbag.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixsandbag.h"
#include "ft/ftremix/ftremixsandbagrules.h"
#include "remix/remix_roster_data.h"
#include <string.h>

/* ftCommonDownBounceCheckUpOrDown reads the X rotation of this joint. */
#define FTREMIXSANDBAG_BODY_JOINT 4

static sb32 ftRemixSandbagIsSandbag(FTStruct *fp)
{
    return fp->fkind == REMIX_ID_SANDBAG;
}

s32 ftRemixSandbagGetBounceStatus(FTStruct *fp, s32 status_id)
{
    s32 landing_status;

    if (!ftRemixSandbagIsSandbag(fp))
    {
        return status_id;
    }
    landing_status = ftRemixSandbagGetLandingStatus(fp->joints[FTREMIXSANDBAG_BODY_JOINT]->rotate.vec.f.x);

    return (landing_status != FTREMIXSANDBAG_STATUS_NONE) ? landing_status : status_id;
}

void ftRemixSandbagResetDownBounceDamageMul(FTStruct *fp)
{
    if (!ftRemixSandbagIsSandbag(fp) || !ftRemixSandbagResetsDamageMul(fp->status_id))
    {
        return;
    }
    fp->damage_mul = 1.0F;
}

void ftRemixSandbagPrepareGetUp(FTStruct *fp)
{
    u32 knockback_x_bits;

    if (!ftRemixSandbagIsSandbag(fp))
    {
        return;
    }
    memcpy(&knockback_x_bits, &fp->physics.vel_damage_air.x, sizeof(knockback_x_bits));
    fp->status_vars.common.downwait.stand_wait = ftRemixSandbagGetStandWait(fp->status_vars.common.downwait.stand_wait, knockback_x_bits == 0);
}

sb32 ftRemixSandbagKeepsKnockbackX(FTStruct *fp)
{
    return ftRemixSandbagIsSandbag(fp);
}

#endif
