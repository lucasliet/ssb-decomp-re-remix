/**
 * ftremixcliffplatform.c - cliff statuses on a platform that disappears. See ftremixcliffplatform.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/map.h>
#include "ft/ftremix/ftremixcliffplatform.h"

sb32 ftRemixCliffPlatformHasVanished(s32 line_id)
{
    s32 yakumono_id;

    if ((line_id == -1) || (line_id == -2))
    {
        return FALSE;
    }
    yakumono_id = mpCollisionSetDObjNoID(line_id);

    return (gMPCollisionYakumonoDObjs->dobjs[yakumono_id]->user_data.s >= nMPYakumonoStatusOff) ? TRUE : FALSE;
}

#endif
