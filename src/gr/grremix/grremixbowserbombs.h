#ifndef _GRREMIXBOWSERBOMBS_H_
#define _GRREMIXBOWSERBOMBS_H_

/*
 * Bowser's Stadium bombs (Hazards.asm replenish_bowser_bombs_, bowser_bomb_flash_main and the patches of
 * bowser_bomb_explosion_, bowser_bomb_air_collision_, bomb_hitstun_, track_hit_object_). The bombs are RTTF bomb items
 * that flash, hover until hit and respawn on their side of the stadium. Every effect is limited to stage BOWSERB.
 */

#include <ssb_types.h>
#include <sys/objdef.h>
#include <ft/fighter.h>

/** Creates the left and right bombs and the respawn process. Called when BOWSERB starts with hazards on. */
void grRemixBowserBombsStart(void);

/** True on stage BOWSERB, the only stage the bomb patches apply to. */
sb32 grRemixBowserBombsIsStage(void);

/** Marks the bomb as exploded (respawn countdown) and plays the bomb explosion FGM. Returns TRUE when the explosion is handled. */
sb32 grRemixBowserBombsExplodeOnHit(GObj *item_gobj);

/** FGM played when a bomb lands on the ground: the Remix bomb sound on BOWSERB, the given FGM otherwise. */
u16 grRemixBowserBombsMapFgm(u16 fgm_id);

/** Records the item that most recently hit a fighter (asm track_hit_object_). Players outside 0..3 are ignored. */
void grRemixBowserBombsTrackHitObject(u8 player, GObj *item_gobj);

/** Hitstun of a fighter hit by a bomb: scaled by 9 / (gravity + 4) as in the asm, otherwise unchanged. */
f32 grRemixBowserBombsHitstunTics(FTStruct *fp, f32 hitstun_tics);

#endif
