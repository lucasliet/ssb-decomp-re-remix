#ifndef _SC1PREMIX1P_H_
#define _SC1PREMIX1P_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <sc/sctypes.h>
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1premix1pdata.h"

/*
 * Remix 1P (SinglePlayerModes.asm remix_1p_randomization): a random path through the 1P game. The stage table of the mode is read
 * from the Remix build, six of its lines get a random fighter and stage, the Star Fox line a random duo and the next to last line one of
 * three bosses. The path is built when START is pressed on the 1P CSS and read by the 1P game through sc1PRemix1PGetStage.
 */

/** TRUE while the 1P game being played is Remix 1P with its path built. */
extern sb32 sc1PRemix1PIsActive(void);

/** Builds a new path (the hook of the 1P CSS START); no-op outside Remix 1P. */
extern void sc1PRemix1PRandomize(void);

/**
 * Line of the stage table of the Remix path.
 *
 * @param stage_index Stage of the 1P game.
 * @return The line, or NULL when Remix 1P is not active or the stage is past the table.
 */
extern SC1PGameStage* sc1PRemix1PGetStage(s32 stage_index);

/**
 * Sizes the CPU of the stage being set up (remix_1p_giant): giant on the Giant slot, tiny on the Tiny Team slot, normal elsewhere.
 * The size state of the ports other than the player's is reset on the other stages so a size does not carry over. No-op outside Remix 1P.
 *
 * @param player Port of the CPU.
 */
extern void sc1PRemix1PSetupEnemy(s32 player);

/**
 * CPU level of the stage being set up (_opponent_cpu): level 10 on Very Hard. The asm tests the stage flag against a register it already
 * overwrote, so none of its stage exceptions ever matches and every stage gets level 10; the other difficulties keep the table level.
 *
 * @param level Level the stage table gives for the chosen difficulty.
 */
extern s32 sc1PRemix1PEnemyLevel(s32 level);

/** CPU settings of a stage of the Remix path (levels, handicaps, team attack, item rate); NULL when Remix 1P is not active. */
extern SC1PGameComputer* sc1PRemix1PGetComputer(s32 stage_index);

/** What the title card of a stage shows; NULL when Remix 1P is not active or the stage is past the path. */
extern const SC1PRemix1PSlotCard* sc1PRemix1PGetCard(s32 stage_index);

/** Kirby power and hat of a slot of the Kirby team (seven random ones and the Magic Hat Kirby). @param slot 0 to 7. */
extern void sc1PRemix1PGetKirby(s32 slot, s32 *power, s32 *hat);

/** Remix polygons (polygon_id_table): the ids NWARIO to NCRASH, which the Fighting Polygon Team and Race to the Finish draw from. */
#define SC1PREMIX_1P_POLYGON_FIRST REMIX_ID_NWARIO
#define SC1PREMIX_1P_POLYGON_LAST REMIX_ID_NCRASH

/** Random Remix polygon, repeats allowed (polygon_selection_match). */
extern s32 sc1PRemix1PPolygonKind(void);

/** TRUE for the Giga Bowser CPU of the final stage of Remix 1P, which takes the damage and defeat path of Master Hand. @param fkind Fighter kind. @param team Battle team. */
extern sb32 sc1PRemix1PIsFinalBoss(s32 fkind, s32 team);

/**
 * The player left the 1P game through the pause menu (pause_exit): in the Remix 1P mode the size the player had from the character
 * select panel does not carry over to the next run (CharacterSelectDebugMenu.Size.clear_settings_for_1p_). No-op in other modes.
 */
extern void sc1PRemix1POnPauseExit(void);

/** TRUE when a CPU of this kind may not start its grounded up special in Remix 1P (_giga_usp_prevent: Mad Piano, Fox and Falco). @param fkind Fighter kind. @param is_cpu TRUE for a CPU. */
extern sb32 sc1PRemix1PBlocksCpuUpSpecial(s32 fkind, sb32 is_cpu);

/** Damage Giga Bowser takes before it is defeated: 200 on Very Easy and Easy, 250 on Normal and Hard, 300 on Very Hard (giga_stamina_check_). */
extern s32 sc1PRemix1PBossHitPoints(void);

/**
 * Test hook: SSB64_REMIX_1P_BOSS_PRESET_DAMAGE=<n> starts the final boss of the Remix 1P with n percent, so the end of the stage (the
 * wallpaper steps at 90 and 180 damage, then the defeat) can be reached with a few hits. No-op for other fighters or without the variable.
 *
 * @param fp Fighter just made.
 */
extern void sc1PRemix1PPresetBossDamage(FTStruct *fp);

/** TRUE when the Star Fox line is a duo whose partner loads first (`duo_spawn`). */
extern sb32 sc1PRemix1PIsDuoPartnerFirst(void);

/** TRUE when the boss of the path is Metal Mario and Metal Luigi (`metal_bros_flag`). */
extern sb32 sc1PRemix1PIsMetalBros(void);

#endif

#endif
