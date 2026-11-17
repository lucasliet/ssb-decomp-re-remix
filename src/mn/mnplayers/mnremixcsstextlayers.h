#ifndef _MNREMIXCSSTEXTLAYERS_H_
#define _MNREMIXCSSTEXTLAYERS_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Text layer of the CSS indicator room 0x38 (CharacterSelect.asm setup_ 4590), shared by the Remix character selects
 * (docs/remix/text_layers.md). The port has no camera for that room, so it is made here. The panel menu texts have their own layer
 * (mnRemixPanelMenuStart) and the other texts of a screen are bound by their owners.
 */

/* Camera priority of room 0x38 (css_settings z-index): 0x15 on the VS and Training selects, 0x0E on the 1P and Bonus ones. */
#define MNREMIX_CSS_TEXT_INDICATOR_PRIORITY_VS 21
#define MNREMIX_CSS_TEXT_INDICATOR_PRIORITY_1P 14

/* Slots drawn by the room 0x38 layer: the rectangle marks of the 12CB Manual stock controls (30..33) and of the D-pad variants
 * (34..37), and the stock numbers of the panels (47..50). */
#define MNREMIX_CSS_TEXT_INDICATOR_RECT_FIRST 30
#define MNREMIX_CSS_TEXT_INDICATOR_RECT_COUNT 8
#define MNREMIX_CSS_TEXT_INDICATOR_LABEL_FIRST 47
#define MNREMIX_CSS_TEXT_INDICATOR_LABEL_COUNT 4

/**
 * Makes the room 0x38 camera and its text layer (group 0x13). Call it once per scene, after the screen made its own objects.
 *
 * @param indicator_priority Camera priority of room 0x38 (MNREMIX_CSS_TEXT_INDICATOR_PRIORITY_*).
 */
extern void mnRemixCssTextLayersMake(s32 indicator_priority);

#endif

#endif
