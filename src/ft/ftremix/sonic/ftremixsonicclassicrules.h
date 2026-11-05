#ifndef _FTREMIXSONICCLASSICRULES_H_
#define _FTREMIXSONICCLASSICRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure Classic Sonic decisions (Sonic/Sonic.asm classic_fgm_patch_1_/2_, select toggle in Costumes.asm select_, Teams.asm
 * classic_sonic_costume_fix_, Sonic.asm classic_sonic_anim_swap_ and the save/load patches), free of game globals so the
 * headless unit tests (tests/remix) can run them.
 */

/* FGM ids of the modern Sonic sounds and their classic replacements (FGM.asm add_sound order). */
#define FTREMIX_SONIC_FGM_SPINDASH_CHARGE 0x3D8
#define FTREMIX_SONIC_FGM_SPINDASH_ROLL 0x3D9
#define FTREMIX_SONIC_FGM_HOMING_CHARGE 0x3DA
#define FTREMIX_SONIC_FGM_HOMING_ATTACK 0x3DB
#define FTREMIX_SONIC_FGM_JUMP 0x3DC
#define FTREMIX_SONIC_FGM_SKID 0x3DD
#define FTREMIX_SONIC_FGM_JUMP_2 0x3FC
#define FTREMIX_SONIC_FGM_CLASSIC_SPINDASH_CHARGE 0x3DE
#define FTREMIX_SONIC_FGM_CLASSIC_JUMP 0x3E0
#define FTREMIX_SONIC_FGM_CLASSIC_SKID 0x3E1
#define FTREMIX_SONIC_FGM_CLASSIC_SPIN 0x3E2
#define FTREMIX_SONIC_FGM_CLASSIC_SPINDASH_ATTACK 0x3E3
#define FTREMIX_SONIC_FGM_CLASSIC_JUMP_2 0x3FD

/* C button ids of the CSS costume cycle (same values as MNREMIX_COSTUME_BUTTON_*). */
#define FTREMIX_SONIC_BUTTON_RIGHT 1
#define FTREMIX_SONIC_BUTTON_LEFT 3

/* Team ids: the Remix yellow team follows red, blue and green (Teams.asm NUM_NEW_TEAMS). */
#define FTREMIX_SONIC_TEAM_YELLOW 3

/* Costume of the yellow team: modern Sonic uses 5 (Costumes.asm select_ _determine_screen), classic Sonic uses 1. */
#define FTREMIX_SONIC_YELLOW_COSTUME_MODERN 5
#define FTREMIX_SONIC_YELLOW_COSTUME_CLASSIC 1

/* Classic costumes are compared and stored as the modern id plus this offset (Sonic.asm adjust_costume_comparison_,
 * Shield.asm color_fix_, Training.asm load_character_). */
#define FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET 6

/* Frame the selection animation restarts on when the swap happens on frame 0 (Costumes.asm: 0x43410000 = 193.0F, "hard coded
 * to length of selection animation"). */
#define FTREMIX_SONIC_SELECT_ANIM_LENGTH 193.0F

/* Scene ids where the classic menu animations replace Sonic's (TwelveCharBattle.asm defeated_action_override_): the 1P intro
 * (SCKind 0x0E, nSCKind1PIntro) and the VS results (SCKind 0x18, nSCKindVSResults). */
#define FTREMIX_SONIC_SCENE_1P_INTRO 0x0E
#define FTREMIX_SONIC_SCENE_VS_RESULTS 0x18

/* Animation files: Sonic's 1P pose and second victory pose, and the classic files that replace them. */
#define FTREMIX_SONIC_FILE_1P_POSE 0xDD1
#define FTREMIX_SONIC_FILE_VICTORY2 0xDD7
#define FTREMIX_SONIC_FILE_CLASSIC_1P_POSE 0xDD2
#define FTREMIX_SONIC_FILE_CLASSIC_VICTORY1 0xE3C

/**
 * FGM of the 0x38 "play FGM" moveset command for a classic Sonic (classic_fgm_patch_1_): the spin dash charge, spin dash roll
 * and homing attack, jump, skid and second jump sounds are swapped, any other id is returned unchanged.
 */
extern int ftRemixSonicClassicRulesPlayFgm(int fgm_id);

/**
 * FGM of the 0x3C "play FGM once" moveset command for a classic Sonic (classic_fgm_patch_2_): the spin dash charge and the
 * homing charge (to the spin sound) are swapped, any other id is returned unchanged.
 */
extern int ftRemixSonicClassicRulesPlayLoopFgm(int fgm_id);

/**
 * TRUE when a C button press on the CSS switches between modern and classic Sonic (Costumes.asm select_ _sonic): C right when
 * the cycle wrapped back to costume 0, C left when the costume was 0, and any C right or left in a team battle. C up and down
 * (shade) never switch.
 */
extern int ftRemixSonicClassicRulesShouldToggle(int is_team_battle, int button, int old_costume, int new_costume);

/**
 * Costume of the yellow team for a Sonic after a CSS cycle (Costumes.asm select_ "Yellow team fix", VS team battle only):
 * classic 1, modern 5. Any other team keeps the cycled costume.
 */
extern int ftRemixSonicClassicRulesCycleTeamCostume(int team_id, int is_classic, int cycled_costume);

/**
 * Team costume of a Sonic (Teams.asm classic_sonic_costume_fix_): classic Sonic on the yellow team uses costume 1, every other
 * case keeps the default team costume.
 */
extern int ftRemixSonicClassicRulesTeamCostume(int team_id, int is_classic, int default_costume);

/**
 * Costume id used when two panels of the same character are compared (Sonic.asm adjust_costume_comparison_): classic Sonic
 * costumes count as the modern id plus FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET, so classic and modern never collide.
 */
extern int ftRemixSonicClassicRulesFudgeCostume(int costume, int is_classic);

/**
 * Splits a Training menu costume (Training.asm load_character_): ids 6 and above are classic Sonic costumes stored as the
 * modern id plus 6. Returns the model costume and writes the classic flag.
 */
extern int ftRemixSonicClassicRulesSplitTrainingCostume(int costume, int *is_classic);

/**
 * Highest value of the Training menu costume item (Training.asm _update_max_costume): Sonic gets the six Classic costumes on top
 * of the highest value of his modern ones.
 */
extern int ftRemixSonicClassicRulesTrainingMenuMax(int modern_max, int is_sonic);

/**
 * Frame the selection animation continues from after the model swap (Costumes.asm select_): the frame of the old model, or the
 * animation length when it was on frame 0.
 */
extern float ftRemixSonicClassicRulesSelectFrame(float current_frame);

/**
 * Replacement animation file for a classic Sonic menu action (Sonic.asm classic_sonic_anim_swap_), or 0 when the file is not one
 * of the two Sonic has a classic version of. The replacement has no moveset commands and no animation flags.
 */
extern int ftRemixSonicClassicRulesMenuAnimFile(int anim_file_id);

/**
 * TRUE on the scenes where the classic menu animations are used (TwelveCharBattle.asm defeated_action_override_).
 */
extern int ftRemixSonicClassicRulesIsMenuAnimScene(int scene_kind);

/**
 * Persistence of the per-port flags (Sonic.asm load_classic_flag_1p_, load_classic_flags_training_): clears the whole table,
 * then writes the flag of one port. A port outside the table is ignored.
 */
extern void ftRemixSonicClassicRulesLoadOne(unsigned char *table, int port_count, int port, int flag);

/**
 * Same as ftRemixSonicClassicRulesLoadOne for two ports (Training: the human and the CPU).
 */
extern void ftRemixSonicClassicRulesLoadTwo(unsigned char *table, int port_count, int port_a, int flag_a, int port_b, int flag_b);

#endif

#endif
