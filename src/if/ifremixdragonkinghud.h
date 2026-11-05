#ifndef _IFREMIXDRAGONKINGHUD_H_
#define _IFREMIXDRAGONKINGHUD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Dragon King HUD (DragonKingHUD.asm, Toggles.entry_dragon_king_hud) and the timer placement shared with Input Display
 * (InputDisplay.asm adjust_timer_position_). Rules: if/ifremixdragonkinghudrules.h. The hooks sit in if/ifcommon.c; the
 * Y helpers below are also called by the Training, Bonus, 1P Game, How to Play and VS demo scenes.
 */

/* Y the Dragon King HUD gives to the HUD parts of the other scenes (the constants of the scopes of DragonKingHUD.asm). */
#define IFREMIX_DKHUD_Y_TRAINING_DAMAGE 196.0F      /* training_mode reposition_training_hud_damage_ (0x4344) */
#define IFREMIX_DKHUD_Y_TRAINING_COMBO 212.0F       /* reposition_training_hud_combo_ (0x4354) */
#define IFREMIX_DKHUD_Y_TRAINING_SPEED_ENEMY 196.0F /* reposition_training_hud_speed_enemy_ (0x4344) */
#define IFREMIX_DKHUD_Y_TRAINING_ITEM 212.0F        /* reposition_training_hud_item_ (0x4354) */
#define IFREMIX_DKHUD_TRAINING_LABEL_OFFSET_Y 176   /* reposition_training_hud_labels_ (0xB0) */
#define IFREMIX_DKHUD_Y_BONUS_TARGETS 206           /* bonus reposition_targets_ (0xCE) */
#define IFREMIX_DKHUD_Y_MULTI_ENEMY_ICONS 196       /* single_player reposition_multi_enemy_icons_ (0xC4) */
#define IFREMIX_DKHUD_Y_VS_DEMO_NAME 200.0F         /* vs_demo move_name_down_ (0x4348) */
#define IFREMIX_DKHUD_KO_DIGIT_SPEED_Y 256.0F       /* disable_ko_falling_damage_animation_ (FASTER_SPEED 0x4380): the KO digits leave the screen at once */

/** TRUE when the Dragon King HUD layout applies to the current match (toggle, then Dragon King and Dragon King Remix stages). */
extern sb32 ifRemixDragonKingHudIsOn(void);

/**
 * Y of a HUD part: the Dragon King one when the layout applies, else the vanilla one.
 *
 * @param vanilla_y Y without the Dragon King HUD.
 * @param dragon_king_y Y of the Dragon King HUD, one of the IFREMIX_DKHUD_Y_ constants.
 * @return The Y to use.
 */
extern f32 ifRemixDragonKingHudPickY(f32 vanilla_y, f32 dragon_king_y);

/**
 * Same as ifRemixDragonKingHudPickY for an integer Y (Bonus targets, 1P Game enemy icons).
 */
extern s32 ifRemixDragonKingHudPickIntY(s32 vanilla_y, s32 dragon_king_y);

/**
 * Y offset added to the Training HUD titles (DAMAGE, COMBO, SPEED, CPU, ITEM): IFREMIX_DKHUD_TRAINING_LABEL_OFFSET_Y or 0.
 * Only the HUD titles take it, not the titles of the pause menu.
 */
extern s32 ifRemixDragonKingHudTrainingLabelOffsetY(void);

/**
 * Y of the player damage numbers for the scene that sets it (set_vertical_position_ at the VS/Training, Bonus, How to Play and
 * One Player game sites).
 *
 * @param vanilla_y Y the scene sets without the Dragon King HUD (210, or 150 in How to Play).
 * @param is_one_player_game TRUE only for the One Player game site, which keeps the vanilla Y on the Dragon King Remix stage.
 * @return The Y to store in gIFCommonPlayerInterface.player_pos_y.
 */
extern s32 ifRemixDragonKingHudDamageY(s32 vanilla_y, sb32 is_one_player_game);

/** Sprite offset of a damage digit in the damage file when the layout applies (dragon_king_numbers). */
extern intptr_t ifRemixDragonKingHudDigitOffset(s32 image_id);

/** Width of a damage digit used for the spacing when the layout applies (dragon_king_widths). */
extern s32 ifRemixDragonKingHudDigitWidth(s32 image_id);

/**
 * Replaces the colour of the damage numbers of a player by the Dragon King one (set_damage_color_).
 *
 * @param player Port of the numbers.
 * @param red In and out.
 * @param green In and out.
 * @param blue In and out.
 */
extern void ifRemixDragonKingHudApplyDamageColor(s32 player, u8 *red, u8 *green, u8 *blue);

/** Y of the match timer digits and colon: Dragon King HUD, then port 4's Input Display at the top, else 30. */
extern f32 ifRemixTimerGetDigitsY(void);

#endif
#endif
