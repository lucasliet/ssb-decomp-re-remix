#ifndef _MNREMIXCSSINDICATORS_H_
#define _MNREMIXCSSINDICATORS_H_

#include <ssb_types.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Variant indicators of the panels of the Remix VS character select (src/CharacterSelect.asm update_variant_indicators_,
 * draw_variant_indicator_ and draw_variant_icon_): a D-pad image under the fighter that has D-pad variants, the image of each
 * variant around it, a yellow mark on the direction the token holder chose, and the J or E flag of a selected regional fighter.
 */

#define MNREMIX_CSS_INDICATOR_PANELS 4

/** What the indicators need to know about one panel each frame. */
typedef struct MNRemixCssPanelState
{
    s32 fkind;             /* hovered or selected fighter, nFTKindNull when the panel has none */
    sb32 is_selected;      /* the fighter was dropped on the panel */
    sb32 is_token_held;    /* some hand holds the token of the panel */
} MNRemixCssPanelState;

/** Forgets the scene objects; called when the character select builds its portraits. */
extern void mnRemixCssIndicatorsReset(void);

/**
 * Brings the indicators of every panel up to date with the state of the panels.
 *
 * @param css_images_file Loaded Remix CSS images file (File.CSS_IMAGES).
 * @param panels State of the panels, one entry per panel.
 * @param panel_count Number of entries, at most MNREMIX_CSS_INDICATOR_PANELS.
 */
extern void mnRemixCssIndicatorsUpdate(void *css_images_file, const MNRemixCssPanelState *panels, s32 panel_count);

/**
 * Adds the icon of a handicap of 10 or more to the handicap value object, with the colour bar of its level under it
 * (AI.asm render_10_._handicap): values 10 to 40 are drawn as the stock icon of a fighter or as a polygon, Metal or Master Hand
 * icon instead of digits.
 *
 * @param gobj Handicap value object of the panel.
 * @param value Handicap value, 10 to 40.
 * @param label_x Left edge of the HANDICAP label of the panel.
 * @param css_images_file Loaded Remix CSS images file (File.CSS_IMAGES).
 * @param common_file Loaded menu common file (file 0x0), which holds the sprite of the level bar.
 */
extern void mnRemixCssMakeHandicapIcon(GObj *gobj, s32 value, f32 label_x, void *css_images_file, void *common_file);

#endif

#endif
