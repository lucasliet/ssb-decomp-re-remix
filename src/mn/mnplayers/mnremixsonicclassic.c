/**
 * mnremixsonicclassic.c - Classic Sonic on the character select screens, see mnremixsonicclassic.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sys/objtypes.h>
#include "ft/ftremix/sonic/ftremixsonicclassic.h"
#include "ft/ftremix/sonic/ftremixsonicclassicrules.h"
#include "mn/mnplayers/mnremixsonicclassic.h"
#include "remix/remix_roster_data.h"

/* Sonic.classic_flags_vs, classic_flag_1p and classic_flags_training. */
static u8 sMNRemixSonicClassicFlagsVS[GMCOMMON_PLAYERS_MAX];
static u8 sMNRemixSonicClassicFlag1P;
static u8 sMNRemixSonicClassicFlagsTraining[2];

static void mnRemixSonicClassicReadTable(u8 *table)
{
    s32 player;

    for (player = 0; player < GMCOMMON_PLAYERS_MAX; player++)
    {
        table[player] = (ftRemixSonicIsClassic(player) != FALSE) ? 1 : 0;
    }
}

static void mnRemixSonicClassicWriteTable(const u8 *table)
{
    s32 player;

    for (player = 0; player < GMCOMMON_PLAYERS_MAX; player++)
    {
        ftRemixSonicSetClassic(player, table[player]);
    }
}

f32 mnRemixSonicClassicToggle(s32 player, s32 fkind, s32 button, s32 old_costume, s32 new_costume, sb32 is_team_battle, GObj *fighter_gobj)
{
    f32 frame;

    if (fkind != REMIX_ID_SONIC)
    {
        return MNREMIX_SONIC_CLASSIC_NO_TOGGLE;
    }
    if (ftRemixSonicClassicRulesShouldToggle(is_team_battle, button, old_costume, new_costume) == 0)
    {
        return MNREMIX_SONIC_CLASSIC_NO_TOGGLE;
    }
    ftRemixSonicSetClassic(player, (ftRemixSonicIsClassic(player) == FALSE) ? TRUE : FALSE);

    frame = (fighter_gobj != NULL) ? fighter_gobj->anim_frame : 0.0F;

    return ftRemixSonicClassicRulesSelectFrame(frame);
}

void mnRemixSonicClassicRestoreFrame(s32 player, GObj *fighter_gobj, f32 frame)
{
    ftRemixSonicClassicSetSelectFrame(player, frame);

    if (fighter_gobj != NULL)
    {
        fighter_gobj->anim_frame = frame;
    }
}

s32 mnRemixSonicClassicCycleTeamCostume(s32 player, s32 fkind, sb32 is_team_battle, s32 team, s32 cycled_costume)
{
    if ((fkind != REMIX_ID_SONIC) || (is_team_battle == FALSE))
    {
        return cycled_costume;
    }
    return ftRemixSonicClassicRulesCycleTeamCostume(team, ftRemixSonicIsClassic(player), cycled_costume);
}

s32 mnRemixSonicClassicGetTeamCostume(s32 player, s32 fkind, s32 team)
{
    s32 costume = ftParamGetCostumeTeamID(fkind, team);

    if (fkind != REMIX_ID_SONIC)
    {
        return costume;
    }
    return ftRemixSonicClassicRulesTeamCostume(team, ftRemixSonicIsClassic(player), costume);
}

s32 mnRemixSonicClassicFudgeCostume(s32 player, s32 fkind, s32 costume)
{
    if (fkind != REMIX_ID_SONIC)
    {
        return costume;
    }
    return ftRemixSonicClassicRulesFudgeCostume(costume, ftRemixSonicIsClassic(player));
}

void mnRemixSonicClassicReset(s32 player)
{
    ftRemixSonicSetClassic(player, FALSE);
}

void mnRemixSonicClassicSaveVS(void)
{
    mnRemixSonicClassicReadTable(sMNRemixSonicClassicFlagsVS);
}

void mnRemixSonicClassicLoadVS(void)
{
    mnRemixSonicClassicWriteTable(sMNRemixSonicClassicFlagsVS);
}

void mnRemixSonicClassicSave1P(s32 man_port)
{
    sMNRemixSonicClassicFlag1P = (ftRemixSonicIsClassic(man_port) != FALSE) ? 1 : 0;
}

void mnRemixSonicClassicLoad1P(s32 man_port)
{
    u8 table[GMCOMMON_PLAYERS_MAX];

    ftRemixSonicClassicRulesLoadOne(table, GMCOMMON_PLAYERS_MAX, man_port, sMNRemixSonicClassicFlag1P);
    mnRemixSonicClassicWriteTable(table);
}

void mnRemixSonicClassicSaveTraining(s32 man_port, s32 com_port)
{
    sMNRemixSonicClassicFlagsTraining[0] = (ftRemixSonicIsClassic(man_port) != FALSE) ? 1 : 0;
    sMNRemixSonicClassicFlagsTraining[1] = (ftRemixSonicIsClassic(com_port) != FALSE) ? 1 : 0;
}

void mnRemixSonicClassicLoadTraining(s32 man_port, s32 com_port)
{
    u8 table[GMCOMMON_PLAYERS_MAX];

    ftRemixSonicClassicRulesLoadTwo(table, GMCOMMON_PLAYERS_MAX, man_port, sMNRemixSonicClassicFlagsTraining[0], com_port, sMNRemixSonicClassicFlagsTraining[1]);
    mnRemixSonicClassicWriteTable(table);
}

#endif
