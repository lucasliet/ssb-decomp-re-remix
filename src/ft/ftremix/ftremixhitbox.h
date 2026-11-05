#if defined(PORT) && defined(REGION_REMIX)

#ifndef _FTREMIXHITBOX_H_
#define _FTREMIXHITBOX_H_

#include <ft/fighter.h>

/*
 * Smash Remix Hitbox.asm display patches. The toggle readers are in ft/ftremix/ftremixhitboxdisplay.c, the display-list
 * builders (hurtbox colours and transparency, reflect and absorb hitbox) in ft/ftremix/ftremixhitbox.c.
 */

/* Toggles.asm entry_special_model == 2 (hitbox+model): the model is drawn and the hitboxes are drawn over it */
sb32 ftRemixHitboxDisplayIsOverlay(void);

/* The display mode Hitbox.asm projectile_ and the item loop of hitbox_mode_ give to projectiles and items: the toggle
 * value 0 to 3 (nDBDisplayMode*) */
s32 ftRemixHitboxDisplayObjectMode(void);

/* Hitbox.asm fix_hitbox_position_ (RAM 0x800F3310): TRUE when the hitbox of a fighter on this screen is drawn without the
 * mooring of the non-main camera */
sb32 ftRemixHitboxDisplayFixesPosition(void);

/* Hitbox.asm grab_immunity_and_armor_frames_ (RAM 0x800F279C): emits the env colour of a hurtbox in the Normal state */
void ftRemixHitboxDisplayNormalHurtboxColor(FTStruct *fp, FTDamageColl *damage_coll);

/* Hitbox.asm make_hurtboxes_transparent_ (RAM 0x800F2830, 0x8017193C): emits the display list command that draws the
 * hurtbox whose prim and env colour were just emitted, as an outline in hitbox+model */
void ftRemixHitboxDisplayDrawHurtbox(void);

/* Hitbox.asm add_reflect_absorb_hitboxes (RAM 0x800F3358): draws the reflect or absorb hitbox of the fighter when `dobj`
 * is its joint, inside the joint traversal of ftDisplayMainDrawParts */
void ftRemixHitboxDisplayDrawSpecialColl(FTStruct *fp, DObj *dobj);

#endif

#endif
