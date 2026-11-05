#ifndef _MNREMIXSTAGEPANEL_H_
#define _MNREMIXSTAGEPANEL_H_

#include <ssb_types.h>
#include <sys/objdef.h>

#if defined(PORT) && defined(REGION_REMIX)

/**
 * Makes the button sprites of the Hazards, Movement, Layout and Roulette lines of the stage select (Stages.asm setup_). The
 * L sprites are left out on the tournament layout, which has no L hazard toggle. Call once the cursor exists.
 */
extern void mnRemixStagePanelMake(void);

/**
 * Draws the four lines for the hovered square, hides the ones that do not apply and colours the cursor (Stages.asm
 * update_text_). On the tournament layout it also forces the Hazard Mode toggle to the value of the hovered stage first.
 * Call every frame: the text overlay lives two frames.
 *
 * @param cursor_gobj Cursor GObj whose sprite takes the hazard colour.
 * @param base_gkind Stage id of the hovered square before its variant, or the RANDOM id.
 * @param shown_gkind Stage the square resolves to with the selected variant.
 * @param source_page Page of the Remix tournament grid the hovered square came from.
 */
extern void mnRemixStagePanelUpdate(GObj *cursor_gobj, s32 base_gkind, s32 shown_gkind, s32 source_page);

/**
 * L released on the normal layout: steps the Hazard Mode toggle for the hovered stage (right_check_ _hazard_toggle).
 *
 * @param base_gkind Stage id of the hovered square before its variant, or the RANDOM id.
 * @param shown_gkind Stage the square resolves to with the selected variant.
 * @return TRUE when the toggle changed, so the caller plays the toggle sound; FALSE on the tournament layout or for a stage
 *         with nothing to switch off.
 */
extern sb32 mnRemixStagePanelToggleHazards(s32 base_gkind, s32 shown_gkind);

/** Forgets the cursor colour that was applied, so the next update paints it again (other code recoloured the cursor). */
extern void mnRemixStagePanelInvalidateCursorColor(void);

#endif

#endif
