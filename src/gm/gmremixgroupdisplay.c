/**
 * gmremixgroupdisplay.c - Remix display groups (Render.asm toggle_group_display_, SpecialZoom.asm
 * temp_toggle_group_display_), see gmremixgroupdisplay.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/objman.h>
#include "gm/gmremixgroupdisplay.h"
#include "gm/gmremixgroupdisplayrules.h"

#include <port_log.h>

static sb32 gmRemixGroupDisplayIsValidLink(s32 link_id)
{
    if ((link_id < 0) || (link_id >= GC_COMMON_MAX_LINKS))
    {
        port_log("gmRemixGroupDisplay: link_id %d outside 0..%d\n", link_id, GC_COMMON_MAX_LINKS - 1);
        return FALSE;
    }
    return TRUE;
}

void gmRemixGroupDisplaySet(s32 link_id, u32 hide)
{
    GObj *gobj;

    if (!gmRemixGroupDisplayIsValidLink(link_id))
    {
        return;
    }
    for (gobj = gGCCommonLinks[link_id]; gobj != NULL; gobj = gobj->link_next)
    {
        gobj->flags = gmRemixGroupDisplayRulesSetFlags(hide);
    }
}

void gmRemixGroupDisplayTempSet(s32 link_id, u32 hide)
{
    GObj *gobj;

    if (!gmRemixGroupDisplayIsValidLink(link_id))
    {
        return;
    }
    for (gobj = gGCCommonLinks[link_id]; gobj != NULL; gobj = gobj->link_next)
    {
        gobj->flags = gmRemixGroupDisplayRulesTempFlags(gobj->flags, hide);
    }
}

#endif
