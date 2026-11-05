#ifndef _FTREMIXMAINRULES_H_
#define _FTREMIXMAINRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix hooks inside ftMainSetStatus and ftMainProcPhysicsMap (ft/ftmain.c): the map star puff fighters
 * (JigglypuffKirbyShared.asm kirby_cpu_inhale_4) and the Marina thrown animation override (Marina/Marina.asm thrown_marina_patch_). Only int, so the headless unit tests
 * (tests/remix) can run them.
 */

/* File.DK_THROWN_MARINA and File.FALCON_THROWN_MARINA (File.asm:2157). */
#define FTREMIXMAIN_FILE_DK_THROWN_MARINA 0x10B4
#define FTREMIXMAIN_FILE_FALCON_THROWN_MARINA 0x10B5

/* Projectile.id.DEDEDE_WADDLEDOO_BEAM (also the weapon kind of the port beam, itremixwaddledoo.c). */
#define FTREMIXMAIN_WPKIND_WADDLEDOO_BEAM 0x27

/**
 * kirby_cpu_inhale_4 (0x800E23E8): whether a fighter spawns the star puff when it touches a wall or the ceiling. Kirby,
 * J Kirby and Dedede do.
 *
 * @param fkind Remix fighter id (also the port fkind).
 * @return 1 for those three fighters, 0 otherwise.
 */
extern int ftRemixMainIsMapStarFighter(int fkind);

/**
 * thrown_marina_patch_: animation file of a fighter grabbed by Marina in the ThrownDK status.
 *
 * @param victim_fkind Remix fighter id of the grabbed fighter.
 * @return FTREMIXMAIN_FILE_DK_THROWN_MARINA for the Donkey Kong family, FTREMIXMAIN_FILE_FALCON_THROWN_MARINA for the Captain
 * Falcon family (Sheik, Dragon King and Sandbag too), 0 when the animation is not replaced.
 */
extern int ftRemixMainMarinaThrownFile(int victim_fkind);

/**
 * WaddleDoo.asm beam_knockback_direction_fix (0x800E44EC): hit direction a weapon gives the fighter it hits. A weapon
 * hit reads the weapon velocity x against a 5.0 threshold; the Waddle Doo beam reads its own direction (lr) against 0.5
 * instead, so the knockback never points back at the Doo.
 *
 * @param weapon_kind Weapon kind (WPStruct kind).
 * @param weapon_lr Weapon direction (WPStruct lr).
 * @param vel_x Horizontal air velocity of the weapon.
 * @return +1 or -1 for the damage_lr the weapon gives (+1 when the value read is negative), 0 when the value is under
 * the threshold and the caller must pick the direction from the positions.
 */
extern int ftRemixMainWeaponHitLr(int weapon_kind, int weapon_lr, float vel_x);

#endif

#endif
