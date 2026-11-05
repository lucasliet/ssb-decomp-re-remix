/**
 * ifremixtagteamhud.c - HUD refresh after a Tag Team swap, see ifremixtagteamhud.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <if/interface.h>
#include <ft/fighter.h>
#include <gr/ground.h>
#include <lb/library.h>
#include <sc/scene.h>
#include <sys/objman.h>
#include "if/ifremixtagteamhud.h"

extern IFPlayerDamage sIFCommonPlayerDamageInterface[GMCOMMON_PLAYERS_MAX];
extern s8 sIFCommonPlayerStocksNum[GMCOMMON_PLAYERS_MAX];
extern s32 dIFCommonPlayerDamageEmblemOffsetsX[GMCOMMON_PLAYERS_MAX];
extern s32 dIFCommonPlayerDamageEmblemOffsetsY[GMCOMMON_PLAYERS_MAX];
extern f32 dIFCommonPlayerDamageEmblemScales[GMCOMMON_PLAYERS_MAX];

/* The emblem goes through lbCommonMakeSObjForGObj on a scratch GObj so it gets the same sprite fixups as in ifCommonPlayerDamageInitInterface. */
static void ifRemixTagTeamCopyEmblemSprite(Sprite *emblem, Sprite *destination)
{
    GObj *scratch_gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, nGCCommonLinkIDInterface, GOBJ_PRIORITY_DEFAULT);
    SObj *scratch_sobj = lbCommonMakeSObjForGObj(scratch_gobj, emblem);

    *destination = scratch_sobj->sprite;
    gcEjectGObj(scratch_gobj);
}

static void ifRemixTagTeamSetEmblem(s32 player, FTStruct *fp)
{
    GObj *interface_gobj = sIFCommonPlayerDamageInterface[player].interface_gobj;
    FTSprites *ft_sprites = (FTSprites*)PORT_RESOLVE(fp->attr->sprites);
    SObj *sobj;
    s32 color_id;

    if (interface_gobj == NULL)
    {
        return;
    }
    sobj = SObjGetStruct(interface_gobj);

    if ((ft_sprites == NULL) || (PORT_RESOLVE(ft_sprites->emblem) == NULL))
    {
        sobj->sprite.attr |= SP_HIDDEN;
        return;
    }
    ifRemixTagTeamCopyEmblemSprite((Sprite*)PORT_RESOLVE(ft_sprites->emblem), &sobj->sprite);

    sobj->pos.x = (s32)((gIFCommonPlayerInterface.player_pos_x[player] - (sobj->sprite.width * dIFCommonPlayerDamageEmblemScales[player] * 0.5F)) + dIFCommonPlayerDamageEmblemOffsetsX[player]);
    sobj->pos.y = (s32)((gIFCommonPlayerInterface.player_pos_y - (sobj->sprite.height * dIFCommonPlayerDamageEmblemScales[player] * 0.5F)) + dIFCommonPlayerDamageEmblemOffsetsY[player]);
    sobj->sprite.scalex = sobj->sprite.scaley = dIFCommonPlayerDamageEmblemScales[player];

    color_id = gSCManagerBattleState->players[player].color;
    sobj->sprite.red = gMPCollisionGroundData->emblem_colors[color_id].r;
    sobj->sprite.green = gMPCollisionGroundData->emblem_colors[color_id].g;
    sobj->sprite.blue = gMPCollisionGroundData->emblem_colors[color_id].b;
    sobj->sprite.attr = SP_TEXSHUF | SP_TRANSPARENT;
}

void ifRemixTagTeamRefreshHud(s32 player, GObj *fighter_gobj)
{
    ifRemixTagTeamSetEmblem(player, ftGetStruct(fighter_gobj));
    sIFCommonPlayerStocksNum[player] = S8_MAX;
}

#endif
