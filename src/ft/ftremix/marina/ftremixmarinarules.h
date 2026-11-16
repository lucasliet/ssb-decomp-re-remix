#ifndef _FTREMIXMARINARULES_H_
#define _FTREMIXMARINARULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of Marina's midair jumps (Marina/Marina.asm:802-882, jump_height_patch_, jump_timing_patch_ and
 * jump_height_table) and of the gem of her Cargo shake (Marina/MarinaSpecial.asm:76-96, shake_main_). Only int, so the
 * headless unit tests (tests/remix) can run them. The callers test the fighter id (Marina or N Marina) first.
 */

/** Entries of jump_height_table: jumps_used 0 to 5. */
#define FTREMIX_MARINA_JUMP_HEIGHTS 6

/** Action.JumpAerialF and Action.JumpAerialB (nFTCommonStatusJumpAerialF/B), the two statuses that lock the next jump. */
#define FTREMIX_MARINA_JUMP_STATUS_AERIAL_F 0x18
#define FTREMIX_MARINA_JUMP_STATUS_AERIAL_B 0x19

/** Hit status 0x800E8AAC (ftParamGetBestHitStatusAll) returns for a fighter that takes damage (nGMHitStatusNormal). */
#define FTREMIX_MARINA_SHAKE_HITSTATUS_VULNERABLE 1

/**
 * Stick term of the height of Marina's next midair jump (jump_height_patch_ replaces the constant 80 with it).
 *
 * @param jumps_used Jumps already used when the midair jump starts (FTStruct 0x148): 1 for the first midair jump.
 * @return jump_height_table[jumps_used]: 0, 80, 70, 60, 50, 40; 0 from 6 on (the table's zero padding).
 */
extern int ftRemixMarinaJumpStickRange(int jumps_used);

/**
 * Whether Marina's next midair jump is refused (jump_timing_patch_): she is in JumpAerialF or JumpAerialB and her jump
 * moveset has not set temp variable 2 yet.
 *
 * @param status_id Current status of the fighter.
 * @param temp_variable_2 Moveset temp variable 2 (FTStruct 0x180).
 * @return 1 when the jump is refused, 0 when it starts (the caller then clears temp variable 2, as the asm does).
 */
extern int ftRemixMarinaJumpIsLocked(int status_id, int temp_variable_2);

/**
 * Whether this Cargo shake frame applies the moveset damage to the carried fighter (shake_main_).
 *
 * @param damage Moveset temp variable 1 (FTStruct 0x17C), the damage to add.
 * @param hit_status Best hit status of the carried fighter.
 * @return 1 when the damage is not 0 and the carried fighter is vulnerable, else 0.
 */
extern int ftRemixMarinaShakeDamageApplies(int damage, int hit_status);

/**
 * Whether this Cargo shake frame spawns the gem (shake_main_): only on a frame that applied the damage, once per throw.
 *
 * @param damage_applied Result of ftRemixMarinaShakeDamageApplies for this frame.
 * @param gem_spawned gem_spawned flag (FTStruct 0xB28).
 * @return 1 when the gem spawns, else 0.
 */
extern int ftRemixMarinaShakeSpawnsGem(int damage_applied, int gem_spawned);

#endif

#endif
