/**
 * mnremixcpulevel.c - CPU level selector of the VS CSS up to level 10, see mnremixcpulevel.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/ftdef.h>
#include <lb/lbcommon.h>
#include <lb/library.h>
#include <mn/menu.h>
#include <PR/sp.h>
#include <reloc_data.h>
#include "mn/mnplayers/mnremixcpulevel.h"
#include "mn/mnplayers/mnremixcpulevelrules.h"

#define MNREMIX_VS_CSS_DIGIT_Y 200.0F
#define MNREMIX_VS_CSS_TENS_X_OFFSET 67
#define MNREMIX_VS_CSS_UNITS_X_OFFSET 75
#define MNREMIX_VS_CSS_PLAYER_STRIDE 69

/**
 * TRUE when the right arrow may raise the value under it (AI.asm handle_right_arrow_press_). A CPU level is raised up to
 * level 10, a human handicap up to 40. The icons for 10 and above are drawn by mnRemixCssMakeHandicapIcon.
 */
sb32 mnRemixVSCanRaiseArrowValue(s32 pkind, s32 value)
{
    if (pkind == nFTPlayerKindCom)
    {
        return mnRemixCpuLevelCanRaise(value) ? TRUE : FALSE;
    }
    return mnRemixHandicapCanRaise(value) ? TRUE : FALSE;
}

/**
 * TRUE when the right arrow hides because the value under it reached its limit (AI.asm render_right_arrow_): CPU level 10
 * for a CPU, handicap 40 for a human.
 */
sb32 mnRemixVSIsArrowRightLimit(s32 pkind, s32 value)
{
    if (pkind == nFTPlayerKindCom)
    {
        return mnRemixCpuLevelCanRaise(value) ? FALSE : TRUE;
    }
    return mnRemixHandicapCanRaise(value) ? FALSE : TRUE;
}

/**
 * TRUE when the value under the arrow is a CPU level that needs two digits.
 */
sb32 mnRemixVSIsCPULevelTens(s32 pkind, s32 value)
{
    return ((pkind == nFTPlayerKindCom) && mnRemixCpuLevelNeedsTens(value)) ? TRUE : FALSE;
}

/**
 * Creates one digit sprite object of a CPU level on the gobj, at the given x position, with the colours of the vanilla
 * value digits.
 */
static void mnRemixVSMakeCPULevelDigit(GObj *gobj, void *digit_file, intptr_t digit_offset, f32 x)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, digit_file, digit_offset));

    sobj->pos.x = x;
    sobj->pos.y = MNREMIX_VS_CSS_DIGIT_Y;
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xFF;
    sobj->sprite.blue = 0xFF;
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
}

/**
 * Draws "1" then "0" for CPU level 10 (AI.asm render_10_._render_1 and _render_0): the "0" sits 8 units right of the "1".
 * digit_file is the VS CSS common file that holds the digit sprites, the same file as the vanilla value digits.
 */
void mnRemixVSMakeCPULevelTens(GObj *gobj, s32 player, void *digit_file)
{
    f32 tens_x = (player * MNREMIX_VS_CSS_PLAYER_STRIDE) + MNREMIX_VS_CSS_TENS_X_OFFSET;
    f32 units_x = (player * MNREMIX_VS_CSS_PLAYER_STRIDE) + MNREMIX_VS_CSS_UNITS_X_OFFSET;

    mnRemixVSMakeCPULevelDigit(gobj, digit_file, llMNCommonDigit1Sprite, tens_x);
    mnRemixVSMakeCPULevelDigit(gobj, digit_file, llMNCommonDigit0Sprite, units_x);
}

#endif
