/**
 * mnremixtagteamcssicons.c - icon rows of the Tag Team queue on the VS character select (TagTeam.asm css.setup_ and
 * slot_icon_update_routine_), see mnremixtagteamcss.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <if/ifcommon.h>
#include <lb/library.h>
#include <reloc_data.h>
#include <sys/obj.h>
#include <sys/objman.h>
#include <sys/taskman.h>
#include "fighter_registry.h"
#include "if/ifremixsprite.h"
#include "mn/mnplayers/mnremixtagteamcssicons.h"

extern void portFixupFTAttributes(void *attr);

/* The default icon is the stock sprite of the 1P CSS: file 0x19, image footer at 0x80 (setup_: Render.load_file(0x0019)). */
#define MNREMIX_TAGTEAM_CSS_DOT_FILE_ID 0x19
#define MNREMIX_TAGTEAM_CSS_DOT_OFFSET 0x80
#define MNREMIX_TAGTEAM_CSS_GOBJ_LINK 22
#define MNREMIX_TAGTEAM_CSS_DL_LINK 28
#define MNREMIX_TAGTEAM_CSS_PORTS 4
#define MNREMIX_TAGTEAM_CSS_RESOLVED_MAX 32
#define MNREMIX_TAGTEAM_CSS_BOSS_SCALE 0.625F

typedef struct MNRemixTagTeamCssResolved
{
    s32 fkind;
    sb32 is_classic;
    FTSprites *sprites;
} MNRemixTagTeamCssResolved;

static GObj *sMNRemixTagTeamCssIcons[MNREMIX_TAGTEAM_CSS_PORTS][MNREMIX_TAGTEAM_CSS_SLOTS];
static Sprite sMNRemixTagTeamCssDotSprite;
static const MNRemixTagTeamCssQueue *sMNRemixTagTeamCssQueues;
static MNRemixTagTeamCssPortInfoFunc sMNRemixTagTeamCssInfoFunc;

/* Sprites of the fighters of the queues. They come from a private copy of the main file, because the CSS replaces the main file of
 * the fighter of a panel while a token moves over the grid (the asm copies the icon texture and palettes into icon_cache). */
static MNRemixTagTeamCssResolved sMNRemixTagTeamCssResolved[MNREMIX_TAGTEAM_CSS_RESOLVED_MAX];
static s32 sMNRemixTagTeamCssResolvedCount;

static FTSprites* mnRemixTagTeamCssIconsLoadSprites(s32 fkind, sb32 is_classic)
{
    FTData *data = port_fighter_data(fkind);
    u32 file_id;
    void *file;
    FTAttributes *attr;
    FTSprites *sprites;

    if (data == NULL)
    {
        return NULL;
    }
    file_id = (is_classic != FALSE) ? data->file_special2_id : data->file_main_id;
    file = lbRelocGetExternHeapFile(file_id, syTaskmanMalloc(lbRelocGetFileSize(file_id), 0x10));
    attr = lbRelocGetFileData(FTAttributes*, file, data->o_attributes);
    portFixupFTAttributes(attr);
    sprites = (FTSprites*)PORT_RESOLVE(attr->sprites);

    if ((sprites == NULL) || (PORT_RESOLVE(sprites->stock_sprite) == NULL))
    {
        return NULL;
    }
    ifCommonPlayerStockPrepareSprite((Sprite*)PORT_RESOLVE(sprites->stock_sprite));

    return sprites;
}

static FTSprites* mnRemixTagTeamCssIconsGetSprites(s32 fkind, sb32 is_classic)
{
    s32 i;

    for (i = 0; i < sMNRemixTagTeamCssResolvedCount; i++)
    {
        if ((sMNRemixTagTeamCssResolved[i].fkind == fkind) && (sMNRemixTagTeamCssResolved[i].is_classic == is_classic))
        {
            return sMNRemixTagTeamCssResolved[i].sprites;
        }
    }
    if (sMNRemixTagTeamCssResolvedCount >= MNREMIX_TAGTEAM_CSS_RESOLVED_MAX)
    {
        return NULL;
    }
    sMNRemixTagTeamCssResolved[sMNRemixTagTeamCssResolvedCount].fkind = fkind;
    sMNRemixTagTeamCssResolved[sMNRemixTagTeamCssResolvedCount].is_classic = is_classic;
    sMNRemixTagTeamCssResolved[sMNRemixTagTeamCssResolvedCount].sprites = mnRemixTagTeamCssIconsLoadSprites(fkind, is_classic);

    return sMNRemixTagTeamCssResolved[sMNRemixTagTeamCssResolvedCount++].sprites;
}

/** Costume of the icon of a slot: the team costume in a team battle, else the costume picked in an FFA (the default one for a slot picked in a team battle). */
static s32 mnRemixTagTeamCssIconsGetCostume(const MNRemixTagTeamCssSlot *slot, const MNRemixTagTeamCssPortInfo *info)
{
    if (info->is_team_battle != FALSE)
    {
        return ftParamGetCostumeTeamID(slot->fkind, info->team);
    }
    return (slot->is_team_costume != 0) ? 0 : slot->costume;
}

static void mnRemixTagTeamCssIconsSetDot(SObj *sobj)
{
    sobj->sprite = sMNRemixTagTeamCssDotSprite;
}

static void mnRemixTagTeamCssIconsSetStock(SObj *sobj, FTSprites *sprites, s32 costume, s32 fkind)
{
    u32 *luts = (u32*)PORT_RESOLVE(sprites->stock_luts);

    sobj->sprite = *(Sprite*)PORT_RESOLVE(sprites->stock_sprite);
    sobj->sprite.LUT = luts[costume];
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    if (fkind == nFTKindBoss)
    {
        sobj->sprite.scalex = MNREMIX_TAGTEAM_CSS_BOSS_SCALE;
        sobj->sprite.scaley = MNREMIX_TAGTEAM_CSS_BOSS_SCALE;
    }
}

/** The fighter on the panel while it is the one of the slot being picked: its own sprites are loaded and always valid. */
static FTSprites* mnRemixTagTeamCssIconsGetLiveSprites(const MNRemixTagTeamCssSlot *slot, const MNRemixTagTeamCssPortInfo *info, s32 *costume)
{
    FTStruct *fp;
    FTSprites *sprites;

    if (info->fighter_gobj == NULL)
    {
        return NULL;
    }
    fp = ftGetStruct(info->fighter_gobj);

    if ((fp == NULL) || (fp->fkind != slot->fkind))
    {
        return NULL;
    }
    sprites = (FTSprites*)PORT_RESOLVE(fp->attr->sprites);

    if ((sprites == NULL) || (PORT_RESOLVE(sprites->stock_sprite) == NULL))
    {
        return NULL;
    }
    /* The stock sprite of a loaded fighter is raw file data; the match HUD and the 12-Character Battle fix it when they make their sprite
     * object. The port fixups run once per address and are forgotten with the file, so repeating the call is safe. */
    ifCommonPlayerStockPrepareSprite((Sprite*)PORT_RESOLVE(sprites->stock_sprite));
    *costume = fp->costume;

    return sprites;
}

static void mnRemixTagTeamCssIconsUpdateIcon(s32 port, s32 icon, const MNRemixTagTeamCssPortInfo *info)
{
    GObj *gobj = sMNRemixTagTeamCssIcons[port][icon];
    SObj *sobj = SObjGetStruct(gobj);
    const MNRemixTagTeamCssQueue *queue = &sMNRemixTagTeamCssQueues[port];
    const MNRemixTagTeamCssSlot *slot = &queue->slots[icon];
    FTSprites *sprites = NULL;
    s32 costume = 0;

    if (slot->fkind != MNREMIX_TAGTEAM_CSS_NONE)
    {
        if (icon == queue->current_slot)
        {
            sprites = mnRemixTagTeamCssIconsGetLiveSprites(slot, info, &costume);
        }
        if (sprites == NULL)
        {
            costume = mnRemixTagTeamCssIconsGetCostume(slot, info);
            sprites = mnRemixTagTeamCssIconsGetSprites(slot->fkind, (slot->is_classic != 0) ? TRUE : FALSE);
        }
    }
    sobj->pos.x = mnRemixTagTeamCssRulesIconX(info->panel_x, icon);
    sobj->pos.y = (f32)MNREMIX_TAGTEAM_CSS_ICON_Y;

    if (sprites == NULL)
    {
        mnRemixTagTeamCssIconsSetDot(sobj);
        return;
    }
    mnRemixTagTeamCssIconsSetStock(sobj, sprites, costume, slot->fkind);
}

static void mnRemixTagTeamCssIconsProcUpdate(GObj *gobj)
{
    MNRemixTagTeamCssPortInfo info;
    s32 port;
    s32 icon;
    s32 count;

    for (port = 0; port < MNREMIX_TAGTEAM_CSS_PORTS; port++)
    {
        sMNRemixTagTeamCssInfoFunc(port, &info);
        count = mnRemixTagTeamCssRulesIconCount(info.stock_count);

        for (icon = 0; icon < MNREMIX_TAGTEAM_CSS_SLOTS; icon++)
        {
            if ((info.is_in_play == FALSE) || (icon >= count))
            {
                sMNRemixTagTeamCssIcons[port][icon]->flags = GOBJ_FLAG_HIDDEN;
                continue;
            }
            sMNRemixTagTeamCssIcons[port][icon]->flags = GOBJ_FLAG_NONE;
            mnRemixTagTeamCssIconsUpdateIcon(port, icon, &info);
        }
    }
}

void mnRemixTagTeamCssIconsMake(const MNRemixTagTeamCssQueue *queues, MNRemixTagTeamCssPortInfoFunc info_func)
{
    void *dot_file = ifRemixSpriteGetFile(MNREMIX_TAGTEAM_CSS_DOT_FILE_ID);
    s32 port;
    s32 icon;

    if (dot_file == NULL)
    {
        return;
    }
    sMNRemixTagTeamCssQueues = queues;
    sMNRemixTagTeamCssInfoFunc = info_func;
    sMNRemixTagTeamCssResolvedCount = 0;

    for (port = 0; port < MNREMIX_TAGTEAM_CSS_PORTS; port++)
    {
        for (icon = 0; icon < MNREMIX_TAGTEAM_CSS_SLOTS; icon++)
        {
            GObj *gobj = ifRemixSpriteMakeFromFile(dot_file, MNREMIX_TAGTEAM_CSS_DOT_OFFSET, MNREMIX_TAGTEAM_CSS_GOBJ_LINK, MNREMIX_TAGTEAM_CSS_DL_LINK, 0.0F,
                                                   (f32)MNREMIX_TAGTEAM_CSS_ICON_Y, 0xFFFFFFFF, 0x000000FF, 1.0F);

            if (gobj == NULL)
            {
                return;
            }
            sMNRemixTagTeamCssIcons[port][icon] = gobj;
            sMNRemixTagTeamCssDotSprite = SObjGetStruct(gobj)->sprite;
        }
    }
    gcAddGObjProcess(gcMakeGObjSPAfter(0, NULL, MNREMIX_TAGTEAM_CSS_GOBJ_LINK, GOBJ_PRIORITY_DEFAULT), mnRemixTagTeamCssIconsProcUpdate, nGCProcessKindFunc, 1);
}

#endif
