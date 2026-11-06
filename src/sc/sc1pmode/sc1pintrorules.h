#ifndef _SC1PINTRORULES_H_
#define _SC1PINTRORULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Rules of the 1P title cards (SinglePlayer.asm fix_port_index_, singleplayer_gdk_polygon_team_costume_preview_fix_,
 * singleplayer_mm_costume_preview_fix_, SinglePlayerModes.asm title_card_text_routine and tiny_team_screen_card) that need no game
 * state, kept apart so the headless unit tests (tests/remix) run them. Fighter kinds are the Remix ids (remix_roster_data.h) and
 * stages are the 1P stage indices of the game (nSC1PGameStage*).
 */

#define SC1PINTRO_RULES_STAGE_YOSHI 1
#define SC1PINTRO_RULES_STAGE_DONKEY 6
#define SC1PINTRO_RULES_STAGE_MMARIO 10

/**
 * Port index stored in the player struct of a card fighter of the human side (fix_port_index_, human and allies title card), so the
 * Visibility option of that port applies. The human takes the positions 0, 1 and 3 of the card; the allies take the others and use
 * the other of P1 and P2.
 *
 * @param card_anim_frame_id Position of the fighter on the card (0 alone, 1 and 3 the human with allies, 2, 4 and 5 the allies).
 * @param human_port Port index of the human player, 0 to 3.
 * @return Port index 0 to 3.
 */
extern int sc1PIntroRulesHumanSidePort(int card_anim_frame_id, int human_port);

/**
 * Port index stored in the player struct of an opponent of the card (fix_port_index_._opponent): P2 when the human is P1, otherwise P1.
 *
 * @param human_port Port index of the human player, 0 to 3.
 * @return 0 or 1.
 */
extern int sc1PIntroRulesOpponentPort(int human_port);

/**
 * Costume index of the opponent shown on the VS card, 0 or 1 (singleplayer_gdk_polygon_team_costume_preview_fix_): the alternate
 * costume 1 when the human has costume 0 and is Giant DK facing DK, or a polygon facing one of the original polygons, and in
 * Remix 1P on the Giant stage when the human is the same fighter as the Giant CPU.
 *
 * @param cpu_fkind Fighter kind of the opponent.
 * @param human_fkind Fighter kind of the human player.
 * @param human_costume Costume id of the human player.
 * @param is_remix_1p Non-zero in the Remix 1P mode.
 * @param stage 1P stage index of the card.
 * @return 0 for the default costume, 1 for the alternate one.
 */
extern int sc1PIntroRulesOpponentCostumeIndex(int cpu_fkind, int human_fkind, int human_costume, int is_remix_1p, int stage);

/**
 * Costume index of the Mario slot of the duo card when its default costume matches the human's (singleplayer_mm_costume_preview_fix_):
 * the alternate costume 1 as in the vanilla card, except for Metal Mario on the Remix 1P boss stage, which takes it only when the
 * human is Metal Mario with costume 0.
 *
 * @param is_remix_1p Non-zero in the Remix 1P mode.
 * @param stage 1P stage index of the card.
 * @param slot_fkind Fighter kind shown in the Mario slot.
 * @param human_fkind Fighter kind of the human player.
 * @param human_costume Costume id of the human player.
 * @return 0 for the default costume, 1 for the alternate one.
 */
extern int sc1PIntroRulesDuoMarioSlotCostumeIndex(int is_remix_1p, int stage, int slot_fkind, int human_fkind, int human_costume);

/**
 * Whether the opponent name sprite is cut to its short width ("Marina", "Banjo", "Crash") on the card of a stage: only the Team and
 * Giant texts of title_card_text_routine do it; the other stages show the full name.
 *
 * @param stage 1P stage index of the card.
 * @return Non-zero for the Team and Giant stages.
 */
extern int sc1PIntroRulesNameWidthApplies(int stage);

/**
 * Costume of a member of the Tiny Team card (tiny_team_screen_card): the member index modulo the fighter kind, so the three tiny
 * opponents differ. The asm divides by the kind and traps on kind 0 (Mario); the port uses costume 0 there.
 *
 * @param member_index Member of the team, 0 to 2.
 * @param fkind Fighter kind of the team.
 * @return Costume id passed to the fighter.
 */
extern int sc1PIntroRulesTinyTeamCostume(int member_index, int fkind);

#endif

#endif
