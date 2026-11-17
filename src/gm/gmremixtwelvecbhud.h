#ifndef _GMREMIXTWELVECBHUD_H_
#define _GMREMIXTWELVECBHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Text of the 12-Character Battle on the CSS (TwelveCharBattle.asm setup_: Stocks Remaining, Character Set, Best Character,
 * TKOs) and on the results screen (update_css_header_: "12-Char. Battle", "Game N"). Call every frame of the screen; does
 * nothing outside the mode. The CSS text is drawn by the layers of gmRemixTwelveCBHudMakeCSSLayers, the results text by the
 * port overlay.
 */
extern void gmRemixTwelveCBHudDrawCSS(void);

/**
 * Makes the text layers of the 12CB CSS in room 0x1E, under the cursor hands and the tokens (TwelveCharBattle.asm 5093-5121).
 * Call once at the end of the CSS setup; does nothing outside the mode.
 */
extern void gmRemixTwelveCBHudMakeCSSLayers(void);
extern void gmRemixTwelveCBHudDrawResults(void);

#endif

#endif
