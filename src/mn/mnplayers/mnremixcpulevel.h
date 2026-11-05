#ifndef _MNREMIXCPULEVEL_H_
#define _MNREMIXCPULEVEL_H_

#include <ssb_types.h>
#include <sys/objdef.h>

#if defined(PORT) && defined(REGION_REMIX)

/* CPU level selector of the VS CSS: the right arrow reaches level 10 (AI.asm handle_right_arrow_press_, render_right_arrow_,
 * render_10_). The pkind argument is nFTPlayerKindMan or nFTPlayerKindCom from ft/ftdef.h. */

/** TRUE when the right arrow may raise the value under it: CPU levels up to 10, human handicap up to 40. */
extern sb32 mnRemixVSCanRaiseArrowValue(s32 pkind, s32 value);

/** TRUE when the right arrow must be hidden for the value under it (the value reached its limit). */
extern sb32 mnRemixVSIsArrowRightLimit(s32 pkind, s32 value);

/** TRUE when the value under the arrow is a CPU level that needs two digits, which is CPU level 10. */
extern sb32 mnRemixVSIsCPULevelTens(s32 pkind, s32 value);

/** Draws the two digits "10" of a CPU level 10 on the value object of a player, in place of the single digit. */
extern void mnRemixVSMakeCPULevelTens(GObj *gobj, s32 player, void *digit_file);

#endif

#endif
