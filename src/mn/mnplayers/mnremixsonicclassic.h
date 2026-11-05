#ifndef _MNREMIXSONICCLASSIC_H_
#define _MNREMIXSONICCLASSIC_H_

#include <ssb_types.h>
#include <sys/objtypes.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Classic Sonic on the character select screens (Costumes.asm select_ _sonic, Teams.asm classic_sonic_costume_fix_,
 * Sonic.asm save/load/reset_classic_*). The player picks classic Sonic with C right / C left (the costume cycle passing
 * through costume 0); the match side is ft/ftremix/sonic/ftremixsonicclassic.c.
 */

/** Value of mnRemixSonicClassicToggle when the press does not switch the model. */
#define MNREMIX_SONIC_CLASSIC_NO_TOGGLE (-1.0F)

/**
 * Applies a C button press to the classic flag of a panel. When the press switches the model (Sonic only, see
 * ftRemixSonicClassicRulesShouldToggle) the flag is flipped and the caller must remake the panel fighter and call
 * mnRemixSonicClassicRestoreFrame.
 *
 * @param player Port of the panel.
 * @param fkind Character of the panel.
 * @param button MNREMIX_COSTUME_BUTTON_*.
 * @param old_costume Costume before the press.
 * @param new_costume Costume after the press.
 * @param is_team_battle TRUE in a VS team battle.
 * @param fighter_gobj Panel fighter before it is remade.
 * @return Animation frame to carry over to the new model, or MNREMIX_SONIC_CLASSIC_NO_TOGGLE.
 */
extern f32 mnRemixSonicClassicToggle(s32 player, s32 fkind, s32 button, s32 old_costume, s32 new_costume, sb32 is_team_battle, GObj *fighter_gobj);

/**
 * Continues the selection animation on the remade panel fighter (Costumes.asm select_ _sonic_end): stores the frame for the
 * selection status and sets it on the fighter.
 */
extern void mnRemixSonicClassicRestoreFrame(s32 player, GObj *fighter_gobj, f32 frame);

/**
 * Costume of the yellow team after a CSS cycle (VS team battle only, Costumes.asm select_), or `cycled_costume` unchanged.
 */
extern s32 mnRemixSonicClassicCycleTeamCostume(s32 player, s32 fkind, sb32 is_team_battle, s32 team, s32 cycled_costume);

/**
 * Team costume of a panel (Teams.asm classic_sonic_costume_fix_): ftParamGetCostumeTeamID, but classic Sonic on the yellow
 * team gets costume 1.
 */
extern s32 mnRemixSonicClassicGetTeamCostume(s32 player, s32 fkind, s32 team);

/**
 * Costume id for the "is this costume already used" comparison of a panel (Sonic.asm adjust_costume_comparison_): classic
 * Sonic costumes are compared as the modern id plus 6.
 */
extern s32 mnRemixSonicClassicFudgeCostume(s32 player, s32 fkind, s32 costume);

/** Clears the classic flag of a port when its character is deselected or its port closed (Sonic.asm reset_classic_flag_). */
extern void mnRemixSonicClassicReset(s32 player);

/** VS: keeps the flags when the CSS hands over to the battle (save_classic_flags_vs_). */
extern void mnRemixSonicClassicSaveVS(void);

/** VS: restores the flags when the CSS opens (load_classic_flag_vs_). */
extern void mnRemixSonicClassicLoadVS(void);

/** 1P: keeps the flag of the human port when the CSS hands over to the game (save_classic_flags_1p_). */
extern void mnRemixSonicClassicSave1P(s32 man_port);

/** 1P: restores the flag of the human port when the CSS opens, clearing the others (load_classic_flag_1p_). */
extern void mnRemixSonicClassicLoad1P(s32 man_port);

/** Training: keeps the flags of the human and CPU ports (save_classic_flags_training_). */
extern void mnRemixSonicClassicSaveTraining(s32 man_port, s32 com_port);

/** Training: restores the flags of the human and CPU ports, clearing the others (load_classic_flags_training_). */
extern void mnRemixSonicClassicLoadTraining(s32 man_port, s32 com_port);

#endif

#endif
