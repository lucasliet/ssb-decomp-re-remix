/**
 * ifremixdragonkinghudrules.c - pure Dragon King HUD rules, see ifremixdragonkinghudrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixdragonkinghudrules.h"
#include "gr/grremix/grremixids.h"

#define IFREMIX_DKHUD_TIMER_Y 212.0F
#define IFREMIX_DKHUD_TIMER_Y_INPUT_DISPLAY_TOP 52.0F
#define IFREMIX_DKHUD_TIMER_Y_NORMAL 30.0F
#define IFREMIX_DKHUD_INPUT_DISPLAY_TOP 1

static const unsigned int sIFRemixDragonKingHudDigitOffsets[IFREMIX_DKHUD_DIGIT_COUNT] =
{
    0x17B8, 0x1998, 0x1C38, 0x1ED8, 0x2178, 0x2418, 0x26B8, 0x2898, 0x2A78, 0x2C58, 0x2EF8, 0x3198
};

static const int sIFRemixDragonKingHudDigitWidths[IFREMIX_DKHUD_DIGIT_COUNT] =
{
    0x11, 0x0C, 0x12, 0x11, 0x12, 0x10, 0x12, 0x11, 0x12, 0x12, 0x14, 0x17
};

/* Color.dragon_king RED, BLUE, YELLOW and GREEN with the alpha dropped */
static const unsigned char sIFRemixDragonKingHudRgbFfa[IFREMIX_DKHUD_COLOR_COUNT][3] =
{
    { 0xF7, 0xD4, 0xD2 }, { 0xC2, 0xC9, 0xFD }, { 0xEB, 0xE7, 0x8D }, { 0xC3, 0xFB, 0xCE }
};

static const unsigned char sIFRemixDragonKingHudRgbTeam[IFREMIX_DKHUD_COLOR_COUNT][3] =
{
    { 0xF7, 0xD4, 0xD2 }, { 0xC2, 0xC9, 0xFD }, { 0xC3, 0xFB, 0xCE }, { 0xEB, 0xE7, 0x8D }
};

static void ifRemixDragonKingHudRulesCopyRgb(const unsigned char *source, unsigned char *rgb)
{
    rgb[0] = source[0];
    rgb[1] = source[1];
    rgb[2] = source[2];
}

int ifRemixDragonKingHudRulesIsOn(int toggle, int gkind)
{
    if (toggle == IFREMIX_DKHUD_TOGGLE_OFF)
    {
        return 0;
    }
    if (toggle == IFREMIX_DKHUD_TOGGLE_ALWAYS)
    {
        return 1;
    }
    return (gkind == nGRRemixStage_DRAGONKING) || (gkind == nGRRemixStage_DRAGONKING_REMIX);
}

int ifRemixDragonKingHudRulesDamageY(int toggle, int gkind, int vanilla_y, int is_one_player_game)
{
    if (ifRemixDragonKingHudRulesIsOn(toggle, gkind) == 0)
    {
        return vanilla_y;
    }
    if ((is_one_player_game != 0) && (toggle != IFREMIX_DKHUD_TOGGLE_ALWAYS) && (gkind == nGRRemixStage_DRAGONKING_REMIX))
    {
        return vanilla_y;
    }
    return IFREMIX_DKHUD_DAMAGE_Y;
}

unsigned int ifRemixDragonKingHudRulesDigitOffset(int image_id)
{
    if ((image_id < 0) || (image_id >= IFREMIX_DKHUD_DIGIT_COUNT))
    {
        return 0;
    }
    return sIFRemixDragonKingHudDigitOffsets[image_id];
}

int ifRemixDragonKingHudRulesDigitWidth(int image_id)
{
    if ((image_id < 0) || (image_id >= IFREMIX_DKHUD_DIGIT_COUNT))
    {
        return 0;
    }
    return sIFRemixDragonKingHudDigitWidths[image_id];
}

void ifRemixDragonKingHudRulesColor(const IFRemixDragonKingHudColorInput *input, unsigned char *rgb)
{
    if ((input->is_vs_teams_battle != 0) && (input->team >= 0) && (input->team < IFREMIX_DKHUD_COLOR_COUNT))
    {
        ifRemixDragonKingHudRulesCopyRgb(sIFRemixDragonKingHudRgbTeam[input->team], rgb);
        return;
    }
    if ((input->is_cpu != 0) || (input->port < 0) || (input->port >= IFREMIX_DKHUD_COLOR_COUNT))
    {
        rgb[0] = 0xFF;
        rgb[1] = 0xFF;
        rgb[2] = 0xFF;
        return;
    }
    ifRemixDragonKingHudRulesCopyRgb(sIFRemixDragonKingHudRgbFfa[input->port], rgb);
}

float ifRemixDragonKingHudRulesTimerY(int is_dragon_king_hud_on, int input_display_state_port4)
{
    if (is_dragon_king_hud_on != 0)
    {
        return IFREMIX_DKHUD_TIMER_Y;
    }
    if (input_display_state_port4 == IFREMIX_DKHUD_INPUT_DISPLAY_TOP)
    {
        return IFREMIX_DKHUD_TIMER_Y_INPUT_DISPLAY_TOP;
    }
    return IFREMIX_DKHUD_TIMER_Y_NORMAL;
}

#endif
