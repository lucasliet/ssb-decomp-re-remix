/**
 * mnremixcsstoken.c - token helpers shared by the Remix character selects, see mnremixcsstoken.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <reloc_data.h>
#include <ft/ftdef.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <sys/obj.h>
#include "remix/remix_css_data.h"
#include "remix/remix_roster_data.h"
#include "mn/mnplayers/mnremixcss.h"
#include "mn/mnplayers/mnremixcssbonus.h"
#include "mn/mnplayers/mnremixcssrandom.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixcsstoken.h"
#include "mn/mnplayers/mnremixcsstokenrules.h"

void mnRemixCssTokenScale(GObj *token)
{
    SObj *sobj = SObjGetStruct(token);

    sobj->sprite.scalex = REMIX_CSS_PORTRAIT_SCALE;
    sobj->sprite.scaley = REMIX_CSS_PORTRAIT_SCALE;
}

void mnRemixCssTokenFollow(GObj *token, GObj *cursor)
{
    SObjGetStruct(token)->pos.x = SObjGetStruct(cursor)->pos.x + MNREMIX_CSS_TOKEN_FOLLOW_X;
    SObjGetStruct(token)->pos.y = SObjGetStruct(cursor)->pos.y + MNREMIX_CSS_TOKEN_FOLLOW_Y;
}

s32 mnRemixCssTokenGetSlot(s32 fkind)
{
    s32 index;

    if (fkind == REMIX_ID_RANDOM)
    {
        return MNREMIX_CSS_TOKEN_RANDOM_SLOT;
    }
    for (index = 0; index < REMIX_CSS_NUM_BONUS_CHARS; index++)
    {
        if (kRemixCssBonusIds[index] == fkind)
        {
            return MNREMIX_CSS_TOKEN_BONUS_SLOT;
        }
    }
    return mnRemixCssGetPortrait(fkind);
}

sb32 mnRemixCssTokenGetBookendFighter(s32 pos_x, s32 pos_y, s32 token, s32 *fkind)
{
    if (mnRemixCssRandomBookendContains(pos_x, pos_y) != 0)
    {
        *fkind = REMIX_ID_RANDOM;
        return TRUE;
    }
    if (mnRemixCssBonusBookendContains(pos_x, pos_y) != 0)
    {
        *fkind = mnRemixCssBonusGetFighterKind(token);
        return TRUE;
    }
    return FALSE;
}

void mnRemixCssTokenPlace(GObj *token, s32 fkind)
{
    f32 pos_x, pos_y;

    mnRemixCssTokenRestPosition(mnRemixCssTokenGetSlot(fkind), &pos_x, &pos_y);
    SObjGetStruct(token)->pos.x = pos_x;
    SObjGetStruct(token)->pos.y = pos_y;
}

void mnRemixCssTokenMakeFlash(GObj *gobj, void *portraits_file, s32 slot)
{
    s32 fkind = mnRemixCssGetFighterKind(slot);
    SObj *sobj;

    if (fkind == nFTKindNull)
    {
        return;
    }
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, portraits_file, kRemixCssPortraitOffset[fkind] + MNREMIX_CSS_TOKEN_FLASH_DELTA));
    sobj->pos.x = mnRemixCssCellX(slot);
    sobj->pos.y = mnRemixCssCellY(slot);
    sobj->sprite.scalex = REMIX_CSS_PORTRAIT_SCALE;
    sobj->sprite.scaley = REMIX_CSS_PORTRAIT_SCALE;
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
}

void mnRemixCssTokenMakeSlotFlash(GObj *gobj, void *portraits_file, void *images_file, s32 slot)
{
    if (slot == MNREMIX_CSS_TOKEN_RANDOM_SLOT)
    {
        mnRemixCssRandomMakeFlash(gobj, images_file);
        return;
    }
    if (slot == MNREMIX_CSS_TOKEN_BONUS_SLOT)
    {
        mnRemixCssBonusMakeFlash(gobj, images_file);
        return;
    }
    mnRemixCssTokenMakeFlash(gobj, portraits_file, slot);
}

#endif
