/**
 * sc1premixallstarfinal.c - HUD of the All-Star final battle, see sc1premixallstarfinal.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <if/ifcommon.h>
#include <if/interface.h>
#include <lb/library.h>
#include <sc/scmanager.h>
#include <sys/objman.h>
#include "sc/sc1pmode/sc1premixallstar.h"
#include "sc/sc1pmode/sc1premixallstarfinal.h"

#define SC1PREMIX_ALLSTAR_FINAL_SPRITES_MAX 4

typedef struct SC1PRemixAllstarFinalFighterSprites
{
    s32 fkind;
    FTSprites *sprites;

} SC1PRemixAllstarFinalFighterSprites;

static SC1PRemixAllstarFinalFighterSprites sSC1PRemixAllstarFinalSprites[SC1PREMIX_ALLSTAR_FINAL_SPRITES_MAX];
static s32 sSC1PRemixAllstarFinalSpritesCount;

static sb32 sc1PRemixAllstarFinalIsActive(void)
{
    if (sc1PRemixAllstarIsActive() == FALSE)
    {
        return FALSE;
    }
    return (gSCManagerSceneData.spgame_stage == SC1PREMIX_ALLSTAR_STAGE_FINAL) ? TRUE : FALSE;
}

static FTSprites* sc1PRemixAllstarFinalGetSprites(const FTStruct *fp)
{
    FTSprites *sprites = (FTSprites*)PORT_RESOLVE(fp->attr->sprites);

    if ((sprites == NULL) || (PORT_RESOLVE(sprites->stock_sprite) == NULL))
    {
        return NULL;
    }
    return sprites;
}

static FTSprites* sc1PRemixAllstarFinalFindSprites(s32 fkind)
{
    s32 i;

    for (i = 0; i < sSC1PRemixAllstarFinalSpritesCount; i++)
    {
        if (sSC1PRemixAllstarFinalSprites[i].fkind == fkind)
        {
            return sSC1PRemixAllstarFinalSprites[i].sprites;
        }
    }
    return NULL;
}

static void sc1PRemixAllstarFinalRemember(const FTStruct *fp)
{
    FTSprites *sprites = sc1PRemixAllstarFinalGetSprites(fp);

    if ((sprites == NULL) || (sc1PRemixAllstarFinalFindSprites(fp->fkind) != NULL))
    {
        return;
    }
    if (sSC1PRemixAllstarFinalSpritesCount >= SC1PREMIX_ALLSTAR_FINAL_SPRITES_MAX)
    {
        return;
    }
    ifCommonPlayerStockPrepareSprite((Sprite*)PORT_RESOLVE(sprites->stock_sprite));

    sSC1PRemixAllstarFinalSprites[sSC1PRemixAllstarFinalSpritesCount].fkind = fp->fkind;
    sSC1PRemixAllstarFinalSprites[sSC1PRemixAllstarFinalSpritesCount].sprites = sprites;
    sSC1PRemixAllstarFinalSpritesCount++;
}

void sc1PRemixAllstarFinalIconsSetup(void)
{
    s32 player;

    sSC1PRemixAllstarFinalSpritesCount = 0;

    if (sc1PRemixAllstarFinalIsActive() == FALSE)
    {
        return;
    }
    for (player = 0; player < ARRAY_COUNT(gSCManagerBattleState->players); player++)
    {
        if ((gSCManagerBattleState->players[player].pkind == nFTPlayerKindNot) || (gSCManagerBattleState->players[player].is_spgame_enemy == FALSE))
        {
            continue;
        }
        if (gSCManagerBattleState->players[player].fighter_gobj == NULL)
        {
            continue;
        }
        sc1PRemixAllstarFinalRemember(ftGetStruct(gSCManagerBattleState->players[player].fighter_gobj));
    }
}

sb32 sc1PRemixAllstarFinalIconsSetStock(SObj *sobj, s32 member)
{
    FTSprites *sprites;
    u32 *luts;
    u16 attr;

    if (sc1PRemixAllstarFinalIsActive() == FALSE)
    {
        return FALSE;
    }
    sprites = sc1PRemixAllstarFinalFindSprites(sc1PRemixAllstarFinalFkind(member));

    if (sprites == NULL)
    {
        return FALSE;
    }
    luts = (u32*)PORT_RESOLVE(sprites->stock_luts);
    attr = sobj->sprite.attr;

    sobj->sprite = *(Sprite*)PORT_RESOLVE(sprites->stock_sprite);
    sobj->sprite.LUT = luts[sc1PRemixAllstarFinalCostume(member)];
    sobj->sprite.attr = attr;

    return TRUE;
}

/** Puts the series logo of a fighter in the panel, keeping the scale, tint and visibility the panel gave its logo. */
static void sc1PRemixAllstarFinalSetSeriesLogo(GObj *panel_gobj, const FTSprites *sprites)
{
    SObj *sobj = SObjGetStruct(panel_gobj);
    Sprite *emblem = (Sprite*)PORT_RESOLVE(sprites->emblem);
    Sprite kept;

    if ((sobj == NULL) || (emblem == NULL))
    {
        return;
    }
    ifCommonPlayerStockPrepareSprite(emblem);

    kept = sobj->sprite;
    sobj->sprite = *emblem;
    sobj->sprite.attr = kept.attr;
    sobj->sprite.scalex = kept.scalex;
    sobj->sprite.scaley = kept.scaley;
    sobj->sprite.red = kept.red;
    sobj->sprite.green = kept.green;
    sobj->sprite.blue = kept.blue;
}

/** Puts the stock icon of a fighter, with its costume palette, in the single stock icon of the panel. */
static void sc1PRemixAllstarFinalSetPanelStock(GObj *stock_gobj, const FTSprites *sprites, s32 costume)
{
    SObj *sobj = SObjGetStruct(stock_gobj);
    u32 *luts = (u32*)PORT_RESOLVE(sprites->stock_luts);
    u16 attr;

    if (sobj == NULL)
    {
        return;
    }
    attr = sobj->sprite.attr;

    sobj->sprite = *(Sprite*)PORT_RESOLVE(sprites->stock_sprite);
    sobj->sprite.LUT = luts[costume];
    sobj->sprite.attr = attr;
}

void sc1PRemixAllstarFinalIconsOnSpawn(s32 player)
{
    GObj *fighter_gobj;
    GObj *interface_gobj;
    FTStruct *fp;
    FTSprites *sprites;

    if (sc1PRemixAllstarFinalIsActive() == FALSE)
    {
        return;
    }
    fighter_gobj = gSCManagerBattleState->players[player].fighter_gobj;

    if (fighter_gobj == NULL)
    {
        return;
    }
    fp = ftGetStruct(fighter_gobj);
    sprites = sc1PRemixAllstarFinalGetSprites(fp);

    if (sprites == NULL)
    {
        return;
    }
    for (interface_gobj = gGCCommonLinks[nGCCommonLinkIDInterface]; interface_gobj != NULL; interface_gobj = interface_gobj->link_next)
    {
        if (ifGetPlayer(interface_gobj) != player)
        {
            continue;
        }
        if (interface_gobj->proc_display == ifCommonPlayerDamageProcDisplay)
        {
            sc1PRemixAllstarFinalSetSeriesLogo(interface_gobj, sprites);
        }
        else if (interface_gobj->proc_display == ifCommonPlayerStockSingleProcDisplay)
        {
            sc1PRemixAllstarFinalSetPanelStock(interface_gobj, sprites, fp->costume);
        }
    }
}

#endif
