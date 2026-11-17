#ifndef _GMREMIXTWELVECBHUD_H_
#define _GMREMIXTWELVECBHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Text of the 12-Character Battle on the CSS (TwelveCharBattle.asm setup_: Stocks Remaining, Character Set, Best Character,
 * TKOs) and on the results screen (update_css_header_: "12-Char. Battle", "Game N"). Call every frame of the screen; does
 * nothing outside the mode. The CSS text is drawn by the layers of gmRemixTwelveCBHudMakeCSSLayers, the results text by the
 * layer of gmRemixTwelveCBHudMakeResultsLayer.
 */
extern void gmRemixTwelveCBHudDrawCSS(void);

/**
 * Makes the text layers of the 12CB CSS in room 0x1E, under the cursor hands and the tokens (TwelveCharBattle.asm 5093-5121).
 * Call once at the end of the CSS setup; does nothing outside the mode.
 */
extern void gmRemixTwelveCBHudMakeCSSLayers(void);

/**
 * Draws "Game NN" of the results screen. Call every frame once the results header exists (the asm creates the string with
 * it, at tic 120); does nothing outside the mode.
 */
extern void gmRemixTwelveCBHudDrawResults(void);

/**
 * Makes the layer of "Game NN" in room 0x1F, group 0x16, right after the results header sprite (TwelveCharBattle.asm
 * 1932-1947), so the Match Stats page hides it with the header. Call when the header is made; does nothing outside the mode.
 */
extern void gmRemixTwelveCBHudMakeResultsLayer(void);

#endif

#endif
