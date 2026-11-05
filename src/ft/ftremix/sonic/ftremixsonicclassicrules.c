/**
 * ftremixsonicclassicrules.c - pure Classic Sonic decisions, see ftremixsonicclassicrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/sonic/ftremixsonicclassicrules.h"

int ftRemixSonicClassicRulesPlayFgm(int fgm_id)
{
    switch (fgm_id)
    {
    case FTREMIX_SONIC_FGM_SPINDASH_CHARGE:
        return FTREMIX_SONIC_FGM_CLASSIC_SPINDASH_CHARGE;

    case FTREMIX_SONIC_FGM_SPINDASH_ROLL:
    case FTREMIX_SONIC_FGM_HOMING_ATTACK:
        return FTREMIX_SONIC_FGM_CLASSIC_SPINDASH_ATTACK;

    case FTREMIX_SONIC_FGM_JUMP:
        return FTREMIX_SONIC_FGM_CLASSIC_JUMP;

    case FTREMIX_SONIC_FGM_SKID:
        return FTREMIX_SONIC_FGM_CLASSIC_SKID;

    case FTREMIX_SONIC_FGM_JUMP_2:
        return FTREMIX_SONIC_FGM_CLASSIC_JUMP_2;

    default:
        return fgm_id;
    }
}

int ftRemixSonicClassicRulesPlayLoopFgm(int fgm_id)
{
    switch (fgm_id)
    {
    case FTREMIX_SONIC_FGM_SPINDASH_CHARGE:
        return FTREMIX_SONIC_FGM_CLASSIC_SPINDASH_CHARGE;

    case FTREMIX_SONIC_FGM_HOMING_CHARGE:
        return FTREMIX_SONIC_FGM_CLASSIC_SPIN;

    default:
        return fgm_id;
    }
}

int ftRemixSonicClassicRulesShouldToggle(int is_team_battle, int button, int old_costume, int new_costume)
{
    if ((button != FTREMIX_SONIC_BUTTON_RIGHT) && (button != FTREMIX_SONIC_BUTTON_LEFT))
    {
        return 0;
    }
    if (is_team_battle != 0)
    {
        return 1;
    }
    if (button == FTREMIX_SONIC_BUTTON_RIGHT)
    {
        return (new_costume == 0) ? 1 : 0;
    }
    return (old_costume == 0) ? 1 : 0;
}

int ftRemixSonicClassicRulesCycleTeamCostume(int team_id, int is_classic, int cycled_costume)
{
    if (team_id != FTREMIX_SONIC_TEAM_YELLOW)
    {
        return cycled_costume;
    }
    return (is_classic != 0) ? FTREMIX_SONIC_YELLOW_COSTUME_CLASSIC : FTREMIX_SONIC_YELLOW_COSTUME_MODERN;
}

int ftRemixSonicClassicRulesTeamCostume(int team_id, int is_classic, int default_costume)
{
    if ((team_id == FTREMIX_SONIC_TEAM_YELLOW) && (is_classic != 0))
    {
        return FTREMIX_SONIC_YELLOW_COSTUME_CLASSIC;
    }
    return default_costume;
}

int ftRemixSonicClassicRulesFudgeCostume(int costume, int is_classic)
{
    return (is_classic != 0) ? (costume + FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET) : costume;
}

int ftRemixSonicClassicRulesSplitTrainingCostume(int costume, int *is_classic)
{
    if (costume < FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET)
    {
        *is_classic = 0;
        return costume;
    }
    *is_classic = 1;
    return costume - FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET;
}

int ftRemixSonicClassicRulesTrainingMenuMax(int modern_max, int is_sonic)
{
    return (is_sonic != 0) ? (modern_max + FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET) : modern_max;
}

float ftRemixSonicClassicRulesSelectFrame(float current_frame)
{
    return (current_frame == 0.0F) ? FTREMIX_SONIC_SELECT_ANIM_LENGTH : current_frame;
}

int ftRemixSonicClassicRulesMenuAnimFile(int anim_file_id)
{
    switch (anim_file_id)
    {
    case FTREMIX_SONIC_FILE_1P_POSE:
        return FTREMIX_SONIC_FILE_CLASSIC_1P_POSE;

    case FTREMIX_SONIC_FILE_VICTORY2:
        return FTREMIX_SONIC_FILE_CLASSIC_VICTORY1;

    default:
        return 0;
    }
}

int ftRemixSonicClassicRulesIsMenuAnimScene(int scene_kind)
{
    return ((scene_kind == FTREMIX_SONIC_SCENE_1P_INTRO) || (scene_kind == FTREMIX_SONIC_SCENE_VS_RESULTS)) ? 1 : 0;
}

void ftRemixSonicClassicRulesLoadOne(unsigned char *table, int port_count, int port, int flag)
{
    ftRemixSonicClassicRulesLoadTwo(table, port_count, port, flag, -1, 0);
}

void ftRemixSonicClassicRulesLoadTwo(unsigned char *table, int port_count, int port_a, int flag_a, int port_b, int flag_b)
{
    int port;

    for (port = 0; port < port_count; port++)
    {
        table[port] = 0;
    }
    if ((port_a >= 0) && (port_a < port_count))
    {
        table[port_a] = (flag_a != 0) ? 1 : 0;
    }
    if ((port_b >= 0) && (port_b < port_count))
    {
        table[port_b] = (flag_b != 0) ? 1 : 0;
    }
}

#endif
