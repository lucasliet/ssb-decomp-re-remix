/**
 * ftremixgoemonheavyitem.c - Goemon golden hair while he carries a heavy item (Goemon.asm heavy_item_*_moveset). The asm
 * replaces the first motion script pointer of the fighter (FTStruct 0x086C, motion_scripts[0][0].p_script) by a Goemon
 * script right after the lift and pickup statuses are set, and adds the preserve model part flag to the lift statuses.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/goemon/ftremixgoemonheavyitem.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"
#include <ft/ftdef.h>

/**
 * Replaces the first motion script of a Goemon fighter with the Remix script at ram_addr.
 * @param fp Fighter struct; any other fighter kind is left untouched.
 * @param ram_addr Address of the script in the Remix RAM image.
 */
static void ftRemixGoemonHeavyItemSetScript(FTStruct *fp, u32 ram_addr)
{
    u32 *script;

    if (fp->fkind != REMIX_ID_GOEMON)
    {
        return;
    }
    script = remix_ram_image_resolve(ram_addr);

    if (script == NULL)
    {
        return;
    }
    fp->motion_scripts[0][0].p_script = script;
}

void ftRemixGoemonHeavyItemPickupScript(FTStruct *fp)
{
    ftRemixGoemonHeavyItemSetScript(fp, REMIX_SYM_Goemon__HEAVY_ITEM_PICKUP);
}

void ftRemixGoemonHeavyItemHoldScript(FTStruct *fp)
{
    ftRemixGoemonHeavyItemSetScript(fp, REMIX_SYM_Goemon__HEAVY_ITEM_HOLD);
}

void ftRemixGoemonHeavyItemTurnScript(FTStruct *fp)
{
    ftRemixGoemonHeavyItemSetScript(fp, REMIX_SYM_Goemon__HEAVY_ITEM_TURN);
}

u32 ftRemixGoemonHeavyItemStatusFlags(FTStruct *fp, u32 vanilla_flags)
{
    if (fp->fkind != REMIX_ID_GOEMON)
    {
        return vanilla_flags;
    }
    return vanilla_flags | FTSTATUS_PRESERVE_MODELPART;
}

#endif
