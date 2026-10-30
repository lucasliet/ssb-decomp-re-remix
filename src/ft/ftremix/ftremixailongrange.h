#ifndef _FTREMIXAILONGRANGE_H_
#define _FTREMIXAILONGRANGE_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* LONG_RANGE routines of the Remix AI (AI.asm LONG_RANGE scope), used by func_ovl3_80138AA8 in ftcomputer.c. */

/* Returned by ftRemixAiLongRangeSwitchKind when no vanilla switch case applies: the far-range move does nothing. */
#define FTREMIX_AI_LONG_RANGE_NO_CASE (-1)

/**
 * Case label of the far-range switch in func_ovl3_80138AA8 for fighter kind fkind. Vanilla fighters return fkind unchanged.
 * A Remix fighter's routine from Character.ai_long_range.table maps to the case that runs the same routine: NSP_SHOOT
 * is the nFTKindMario case and PHYSICAL_SHOOT is the nFTKindLink case. NONE and polygon fighters return NO_CASE.
 */
s32 ftRemixAiLongRangeSwitchKind(s32 fkind);

/**
 * Far-range command after the vanilla choice of default_command. A non-zero entry of Character.nsp_shoot_custom_move.table
 * for fp->fkind replaces it (AI.asm custom_move, the end of the far-range routine).
 */
s32 ftRemixAiLongRangeCommand(FTStruct *fp, s32 default_command);

/**
 * Sets the far-range command of fp: the vanilla AutoX B, or the tilt-then-smash variant when is_delay is TRUE, after
 * ftRemixAiLongRangeCommand. Replaces the two vanilla calls at the end of func_ovl3_80138AA8.
 */
void ftRemixAiLongRangeSetCommand(FTStruct *fp, sb32 is_delay);

#endif

#endif
