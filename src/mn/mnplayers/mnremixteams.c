/**
 * mnremixteams.c - values of the Remix yellow team, see mnremixteams.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "ft/ftcommondata.h"
#include "mn/mnplayers/mnremixteams.h"
#include "mn/mnplayers/mnremixteamsrules.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define MNREMIX_TEAMS_WORD_SIZE 4
#define MNREMIX_TEAMS_NAME_SIZE 16
#define MNREMIX_TEAMS_PLAYERS_MAX 4

static u32 mnRemixTeamsReadWord(u32 table, s32 index)
{
    return remix_ram_image_read_word(table + (index * MNREMIX_TEAMS_WORD_SIZE));
}

static SYColorRGB mnRemixTeamsReadRGB(u32 table, s32 index)
{
    u32 rgba = mnRemixTeamsReadWord(table, index);
    SYColorRGB color;

    color.r = (u8)(rgba >> 24);
    color.g = (u8)(rgba >> 16);
    color.b = (u8)(rgba >> 8);

    return color;
}

intptr_t mnRemixTeamsGetButtonOffset(s32 team, const intptr_t *vanilla_offsets)
{
    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return vanilla_offsets[team];
    }
    return (intptr_t)mnRemixTeamsReadWord(REMIX_SYM_Teams__team_button_table, team);
}

s32 mnRemixTeamsGetColor(s32 team)
{
    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return (mnRemixTeamsRulesVanillaColor(team) >= 0) ? mnRemixTeamsRulesVanillaColor(team) : 0;
    }
    return (s32)mnRemixTeamsReadWord(REMIX_SYM_Teams__team_panel_table, team);
}

SYColorRGBA mnRemixTeamsGetShadowColor(s32 team)
{
    SYColorRGBA color;
    u32 rgba;

    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return dFTCommonDataShadowColorTeams[team];
    }
    rgba = mnRemixTeamsReadWord(REMIX_SYM_Teams__team_shadow_colors, team);
    color.r = (u8)(rgba >> 24);
    color.g = (u8)(rgba >> 16);
    color.b = (u8)(rgba >> 8);
    color.a = (u8)rgba;

    return color;
}

sb32 mnRemixTeamsGetCostume(s32 fkind, s32 team, s32 *costume)
{
    s32 index = mnRemixTeamsRulesCostumeIndex(fkind, team);

    if (index < 0)
    {
        return FALSE;
    }
    *costume = (s32)remix_ram_image_read_byte(REMIX_SYM_Teams__new_costume_table + index);

    return TRUE;
}

const char* mnRemixTeamsGetName(s32 team, const char *vanilla_name)
{
    static char sName[MNREMIX_TEAMS_NAME_SIZE];
    u32 address;
    s32 i;

    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return vanilla_name;
    }
    address = mnRemixTeamsReadWord(REMIX_SYM_Teams__team_name_table, team);

    for (i = 0; i < (MNREMIX_TEAMS_NAME_SIZE - 1); i++)
    {
        sName[i] = (char)remix_ram_image_read_byte(address + i);

        if (sName[i] == '\0')
        {
            break;
        }
    }
    sName[i] = '\0';

    return sName;
}

f32 mnRemixTeamsGetNameX(s32 team, f32 vanilla_x)
{
    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return vanilla_x;
    }
    return mnRemixTeamsRulesWordToFloat(mnRemixTeamsReadWord(REMIX_SYM_Teams__team_name_x_position_table, team));
}

f32 mnRemixTeamsGetNameScale(s32 team)
{
    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return 1.0F;
    }
    return mnRemixTeamsRulesWordToFloat(mnRemixTeamsReadWord(REMIX_SYM_Teams__team_name_scale_table, team));
}

s32 mnRemixTeamsGetNamePalette(s32 team)
{
    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return team;
    }
    return (s32)remix_ram_image_read_byte(REMIX_SYM_Teams__team_name_palette_index_table + team);
}

sb32 mnRemixTeamsGetBigTextColors(s32 palette_index, SYColorRGBPair *pair)
{
    u32 address = REMIX_SYM_Teams__big_text_palette_table + mnRemixTeamsRulesBigTextOffset(palette_index);

    if (remix_ram_image_resolve(address) == NULL)
    {
        return FALSE;
    }
    pair->prim.r = (u8)remix_ram_image_read_byte(address);
    pair->prim.g = (u8)remix_ram_image_read_byte(address + 1);
    pair->prim.b = (u8)remix_ram_image_read_byte(address + 2);
    pair->env.r = (u8)remix_ram_image_read_byte(address + 3);
    pair->env.g = (u8)remix_ram_image_read_byte(address + 4);
    pair->env.b = (u8)remix_ram_image_read_byte(address + 5);

    return TRUE;
}

u32 mnRemixTeamsGetAnnouncerFgm(s32 team, u32 vanilla_fgm)
{
    if (mnRemixTeamsRulesIsAdded(team) == 0)
    {
        return vanilla_fgm;
    }
    return mnRemixTeamsReadWord(REMIX_SYM_Teams__team_name_fgm_table, team);
}

SYColorRGB mnRemixTeamsGetShieldTeamColor(s32 team)
{
    return mnRemixTeamsReadRGB(REMIX_SYM_Shield__color_fix___table_team, team);
}

SYColorRGB mnRemixTeamsGetShieldPortColor(s32 port)
{
    return mnRemixTeamsReadRGB(REMIX_SYM_Shield__color_fix___table_ffa, port);
}

#endif
