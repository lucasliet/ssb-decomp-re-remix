#ifndef _FTREMIXPIKASHARED_H_
#define _FTREMIXPIKASHARED_H_

/*
 * Smash Remix patches of the Pikachu clones (src/pikashared.asm, src/JPika/JPika.asm) that vanilla code calls (REGION_REMIX
 * only): E Pikachu and J Pikachu Thunder read the attributes of their own loaded file, J Pikachu Thunder Jolt too and lasts
 * longer. The values are in ftremixpikarules.h. Each helper keeps the vanilla value for every other fighter.
 *
 * Implemented in ftremixpikashared.c, called from wp/wppikachu.
 */

#include <ft/fighter.h>
#include <wp/weapon.h>

/** get_thunder_special_struct_1/2: points the Thunder head or trail descriptor copy `desc` at the main file of E or J Pikachu. */
void ftRemixPikaSharedThunderDesc(FTStruct *fp, WPDesc *desc);

/** get_thunder_jolt_special_struct_1: points the aerial Thunder Jolt descriptor copy `desc` at the file 6 of J or E Pikachu (or the Kirby that copied it). */
void ftRemixPikaSharedThunderJoltDesc(FTStruct *fp, WPDesc *desc);

/** get_thunder_playerstruct1_: remembers in the aerial Thunder Jolt the power kind of its owner (the asm projectile word 0x78). */
void ftRemixPikaSharedThunderJoltSetKind(WPStruct *wp, FTStruct *fp);

/** get_thunder_jolt_special_struct_2: like ftRemixPikaSharedThunderJoltDesc for the ground Thunder Jolt that replaces `prev_wp`. */
void ftRemixPikaSharedThunderJoltGroundDesc(WPStruct *prev_wp, WPDesc *desc);

/** get_thunder_playerstruct2_: the ground Thunder Jolt takes the power kind of the weapon it replaces. */
void ftRemixPikaSharedThunderJoltInheritKind(WPStruct *new_wp, WPStruct *prev_wp);

/**
 * The file 8 data the animation of the ground Thunder Jolt `wp` reads: the one of J or E Pikachu if that fired it, so it
 * does not depend on the Pikachu globals being loaded (not patched by the asm, the file content is the same).
 */
void* ftRemixPikaSharedThunderJoltAnimFile(WPStruct *wp, void *vanilla_file_data);

/** thunderjolt_duration_: lifetime of the aerial Thunder Jolt of `fp` (J Pikachu, or the Kirby that copied it). */
s32 ftRemixPikaSharedThunderJoltLifetime(FTStruct *fp, s32 vanilla_lifetime);

/** get_thunder_jolt_anim_struct_: the power kind stored in the Thunder Jolt `wp` (the asm projectile word 0x78). For ftremixeffects.c. */
s32 ftRemixPikaSharedThunderJoltPowerKind(WPStruct *wp);

/** get_thunder_anim_struct_: the model file (Remix file 4) the Thunder trail gfx of E and J Pikachu read. For efmanager.c. */
void **ftRemixPikaSharedModelFile(s32 fkind, void **vanilla_file);

/** get_thunder_jolt_anim_struct_: the file 8 (p_file_special3) the Thunder Jolt gfx of `power_kind` read. For efmanager.c. */
void **ftRemixPikaSharedJoltEffectFile(s32 power_kind, void **vanilla_file);

#endif
