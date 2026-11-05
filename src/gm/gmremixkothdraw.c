/**
 * gmremixkothdraw.c - colour of the hill of Remix King of the Hill (KingOfTheHill.asm render_ground_gfx_) and the flat strip that
 * stands in for the model when the Remix file 0x14F3 is not available (gmremixkothmodel.c draws the model: one piece per segment of
 * the hill line and the marker). The flat strip is a translucent wall, 128 high and fading out upward, along the vertices of the
 * hill line.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/gbi.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/taskman.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothdraw.h"
#include "gm/gmremixkothhill.h"
#include "gm/gmremixkothmodel.h"
#include "gm/gmremixkothrules.h"

#define GMREMIX_KOTH_STRIP_TOP 128.0F
#define GMREMIX_KOTH_STRIP_BOTTOM 0.0F
#define GMREMIX_KOTH_STRIP_VERTICES_MAX 32
#define GMREMIX_KOTH_STRIP_ALPHA_BASE 0xBF
#define GMREMIX_KOTH_STRIP_ALPHA_TOP 0x20
#define GMREMIX_KOTH_STRIP_DISPLAY_LINK 6

typedef struct GMRemixKothColor
{
    u8 r, g, b;
} GMRemixKothColor;

/* render_ground_gfx_: white without a king, the Color.tag of the king otherwise, a gray that depends on the contest buffer while
 * the hill is contested (the buffer wins over the king). */
static const GMRemixKothColor dGMRemixKothWhite = { 0xFF, 0xFF, 0xFF };

/* use_cpu_color: a free-for-all team is a CPU when the fighter of that team is one. */
static sb32 gmRemixKothKeyIsCpu(s32 key)
{
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        if ((gSCManagerBattleState->players[port].pkind != nFTPlayerKindNot) && (gSCManagerBattleState->players[port].team == key))
        {
            return (gSCManagerBattleState->players[port].pkind == nFTPlayerKindCom) ? TRUE : FALSE;
        }
    }
    return FALSE;
}

static GMRemixKothColor gmRemixKothGetStripColor(void)
{
    s32 king = gmRemixKothGetKing();
    s32 buffer = gmRemixKothGetContestedBuffer();
    GMRemixKothColor color;
    u32 rgba;

    if (buffer != 0)
    {
        color.r = color.g = color.b = (u8)gmRemixKothRulesContestedGray(buffer);
        return color;
    }
    if (king < 0)
    {
        return dGMRemixKothWhite;
    }
    rgba = gmRemixKothRulesTagColor(gmRemixKothRulesTag(king, (gSCManagerBattleState->is_team_battle != FALSE) ? 1 : 0,
                                                       (gmRemixKothKeyIsCpu(king) != FALSE) ? 1 : 0));
    color.r = (rgba >> 24) & 0xFF;
    color.g = (rgba >> 16) & 0xFF;
    color.b = (rgba >> 8) & 0xFF;

    return color;
}

static Vtx sGMRemixKothStripVtx[GMREMIX_KOTH_STRIP_VERTICES_MAX * 2];

/* The prim colour paints the strip; the vertices only carry the alpha that fades from the floor to the top (model vertices of
 * the Remix hill, alpha 0xBF at the bottom and 0x20 at the top). */
static void gmRemixKothFillStrip(Vtx *vtx, s32 line_id, s32 count)
{
    s32 i;

    for (i = 0; i < count; i++)
    {
        f32 x, y;

        gmRemixKothHillGetVertex(line_id, i, &x, &y);

        vtx[(i * 2) + 0].v.ob[0] = (s16)x;
        vtx[(i * 2) + 0].v.ob[1] = (s16)(y + GMREMIX_KOTH_STRIP_TOP);
        vtx[(i * 2) + 1].v.ob[0] = (s16)x;
        vtx[(i * 2) + 1].v.ob[1] = (s16)(y + GMREMIX_KOTH_STRIP_BOTTOM);
        vtx[(i * 2) + 0].v.ob[2] = vtx[(i * 2) + 1].v.ob[2] = 0;
        vtx[(i * 2) + 0].v.flag = vtx[(i * 2) + 1].v.flag = 0;
        vtx[(i * 2) + 0].v.tc[0] = vtx[(i * 2) + 1].v.tc[0] = 0;
        vtx[(i * 2) + 0].v.tc[1] = vtx[(i * 2) + 1].v.tc[1] = 0;
        vtx[(i * 2) + 0].v.cn[0] = vtx[(i * 2) + 1].v.cn[0] = 0xFF;
        vtx[(i * 2) + 0].v.cn[1] = vtx[(i * 2) + 1].v.cn[1] = 0xFF;
        vtx[(i * 2) + 0].v.cn[2] = vtx[(i * 2) + 1].v.cn[2] = 0xFF;
        vtx[(i * 2) + 0].v.cn[3] = GMREMIX_KOTH_STRIP_ALPHA_TOP;
        vtx[(i * 2) + 1].v.cn[3] = GMREMIX_KOTH_STRIP_ALPHA_BASE;
    }
}

static void gmRemixKothEmitStrip(Vtx *vtx, s32 count, GMRemixKothColor color)
{
    s32 i;

    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, color.r, color.g, color.b, 0xFF);
    gDPSetCombineLERP(gSYTaskmanDLHeads[0]++, 0, 0, 0, PRIMITIVE, SHADE, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE, SHADE, 0, PRIMITIVE, 0);
    gSPTexture(gSYTaskmanDLHeads[0]++, 0, 0, 0, G_TX_RENDERTILE, G_OFF);
    gSPClearGeometryMode(gSYTaskmanDLHeads[0]++, G_LIGHTING | G_CULL_BOTH | G_ZBUFFER);
    gSPSetGeometryMode(gSYTaskmanDLHeads[0]++, G_SHADE | G_SHADING_SMOOTH);
    gSPVertex(gSYTaskmanDLHeads[0]++, vtx, count * 2, 0);

    for (i = 0; i < count - 1; i++)
    {
        gSP2Triangles(gSYTaskmanDLHeads[0]++, i * 2, (i * 2) + 1, (i * 2) + 2, 0, (i * 2) + 1, (i * 2) + 3, (i * 2) + 2, 0);
    }
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
}

static void gmRemixKothStripDisplay(GObj *gobj)
{
    s32 line_id = gmRemixKothGetHillLine();
    s32 count;

    if ((line_id < 0) || (gmRemixKothHillLineIsUsable(line_id) == FALSE))
    {
        return;
    }
    count = gmRemixKothHillGetVertexCount(line_id);

    if (count > GMREMIX_KOTH_STRIP_VERTICES_MAX)
    {
        count = GMREMIX_KOTH_STRIP_VERTICES_MAX;
    }
    gmRemixKothFillStrip(sGMRemixKothStripVtx, line_id, count);
    gmRemixKothEmitStrip(sGMRemixKothStripVtx, count, gmRemixKothGetStripColor());
}

void gmRemixKothDrawGetPrimColor(u8 *r, u8 *g, u8 *b)
{
    GMRemixKothColor color = gmRemixKothGetStripColor();

    *r = color.r;
    *g = color.g;
    *b = color.b;
}

void gmRemixKothDrawMake(void)
{
    if (gmRemixKothModelMake() != FALSE)
    {
        return;
    }
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindEffect, NULL, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, gmRemixKothStripDisplay, GMREMIX_KOTH_STRIP_DISPLAY_LINK, GOBJ_PRIORITY_DEFAULT, ~0);
}

#endif
