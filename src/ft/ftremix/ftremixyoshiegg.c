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
 * the same technique, once when the egg is made (FTRemixYoshiEggSite gives the command and the branch of each list):
 *
 * - thrown egg: palette load at command 10, the colour of the Yoshi that throws it, as the shield;
 * - egg lay: palette load at command 11 of the third joint, the colour of the fighter that laid the egg (its shield option and
 *   costume), or the costume colour of the fighter in the egg when nobody laid it (Cruel Z Punish egg);
 * - entry egg: palette load at command 9 of the second joint, branch to 0x0E000008, palette array table_entry_palettes. The
 *   entry egg already has an MObj, which only gets the palette array and the palette flag.
 *
 * Character.yoshi_egg_color.table is never filled (see docs/remix/notes/i7_part_image.md), so the palette index is 0 unless the
 * Shield option is Costume.
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
#define FTREMIX_EGG_GBI_SETTIMG_RGBA16 0xFD100000u
#define FTREMIX_EGG_GBI_LOADTLUT_OPCODE 0xF0u
#define FTREMIX_EGG_GBI_CALL_SEGMENT_E 0xDE000000u
#define FTREMIX_EGG_SEGMENT_E_START 0x0E000000u
#define FTREMIX_EGG_SEGMENT_E_SECOND_ENTRY 0x0E000008u
#define FTREMIX_EGG_LAY_PALETTE_LOAD_COMMAND 11
#define FTREMIX_EGG_ENTRY_PALETTE_LOAD_COMMAND 9
#define FTREMIX_EGG_TLUT_SEARCH_COMMANDS 4

/* Where one egg display list loads its palette, which palettes the egg gets and where the replaced command branches to */
typedef struct FTRemixYoshiEggSite
{
    const char *name;
    u32 palettes_addr;
    s32 palette_load_command;
    u32 branch_address;

} FTRemixYoshiEggSite;

static const FTRemixYoshiEggSite sFTRemixYoshiEggShieldSite =
{
    "shield", REMIX_SYM_Shield__alter_yoshi_egg_color___table_palettes, FTREMIX_EGG_PALETTE_LOAD_COMMAND, FTREMIX_EGG_SEGMENT_E_START
};
static const FTRemixYoshiEggSite sFTRemixYoshiEggThrownSite =
{
    "thrown egg", REMIX_SYM_Shield__alter_yoshi_egg_color___table_palettes, FTREMIX_EGG_PALETTE_LOAD_COMMAND, FTREMIX_EGG_SEGMENT_E_START
};
static const FTRemixYoshiEggSite sFTRemixYoshiEggLaySite =
{
    "egg lay", REMIX_SYM_Shield__alter_yoshi_egg_color___table_palettes, FTREMIX_EGG_LAY_PALETTE_LOAD_COMMAND, FTREMIX_EGG_SEGMENT_E_START
};
static const FTRemixYoshiEggSite sFTRemixYoshiEggEntrySite =
{
    "entry egg", REMIX_SYM_Shield__alter_yoshi_egg_color___table_entry_palettes, FTREMIX_EGG_ENTRY_PALETTE_LOAD_COMMAND, FTREMIX_EGG_SEGMENT_E_SECOND_ENTRY
};

static MObjSub sFTRemixYoshiEggMObjSub;
static GObj *sFTRemixYoshiEggCapturers[GMCOMMON_PLAYERS_MAX];

/**
 * Replaces the palette G_SETTIMG of an egg display list with a call to the MObj display list. The display list is packed
 * N64 data (two u32 per command, native word order). Returns TRUE when the list is patched, now or before, and FALSE when it does not
 * look like an egg list (the palette load is not an RGBA16 G_SETTIMG followed by a G_LOADTLUT), which then keeps its own palette.
 */
static sb32 ftRemixYoshiEggPatchPaletteLoad(Gfx *dl, const FTRemixYoshiEggSite *site)
{
    u32 *palette_load = (u32*)dl + (site->palette_load_command * 2);
    s32 command;

    if ((palette_load[0] == FTREMIX_EGG_GBI_CALL_SEGMENT_E) && (palette_load[1] == site->branch_address))
    {
        return TRUE;
    }
    if (palette_load[0] != FTREMIX_EGG_GBI_SETTIMG_RGBA16)
    {
        return FALSE;
    }
    for (command = 1; command <= FTREMIX_EGG_TLUT_SEARCH_COMMANDS; command++)
    {
        if ((palette_load[command * 2] >> 24) == FTREMIX_EGG_GBI_LOADTLUT_OPCODE)
        {
            palette_load[0] = FTREMIX_EGG_GBI_CALL_SEGMENT_E;
            palette_load[1] = site->branch_address;

            return TRUE;
        }
    }
    return FALSE;
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

/** Palette index of the egg of a fighter that owns it: its Shield option, and the costume colour for the Costume option. */
static s32 ftRemixYoshiEggOwnerPaletteIndex(const FTStruct *fp)
{
    return ftRemixPartImageRulesEggPaletteIndex(gmRemixPanel(SHIELD, fp->player), ftRemixShieldGetCostumeColorIndex(fp));
}

/** Palette index of an egg nobody laid (Cruel Z Punish): the costume colour of the fighter in it, whatever the Shield option. */
static s32 ftRemixYoshiEggCostumePaletteIndex(const FTStruct *fp)
{
    return ftRemixPartImageRulesEggPaletteIndex(FTREMIX_PARTIMAGE_SHIELD_COSTUME, ftRemixShieldGetCostumeColorIndex(fp));
}

/** Logs once per site that its display list does not have the palette load the asm patches. */
static void ftRemixYoshiEggLogRefused(const FTRemixYoshiEggSite *site)
{
    static const FTRemixYoshiEggSite *sLogged[4];
    s32 index;

    for (index = 0; index < ARRAY_COUNT(sLogged); index++)
    {
        if (sLogged[index] == site)
        {
            return;
        }
        if (sLogged[index] == NULL)
        {
            sLogged[index] = site;
            port_log("SSB64: Remix Yoshi %s display list has no RGBA16 palette load at command %d, colour not applied\n", site->name, (int)site->palette_load_command);

            return;
        }
    }
}

/**
 * Patches the palette load of the display list of `dobj` for `site`. Returns the palette array token, or 0 when the list is
 * not patchable (nothing is changed then).
 */
static u32 ftRemixYoshiEggPatchDObj(DObj *dobj, const FTRemixYoshiEggSite *site)
{
    u32 palettes;

    if ((dobj == NULL) || (dobj->dl == NULL))
    {
        return 0;
    }
    palettes = ftRemixPartImageGetPaletteArrayToken(site->palettes_addr, FTREMIX_PARTIMAGE_EGG_PALETTE_COUNT);

    if (palettes == 0)
    {
        return 0;
    }
    if (!ftRemixYoshiEggPatchPaletteLoad(dobj->dl, site))
    {
        ftRemixYoshiEggLogRefused(site);

        return 0;
    }
    return palettes;
}

/** Gives a new egg its own palette MObj (Render.MODEL_PART_IMAGE_INIT_) and sets the palette index. */
static void ftRemixYoshiEggAttachPalette(DObj *dobj, const FTRemixYoshiEggSite *site, s32 palette_index)
{
    u32 palettes = ftRemixYoshiEggPatchDObj(dobj, site);
    MObj *mobj;

    if (palettes == 0)
    {
        return;
    }
    sFTRemixYoshiEggMObjSub.flags = MOBJ_FLAG_PALETTE;
    sFTRemixYoshiEggMObjSub.palettes = palettes;

    mobj = gcAddMObjForDObj(dobj, &sFTRemixYoshiEggMObjSub);
    mobj->palette_id = palette_index;
}

/** Colours the egg shield of a fighter by its Shield option. */
void ftRemixPartImageYoshiEggShieldApply(GObj *effect_gobj, FTStruct *fp)
{
    DObj *dobj = DObjGetStruct(effect_gobj);
    u32 palettes = ftRemixYoshiEggPatchDObj(dobj, &sFTRemixYoshiEggShieldSite);
    MObj *mobj;

    if (palettes == 0)
    {
        return;
    }
    mobj = ftRemixYoshiEggGetMObj(dobj, palettes);
    mobj->palette_id = ftRemixYoshiEggOwnerPaletteIndex(fp);
}

/** alter_yoshi_egg_color_ _eggthrow (0x8015EAF4): colours the egg a Yoshi throws like its shield. */
void ftRemixYoshiEggColorThrownEgg(GObj *egg_gobj, FTStruct *owner_fp)
{
    if (egg_gobj == NULL)
    {
        return;
    }
    ftRemixYoshiEggAttachPalette(DObjGetStruct(egg_gobj), &sFTRemixYoshiEggThrownSite, ftRemixYoshiEggOwnerPaletteIndex(owner_fp));
}

/**
 * alter_yoshi_egg_color_ _egglay_setup (0x8014D080): remembers who captured the fighter that is about to be put in an egg,
 * because the capture is cleared before the egg is made. NULL stays NULL (Cruel Z Punish egg).
 */
void ftRemixYoshiEggRememberCapturer(FTStruct *victim_fp, GObj *capturer_gobj)
{
    if (victim_fp->player >= ARRAY_COUNT(sFTRemixYoshiEggCapturers))
    {
        return;
    }
    sFTRemixYoshiEggCapturers[victim_fp->player] = capturer_gobj;
}

/** Takes the remembered capturer of a fighter (asm: reads 0x07A0 and clears it). */
static GObj* ftRemixYoshiEggTakeCapturer(const FTStruct *victim_fp)
{
    GObj *capturer_gobj;

    if (victim_fp->player >= ARRAY_COUNT(sFTRemixYoshiEggCapturers))
    {
        return NULL;
    }
    capturer_gobj = sFTRemixYoshiEggCapturers[victim_fp->player];
    sFTRemixYoshiEggCapturers[victim_fp->player] = NULL;

    return capturer_gobj;
}

/**
 * alter_yoshi_egg_color_ _egglay (0x8014C984): colours the egg that holds `victim_gobj` by the Shield option and costume of the
 * fighter that captured it, or by the costume of the victim when nobody did. The palette load sits in the third joint.
 */
void ftRemixYoshiEggColorLayEgg(GObj *egg_gobj, GObj *victim_gobj)
{
    FTStruct *victim_fp = ftGetStruct(victim_gobj);
    GObj *capturer_gobj = ftRemixYoshiEggTakeCapturer(victim_fp);
    DObj *top_dobj;
    s32 palette_index;

    if (egg_gobj == NULL)
    {
        return;
    }
    top_dobj = DObjGetStruct(egg_gobj);

    if ((top_dobj == NULL) || (top_dobj->child == NULL) || (top_dobj->child->child == NULL))
    {
        return;
    }
    palette_index = (capturer_gobj != NULL) ? ftRemixYoshiEggOwnerPaletteIndex(ftGetStruct(capturer_gobj)) : ftRemixYoshiEggCostumePaletteIndex(victim_fp);

    ftRemixYoshiEggAttachPalette(top_dobj->child->child, &sFTRemixYoshiEggLaySite, palette_index);
}

/**
 * alter_yoshi_egg_color_ _entry (0x80102F5C): colours the entry egg of a Yoshi like its shield. The second joint already has
 * an MObj, which gets the entry palette array and the palette flag.
 */
void ftRemixYoshiEggColorEntryEgg(GObj *egg_gobj, FTStruct *owner_fp)
{
    DObj *top_dobj;
    DObj *egg_dobj;
    u32 palettes;

    if (egg_gobj == NULL)
    {
        return;
    }
    top_dobj = DObjGetStruct(egg_gobj);

    if ((top_dobj == NULL) || (top_dobj->child == NULL) || (top_dobj->child->mobj == NULL))
    {
        return;
    }
    egg_dobj = top_dobj->child;
    palettes = ftRemixYoshiEggPatchDObj(egg_dobj, &sFTRemixYoshiEggEntrySite);

    if (palettes == 0)
    {
        return;
    }
    egg_dobj->mobj->sub.palettes = palettes;
    egg_dobj->mobj->sub.flags |= MOBJ_FLAG_PALETTE;
    egg_dobj->mobj->palette_id = ftRemixYoshiEggOwnerPaletteIndex(owner_fp);
}

#endif
