/**
 * gmremixmagnify.c - per-fighter zoom of the magnifying glass (MagnifyingGlass.asm, apply_custom_zoom_).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include "gm/gmremixmagnify.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define GMREMIX_MAGNIFY_PLAYERS 4
/* NUM_CHARACTERS = NUM_VANILLA_CHARACTERS (27) + 2 + ADD_CHARACTERS (68), Character.asm:15-27. */
#define GMREMIX_MAGNIFY_TABLE_KINDS 97
/* N64 Mtx fraction words holding the elements [0][0] (x scale, high half) and [1][1] (y scale, low half). */
#define GMREMIX_MAGNIFY_FRAC_X_WORD 8
#define GMREMIX_MAGNIFY_FRAC_Y_WORD 10

static Mtx sGMRemixMagnifyCustomMatrix[GMREMIX_MAGNIFY_PLAYERS];

/**
 * Reads the magnifying glass zoom override of a fighter kind.
 * @param fkind Fighter kind, used as index into Character.magnifying_glass_zoom.table (one u16 per kind).
 * @return The zoom value, or 0 when the kind has no override or is outside the table.
 */
static u16 gmRemixMagnifyGetZoom(s32 fkind)
{
    if ((fkind < 0) || (fkind >= GMREMIX_MAGNIFY_TABLE_KINDS))
    {
        return 0;
    }
    return (u16)remix_ram_image_read_half(REMIX_SYM_Character__magnifying_glass_zoom__table + (fkind * 2));
}

/**
 * Writes the zoom into the fraction words of an N64-format Mtx: the asm `sh` at byte 0x20 is the high half of word 8 (fraction of
 * element [0][0], the x scale) and the `sh` at 0x2A is the low half of word 10 (fraction of [1][1], the y scale). The port keeps
 * the words in host order, so the halves are set by shifts, not by byte offsets.
 *
 * @param mtx Matrix in the N64 layout.
 * @param zoom Fraction value to store.
 */
static void gmRemixMagnifySetZoom(Mtx *mtx, u16 zoom)
{
    u32 *words = (u32 *)mtx;

    words[GMREMIX_MAGNIFY_FRAC_X_WORD] = (words[GMREMIX_MAGNIFY_FRAC_X_WORD] & 0x0000FFFFU) | ((u32)zoom << 16);
    words[GMREMIX_MAGNIFY_FRAC_Y_WORD] = (words[GMREMIX_MAGNIFY_FRAC_Y_WORD] & 0xFFFF0000U) | zoom;
}

/**
 * Returns the projection matrix the magnifying glass display list loads for a fighter.
 * Mirrors MagnifyingGlass.asm apply_custom_zoom_: without an override the real matrix is returned unchanged.
 * @param fp Fighter drawn in the magnifying glass (a1 in the asm). fkind selects the zoom, player selects the copy.
 * @param real_mtx Matrix the vanilla display list would load (t9 in the asm).
 * @return real_mtx, or the player's copy of it with the zoom in the x and y scale.
 */
Mtx *gmRemixMagnifyApplyCustomZoom(FTStruct *fp, Mtx *real_mtx)
{
    u16 zoom = gmRemixMagnifyGetZoom(fp->fkind);
    Mtx *custom_mtx;

    if ((zoom == 0) || (fp->player >= GMREMIX_MAGNIFY_PLAYERS))
    {
        return real_mtx;
    }
    custom_mtx = &sGMRemixMagnifyCustomMatrix[fp->player];
    *custom_mtx = *real_mtx;
    gmRemixMagnifySetZoom(custom_mtx, zoom);
    return custom_mtx;
}

#endif
