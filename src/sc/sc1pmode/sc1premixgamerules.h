#ifndef _SC1PREMIXGAMERULES_H_
#define _SC1PREMIXGAMERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the Remix 1P modes that sc1pgame.c applies while it sets up and runs a stage (SinglePlayerModes.asm and
 * Item.asm), kept apart so the headless unit tests (tests/remix) run them. Modes are SC1PREMIX_MODE_* ids.
 */

/* battle_style: alpha of the screen flash of the horde modes and Bonus 3. */
#define SC1PREMIX_GAME_HORDE_FLASH_ALPHA 0x80

/* Stage numbers (STAGE_FLAG) and computer traits the enemy rules compare or write. */
#define SC1PREMIX_GAME_STAGE_ALLSTAR_TRIPLE 5
#define SC1PREMIX_GAME_STAGE_ALLSTAR_FINAL 0x0C
#define SC1PREMIX_GAME_TRAIT_DEFAULT 0
#define SC1PREMIX_GAME_TRAIT_POLYGON_TEAM 4
#define SC1PREMIX_GAME_TRAIT_BONUS3 8

/* _opponent_cpu: CPU levels of the polygons of the horde modes. */
#define SC1PREMIX_GAME_MULTIMAN_LEVEL 4
#define SC1PREMIX_GAME_CRUEL_LEVEL 9

/* prevent_1p_custom_item_spawns_ */
#define SC1PREMIX_GAME_VANILLA_ITEM_MASK 0x000FFFFFu
#define SC1PREMIX_GAME_ALLSTAR_ITEM_MASK 0xDFFFFFCFu

/**
 * Direction a fighter faces when it spawns (spawn_human_left_remix_rttf_): every fighter faces left on the Remix Race to the
 * Finish course, the asm only meant the human.
 *
 * @param gkind Remix stage id (nGRRemixStage_*) of the battle.
 * @param lr Direction the stage tables give, +1 right or -1 left.
 * @return The direction to use.
 */
extern int sc1PRemixModeSpawnLR(int gkind, int lr);

/**
 * Alpha of the screen flash (battle_style): 0x80 in Bonus 3, Multi-Man and Cruel Multi-Man, the stage table value otherwise.
 *
 * @param mode Mode id.
 * @param table_alpha Alpha of the stage table row.
 * @return The alpha to use.
 */
extern int sc1PRemixModeFlashAlpha(int mode, int table_alpha);

/**
 * Computer trait of an enemy (_opponent_behavior): the Race to the Finish trait in Bonus 3, the polygon team trait in the horde
 * modes, the standard trait on the All-Star triple battle, the stage table value otherwise.
 *
 * @param mode Mode id.
 * @param stage Stage the 1P run is on.
 * @param table_trait Trait of the stage table row.
 * @return The trait to use.
 */
extern int sc1PRemixModeEnemyTrait(int mode, int stage, int table_trait);

/**
 * CPU level of a polygon of the horde modes (_opponent_cpu): 4 in Multi-Man and 9 in Cruel Multi-Man. Bonus 3 and Remix 1P
 * have their own level rules.
 *
 * @param mode Mode id.
 * @param level Level the stage table gives.
 * @return The level to use.
 */
extern int sc1PRemixModeHordeEnemyLevel(int mode, int level);

/**
 * Costume of the enemies of a stage when the human plays a default-costume polygon on a polygon stage
 * (singleplayer_polygon_team_costume_fix_): the alternate costume 1, so the enemies are told apart from the human. Polygon stages
 * are Duel Zone up to Race to the Finish, Battlefield and the Remix Race to the Finish.
 *
 * @param human_costume Costume of the human fighter.
 * @param human_fkind Fighter kind of the human.
 * @param gkind Remix stage id (nGRRemixStage_*) of the battle.
 * @return 1 for the alternate costume, 0 for the default one.
 */
extern int sc1PRemixModePolygonTeamCostume(int human_costume, int human_fkind, int gkind);

/**
 * Whether the KO of an enemy leaves the stock counter alone (_infinite_ko): Bonus 3, Multi-Man, Cruel Multi-Man and the
 * Home-Run Contest never end through the enemy stocks.
 *
 * @param mode Mode id.
 * @return Non-zero when the enemy stocks are not decremented.
 */
extern int sc1PRemixModeKeepsEnemyStocks(int mode);

/**
 * Whether the end of the stage turns into the Game Clear stage (end_allstar): when All-Star wins its final battle, the 1P stage
 * becomes the Master Hand one (0xD), which gives the Game Clear screen, bonuses and music. A lost final is not changed: the
 * asm repairs the stage in progress_advancement, the port retries the stage it has.
 *
 * @param mode Mode id.
 * @param stage Stage the 1P run is on.
 * @param is_stage_won Non-zero when the last enemy of the stage was beaten.
 * @return Non-zero when the stage has to become the Game Clear one.
 */
extern int sc1PRemixModeEndsAllstarRun(int mode, int stage, int is_stage_won);

/**
 * Whether the run awards the No Damage bonus (no_miss_2): All-Star never does.
 *
 * @param mode Mode id.
 */
extern int sc1PRemixModeAwardsNoDamage(int mode);

/**
 * Whether the run awards the Bros. Calamity bonus (no_calamity): only the standard 1P run does.
 *
 * @param mode Mode id.
 */
extern int sc1PRemixModeAwardsBrosCalamity(int mode);

/**
 * Item switch mask of the 1P game (prevent_1p_custom_item_spawns_): Remix 1P keeps the stage mask, All-Star also removes the
 * healing items, every other mode keeps the 20 vanilla items only.
 *
 * @param mode Mode id.
 * @param item_toggles Item mask of the stage table.
 * @return The mask the battle uses.
 */
extern unsigned int sc1PRemixModeItemToggles(int mode, unsigned int item_toggles);

/**
 * Remix extended item switches of the 1P game (prevent_1p_custom_item_spawns_): Remix 1P keeps the current ones, All-Star
 * uses the defaults, every other mode has none.
 *
 * @param mode Mode id.
 * @param current Extended switches in use.
 * @param default_toggles Extended switches of a fresh game.
 * @return The switches the battle uses.
 */
extern unsigned int sc1PRemixModeExtendedItemToggles(int mode, unsigned int current, unsigned int default_toggles);

#endif

#endif
