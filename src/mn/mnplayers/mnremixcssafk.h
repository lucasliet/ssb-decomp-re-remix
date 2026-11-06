#ifndef _MNREMIXCSSAFK_H_
#define _MNREMIXCSSAFK_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * "afk" player tag automation on the VS character select (CharacterSelect.asm awayMode): with the second custom player tag named "afk"
 * the CSS presses START by itself after it was ready for 120 frames, and the denied sound of a refused START is muted.
 */

/**
 * Counts the frames the CSS stays ready to fight; called every frame.
 *
 * @param is_ready TRUE when the CSS is ready to fight.
 * @param total_tics Frames since the CSS opened.
 */
extern void mnRemixCssAfkUpdate(sb32 is_ready, s32 total_tics);

/** TRUE when START counts as pressed this frame: the automation pressed it, or a player did. */
extern sb32 mnRemixCssAfkTapStart(void);

/** TRUE when the denied sound of a refused START must play, that is unless the automation is on. */
extern sb32 mnRemixCssAfkPlaysDeniedSound(void);

#endif

#endif
