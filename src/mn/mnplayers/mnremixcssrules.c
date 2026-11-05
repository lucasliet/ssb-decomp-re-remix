/**
 * mnremixcssrules.c - pure rules of the Remix VS character select, see mnremixcssrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "mn/mnplayers/mnremixcssrules.h"
#include "remix/remix_roster_data.h"

#define MNREMIX_CSS_FLAG_SCALE 0.625F
#define MNREMIX_CSS_MASTER_HAND_SCALE 0.5625F

int mnRemixCssCostumeButtonsActive(int is_team_battle, int is_fighter_selected, int held_player)
{
    if (is_team_battle == 0)
    {
        return 1;
    }
    return ((is_fighter_selected != 0) && (held_player < 0)) ? 1 : 0;
}

int mnRemixCssCursorPressesImage(float cursor_x, float cursor_y, float image_x, float image_y, int image_width, int image_height)
{
    float press_x = cursor_x + MNREMIX_CSS_PRESS_OFFSET_X;
    float press_y = cursor_y + MNREMIX_CSS_PRESS_OFFSET_Y;

    if ((press_y < image_y) || (press_y > (image_y + (float)image_height)))
    {
        return 0;
    }
    if ((press_x < image_x) || (press_x > (image_x + (float)image_width)))
    {
        return 0;
    }
    return 1;
}

int mnRemixCssCpuCostumePanel(int own_panel, const int *is_cpu_panel, const int *is_cursor_over, int panel_count)
{
    int panel;

    for (panel = 0; panel < panel_count; panel++)
    {
        if ((panel != own_panel) && (is_cpu_panel[panel] != 0) && (is_cursor_over[panel] != 0))
        {
            return panel;
        }
    }
    return own_panel;
}

int mnRemixCssRandomBookendContains(int pos_x, int pos_y)
{
    if ((pos_x < MNREMIX_CSS_RANDOM_BOOKEND_X) || (pos_x >= (MNREMIX_CSS_RANDOM_BOOKEND_X + MNREMIX_CSS_BOOKEND_WIDTH)))
    {
        return 0;
    }
    return ((pos_y >= MNREMIX_CSS_BOOKEND_Y) && (pos_y < (MNREMIX_CSS_BOOKEND_Y + MNREMIX_CSS_BOOKEND_HEIGHT))) ? 1 : 0;
}

void mnRemixCssRecentRandomsReset(MNRemixCssRecentRandoms *recent)
{
    int slot;

    for (slot = 0; slot < MNREMIX_CSS_RECENT_RANDOMS_SLOTS; slot++)
    {
        recent->entries[slot] = -1;
    }
    recent->index = 0;
}

int mnRemixCssRecentRandomsStore(MNRemixCssRecentRandoms *recent, int fkind)
{
    int slot;
    int matches = 0;

    recent->entries[recent->index] = (signed char)fkind;

    for (slot = 0; slot < MNREMIX_CSS_RECENT_RANDOMS_SLOTS; slot++)
    {
        if (recent->entries[slot] == fkind)
        {
            matches++;
        }
    }
    return matches;
}

int mnRemixCssRecentRandomsDiceRange(int matches)
{
    return (matches > 1) ? matches : 0;
}

void mnRemixCssRecentRandomsCommit(MNRemixCssRecentRandoms *recent)
{
    recent->index = ((recent->index + 1) < MNREMIX_CSS_RECENT_RANDOMS_SLOTS) ? (recent->index + 1) : 0;
}

int mnRemixCssRandomTeamShade(int port, const int *fkinds, const int *teams)
{
    int earlier;
    int shade = 0;

    for (earlier = 0; earlier < port; earlier++)
    {
        if ((fkinds[earlier] == fkinds[port]) && (teams[earlier] == teams[port]))
        {
            shade++;
        }
    }
    return shade;
}

int mnRemixCssBonusBookendContains(int pos_x, int pos_y)
{
    if ((pos_x < MNREMIX_CSS_BOOKEND_X) || (pos_x >= (MNREMIX_CSS_BOOKEND_X + MNREMIX_CSS_BOOKEND_WIDTH)))
    {
        return 0;
    }
    return ((pos_y >= MNREMIX_CSS_BOOKEND_Y) && (pos_y < (MNREMIX_CSS_BOOKEND_Y + MNREMIX_CSS_BOOKEND_HEIGHT))) ? 1 : 0;
}

int mnRemixCssBonusStep(int index, int step)
{
    int next = index + step;

    if (next < 0)
    {
        return MNREMIX_CSS_BONUS_COUNT - 1;
    }
    if (next >= MNREMIX_CSS_BONUS_COUNT)
    {
        return 0;
    }
    return next;
}

int mnRemixCssBonusHeldStep(int is_z_held, int is_r_held)
{
    if (is_z_held != 0)
    {
        return -1;
    }
    return (is_r_held != 0) ? 1 : 0;
}

int mnRemixCssBonusRepeat(int *timer, int is_held)
{
    if (is_held == 0)
    {
        *timer = MNREMIX_CSS_BONUS_REPEAT_IDLE;
        return 0;
    }
    (*timer)++;

    if (*timer == 0)
    {
        return 1;
    }
    if (*timer < MNREMIX_CSS_BONUS_REPEAT_DELAY)
    {
        return 0;
    }
    *timer -= MNREMIX_CSS_BONUS_REPEAT_STEP;

    return 1;
}

int mnRemixCssRandomPoolSize(int random_select, int slot_count)
{
    return (random_select != 0) ? (slot_count + MNREMIX_CSS_BONUS_COUNT) : slot_count;
}

int mnRemixCssRandomPoolIsBonus(int index, int slot_count)
{
    return (index >= slot_count) ? 1 : 0;
}

int mnRemixCssDpadPanelX(int panel)
{
    return MNREMIX_CSS_DPAD_X + (MNREMIX_CSS_DPAD_PANEL_STRIDE * panel);
}

void mnRemixCssDpadIconOffset(int direction, int *offset_x, int *offset_y)
{
    *offset_x = 4;
    *offset_y = 4;

    switch (direction)
    {
    case MNREMIX_CSS_DPAD_UP:
        *offset_y = -9;
        break;

    case MNREMIX_CSS_DPAD_DOWN:
        *offset_y = 14;
        break;

    case MNREMIX_CSS_DPAD_LEFT:
        *offset_x = -10;
        break;

    case MNREMIX_CSS_DPAD_RIGHT:
        *offset_x = 16;
        break;
    }
}

void mnRemixCssDpadMarkOffset(int direction, int *offset_x, int *offset_y)
{
    *offset_x = 7;
    *offset_y = 7;

    switch (direction)
    {
    case MNREMIX_CSS_DPAD_UP:
        *offset_y = 3;
        break;

    case MNREMIX_CSS_DPAD_DOWN:
        *offset_y = 11;
        break;

    case MNREMIX_CSS_DPAD_LEFT:
        *offset_x = 3;
        break;

    case MNREMIX_CSS_DPAD_RIGHT:
        *offset_x = 11;
        break;
    }
}

MNRemixCssVariantIcon mnRemixCssVariantIconKind(int variant_type, int variant_id)
{
    if (variant_type == REMIX_VARIANT_J)
    {
        return MNREMIX_CSS_ICON_FLAG_J;
    }
    if (variant_type == REMIX_VARIANT_E)
    {
        return MNREMIX_CSS_ICON_FLAG_E;
    }
    if (variant_type == REMIX_VARIANT_POLYGON)
    {
        return MNREMIX_CSS_ICON_POLYGON;
    }
    switch (variant_id)
    {
    case REMIX_ID_PEPPY:
        return MNREMIX_CSS_ICON_PEPPY;

    case REMIX_ID_SLIPPY:
        return MNREMIX_CSS_ICON_SLIPPY;

    case REMIX_ID_METAL:
        return MNREMIX_CSS_ICON_METAL;

    case REMIX_ID_MLUIGI:
        return MNREMIX_CSS_ICON_MLUIGI;

    case REMIX_ID_EBI:
        return MNREMIX_CSS_ICON_EBI;

    case REMIX_ID_GBOWSER:
        return MNREMIX_CSS_ICON_GBOWSER;

    case REMIX_ID_PIANO:
        return MNREMIX_CSS_ICON_PIANO;

    case REMIX_ID_SSONIC:
        return MNREMIX_CSS_ICON_SSONIC;

    case REMIX_ID_DRAGONKING:
        return MNREMIX_CSS_ICON_DRAGONKING;

    case REMIX_ID_DSAMUS:
        return MNREMIX_CSS_ICON_DSAMUS;

    case REMIX_ID_LUCAS:
        return MNREMIX_CSS_ICON_LUCAS;

    case REMIX_ID_ROY:
        return MNREMIX_CSS_ICON_ROY;

    case REMIX_ID_BOSS:
        return MNREMIX_CSS_ICON_MASTER_HAND;

    default:
        return MNREMIX_CSS_ICON_DONKEY_STOCK;
    }
}

unsigned int mnRemixCssVariantIconOffset(MNRemixCssVariantIcon icon)
{
    switch (icon)
    {
    case MNREMIX_CSS_ICON_FLAG_J:
        return 0x0558;

    case MNREMIX_CSS_ICON_FLAG_E:
        return 0x03B8;

    case MNREMIX_CSS_ICON_POLYGON:
        return 0x13A8;

    case MNREMIX_CSS_ICON_METAL:
        return 0x2448 + 0x10;

    case MNREMIX_CSS_ICON_PIANO:
        return 0x2528 + 0x10;

    case MNREMIX_CSS_ICON_GBOWSER:
        return 0x2608 + 0x10;

    case MNREMIX_CSS_ICON_SSONIC:
        return 0x26E8 + 0x10;

    case MNREMIX_CSS_ICON_MASTER_HAND:
        return 0x27F0 + 0x10;

    case MNREMIX_CSS_ICON_PEPPY:
        return 0x28D8 + 0x10;

    case MNREMIX_CSS_ICON_SLIPPY:
        return 0x29B0 + 0x10;

    case MNREMIX_CSS_ICON_MLUIGI:
        return 0x2A88 + 0x10;

    case MNREMIX_CSS_ICON_EBI:
        return 0x2B60 + 0x10;

    case MNREMIX_CSS_ICON_DRAGONKING:
        return 0x2C38 + 0x10;

    case MNREMIX_CSS_ICON_DSAMUS:
        return 0x2D10 + 0x10;

    case MNREMIX_CSS_ICON_LUCAS:
        return 0x2DE8 + 0x10;

    case MNREMIX_CSS_ICON_ROY:
        return 0x2EC0 + 0x10;

    default:
        return 0;
    }
}

float mnRemixCssVariantIconScale(MNRemixCssVariantIcon icon)
{
    switch (icon)
    {
    case MNREMIX_CSS_ICON_FLAG_J:
    case MNREMIX_CSS_ICON_FLAG_E:
        return MNREMIX_CSS_FLAG_SCALE;

    case MNREMIX_CSS_ICON_MASTER_HAND:
        return MNREMIX_CSS_MASTER_HAND_SCALE;

    default:
        return 1.0F;
    }
}

#define MNREMIX_CSS_HANDICAP_LEVELS 5

int mnRemixCssHandicapIconSource(int index)
{
    static const int sSources[MNREMIX_CSS_HANDICAP_ICON_COUNT / MNREMIX_CSS_HANDICAP_LEVELS] =
    {
        REMIX_ID_YOSHI, REMIX_ID_KIRBY, MNREMIX_CSS_HANDICAP_SOURCE_POLYGON, REMIX_ID_DONKEY, MNREMIX_CSS_HANDICAP_SOURCE_METAL,
        MNREMIX_CSS_HANDICAP_SOURCE_MASTER_HAND
    };

    if ((index < 0) || (index >= MNREMIX_CSS_HANDICAP_ICON_COUNT))
    {
        return MNREMIX_CSS_HANDICAP_SOURCE_NONE;
    }
    if (index == (MNREMIX_CSS_HANDICAP_ICON_COUNT - 1))
    {
        return REMIX_ID_SAMUS;
    }
    return sSources[index / MNREMIX_CSS_HANDICAP_LEVELS];
}

int mnRemixCssHandicapIconLevel(int index)
{
    if ((index < 0) || (index >= (MNREMIX_CSS_HANDICAP_ICON_COUNT - 1)))
    {
        return 0;
    }
    return (index % MNREMIX_CSS_HANDICAP_LEVELS) + 1;
}

unsigned int mnRemixCssHandicapLevelColor(int level)
{
    static const unsigned int sColors[MNREMIX_CSS_HANDICAP_LEVELS] =
    {
        0x416FE4FF, 0x8DBB5AFF, 0xE4BE41FF, 0xE47841FF, 0xE44141FF
    };

    if ((level < 1) || (level > MNREMIX_CSS_HANDICAP_LEVELS))
    {
        return 0;
    }
    return sColors[level - 1];
}

int mnRemixCssHasVariants(const int *variant_ids, int none_id)
{
    int direction;

    for (direction = 0; direction < MNREMIX_CSS_DPAD_DIRECTION_COUNT; direction++)
    {
        if (variant_ids[direction] != none_id)
        {
            return 1;
        }
    }
    return 0;
}

#endif
