#ifndef _GMREMIXTWELVECBHUD_H_
#define _GMREMIXTWELVECBHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Text of the 12-Character Battle on the CSS (TwelveCharBattle.asm setup_: Stocks Remaining, Best Character, TKOs) and on the
 * results screen (update_css_header_: "12-Char. Battle", "Game N"), drawn by the port overlay. Call every frame of the
 * screen; does nothing outside the mode.
 */
extern void gmRemixTwelveCBHudDrawCSS(void);
extern void gmRemixTwelveCBHudDrawResults(void);

#endif

#endif
