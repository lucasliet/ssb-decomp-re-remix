/**
 * ftremixjkirby.c - J Kirby (Remix id 0x30) grounded_script hook.
 *
 * Kirby.asm clear_marth_flag_ (0x8058E6D0) is the grounded_script entry JKirby
 * installs (JKirby.asm, Character.table_patch_start(grounded_script, JKIRBY)).
 * The vanilla grounded routine then returns to 0x800DE44C.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"

/** Kirby.clear_marth_flag_: a Marth or Roy copy clears the word at N64 0x0AE0 (pseudo-jump flag) on landing. */
void ftRemixJKirbyGroundedScript(FTStruct *fp)
{
    s32 copy_id = fp->passive_vars.kirby.copy_id;

    if ((copy_id == REMIX_ID_MARTH) || (copy_id == REMIX_ID_ROY))
    {
        fp->passive_vars.kirby.copysamus_charge_level = 0;
    }
}

#endif
