/**
 * gmremixkothmodel.c - hill model of Remix King of the Hill (KingOfTheHill.asm create_ground_gfx_, update_ground_gfx_,
 * update_indicator_gfx_, render_ground_gfx_). See gmremixkothmodel.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/gbi.h>
#include <ft/fighter.h>
#include <ef/efmanager.h>
#include <sys/objanim.h>
#include <sys/objdisplay.h>
#include <sys/objman.h>
#include <sys/taskman.h>
#include <port_log.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothdraw.h"
#include "gm/gmremixkothhill.h"
#include "gm/gmremixkothmodel.h"
#include "if/ifremixsprite.h"
#include "remix/remix_debug.h"

#define GMREMIX_KOTH_MODEL_FILE_ID IFREMIX_SPRITE_FILE_KOTH
#define GMREMIX_KOTH_MODEL_STRIP_O_DOBJSETUP 0x400
#define GMREMIX_KOTH_MODEL_MARKER_O_DOBJSETUP 0xF50
#define GMREMIX_KOTH_MODEL_EFFECT_FLAGS 0x04
#define GMREMIX_KOTH_MODEL_DL_LINK 0x0B
#define GMREMIX_KOTH_MODEL_TRANSFORM_KIND 0x1C
#define GMREMIX_KOTH_MODEL_PIECES_MAX 32
#define GMREMIX_KOTH_MODEL_MARKER_RAISE 500.0F

static void *sGMRemixKothModelFile;
static GObj *sGMRemixKothModelPieces[GMREMIX_KOTH_MODEL_PIECES_MAX];
static s32 sGMRemixKothModelPieceCount;
static GObj *sGMRemixKothModelMarker;

/* render_ground_gfx_ puts the colour of the owner in the prim colour, then draws the joint tree. */
static void gmRemixKothModelDisplay(GObj *gobj)
{
    u8 r, g, b;

    gmRemixKothDrawGetPrimColor(&r, &g, &b);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, r, g, b, 0xFF);
    gcDrawDObjTreeForGObj(gobj);
}

/* The asm sets 0x14 of the GObject (func_run) to its update routine, which ends in the vanilla animation update (0x8000DF34). The
 * default effect routine would eject an effect without animation. */
static void gmRemixKothModelRun(GObj *gobj)
{
    gcPlayAnimAll(gobj);
}

/* koth_ground_gfx_struct and koth_indicator_gfx_struct: flags 0x04 (custom joint tree), display link 0x0B, matrix kinds 0x1C. */
static EFDesc sGMRemixKothModelStripDesc =
{
    .flags = GMREMIX_KOTH_MODEL_EFFECT_FLAGS,
    .dl_link = GMREMIX_KOTH_MODEL_DL_LINK,
    .file_head = &sGMRemixKothModelFile,
    .transform_types1 = { GMREMIX_KOTH_MODEL_TRANSFORM_KIND, 0, 0 },
    .transform_types2 = { GMREMIX_KOTH_MODEL_TRANSFORM_KIND, 0, 0 },
    .proc_update = NULL,
    .proc_display = gmRemixKothModelDisplay,
    .o_dobjsetup = GMREMIX_KOTH_MODEL_STRIP_O_DOBJSETUP
};

static EFDesc sGMRemixKothModelMarkerDesc =
{
    .flags = GMREMIX_KOTH_MODEL_EFFECT_FLAGS,
    .dl_link = GMREMIX_KOTH_MODEL_DL_LINK,
    .file_head = &sGMRemixKothModelFile,
    .transform_types1 = { GMREMIX_KOTH_MODEL_TRANSFORM_KIND, 0, 0 },
    .transform_types2 = { GMREMIX_KOTH_MODEL_TRANSFORM_KIND, 0, 0 },
    .proc_update = NULL,
    .proc_display = gmRemixKothModelDisplay,
    .o_dobjsetup = GMREMIX_KOTH_MODEL_MARKER_O_DOBJSETUP
};

static GObj *gmRemixKothModelMakeObject(EFDesc *effect_desc)
{
    GObj *gobj = efManagerMakeEffectForce(effect_desc);

    if (gobj != NULL)
    {
        gobj->func_run = gmRemixKothModelRun;
    }
    return gobj;
}

sb32 gmRemixKothModelMake(void)
{
    sGMRemixKothModelPieceCount = 0;
    sGMRemixKothModelMarker = NULL;
    sGMRemixKothModelFile = NULL;

    /* SSB64_KOTH_NO_MODEL=1 draws the flat strip instead, to tell the model apart from the rest of the stage in a capture. */
    if (remix_debug_env_int("SSB64_KOTH_NO_MODEL", 0) != 0)
    {
        return FALSE;
    }
    sGMRemixKothModelFile = ifRemixSpriteGetFile(GMREMIX_KOTH_MODEL_FILE_ID);

    if (sGMRemixKothModelFile == NULL)
    {
        port_log("koth: model file 0x%X is not available, the hill uses the flat strip\n", (unsigned int)GMREMIX_KOTH_MODEL_FILE_ID);
        return FALSE;
    }
    return TRUE;
}

static void gmRemixKothModelHideAll(void)
{
    s32 i;

    for (i = 0; i < sGMRemixKothModelPieceCount; i++)
    {
        sGMRemixKothModelPieces[i]->flags |= GOBJ_FLAG_HIDDEN;
    }
    if (sGMRemixKothModelMarker != NULL)
    {
        sGMRemixKothModelMarker->flags |= GOBJ_FLAG_HIDDEN;
    }
}

/* The asm creates one object per segment and keeps the ones it no longer needs hidden, so the pool never shrinks. */
static sb32 gmRemixKothModelEnsurePieces(s32 count)
{
    while (sGMRemixKothModelPieceCount < count)
    {
        GObj *gobj = gmRemixKothModelMakeObject(&sGMRemixKothModelStripDesc);

        if (gobj == NULL)
        {
            return FALSE;
        }
        sGMRemixKothModelPieces[sGMRemixKothModelPieceCount++] = gobj;
        port_log("koth: model piece %d made (gobj %p)\n", (int)sGMRemixKothModelPieceCount, (void *)gobj);
    }
    return TRUE;
}

/*
 * One segment: joint 1 sits on the start vertex, joint 2-0 on the end vertex relative to it, joint 3 (the end cap on the left) only
 * shows on the last piece and joint 2-1 (the end cap on the right) only on the first (create_ground_gfx_ _update_position).
 * The vertices are world positions, so the top joint stays at the origin instead of following the platform object.
 */
static void gmRemixKothModelPlacePiece(GObj *gobj, s32 line_id, s32 index, s32 segments)
{
    DObj *top = DObjGetStruct(gobj);
    DObj *joint_start = top->child;
    DObj *joint_end = joint_start->child;
    f32 x1, y1, x2, y2;

    if ((joint_start == NULL) || (joint_end == NULL) || (joint_end->child == NULL) || (joint_end->sib_next == NULL))
    {
        port_log("koth: the model piece has no joints 1, 2-0, 3 and 2-1, the file 0x%X does not match the asm\n",
                 (unsigned int)GMREMIX_KOTH_MODEL_FILE_ID);
        return;
    }
    gmRemixKothHillGetVertex(line_id, index, &x1, &y1);
    gmRemixKothHillGetVertex(line_id, index + 1, &x2, &y2);

    gobj->flags &= ~GOBJ_FLAG_HIDDEN;
    top->flags = DOBJ_FLAG_NONE;
    joint_start->translate.vec.f.x = x1;
    joint_start->translate.vec.f.y = y1;
    joint_end->translate.vec.f.x = x2 - x1;
    joint_end->translate.vec.f.y = y2 - y1;
    joint_end->child->flags = (index < (segments - 1)) ? DOBJ_FLAG_HIDDEN : DOBJ_FLAG_NONE;
    joint_end->sib_next->flags = (index != 0) ? DOBJ_FLAG_HIDDEN : DOBJ_FLAG_NONE;
}

static f32 gmRemixKothModelAbs(f32 value)
{
    return (value < 0.0F) ? -value : value;
}

/*
 * create_ground_gfx_ _create_indicator: the marker stands over the middle of the line (x halfway between the first and last vertex),
 * 500 above the line. The height is found like the asm does: walk the segments adding their widths until the half width of the line
 * is covered, then take the slope (height over width) of that segment from its start.
 */
static void gmRemixKothModelPlaceMarker(s32 line_id, s32 segments)
{
    f32 first_x, first_y, last_x, last_y;
    f32 half_width;
    f32 running = 0.0F;
    f32 height = 0.0F;
    f32 start_y = 0.0F;
    s32 i;
    DObj *top;
    DObj *texture_joint;

    gmRemixKothHillGetVertex(line_id, 0, &first_x, &first_y);
    gmRemixKothHillGetVertex(line_id, segments, &last_x, &last_y);
    half_width = (last_x - first_x) * 0.5F;

    for (i = 0; i < segments; i++)
    {
        f32 x1, y1, x2, y2;
        f32 width;

        gmRemixKothHillGetVertex(line_id, i, &x1, &y1);
        gmRemixKothHillGetVertex(line_id, i + 1, &x2, &y2);
        width = gmRemixKothModelAbs(x2 - x1);
        running += width;
        start_y = y1;
        height = 0.0F;

        if (width != 0.0F)
        {
            height = ((y2 - y1) / width) * (width - (running - gmRemixKothModelAbs(half_width)));
        }
        if (gmRemixKothModelAbs(half_width) <= running)
        {
            break;
        }
    }
    top = DObjGetStruct(sGMRemixKothModelMarker);
    texture_joint = top->child;

    if (texture_joint == NULL)
    {
        return;
    }

    sGMRemixKothModelMarker->flags &= ~GOBJ_FLAG_HIDDEN;
    top->flags = DOBJ_FLAG_NONE;
    texture_joint->flags = DOBJ_FLAG_NONE;
    texture_joint->translate.vec.f.x = first_x + half_width;
    texture_joint->translate.vec.f.y = start_y + GMREMIX_KOTH_MODEL_MARKER_RAISE + height;
}

void gmRemixKothModelSync(void)
{
    s32 line_id = gmRemixKothGetHillLine();
    s32 segments;
    s32 i;

    if ((sGMRemixKothModelFile == NULL) || (line_id < 0) || (gmRemixKothHillLineIsUsable(line_id) == FALSE))
    {
        gmRemixKothModelHideAll();
        return;
    }
    segments = gmRemixKothHillGetVertexCount(line_id) - 1;

    if (segments > GMREMIX_KOTH_MODEL_PIECES_MAX)
    {
        segments = GMREMIX_KOTH_MODEL_PIECES_MAX;
    }
    if (gmRemixKothModelEnsurePieces(segments) == FALSE)
    {
        gmRemixKothModelHideAll();
        return;
    }
    if (sGMRemixKothModelMarker == NULL)
    {
        sGMRemixKothModelMarker = gmRemixKothModelMakeObject(&sGMRemixKothModelMarkerDesc);
    }
    for (i = 0; i < sGMRemixKothModelPieceCount; i++)
    {
        if (i < segments)
        {
            gmRemixKothModelPlacePiece(sGMRemixKothModelPieces[i], line_id, i, segments);
        }
        else sGMRemixKothModelPieces[i]->flags |= GOBJ_FLAG_HIDDEN;
    }
    if (sGMRemixKothModelMarker != NULL)
    {
        gmRemixKothModelPlaceMarker(line_id, segments);
    }
}

sb32 gmRemixKothModelGetMarkerPosition(f32 *x, f32 *y)
{
    DObj *top;
    DObj *texture_joint;

    if ((sGMRemixKothModelMarker == NULL) || (sGMRemixKothModelMarker->flags & GOBJ_FLAG_HIDDEN))
    {
        return FALSE;
    }
    top = DObjGetStruct(sGMRemixKothModelMarker);
    texture_joint = top->child;

    if (texture_joint == NULL)
    {
        return FALSE;
    }
    *x = top->translate.vec.f.x + texture_joint->translate.vec.f.x;
    *y = top->translate.vec.f.y + texture_joint->translate.vec.f.y;
    return TRUE;
}

#endif
