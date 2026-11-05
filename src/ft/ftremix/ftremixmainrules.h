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

#endif

#endif
