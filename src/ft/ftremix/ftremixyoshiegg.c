/**
 * ftremixyoshiegg.c - colour of the Yoshi egg shield (Shield.asm alter_yoshi_egg_color_ _shield and _shield_roll).
 *
 * The egg shield display list loads its 16 colour palette with a G_SETTIMG at byte 0x50 (command 10). The asm overwrites that command
 * with a call to segment 0xE offset 0 (`DE000000 0E000000`), where gcDrawMObjForDObj puts the display list of the DObj's first MObj,
 * and attaches an MObj with a 16 entry palette array (Render.MODEL_PART_IMAGE_INIT_ is gcAddMObjForDObj) whose palette index is the
 * float at MObj +0x88 (MObj.palette_id). The MObj's display list is the G_SETTIMG of the palette that the index picks.
 *
 * The port does the same on the first draw of the shield (efManagerYoshiShieldProcDisplay), and sets the index every frame. The
 * display list is shared file data, but every DObj that draws it goes through that proc, so each one gets its own MObj.
 *
 * The thrown egg (0x8015EAF4), the egg lay egg (0x8014D080, 0x8014C984) and the entry egg (0x80102F5C) patch other display lists with
 * the same technique; they are not wired in the port.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <gm/gmremixpanel.h>
#include <port_log.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixpartimage.h"
#include "ft/ftremix/ftremixpartimagepalette.h"
#include "ft/ftremix/ftremixpartimagerules.h"

#define FTREMIX_EGG_PALETTE_LOAD_COMMAND 10
#define FTREMIX_EGG_TLUT_LOAD_COMMAND 12
#define FTREMIX_EGG_GBI_SETTIMG_RGBA16 0xFD100000u
#define FTREMIX_EGG_GBI_LOADTLUT_OPCODE 0xF0u
#define FTREMIX_EGG_GBI_CALL_SEGMENT_E 0xDE000000u
#define FTREMIX_EGG_SEGMENT_E_START 0x0E000000u

static MObjSub sFTRemixYoshiEggMObjSub;

/**
 * Replaces the palette G_SETTIMG of an egg shield display list with a call to the MObj display list. The display list is packed
 * N64 data (two u32 per command, native word order). Returns TRUE when the list is patched, now or before, and FALSE when it does not
 * look like the egg shield list, which then keeps its own palette.
 */
static sb32 ftRemixYoshiEggPatchPaletteLoad(Gfx *dl)
{
    u32 *palette_load = (u32*)dl + (FTREMIX_EGG_PALETTE_LOAD_COMMAND * 2);
    u32 *tlut_load = (u32*)dl + (FTREMIX_EGG_TLUT_LOAD_COMMAND * 2);

    if ((palette_load[0] == FTREMIX_EGG_GBI_CALL_SEGMENT_E) && (palette_load[1] == FTREMIX_EGG_SEGMENT_E_START))
    {
        return TRUE;
    }
    if ((palette_load[0] != FTREMIX_EGG_GBI_SETTIMG_RGBA16) || ((tlut_load[0] >> 24) != FTREMIX_EGG_GBI_LOADTLUT_OPCODE))
    {
        return FALSE;
    }
    palette_load[0] = FTREMIX_EGG_GBI_CALL_SEGMENT_E;
    palette_load[1] = FTREMIX_EGG_SEGMENT_E_START;

    return TRUE;
}

/** The egg shield MObj of a DObj, made on first use with the palette array `palettes`. */
static MObj* ftRemixYoshiEggGetMObj(DObj *dobj, u32 palettes)
{
    if (dobj->mobj == NULL)
    {
        sFTRemixYoshiEggMObjSub.flags = MOBJ_FLAG_PALETTE;
        sFTRemixYoshiEggMObjSub.palettes = palettes;

        gcAddMObjForDObj(dobj, &sFTRemixYoshiEggMObjSub);
    }
    dobj->mobj->sub.palettes = palettes;

    return dobj->mobj;
}

/** Colours the egg shield of a fighter by its Shield option. */
void ftRemixPartImageYoshiEggShieldApply(GObj *effect_gobj, FTStruct *fp)
{
    DObj *dobj = DObjGetStruct(effect_gobj);
    u32 palettes;
    MObj *mobj;

    if ((dobj == NULL) || (dobj->dl == NULL))
    {
        return;
    }
    palettes = ftRemixPartImageGetPaletteArrayToken(REMIX_SYM_Shield__alter_yoshi_egg_color___table_palettes, FTREMIX_PARTIMAGE_EGG_PALETTE_COUNT);

    if (palettes == 0)
    {
        return;
    }
    if (!ftRemixYoshiEggPatchPaletteLoad(dobj->dl))
    {
        return;
    }
    mobj = ftRemixYoshiEggGetMObj(dobj, palettes);
    mobj->palette_id = ftRemixPartImageRulesEggPaletteIndex(gmRemixPanel(SHIELD, fp->player), ftRemixShieldGetCostumeColorIndex(fp));
}

#endif
