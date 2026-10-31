/**
 * ftremixskeleton.c - Skeleton display forced per port (Skeleton.asm skeleton_mode_1_ and skeleton_mode_2_).
 *
 * Both hooks load the colanim word at FTStruct 0x0A88 and replace it with 0x08000000 when the port's switch is on.
 * Bits 3-4 of that word are colanim.skeleton_id (IDO big-endian bit numbering), and 0x08 in the first byte is bit 4,
 * so the replaced word reads skeleton_id 1. The asm does not store the word back, so only the display path changes.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "gm/gmremixpanel.h"
#include "ft/ftremix/ftremixhooks.h"

#define FTREMIX_SKELETON_FORCED_ID 1

/**
 * Skeleton id the display path uses for a fighter: 1 when the port's Skeleton panel option is on, otherwise the id
 * of the colanim script.
 */
s32 ftRemixSkeletonDisplayId(FTStruct *fp)
{
    if (gmRemixPanel(SKELETON, fp->player) != 0)
    {
        return FTREMIX_SKELETON_FORCED_ID;
    }
    return fp->colanim.skeleton_id;
}

#endif
