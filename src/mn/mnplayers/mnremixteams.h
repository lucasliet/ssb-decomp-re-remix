#ifndef _MNREMIXTEAMS_H_
#define _MNREMIXTEAMS_H_

#include <ssb_types.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Yellow team (src/Teams.asm). The Remix adds a fourth team to the VS team battle. Its values live in the Teams.* tables of the
 * Remix RAM image (Teams.add_team, Teams.add_team_costume in the character files, Shield.asm table_team); the vanilla teams keep the
 * decomp values, so these getters take the vanilla value or read it from the decomp tables and only read the image for team 3.
 *
 * Team ids: 0 red, 1 blue, 2 green, 3 yellow (MNREMIX_TEAM_YELLOW). The colour index of the panel, the gate, the series logo, the
 * results wallpaper and columns and the port colour is the same value (Teams.team_panel_table, Teams.team_port_color_table).
 */

/**
 * Sprite offset of the team button image (Teams.team_button_table).
 *
 * @param team Team id.
 * @param vanilla_offsets Offsets of red, blue and green in the CSS common file.
 */
extern intptr_t mnRemixTeamsGetButtonOffset(s32 team, const intptr_t *vanilla_offsets);

/** Colour index of a team: red 0, blue 1, green 3, yellow from Teams.team_panel_table. 0 for an invalid team. */
extern s32 mnRemixTeamsGetColor(s32 team);

/** Shadow colour under a fighter of the team (Teams.team_shadow_colors). */
extern SYColorRGBA mnRemixTeamsGetShadowColor(s32 team);

/**
 * Costume of a character on the yellow team (Teams.new_costume_table, filled by Teams.add_team_costume).
 *
 * @param fkind Fighter kind.
 * @param team Team id.
 * @param costume Receives the costume.
 * @return TRUE when `team` is an added team and the table was read, FALSE (nothing written) otherwise.
 */
extern sb32 mnRemixTeamsGetCostume(s32 fkind, s32 team, s32 *costume);

/** Name drawn on the results screen (Teams.team_name_table): the vanilla string for red, blue and green. */
extern const char* mnRemixTeamsGetName(s32 team, const char *vanilla_name);

/** X of the team name on the results screen (Teams.team_name_x_position_table). */
extern f32 mnRemixTeamsGetNameX(s32 team, f32 vanilla_x);

/** Scale of the team name on the results screen (Teams.team_name_scale_table; the vanilla teams are hard coded to 1.0). */
extern f32 mnRemixTeamsGetNameScale(s32 team);

/** Palette index of the team name (Teams.team_name_palette_index_table): the team id for the vanilla teams. */
extern s32 mnRemixTeamsGetNamePalette(s32 team);

/**
 * Colours of a big text palette index (Teams.big_text_palette_table).
 *
 * @param palette_index Index past the five vanilla palettes.
 * @param pair Receives the primary and secondary colour.
 * @return TRUE when read, FALSE (pair untouched) when the Remix image is not loaded.
 */
extern sb32 mnRemixTeamsGetBigTextColors(s32 palette_index, SYColorRGBPair *pair);

/** Announcer FGM of "<team> team wins" (Teams.team_name_fgm_table); the vanilla id for red, blue and green. */
extern u32 mnRemixTeamsGetAnnouncerFgm(s32 team, u32 vanilla_fgm);

/** Env colour of a Default shield in a team battle (Shield.asm table_team). */
extern SYColorRGB mnRemixTeamsGetShieldTeamColor(s32 team);

/** Env colour of a Default shield by port (Shield.asm table_ffa). */
extern SYColorRGB mnRemixTeamsGetShieldPortColor(s32 port);

#endif

#endif
