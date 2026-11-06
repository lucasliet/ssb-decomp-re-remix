#ifndef _SC1PREMIXGAME_H_
#define _SC1PREMIXGAME_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Glue between the stage setup of the 1P game (sc1pgame.c) and the rules of the Remix 1P modes (sc1premixgamerules.h): reads the
 * mode and the stage being played and applies the rule.
 */

/**
 * Direction a fighter spawns facing (spawn_human_left_remix_rttf_).
 *
 * @param lr Direction the stage tables give, +1 right or -1 left.
 * @return The direction to use.
 */
extern s32 sc1PRemixGameSpawnFacing(s32 lr);

/**
 * Alpha of the screen flash of the stage (battle_style).
 *
 * @param table_alpha Alpha of the stage table row.
 * @return The alpha to use.
 */
extern s32 sc1PRemixGameFlashAlpha(s32 table_alpha);

/**
 * Computer trait of an enemy of the stage (_opponent_behavior).
 *
 * @param table_trait Trait of the stage table row.
 * @return The trait to use.
 */
extern s32 sc1PRemixGameEnemyTrait(s32 table_trait);

/**
 * CPU level of an enemy before the continue level drop (_opponent_cpu of the horde modes).
 *
 * @param level Level the stage table gives.
 * @return The level to use.
 */
extern s32 sc1PRemixGameHordeEnemyLevel(s32 level);

/**
 * Costume the enemies of the stage being set up start with (singleplayer_polygon_team_costume_fix_): 1 when the human plays a
 * default-costume polygon on a polygon stage, 0 otherwise.
 */
extern s32 sc1PRemixGamePolygonTeamCostume(void);

/** TRUE when the KO of an enemy leaves the enemy stock counter alone (_infinite_ko). */
extern sb32 sc1PRemixGameKeepsEnemyStocks(void);

/**
 * The stage just ended: turns the won All-Star final into the Game Clear stage (end_allstar), so the stage clear, the bonuses and the
 * music are the ones of the end of the game.
 *
 * @param is_stage_won TRUE when the last enemy of the stage was beaten.
 */
extern void sc1PRemixGameOnStageEnd(sb32 is_stage_won);

/** TRUE when the stage clear awards the No Damage bonus (no_miss_2). */
extern sb32 sc1PRemixGameAwardsNoDamage(void);

/** TRUE when the stage clear awards the Bros. Calamity bonus (no_calamity). */
extern sb32 sc1PRemixGameAwardsBrosCalamity(void);

/** Masks the item switches of the battle being set up and sets the extended ones (prevent_1p_custom_item_spawns_). */
extern void sc1PRemixGameSetupItemToggles(void);

#endif

#endif
