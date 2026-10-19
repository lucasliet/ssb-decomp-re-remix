/**
 * ftremixitemdrop.c - Smash Remix Item.asm item_drop: the All Items R Drop (Aerial) setting.
 *
 * ftCommonAttackAirCheckInterruptCommon calls this instead of comparing the held item type with nITTypeThrow. With the
 * setting on, holding R in the air drops any item instead of throwing it. Off, it is the vanilla type check.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <PR/os.h>
#include <ft/fighter.h>
#include <it/item.h>

/* Item.asm item_drop (0x80150BAC, ftCommonAttackAirCheckInterruptCommon). TRUE when the aerial attack throws the held item. */
sb32 ftRemixAerialItemIsThrowType(FTStruct *fp)
{
    ITStruct *ip = itGetStruct(fp->item_gobj);

    if (gmRemixToggle(item_dropping) != FALSE && (fp->input.pl.button_tap & CONT_R) != 0)
    {
        return FALSE;
    }
    return ip->type == nITTypeThrow;
}

#endif
