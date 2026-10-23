/**
 * gmremixkothdraw.c - hill strip of Remix King of the Hill (KingOfTheHill.asm render_ground_gfx_). The Remix builds the
 * strip from a Remix-only model file; the port draws it as a flat coloured band along the vertices of the hill line.
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

#define GMREMIX_KOTH_STRIP_TOP 12.0F
#define GMREMIX_KOTH_STRIP_BOTTOM (-2.0F)
#define GMREMIX_KOTH_STRIP_VERTICES_MAX 32
#define GMREMIX_KOTH_STRIP_ALPHA 0xB0
#define GMREMIX_KOTH_STRIP_DISPLAY_LINK 6
#define GMREMIX_KOTH_CONTESTED_GRAY_BASE 0x60
#define GMREMIX_KOTH_CONTESTED_GRAY_STEP 3

typedef struct GMRemixKothColor
{
    u8 r, g, b;
} GMRemixKothColor;

/* Tag colours of KingOfTheHill.colors (red, blue, yellow, green) and team_colors (red, blue, green, yellow). */
static const GMRemixKothColor dGMRemixKothPortColors[GMCOMMON_PLAYERS_MAX] =
{
    { 0xE0, 0x40, 0x40 }, { 0x40, 0x60, 0xE0 }, { 0xE0, 0xD0, 0x30 }, { 0x40, 0xC0, 0x50 }
};
static const GMRemixKothColor dGMRemixKothTeamColors[GMCOMMON_PLAYERS_MAX] =
{
    { 0xE0, 0x40, 0x40 }, { 0x40, 0x60, 0xE0 }, { 0x40, 0xC0, 0x50 }, { 0xE0, 0xD0, 0x30 }
};
static const GMRemixKothColor dGMRemixKothWhite = { 0xFF, 0xFF, 0xFF };
static const GMRemixKothColor dGMRemixKothGray = { 0x88, 0x88, 0x88 };

static GMRemixKothColor gmRemixKothGetStripColor(void)
{
    s32 king = gmRemixKothGetKing();
    s32 buffer = gmRemixKothGetContestedBuffer();
    GMRemixKothColor color;

    if (buffer != 0)
    {
        color.r = color.g = color.b = GMREMIX_KOTH_CONTESTED_GRAY_BASE + (buffer * GMREMIX_KOTH_CONTESTED_GRAY_STEP);
        return color;
    }
    if (king < 0)
    {
        return dGMRemixKothWhite;
    }
    if (gSCManagerBattleState->is_team_battle != FALSE)
    {
        return dGMRemixKothTeamColors[king];
    }
    return (gSCManagerBattleState->players[king].pkind == nFTPlayerKindCom) ? dGMRemixKothGray : dGMRemixKothPortColors[king];
}

static Vtx sGMRemixKothStripVtx[GMREMIX_KOTH_STRIP_VERTICES_MAX * 2];

static void gmRemixKothFillStrip(Vtx *vtx, s32 line_id, s32 count, GMRemixKothColor color)
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
        vtx[(i * 2) + 0].v.cn[0] = vtx[(i * 2) + 1].v.cn[0] = color.r;
        vtx[(i * 2) + 0].v.cn[1] = vtx[(i * 2) + 1].v.cn[1] = color.g;
        vtx[(i * 2) + 0].v.cn[2] = vtx[(i * 2) + 1].v.cn[2] = color.b;
        vtx[(i * 2) + 0].v.cn[3] = vtx[(i * 2) + 1].v.cn[3] = GMREMIX_KOTH_STRIP_ALPHA;
    }
}

static void gmRemixKothEmitStrip(Vtx *vtx, s32 count)
{
    s32 i;

    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_SHADE, G_CC_SHADE);
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
    gmRemixKothFillStrip(sGMRemixKothStripVtx, line_id, count, gmRemixKothGetStripColor());
    gmRemixKothEmitStrip(sGMRemixKothStripVtx, count);
}

void gmRemixKothDrawMake(void)
{
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindEffect, NULL, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, gmRemixKothStripDisplay, GMREMIX_KOTH_STRIP_DISPLAY_LINK, GOBJ_PRIORITY_DEFAULT, ~0);
}

#endif
