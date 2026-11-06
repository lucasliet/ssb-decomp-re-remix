#ifndef _FTREMIXTWELVECBDEFEATED_H_
#define _FTREMIXTWELVECBDEFEATED_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/*
 * Defeated pose of the 12-Character Battle character select (TwelveCharBattle.asm defeated_action_override_, 12CB branch): the
 * fighter model of a character with no stocks left plays its DownStandU animation, with the texture changes and the frozen frame of
 * the Remix defeated_array row, instead of the status animation. The rules live in ftremixtwelvecbdefeatedrules.c.
 */

/**
 * Tells whether the select screen fighter of a port shows a defeated character. Call it before the fighter of the port is made or its
 * status changes.
 *
 * @param player Port index of the select screen fighter.
 * @param is_defeated TRUE when the character has no stocks left.
 */
extern void ftRemixTwelveCBDefeatedSetPose(s32 player, sb32 is_defeated);

/**
 * Motion row of a status change on the select screen (ftMainSetStatus, after the Classic Sonic swap).
 *
 * @param fp Fighter struct.
 * @param motion_desc Motion row the status selected.
 * @return `motion_desc`, or a row valid until the next call for the same port that holds the defeated pose of the character.
 */
extern FTMotionDesc* ftRemixTwelveCBDefeatedSwapMotion(const FTStruct *fp, FTMotionDesc *motion_desc);

#endif

#endif
