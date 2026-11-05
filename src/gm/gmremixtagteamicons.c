/**
 * gmremixtagteamicons.c - stock icons of the Tag Team queue, see gmremixtagteamicons.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <if/ifcommon.h>
#include <lb/library.h>
#include <reloc_data.h>
#include <sys/taskman.h>
#include "fighter_registry.h"
#include "gm/gmremixtagteam.h"
#include "gm/gmremixtagteamicons.h"

extern void portFixupFTAttributes(void *attr);

typedef struct GMRemixTagTeamIcon
{
    s32 fkind;
    FTSprites *sprites;
    s32 costume;
    sb32 is_classic;
} GMRemixTagTeamIcon;

static GMRemixTagTeamIcon sGMRemixTagTeamIcons[GMCOMMON_PLAYERS_MAX][GMREMIX_TAGTEAM_QUEUE_MAX];

void gmRemixTagTeamIconsReset(void)
{
    s32 port;
    s32 slot;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        for (slot = 0; slot < GMREMIX_TAGTEAM_QUEUE_MAX; slot++)
        {
            sGMRemixTagTeamIcons[port][slot].fkind = -1;
            sGMRemixTagTeamIcons[port][slot].sprites = NULL;
            sGMRemixTagTeamIcons[port][slot].costume = 0;
            sGMRemixTagTeamIcons[port][slot].is_classic = FALSE;
        }
    }
}

/* The sprites of a character already resolved for another slot, so its main file is read once per match. */
static FTSprites* gmRemixTagTeamIconsFindResolved(s32 fkind, sb32 is_classic)
{
    s32 port;
    s32 slot;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        for (slot = 0; slot < GMREMIX_TAGTEAM_QUEUE_MAX; slot++)
        {
            if ((sGMRemixTagTeamIcons[port][slot].fkind == fkind) && (sGMRemixTagTeamIcons[port][slot].is_classic == is_classic)
                && (sGMRemixTagTeamIcons[port][slot].sprites != NULL))
            {
                return sGMRemixTagTeamIcons[port][slot].sprites;
            }
        }
    }
    return NULL;
}

/* Main file of the character: the loaded one when the character is in the match, else a private copy that the swap never uses.
 * A Classic Sonic slot reads the classic main file (the special 2 file, Sonic.asm load_classic_model_). */
static void* gmRemixTagTeamIconsGetMainFile(FTData *data, sb32 is_classic)
{
    void **p_file = (is_classic != FALSE) ? data->p_file_special2 : data->p_file_main;
    u32 file_id = (is_classic != FALSE) ? data->file_special2_id : data->file_main_id;

    if (*p_file != NULL)
    {
        return *p_file;
    }
    return lbRelocGetExternHeapFile(file_id, syTaskmanMalloc(lbRelocGetFileSize(file_id), 0x10));
}

static FTSprites* gmRemixTagTeamIconsResolveSprites(s32 fkind, sb32 is_classic)
{
    FTSprites *sprites = gmRemixTagTeamIconsFindResolved(fkind, is_classic);
    FTData *data;
    FTAttributes *attr;

    if (sprites != NULL)
    {
        return sprites;
    }
    data = port_fighter_data(fkind);

    if (data == NULL)
    {
        return NULL;
    }
    attr = lbRelocGetFileData(FTAttributes*, gmRemixTagTeamIconsGetMainFile(data, is_classic), data->o_attributes);
    portFixupFTAttributes(attr);
    sprites = (FTSprites*)PORT_RESOLVE(attr->sprites);

    if ((sprites == NULL) || (PORT_RESOLVE(sprites->stock_sprite) == NULL))
    {
        return NULL;
    }
    ifCommonPlayerStockPrepareSprite((Sprite*)PORT_RESOLVE(sprites->stock_sprite));

    return sprites;
}

void gmRemixTagTeamIconsSet(s32 port, s32 slot, s32 fkind, s32 costume, sb32 is_classic)
{
    GMRemixTagTeamIcon *icon = &sGMRemixTagTeamIcons[port][slot];

    icon->fkind = fkind;
    icon->costume = costume;
    icon->is_classic = is_classic;
    icon->sprites = gmRemixTagTeamIconsResolveSprites(fkind, is_classic);
}

sb32 gmRemixTagTeamIconsApply(SObj *sobj, s32 port, s32 slot)
{
    GMRemixTagTeamIcon *icon = &sGMRemixTagTeamIcons[port][slot];
    u32 *luts;

    if (icon->sprites == NULL)
    {
        return FALSE;
    }
    luts = (u32*)PORT_RESOLVE(icon->sprites->stock_luts);
    sobj->sprite = *(Sprite*)PORT_RESOLVE(icon->sprites->stock_sprite);
    sobj->sprite.LUT = luts[icon->costume];

    return TRUE;
}

#endif
