#ifndef _MNREMIXVSSTATSSCENE_H_
#define _MNREMIXVSSTATSSCENE_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Game-display-list objects of the VS match stats on the Results screen: a camera drawn over the results, the dimmed
 * background of the stats pages, and the A, B and R button sprites (VsStats.asm press_a_obj, press_b and press_r).
 * The text stays in the ImGui overlay (mnremixvsstatsview.c, mnremixvsstatspages.c) and draws over these objects.
 */

/** Display groups (GObj links) of the Remix VS stats; the asm toggles them with Render.toggle_group_display_. */
#define MNREMIX_VSSTATS_GROUP_PROMPT 0x0D
#define MNREMIX_VSSTATS_GROUP_PAGES 0x0E

/** Makes the camera, the background and the button sprites; the prompt group and the pages group start hidden. */
extern void mnRemixVSStatsSceneMake(void);

/**
 * Makes the stock icon of every present fighter above its column of the stats pages (VsStats.asm draw_port_headers_ draws it at
 * X 158 + 35 per column, Y 16). The fighters must exist, so the results screen calls it once they are made. The icons join the
 * pages group and start hidden.
 */
extern void mnRemixVSStatsSceneMakeHeaderIcons(void);

#endif

#endif
