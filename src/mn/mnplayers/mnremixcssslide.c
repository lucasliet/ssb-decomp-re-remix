/**
 * mnremixcssslide.c - slide-in of the Remix VS character select portraits, see mnremixcssslide.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/obj.h>
#include <sys/objman.h>
#include "mn/mnplayers/mnremixcssslide.h"
#include "mn/mnplayers/mnremixcsssliderules.h"

/** Moves the portrait one step towards its cell (the asm portrait animation routine). */
static void mnRemixCssSlideProcUpdate(GObj *gobj)
{
    SObj *sobj = SObjGetStruct(gobj);
    s32 column = mnRemixCssSlideRulesColumn(gobj->user_data.s);

    if (column < 0)
    {
        return;
    }
    sobj->pos.x = mnRemixCssSlideRulesNextX(column, sobj->pos.x);
}

void mnRemixCssSlideAttach(GObj *gobj, s32 slot)
{
    s32 column = mnRemixCssSlideRulesColumn(slot);

    if (column < 0)
    {
        return;
    }
    gobj->user_data.s = slot;
    SObjGetStruct(gobj)->pos.x = mnRemixCssSlideRulesStartX(column);
    gcAddGObjProcess(gobj, mnRemixCssSlideProcUpdate, nGCProcessKindFunc, 1);
}

#endif
