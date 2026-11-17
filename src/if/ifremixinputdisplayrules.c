/**
 * ifremixinputdisplayrules.c - pure Input Display rules, see ifremixinputdisplayrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixinputdisplayrules.h"

#define IFREMIX_INPUTDISPLAY_STICK_ALPHA_CENTERED 0x8F
#define IFREMIX_INPUTDISPLAY_STICK_ALPHA_MOVED 0xFF
#define IFREMIX_INPUTDISPLAY_GATE_ALPHA_CENTERED 0x4F
#define IFREMIX_INPUTDISPLAY_GATE_ALPHA_MOVED 0xAF
#define IFREMIX_INPUTDISPLAY_OFFSET_BONUS_TOP 20.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_MULTIMAN_TOP 14.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_ALLSTAR_YOSHI_TOP 30.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_SINGLE_YOSHI_TOP 20.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_SINGLE_KIRBY_TOP 14.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_SINGLE_POLYGON_TOP 30.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_DRAGON_KING_TOP 30.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_DRAGON_KING_BOTTOM 14.0F
#define IFREMIX_INPUTDISPLAY_OFFSET_DRAGON_KING_REST_AREA (-3.0F)

/** Offset and edge of the mode branches of create_input_display_objects_ (everything before _begin). */
static void ifRemixInputDisplayRulesApplyMode(const IFRemixInputDisplayLayoutInput *input, IFRemixInputDisplayLayout *layout)
{
    int is_top = (layout->is_bottom == 0) ? 1 : 0;

    switch (input->mode)
    {
    case IFREMIX_INPUTDISPLAY_MODE_MULTIMAN:
        if (is_top != 0)
        {
            layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_MULTIMAN_TOP;
        }
        return;

    case IFREMIX_INPUTDISPLAY_MODE_ALLSTAR:
        if (input->stage == IFREMIX_INPUTDISPLAY_STAGE_REST_AREA)
        {
            layout->is_bottom = 1;
        }
        else if ((is_top != 0) && (input->stage == IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM))
        {
            layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_ALLSTAR_YOSHI_TOP;
        }
        return;

    case IFREMIX_INPUTDISPLAY_MODE_SINGLE:
        if (is_top == 0)
        {
            return;
        }
        if (input->stage == IFREMIX_INPUTDISPLAY_STAGE_YOSHI_TEAM)
        {
            layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_SINGLE_YOSHI_TOP;
        }
        else if (input->stage == IFREMIX_INPUTDISPLAY_STAGE_KIRBY_TEAM)
        {
            layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_SINGLE_KIRBY_TOP;
        }
        else if (input->stage == IFREMIX_INPUTDISPLAY_STAGE_POLYGON_TEAM)
        {
            layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_SINGLE_POLYGON_TOP;
        }
        return;

    default:
        return;
    }
}

/** Dragon King HUD branch: the requested edge is back (Training no longer forces the bottom) and the offsets are fixed. */
static void ifRemixInputDisplayRulesApplyDragonKing(const IFRemixInputDisplayLayoutInput *input, IFRemixInputDisplayLayout *layout)
{
    layout->is_bottom = input->is_bottom_requested;

    if (layout->is_bottom == 0)
    {
        layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_DRAGON_KING_TOP;
        return;
    }
    layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_DRAGON_KING_BOTTOM;

    if ((input->mode == IFREMIX_INPUTDISPLAY_MODE_ALLSTAR) && (input->stage == IFREMIX_INPUTDISPLAY_STAGE_REST_AREA))
    {
        layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_DRAGON_KING_REST_AREA;
    }
}

void ifRemixInputDisplayRulesResolveLayout(const IFRemixInputDisplayLayoutInput *input, IFRemixInputDisplayLayout *layout)
{
    layout->is_bottom = (input->is_bottom_requested != 0) ? 1 : 0;
    layout->offset_y = 0.0F;

    if ((input->screen == IFREMIX_INPUTDISPLAY_SCREEN_BONUS) && (layout->is_bottom == 0))
    {
        layout->offset_y = IFREMIX_INPUTDISPLAY_OFFSET_BONUS_TOP;
    }
    if (input->screen == IFREMIX_INPUTDISPLAY_SCREEN_TRAINING)
    {
        layout->is_bottom = 1;
    }
    else if (input->screen != IFREMIX_INPUTDISPLAY_SCREEN_VS)
    {
        ifRemixInputDisplayRulesApplyMode(input, layout);
    }
    if (input->is_dragon_king_hud != 0)
    {
        ifRemixInputDisplayRulesApplyDragonKing(input, layout);
    }
}

/** Clamps a stick axis to the range the display draws. */
static int ifRemixInputDisplayRulesClampAxis(int value)
{
    if (value > IFREMIX_INPUTDISPLAY_STICK_RANGE)
    {
        return IFREMIX_INPUTDISPLAY_STICK_RANGE;
    }
    if (value < -IFREMIX_INPUTDISPLAY_STICK_RANGE)
    {
        return -IFREMIX_INPUTDISPLAY_STICK_RANGE;
    }
    return value;
}

void ifRemixInputDisplayRulesUpdateStick(int stick_x, int stick_y, int cooldown, IFRemixInputDisplayStick *stick)
{
    int x = ifRemixInputDisplayRulesClampAxis(stick_x);
    int y = ifRemixInputDisplayRulesClampAxis(stick_y);
    int is_moved = ((x != 0) || (y != 0)) ? 1 : 0;

    stick->offset_x = x * IFREMIX_INPUTDISPLAY_STICK_PIXELS_PER_UNIT;
    stick->offset_y = -y * IFREMIX_INPUTDISPLAY_STICK_PIXELS_PER_UNIT;
    stick->stick_alpha = ((is_moved != 0) || (cooldown < 0)) ? IFREMIX_INPUTDISPLAY_STICK_ALPHA_MOVED : IFREMIX_INPUTDISPLAY_STICK_ALPHA_CENTERED;
    stick->gate_alpha = IFREMIX_INPUTDISPLAY_GATE_ALPHA_MOVED;
    stick->cooldown = cooldown;

    if (is_moved != 0)
    {
        stick->cooldown = IFREMIX_INPUTDISPLAY_GATE_COOLDOWN;
    }
    else if (cooldown < 0)
    {
        stick->cooldown = cooldown + 1;
    }
    else
    {
        stick->gate_alpha = IFREMIX_INPUTDISPLAY_GATE_ALPHA_CENTERED;
    }
}

#endif
