#ifndef _MNREMIXTWELVECBCUSTOMLEGEND_H_
#define _MNREMIXTWELVECBCUSTOMLEGEND_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Legend of the Custom character set in the panel of the player holding a token over a portrait (TwelveCharBattle.asm
 * draw_custom_portrait_indicators_ 4615-5046): the Z and R buttons with blinking arrows, L "Set All", D-pad up "Presets",
 * D-pad down "Random" and D-pad up held "Copy". The buttons are native sprites, the texts and the square pips of the D-pad
 * icons go through the HUD overlay.
 */

/** Forgets the sprites of the previous visit of the CSS (their scene is gone). */
extern void mnRemixTwelveCBCustomLegendReset(void);

/**
 * Shows or hides the legend of one panel, once per frame.
 *
 * @param side Player (0 or 1) whose token is held; the legend sits in that player's panel.
 * @param is_shown TRUE while that token hovers a portrait of a Custom set before the battle started.
 * @param arrows_file Loaded CSS file that holds the arrow sprites (sMNPlayersVSFiles[0]).
 */
extern void mnRemixTwelveCBCustomLegendUpdate(s32 side, sb32 is_shown, void *arrows_file);

#endif

#endif
