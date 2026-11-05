/**
 * mnremixcssrandom.c - Random bookend of the Remix VS character select, see mnremixcssrandom.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include "remix/remix_css_data.h"
#include "mn/mnplayers/mnremixcssrandom.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixcssslide.h"

/* The recent randoms live as long as the session, like the asm table in Remix RAM. */
static MNRemixCssRecentRandoms sMNRemixCssRecentRandoms[MNREMIX_CSS_RANDOM_PLAYERS];
static sb32 sMNRemixCssRecentRandomsReady;

static void mnRemixCssRandomPrepareBookendSprite(SObj *sobj)
{
    sobj->pos.x = MNREMIX_CSS_RANDOM_BOOKEND_X;
    sobj->pos.y = MNREMIX_CSS_BOOKEND_Y;
    sobj->sprite.scalex = MNREMIX_CSS_BONUS_BOOKEND_SCALE_X;
    sobj->sprite.scaley = MNREMIX_CSS_BONUS_BOOKEND_SCALE_Y;
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
}

void mnRemixCssRandomMakeBookend(void *css_images_file, s32 gobj_link, s32 dl_link)
{
    GObj *gobj = gcMakeGObjSPAfter(0, NULL, gobj_link, GOBJ_PRIORITY_DEFAULT);
    SObj *sobj;

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, dl_link, GOBJ_PRIORITY_DEFAULT, ~0);

    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, css_images_file, REMIX_CSS_RANDOM_BOOKEND_OFFSET));
    mnRemixCssRandomPrepareBookendSprite(sobj);
    mnRemixCssSlideAttach(gobj, MNREMIX_CSS_RANDOM_BOOKEND_SLOT);
}

void mnRemixCssRandomMakeFlash(GObj *gobj, void *css_images_file)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, css_images_file, REMIX_CSS_RANDOM_BOOKEND_OFFSET + MNREMIX_CSS_RANDOM_FLASH_DELTA));

    mnRemixCssRandomPrepareBookendSprite(sobj);
}

static void mnRemixCssRandomPrepareRecentRandoms(void)
{
    s32 port;

    if (sMNRemixCssRecentRandomsReady != FALSE)
    {
        return;
    }
    for (port = 0; port < MNREMIX_CSS_RANDOM_PLAYERS; port++)
    {
        mnRemixCssRecentRandomsReset(&sMNRemixCssRecentRandoms[port]);
    }
    sMNRemixCssRecentRandomsReady = TRUE;
}

sb32 mnRemixCssRandomKeepRoll(s32 port, s32 fkind)
{
    MNRemixCssRecentRandoms *recent;
    s32 dice_range;

    if ((port < 0) || (port >= MNREMIX_CSS_RANDOM_PLAYERS))
    {
        return TRUE;
    }
    mnRemixCssRandomPrepareRecentRandoms();

    recent = &sMNRemixCssRecentRandoms[port];
    dice_range = mnRemixCssRecentRandomsDiceRange(mnRemixCssRecentRandomsStore(recent, fkind));

    if ((dice_range != 0) && (syUtilsRandIntRange(dice_range) != 0))
    {
        return FALSE;
    }
    mnRemixCssRecentRandomsCommit(recent);

    return TRUE;
}

#endif
