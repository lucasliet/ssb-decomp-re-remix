/**
 * ftremixslippyhips.c - Slippy hip Y translation per action (Slippy.asm:329 animate_part_patch_, 342 apply_y_multiplier_,
 * 374 hips_y_translation_). Slippy's hip joint plays its animation with the Y translation multiplied by a per action value of
 * Slippy.hip_translation_table, which makes the body animations shorter.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <string.h>
#include <sys/objanim.h>
#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixparamhooks.h"
#include "ft/ftremix/slippy/ftremixslippyhipsrules.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define SLIPPY_HIP_DEFAULT_MULTIPLIER 1.0F

/** Y translation multiplier of the current Slippy action, read from the Remix RAM image. */
static f32 ftRemixSlippyHipMultiplier(FTStruct *fp)
{
    unsigned int word_address;
    u32 word;
    f32 multiplier;

    if (!ftRemixSlippyHipFindWordAddress(fp->status_id, REMIX_SYM_Slippy__hip_translation_table, REMIX_SYM_Slippy__hip_translation_menu_table, &word_address))
    {
        return SLIPPY_HIP_DEFAULT_MULTIPLIER;
    }
    word = remix_ram_image_read_word(word_address);
    memcpy(&multiplier, &word, sizeof(multiplier));

    return multiplier;
}

/**
 * Plays the animation of one joint of a fighter in ftParamUpdateAnimKeys (0x800E83D8). Slippy's hip joint (the first common
 * joint, FTStruct 0x08F8) gets the Y translation multiplier of its action; every other joint plays as in vanilla.
 *
 * @param fp Fighter that owns the joint.
 * @param joint Joint to animate.
 */
void ftRemixPlayFighterJointAnim(FTStruct *fp, DObj *joint)
{
    if ((fp->fkind != REMIX_ID_SLIPPY) || (joint != fp->joints[nFTPartsJointCommonStart]))
    {
        gcPlayDObjAnimJoint(joint);
        return;
    }
    gcRemixPlayDObjAnimJointScaledY(joint, ftRemixSlippyHipMultiplier(fp));
}

#endif
