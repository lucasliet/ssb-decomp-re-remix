/**
 * mnremixstagepanelrules.c - pure rules of the hazard, movement and layout lines of the stage select, see mnremixstagepanelrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixids.h"
#include "mn/mnmaps/mnremixstagepanelrules.h"

#define MNREMIX_VARIANT_TYPE_DEFAULT 0
#define MNREMIX_VARIANT_TYPE_DL 1
#define MNREMIX_VARIANT_TYPE_OMEGA 2

/* Char 0x7E+1 of Stages.layout_O: the Omega glyph of the Remix font. */
#define MNREMIX_LAYOUT_OMEGA "\x7F"

static int mnRemixStagePanelIsWhispyStage(int stage_id)
{
    return (stage_id == nGRRemixStage_DREAM_LAND) || (stage_id == nGRRemixStage_WINTER_DL) ||
           (stage_id == nGRRemixStage_DREAM_LAND_O) || (stage_id == nGRRemixStage_DREAM_LAND_SR);
}

static const char *mnRemixStagePanelWhispyText(int whispy_mode)
{
    switch (whispy_mode)
    {
    case MNREMIX_WHISPY_MODE_JAPANESE:
        return "On (J)";

    case MNREMIX_WHISPY_MODE_SUPER:
        return "On (S)";

    case MNREMIX_WHISPY_MODE_HYPER:
        return "On (H)";
    }
    return "On";
}

static const char *mnRemixStagePanelSaffronText(int saffron_poke_rate)
{
    switch (saffron_poke_rate)
    {
    case MNREMIX_SAFFRON_RATE_SUPER:
        return "On (S)";

    case MNREMIX_SAFFRON_RATE_HYPER:
        return "On (H)";

    case MNREMIX_SAFFRON_RATE_QUICK_ATTACK:
        return "On (Q)";
    }
    return "On";
}

/** The "On" text of the Hazards line: the Whispy Mode mark on the Dream Land stages, else the Saffron rate mark on Saffron City. */
static const char *mnRemixStagePanelHazardsOnText(const MNRemixStagePanelInput *input)
{
    if ((input->whispy_mode != 0) && (mnRemixStagePanelIsWhispyStage(input->stage_id) != 0))
    {
        return mnRemixStagePanelWhispyText(input->whispy_mode);
    }
    if ((input->saffron_poke_rate != 0) && (input->stage_id == nGRRemixStage_SAFFRON_CITY))
    {
        return mnRemixStagePanelSaffronText(input->saffron_poke_rate);
    }
    return "On";
}

/** update_cursor_color_ index: 0 red, 1 light blue, 2 lighter blue, 3 blue. */
static int mnRemixStagePanelCursorColorIndex(const MNRemixStagePanelInput *input)
{
    if ((input->is_tournament != 0) && (input->is_random != 0))
    {
        return 0;
    }
    if ((input->hazard_type == MNREMIX_HAZARD_TYPE_BOTH) || (input->is_random != 0))
    {
        return input->hazard_mode;
    }
    if ((input->hazard_type == MNREMIX_HAZARD_TYPE_NONE) || ((input->hazard_type & input->hazard_mode) == 0))
    {
        return 0;
    }
    return (input->hazard_type == MNREMIX_HAZARD_TYPE_HAZARDS) ? 1 : 2;
}

void mnRemixStagePanelRulesCompute(const MNRemixStagePanelInput *input, MNRemixStagePanelView *view)
{
    int shown_mode = ((input->is_tournament != 0) && (input->is_random != 0)) ? MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON : input->hazard_mode;

    view->show_hazards = (((input->hazard_type & MNREMIX_HAZARD_TYPE_HAZARDS) != 0) || (input->is_random != 0)) ? 1 : 0;
    view->show_movement = (((input->hazard_type & MNREMIX_HAZARD_TYPE_MOVEMENT) != 0) || (input->is_random != 0)) ? 1 : 0;
    view->hazards_value = ((shown_mode & 1) != 0) ? "Off" : mnRemixStagePanelHazardsOnText(input);
    view->movement_value = ((shown_mode >> 1) != 0) ? "Off" : "On";
    view->cursor_color_index = mnRemixStagePanelCursorColorIndex(input);
}

int mnRemixStagePanelRulesNextHazardMode(int mode, int hazard_type, int is_random, int *next_mode)
{
    if (is_random != 0)
    {
        *next_mode = (mode + 1) & 3;
        return 1;
    }
    switch (hazard_type)
    {
    case MNREMIX_HAZARD_TYPE_NONE:
        return 0;

    case MNREMIX_HAZARD_TYPE_HAZARDS:
        *next_mode = mode ^ 1;
        return 1;

    case MNREMIX_HAZARD_TYPE_MOVEMENT:
        *next_mode = mode ^ 2;
        return 1;
    }
    *next_mode = (mode + 1) & 3;
    return 1;
}

int mnRemixStagePanelRulesShowLayout(int is_random, int has_variants, int is_tournament)
{
    return ((is_random == 0) && (has_variants != 0) && (is_tournament == 0)) ? 1 : 0;
}

const char *mnRemixStagePanelRulesLayoutLabel(int variant_type)
{
    switch (variant_type)
    {
    case MNREMIX_VARIANT_TYPE_DEFAULT:
        return "Def.";

    case MNREMIX_VARIANT_TYPE_DL:
        return "DL";

    case MNREMIX_VARIANT_TYPE_OMEGA:
        return MNREMIX_LAYOUT_OMEGA;
    }
    return "Remix";
}

#endif
