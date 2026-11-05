/**
 * mnremixstagepanel.c - stage name, Hazards, Movement, Layout and Roulette lines of the stage select (src/Stages.asm setup_, update_text_,
 * update_cursor_color_, right_check_ _hazard_toggle).
 *
 * Remix draws four short lines under the stage preview, each with a button image: "Hazards (L):" and "Movement (L):" with their
 * On or Off value, "Layout (C-down):" with the variant type, and "Roulette (C-down)" over the RANDOM square. The tournament
 * layout drops the button images and the Layout line and shows "Hazards:" and "Movement:". The text goes through the HUD
 * overlay (remix_hud_text) and the buttons are native sprites.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sys/obj.h>
#include <sys/objman.h>
#include "gm/gmremixtoggle.h"
#include "gr/grremix/grremixids.h"
#include "gr/grremix/grremixstage.h"
#include "if/ifremixsprite.h"
#include "mn/mnmaps/mnremixstagepanel.h"
#include "mn/mnmaps/mnremixstagepanelrules.h"
#include "mn/mnmaps/mnremixstagetournament.h"
#include "mn/mnmaps/mnremixstagevariant.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

/* Text slots 118..125 of remix_hud_text (stage select scene). */
#define PANEL_TEXT_SLOT_HAZARDS_LABEL 118
#define PANEL_TEXT_SLOT_HAZARDS_VALUE 119
#define PANEL_TEXT_SLOT_MOVEMENT_LABEL 120
#define PANEL_TEXT_SLOT_MOVEMENT_VALUE 121
#define PANEL_TEXT_SLOT_LAYOUT_LABEL 122
#define PANEL_TEXT_SLOT_LAYOUT_VALUE 123
#define PANEL_TEXT_SLOT_ROULETTE_LABEL 124
#define PANEL_TEXT_SLOT_STAGE_NAME 125

/* Stages.asm setup_ draw_string_ arguments: right aligned labels, left aligned values, scale 0.75, white. */
#define PANEL_LABEL_X 248.0F
#define PANEL_VALUE_X 252.0F
#define PANEL_ROULETTE_LABEL_X 259.0F
#define PANEL_HAZARDS_Y 181.0F
#define PANEL_MOVEMENT_Y 189.0F
#define PANEL_LAYOUT_Y 199.0F
#define PANEL_TEXT_SCALE 0.75F
#define PANEL_TEXT_COLOR 0xFFFFFFFF

/* Stages.asm setup_ draws the stage name centered at (230, 209) with Render.FONTSIZE_DEFAULT, in place of the name plate. */
#define PANEL_STAGE_NAME_X 230.0F
#define PANEL_STAGE_NAME_Y 209.0F
#define PANEL_STAGE_NAME_SCALE 1.0F

/* Stages.asm setup_ draw_texture_at_offset_ arguments: the button sits in the "(  )" gap of its label. */
#define PANEL_LEFT_ICON_X 234.0F
#define PANEL_ROULETTE_ICON_X 248.5F
#define PANEL_HAZARDS_ICON_Y 180.25F
#define PANEL_MOVEMENT_ICON_Y 188.5F
#define PANEL_ARROW_ICON_Y 198.5F
#define PANEL_ICON_SCALE 0.75F
#define PANEL_ARROW_SCALE_X 0.71875F
#define PANEL_L_TINT 0x848484FF
#define PANEL_L_PALETTE 0x303030FF
#define PANEL_ARROW_TINT 0xC0CC00FF
#define PANEL_ARROW_PALETTE 0x000000FF

/* The sprites share the GObj link 4 and the name and emblem camera (dl link 2) with the stage name plate. */
#define PANEL_GOBJ_LINK 4
#define PANEL_DL_LINK 2

#define PANEL_CURSOR_COLOR_UNSET (-1)
#define PANEL_CURSOR_COLOR_COUNT 4

/* Stages.colors: Color.high.RED, light blue, lighter blue and Color.high.BLUE. */
static const u32 sMNRemixPanelCursorColors[PANEL_CURSOR_COLOR_COUNT] = { 0xFF0000FF, 0x0088FFFF, 0x39E5BAFF, 0x0000FFFF };

static GObj *sMNRemixPanelHazardsButton;
static GObj *sMNRemixPanelMovementButton;
static GObj *sMNRemixPanelLayoutButton;
static GObj *sMNRemixPanelRouletteButton;
static s32 sMNRemixPanelCursorColor = PANEL_CURSOR_COLOR_UNSET;

/** Makes one button sprite, hidden until the first update decides whether its line shows. */
static GObj *mnRemixStagePanelMakeButton(s32 sprite_id, f32 x, f32 y, u32 tint, u32 palette)
{
    GObj *gobj = ifRemixSpriteMake(sprite_id, PANEL_GOBJ_LINK, PANEL_DL_LINK, x, y, tint, palette, PANEL_ICON_SCALE);

    if (gobj != NULL)
    {
        gobj->flags = GOBJ_FLAG_HIDDEN;
    }
    return gobj;
}

/** Makes a C-down arrow button; Stages.asm squeezes it to 0.71875 on X after TEXTURE_INIT_. */
static GObj *mnRemixStagePanelMakeArrow(f32 x)
{
    GObj *gobj = mnRemixStagePanelMakeButton(IFREMIX_SPRITE_C_ARROW_B, x, PANEL_ARROW_ICON_Y, PANEL_ARROW_TINT, PANEL_ARROW_PALETTE);

    if (gobj != NULL)
    {
        SObjGetStruct(gobj)->sprite.scalex = PANEL_ARROW_SCALE_X;
    }
    return gobj;
}

void mnRemixStagePanelMake(void)
{
    sMNRemixPanelCursorColor = PANEL_CURSOR_COLOR_UNSET;
    sMNRemixPanelHazardsButton = NULL;
    sMNRemixPanelMovementButton = NULL;

    if (mnRemixTournamentIsActive() == FALSE)
    {
        sMNRemixPanelHazardsButton = mnRemixStagePanelMakeButton(IFREMIX_SPRITE_BUTTON_L_THIN, PANEL_LEFT_ICON_X, PANEL_HAZARDS_ICON_Y, PANEL_L_TINT, PANEL_L_PALETTE);
        sMNRemixPanelMovementButton = mnRemixStagePanelMakeButton(IFREMIX_SPRITE_BUTTON_L_THIN, PANEL_LEFT_ICON_X, PANEL_MOVEMENT_ICON_Y, PANEL_L_TINT, PANEL_L_PALETTE);
    }
    sMNRemixPanelLayoutButton = mnRemixStagePanelMakeArrow(PANEL_LEFT_ICON_X);
    sMNRemixPanelRouletteButton = mnRemixStagePanelMakeArrow(PANEL_ROULETTE_ICON_X);
}

static void mnRemixStagePanelSetVisible(GObj *gobj, sb32 is_visible)
{
    if (gobj != NULL)
    {
        gobj->flags = (is_visible != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
    }
}

/** Stages.stage_hazard_table entry; the RANDOM square and ids past the table have nothing to switch off. */
static s32 mnRemixStagePanelReadHazardType(s32 gkind)
{
    if ((gkind < 0) || (gkind > GRREMIX_KIND_MAX))
    {
        return MNREMIX_HAZARD_TYPE_NONE;
    }
    return (s32)remix_ram_image_read_byte(REMIX_SYM_Stages__stage_hazard_table + gkind);
}

/** "Hazards (  ):" and "Movement (  ):" lose the parentheses on the tournament layout (setup_ _set_strings). */
static const char *mnRemixStagePanelLabel(const char *normal_label, const char *tournament_label)
{
    return (mnRemixTournamentIsActive() != FALSE) ? tournament_label : normal_label;
}

static void mnRemixStagePanelPutLine(s32 label_slot, const char *label, const char *value, f32 y)
{
    remix_hud_text_put(label_slot, label, PANEL_LABEL_X, y, PANEL_TEXT_COLOR, PANEL_TEXT_SCALE, REMIX_HUD_ALIGN_RIGHT);
    remix_hud_text_put(label_slot + 1, value, PANEL_VALUE_X, y, PANEL_TEXT_COLOR, PANEL_TEXT_SCALE, REMIX_HUD_ALIGN_LEFT);
}

static void mnRemixStagePanelDrawHazardLines(const MNRemixStagePanelView *view)
{
    if (view->show_hazards != 0)
    {
        mnRemixStagePanelPutLine(PANEL_TEXT_SLOT_HAZARDS_LABEL, mnRemixStagePanelLabel("Hazards (  ):", "Hazards:"), view->hazards_value, PANEL_HAZARDS_Y);
    }
    if (view->show_movement != 0)
    {
        mnRemixStagePanelPutLine(PANEL_TEXT_SLOT_MOVEMENT_LABEL, mnRemixStagePanelLabel("Movement (  ):", "Movement:"), view->movement_value, PANEL_MOVEMENT_Y);
    }
    mnRemixStagePanelSetVisible(sMNRemixPanelHazardsButton, view->show_hazards);
    mnRemixStagePanelSetVisible(sMNRemixPanelMovementButton, view->show_movement);
}

/** The Layout line over a stage with variants, the Roulette line over RANDOM (update_text_ _check_layout_table). */
static void mnRemixStagePanelDrawLayoutAndRoulette(s32 base_gkind, sb32 is_random)
{
    sb32 show_layout = mnRemixStagePanelRulesShowLayout(is_random, mnRemixMapsHasVariants(base_gkind), mnRemixTournamentIsActive());

    if (show_layout != FALSE)
    {
        mnRemixStagePanelPutLine(PANEL_TEXT_SLOT_LAYOUT_LABEL, "Layout (  ):", mnRemixStagePanelRulesLayoutLabel(mnRemixMapsGetShownVariantType(base_gkind)), PANEL_LAYOUT_Y);
    }
    if (is_random != FALSE)
    {
        remix_hud_text_put(PANEL_TEXT_SLOT_ROULETTE_LABEL, "Roulette (  )", PANEL_ROULETTE_LABEL_X, PANEL_LAYOUT_Y, PANEL_TEXT_COLOR, PANEL_TEXT_SCALE, REMIX_HUD_ALIGN_RIGHT);
    }
    mnRemixStagePanelSetVisible(sMNRemixPanelLayoutButton, show_layout);
    mnRemixStagePanelSetVisible(sMNRemixPanelRouletteButton, is_random);
}

/** The stage name under the preview; blank over RANDOM (update_text_ _stage_name). */
static void mnRemixStagePanelDrawStageName(s32 shown_gkind, sb32 is_random)
{
    char title[GRREMIX_NAME_MAX];

    if (is_random != FALSE)
    {
        return;
    }
    grRemixGetStageTitle(shown_gkind, title);
    remix_hud_text_put(PANEL_TEXT_SLOT_STAGE_NAME, title, PANEL_STAGE_NAME_X, PANEL_STAGE_NAME_Y, PANEL_TEXT_COLOR, PANEL_STAGE_NAME_SCALE,
                       REMIX_HUD_ALIGN_CENTER);
}

/** Paints the cursor with the colour of the hazard mode, once per change (update_cursor_color_). */
static void mnRemixStagePanelPaintCursor(GObj *cursor_gobj, s32 color_index)
{
    u32 color;
    SObj *sobj;

    if ((cursor_gobj == NULL) || (color_index == sMNRemixPanelCursorColor) || (color_index < 0) || (color_index >= PANEL_CURSOR_COLOR_COUNT))
    {
        return;
    }
    color = sMNRemixPanelCursorColors[color_index];
    sobj = SObjGetStruct(cursor_gobj);
    sobj->sprite.red = (u8)(color >> 24);
    sobj->sprite.green = (u8)(color >> 16);
    sobj->sprite.blue = (u8)(color >> 8);
    sMNRemixPanelCursorColor = color_index;
}

void mnRemixStagePanelUpdate(GObj *cursor_gobj, s32 base_gkind, s32 shown_gkind, s32 source_page)
{
    MNRemixStagePanelInput input;
    MNRemixStagePanelView view;
    sb32 is_random = (base_gkind == nGRRemixStage_RANDOM) ? TRUE : FALSE;

    if ((mnRemixTournamentIsActive() != FALSE) && (is_random == FALSE))
    {
        mnRemixTournamentApplyHazardMode(shown_gkind, source_page);
    }
    input.stage_id = shown_gkind;
    input.is_random = is_random;
    input.hazard_type = (is_random != FALSE) ? MNREMIX_HAZARD_TYPE_NONE : mnRemixStagePanelReadHazardType(shown_gkind);
    input.hazard_mode = gmRemixToggle(hazard_mode);
    input.whispy_mode = gmRemixToggle(whispy_mode);
    input.saffron_poke_rate = gmRemixToggle(saffron_poke_rate);
    input.is_tournament = mnRemixTournamentIsActive();
    mnRemixStagePanelRulesCompute(&input, &view);
    mnRemixStagePanelDrawHazardLines(&view);
    mnRemixStagePanelDrawLayoutAndRoulette(base_gkind, is_random);
    mnRemixStagePanelDrawStageName(shown_gkind, is_random);
    mnRemixStagePanelPaintCursor(cursor_gobj, view.cursor_color_index);
}

sb32 mnRemixStagePanelToggleHazards(s32 base_gkind, s32 shown_gkind)
{
    s32 next_mode;
    sb32 is_random = (base_gkind == nGRRemixStage_RANDOM) ? TRUE : FALSE;
    s32 hazard_type = (is_random != FALSE) ? MNREMIX_HAZARD_TYPE_NONE : mnRemixStagePanelReadHazardType(shown_gkind);

    if (mnRemixTournamentIsActive() != FALSE)
    {
        return FALSE;
    }
    if (mnRemixStagePanelRulesNextHazardMode(gmRemixToggle(hazard_mode), hazard_type, is_random, &next_mode) == 0)
    {
        return FALSE;
    }
    remix_toggle_set(REMIX_SYM_Toggles__entry_hazard_mode, next_mode);

    return TRUE;
}

void mnRemixStagePanelInvalidateCursorColor(void)
{
    sMNRemixPanelCursorColor = PANEL_CURSOR_COLOR_UNSET;
}

#endif
