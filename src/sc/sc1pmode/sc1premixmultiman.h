#ifndef _SC1PREMIXMULTIMAN_H_
#define _SC1PREMIXMULTIMAN_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Multi-Man and Cruel Multi-Man (SinglePlayerModes.asm): the Fighting Polygon Team stage of the 1P game turned into an endless
 * horde. The polygons respawn forever as random Fighting Polygons, the player has one stock, there are no items, no team attack
 * and no time limit, and the KOs the player scores are counted until the player is KO'd.
 */

/** First 1P stage of the current mode: the polygon stage for the horde modes, the race stage for Bonus 3, the first stage otherwise. */
extern s32 sc1PRemixMultimanFirstStage(s32 mode);

/** TRUE while the 1P game being played is a horde mode on its polygon stage. */
extern sb32 sc1PRemixMultimanIsActive(void);

/** Applies the horde rules to the battle of the stage being set up and starts a new KO count; no-op outside the horde modes. */
extern void sc1PRemixMultimanSetupBattle(void);

/** Makes the polygon icon and the "x" of the KO counter at the scene start; no-op outside the horde modes. */
extern void sc1PRemixMultimanSetupHud(void);

/** Gives the 1P game player the knockback ratio and the single stock of the horde modes; no-op outside them. */
extern void sc1PRemixMultimanSetupPlayer(s32 player);

/** Gives a polygon of the horde the knockback ratio of the mode; no-op outside the horde modes. */
extern void sc1PRemixMultimanSetupEnemy(s32 player);

/** Fighting Polygon kind of the next enemy of the horde, random among the 12. */
extern s32 sc1PRemixMultimanPolygonKind(void);

/** Counts an enemy KO of the player. */
extern void sc1PRemixMultimanOnEnemyKo(void);

/** Stops counting: the player was KO'd. */
extern void sc1PRemixMultimanOnPlayerKo(void);

/** KOs scored so far in the run. */
extern s32 sc1PRemixMultimanGetKos(void);

/** Ends the horde run: back to the Bonus 2 CSS when the player was KO'd, a new run when the reset button was used. */
extern void sc1PRemixMultimanLeaveRun(void);

/**
 * Best KO count of a fighter in the mode in use, Multi-Man or Cruel Multi-Man (_ko_amount_or_feet).
 *
 * @param fkind Fighter kind.
 * @return The record, 0 when the fighter has none or has no record row.
 */
extern u32 sc1PRemixMultimanRecordGet(s32 fkind);

/** Draws the KO counter of the horde modes as an overlay label (1P slot); no-op outside them. Call once per frame. */
extern void sc1PRemixMultimanDrawHud(void);

#endif

#endif
