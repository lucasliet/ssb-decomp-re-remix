#ifndef _MNREMIXTEAMSRULES_H_
#define _MNREMIXTEAMSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix team extension (Teams.asm): a fourth team, yellow, after red, blue and green. Ints only, so the headless
 * unit tests (tests/remix) can run them. The values of the added team (button image, colours, costumes, name, announcer) are read
 * from the Remix tables by mnremixteams.c.
 */

#define MNREMIX_TEAMS_VANILLA_COUNT 3
#define MNREMIX_TEAMS_ADDED_COUNT 1
#define MNREMIX_TEAMS_COUNT 4
#define MNREMIX_TEAM_YELLOW 3
#define MNREMIX_TEAMS_LAST MNREMIX_TEAM_YELLOW

/** Size of one Teams.big_text_palette_table entry: the primary and the secondary colour, three bytes each. */
#define MNREMIX_TEAMS_BIG_TEXT_ENTRY_SIZE 6

/** First palette index of the added teams in the big text palette table (Teams.add_team: the team index plus 2). */
#define MNREMIX_TEAMS_NAME_PALETTE_SHIFT 2

/**
 * "WINS!" x of the results screen for the yellow team. The Remix leaves the team branch of mnVSResultsMakeWinnerText
 * (0x8013423C) unpatched, so x_teams[3] (sp+0x28 + 12) reads sp+0x34, the first entry of the x_fkinds copy: 175.0.
 */
#define MNREMIX_TEAMS_YELLOW_WINS_X 175.0F

/** What colours the Default shield of a fighter (Shield.asm color_fix_ _teams_check). */
#define MNREMIX_TEAMS_SHIELD_TEAM 0
#define MNREMIX_TEAMS_SHIELD_CPU 1
#define MNREMIX_TEAMS_SHIELD_PORT 2

/**
 * Team after `team` in the team button cycle (Teams.asm extend_button_click_: the last team id is NUM_TEAMS - 1): red, blue,
 * green, yellow, then red again.
 */
extern int mnRemixTeamsRulesNextTeam(int team);

/** TRUE (1) for a team the Remix adds, FALSE (0) for red, blue and green and for ids that are not a team. */
extern int mnRemixTeamsRulesIsAdded(int team);

/**
 * Colour index (panel, gate, series logo, results wallpaper and columns, port colour) of a vanilla team: red 0, blue 1, green 3.
 * The added teams read their own value from the Remix table. Returns -1 for an added team or an invalid id.
 */
extern int mnRemixTeamsRulesVanillaColor(int team);

/**
 * Index in Teams.new_costume_table of the costume of a character on an added team (one byte per character and added team), or
 * -1 when the team is not an added one or the fighter kind is negative.
 */
extern int mnRemixTeamsRulesCostumeIndex(int fkind, int team);

/** Offset in Teams.big_text_palette_table of a big text palette index. */
extern int mnRemixTeamsRulesBigTextOffset(int palette_index);

/** Float of a 32 bit pattern (the x position and scale tables hold floats as words). */
extern float mnRemixTeamsRulesWordToFloat(unsigned int word);

/**
 * Where the Default shield colour of a fighter comes from: in a VS team battle the team colour, otherwise a CPU is black and a
 * human has the colour of its port (Shield.asm color_fix_ _teams_check).
 *
 * @param is_vs_team_battle Nonzero in a VS team battle (the asm checks that the match is the VS one).
 * @param is_cpu Nonzero for a CPU fighter.
 * @return MNREMIX_TEAMS_SHIELD_TEAM, _CPU or _PORT.
 */
extern int mnRemixTeamsRulesShieldSource(int is_vs_team_battle, int is_cpu);

/** Highest team id the results screen counts (Teams.asm: the winner loop bound 3 + NUM_NEW_TEAMS). */
extern int mnRemixTeamsRulesLastResultTeam(void);

#endif

#endif
