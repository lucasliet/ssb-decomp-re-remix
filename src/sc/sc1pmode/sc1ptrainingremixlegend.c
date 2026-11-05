/**
 * sc1ptrainingremixlegend.c - D-pad legend of the Remix Training pause, see sc1ptrainingremixlegend.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixgroupdisplay.h"
#include "gm/gmremixtoggle.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_training.h"
#include "sc/sc1pmode/sc1ptrainingremixlegend.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"
#include "sc/sc1pmode/sc1ptrainingrules.h"

/* Training.asm: the Render groups of the legend are GObj links, drawn by the interface camera on the pause display link. */
#define SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES 0x19
#define SC1PTRAININGREMIXLEGEND_GROUP_RESET 0x1A
#define SC1PTRAININGREMIXLEGEND_DL_LINK 24
#define SC1PTRAININGREMIXLEGEND_TINT 0x848484FF
#define SC1PTRAININGREMIXLEGEND_PALETTE 0x303030FF
#define SC1PTRAININGREMIXLEGEND_SQUARE_COLOR 0xFFFF00FF
#define SC1PTRAININGREMIXLEGEND_TEXT_COLOR 0xFFFFFFFF
#define SC1PTRAININGREMIXLEGEND_TEXT_SLOT_FIRST 73
#define SC1PTRAININGREMIXLEGEND_RECT_SLOT_FIRST 24
#define SC1PTRAININGREMIXLEGEND_SQUARE_SIZE 2.0F

/* "Press (Z) for Custom Menu" (Training.asm setup_ 1583-1584): Z sprite at (117.5, 49), the text centred at (160, 50). */
#define SC1PTRAININGREMIXLEGEND_GROUP_PROMPT 0x1B
#define SC1PTRAININGREMIXLEGEND_PROMPT_SLOT 77
#define SC1PTRAININGREMIXLEGEND_PROMPT_X 160.0F
#define SC1PTRAININGREMIXLEGEND_PROMPT_Y 50.0F
#define SC1PTRAININGREMIXLEGEND_PROMPT_GLYPH_X 117.5F
#define SC1PTRAININGREMIXLEGEND_PROMPT_GLYPH_Y 49.0F

typedef struct SC1PTrainingRemixLegendLine
{
    s32 group;
    f32 glyph_x;
    f32 glyph_y;
    f32 square_x;
    f32 square_y;
    f32 text_x;
    f32 text_y;
    const char *text;

} SC1PTrainingRemixLegendLine;

static const SC1PTrainingRemixLegendLine dSC1PTrainingRemixLegendLines[] =
{
    { SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES, 45.25F, 200.0F, 52.0F, 203.0F, 63.25F, 202.0F, "Toggle Pause" },
    { SC1PTRAININGREMIXLEGEND_GROUP_RESET, 45.25F, 215.0F, 48.0F, 222.0F, 63.25F, 217.0F, "Quick Reset" },
    { SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES, 160.0F, 200.0F, 171.0F, 207.0F, 178.0F, 202.0F, "Frame Advance" },
    { SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES, 160.0F, 215.0F, 167.0F, 226.0F, 178.0F, 217.0F, "Model Display" }
};

#define SC1PTRAININGREMIXLEGEND_LINE_COUNT ((s32)(sizeof(dSC1PTrainingRemixLegendLines) / sizeof(dSC1PTrainingRemixLegendLines[0])))

void sc1PTrainingRemixLegendMake(void)
{
    s32 index;

    for (index = 0; index < SC1PTRAININGREMIXLEGEND_LINE_COUNT; index++)
    {
        const SC1PTrainingRemixLegendLine *line = &dSC1PTrainingRemixLegendLines[index];

        ifRemixSpriteMake(IFREMIX_SPRITE_DPAD, line->group, SC1PTRAININGREMIXLEGEND_DL_LINK, line->glyph_x, line->glyph_y,
                          SC1PTRAININGREMIXLEGEND_TINT, SC1PTRAININGREMIXLEGEND_PALETTE, 1.0F);
    }
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_Z, SC1PTRAININGREMIXLEGEND_GROUP_PROMPT, SC1PTRAININGREMIXLEGEND_DL_LINK,
                      SC1PTRAININGREMIXLEGEND_PROMPT_GLYPH_X, SC1PTRAININGREMIXLEGEND_PROMPT_GLYPH_Y, SC1PTRAININGREMIXLEGEND_TINT,
                      SC1PTRAININGREMIXLEGEND_PALETTE, 1.0F);
}

/* The prompt shows on the pause, with the HUD on, until the Z menu is opened (the normal menu group is hidden then). */
static sb32 sc1PTrainingRemixLegendIsPromptShown(void)
{
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusPause)
    {
        return FALSE;
    }
    if (gmRemixToggle(disable_hud) != 0)
    {
        return FALSE;
    }
    return (sc1PTrainingRemixMenuIsOpen() == FALSE) ? TRUE : FALSE;
}

static sb32 sc1PTrainingRemixLegendIsGroupShown(s32 group, s32 dpad_setting)
{
    if (gSCManagerBattleState->game_status != nSCBattleGameStatusPause)
    {
        return FALSE;
    }
    if (group == SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES)
    {
        return (sc1PTrainingRulesIsDpadToggleLegendShown(dpad_setting) != 0) ? TRUE : FALSE;
    }
    return (sc1PTrainingRulesIsDpadResetLegendShown(dpad_setting) != 0) ? TRUE : FALSE;
}

void sc1PTrainingRemixLegendUpdate(void)
{
    s32 dpad_setting = remix_training_get(REMIX_TRAINING_DPAD);
    s32 index;

    gmRemixGroupDisplaySet(SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES,
                           (sc1PTrainingRemixLegendIsGroupShown(SC1PTRAININGREMIXLEGEND_GROUP_TOGGLES, dpad_setting) != FALSE) ? 0 : 1);
    gmRemixGroupDisplaySet(SC1PTRAININGREMIXLEGEND_GROUP_RESET,
                           (sc1PTrainingRemixLegendIsGroupShown(SC1PTRAININGREMIXLEGEND_GROUP_RESET, dpad_setting) != FALSE) ? 0 : 1);

    gmRemixGroupDisplaySet(SC1PTRAININGREMIXLEGEND_GROUP_PROMPT, (sc1PTrainingRemixLegendIsPromptShown() != FALSE) ? 0 : 1);

    if (sc1PTrainingRemixLegendIsPromptShown() != FALSE)
    {
        remix_hud_text_put(SC1PTRAININGREMIXLEGEND_PROMPT_SLOT, "Press    for Custom Menu", SC1PTRAININGREMIXLEGEND_PROMPT_X,
                           SC1PTRAININGREMIXLEGEND_PROMPT_Y, SC1PTRAININGREMIXLEGEND_TEXT_COLOR, 1.0F, REMIX_HUD_ALIGN_CENTER);
    }
    for (index = 0; index < SC1PTRAININGREMIXLEGEND_LINE_COUNT; index++)
    {
        const SC1PTrainingRemixLegendLine *line = &dSC1PTrainingRemixLegendLines[index];

        if (sc1PTrainingRemixLegendIsGroupShown(line->group, dpad_setting) == FALSE)
        {
            continue;
        }
        remix_hud_rect_put(SC1PTRAININGREMIXLEGEND_RECT_SLOT_FIRST + index, line->square_x, line->square_y,
                           SC1PTRAININGREMIXLEGEND_SQUARE_SIZE, SC1PTRAININGREMIXLEGEND_SQUARE_SIZE,
                           SC1PTRAININGREMIXLEGEND_SQUARE_COLOR);
        remix_hud_text_put(SC1PTRAININGREMIXLEGEND_TEXT_SLOT_FIRST + index, line->text, line->text_x, line->text_y,
                           SC1PTRAININGREMIXLEGEND_TEXT_COLOR, 1.0F, REMIX_HUD_ALIGN_LEFT);
    }
}

#endif
