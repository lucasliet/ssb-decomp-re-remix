/**
 * sc1ptrainingremixmenudraw.c - drawing of the Training Z menu, see sc1ptrainingremixmenudraw.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/gbi.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <sys/taskman.h>
#include <string.h>
#include "gm/gmremixgroupdisplay.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"
#include "sc/sc1pmode/sc1ptrainingmenuentries.h"
#include "sc/sc1pmode/sc1ptrainingmenurules.h"
#include "sc/sc1pmode/sc1ptrainingmode.h"
#include "sc/sc1pmode/sc1ptrainingremixmenu.h"
#include "sc/sc1pmode/sc1ptrainingremixmenudraw.h"

/* Training.asm: the custom menu group (Render group 0x16) is the GObj link of the same number. */
#define SC1PTRAININGREMIXMENUDRAW_GROUP 0x16
#define SC1PTRAININGREMIXMENUDRAW_BACKGROUND_DL_LINK 22
#define SC1PTRAININGREMIXMENUDRAW_SPRITE_DL_LINK 24

/* Menu.info(head, 68, 50, 0x17, 0x16, 23, RED, WHITE, WHITE, 0x3F6C0000, 0xE, 12, ...). */
#define SC1PTRAININGREMIXMENUDRAW_MENU_X 68
#define SC1PTRAININGREMIXMENUDRAW_MENU_Y 50
#define SC1PTRAININGREMIXMENUDRAW_WIDTH_CHARS 23
#define SC1PTRAININGREMIXMENUDRAW_SCALE 0.921875F
#define SC1PTRAININGREMIXMENUDRAW_LABEL_X 76.0F
#define SC1PTRAININGREMIXMENUDRAW_VALUE_X 252.0F
#define SC1PTRAININGREMIXMENUDRAW_LABEL_MAX_WIDTH 150.0F
#define SC1PTRAININGREMIXMENUDRAW_VALUE_GAP 10.0F
#define SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR 0xFFFFFFFF

/* Menu.draw_ cursor square (menu x + 2, 4 x 4) and underline (width * 8 - 2, 1 pixel); update_cursor_ puts them 5 and 11 below the row. */
#define SC1PTRAININGREMIXMENUDRAW_CURSOR_X 70.0F
#define SC1PTRAININGREMIXMENUDRAW_CURSOR_SIZE 4.0F
#define SC1PTRAININGREMIXMENUDRAW_CURSOR_OFFSET_Y 5.0F
#define SC1PTRAININGREMIXMENUDRAW_UNDERLINE_OFFSET_Y 11.0F
#define SC1PTRAININGREMIXMENUDRAW_UNDERLINE_WIDTH 182.0F
#define SC1PTRAININGREMIXMENUDRAW_CURSOR_COLOR 0xFF0000FF

/* setup_ 1639-1644: pagination legend, R glyph and "/" on the first line, Z glyph, then "Next/" and "Prev" below. */
#define SC1PTRAININGREMIXMENUDRAW_PAGE_SCALE 0.875F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_GLYPH_TINT 0x848484FF
#define SC1PTRAININGREMIXMENUDRAW_PAGE_GLYPH_PALETTE 0x303030FF
#define SC1PTRAININGREMIXMENUDRAW_PAGE_R_X 272.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_R_Y 16.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_Z_X 294.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_Z_Y 15.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_SLASH_X 288.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_SLASH_Y 16.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_NEXT_X 272.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_NEXT_Y 27.25F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_X 272.0F
#define SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_Y 38.5F

/* Text slots 78..89 labels, 90..101 values, 102..104 pagination; rect slots 28 and 29 (see the slot map of remix_hud_text.h). */
#define SC1PTRAININGREMIXMENUDRAW_LABEL_SLOT_FIRST 78
#define SC1PTRAININGREMIXMENUDRAW_VALUE_SLOT_FIRST 90
#define SC1PTRAININGREMIXMENUDRAW_PAGE_SLASH_SLOT 102
#define SC1PTRAININGREMIXMENUDRAW_PAGE_NEXT_SLOT 103
#define SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_SLOT 104
#define SC1PTRAININGREMIXMENUDRAW_CURSOR_RECT_SLOT 28
#define SC1PTRAININGREMIXMENUDRAW_UNDERLINE_RECT_SLOT 29

/* Training.asm setup_ 1647: Render.draw_rectangle(0x16, 0x16, 66, 45, 189, 154, 0x0064FF64, TRUE). */
#define SC1PTRAININGREMIXMENUDRAW_BACKGROUND_ULX 66
#define SC1PTRAININGREMIXMENUDRAW_BACKGROUND_ULY 45
#define SC1PTRAININGREMIXMENUDRAW_BACKGROUND_LRX 255
#define SC1PTRAININGREMIXMENUDRAW_BACKGROUND_LRY 199

static sb32 sSC1PTrainingRemixMenuDrawWasOpen;

/** The translucent blue panel behind the rows, the fill of the vanilla pause menu at the Remix rectangle. */
static void sc1PTrainingRemixMenuDrawProcBackground(GObj *background_gobj)
{
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, 0x00, 0x64, 0xFF, 0x64);
    gDPFillRectangle(gSYTaskmanDLHeads[0]++, SC1PTRAININGREMIXMENUDRAW_BACKGROUND_ULX, SC1PTRAININGREMIXMENUDRAW_BACKGROUND_ULY,
                     SC1PTRAININGREMIXMENUDRAW_BACKGROUND_LRX, SC1PTRAININGREMIXMENUDRAW_BACKGROUND_LRY);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
}

static void sc1PTrainingRemixMenuDrawMakeBackground(void)
{
    GObj *background_gobj = gcMakeGObjSPAfter(nGCCommonKindInterface, NULL, SC1PTRAININGREMIXMENUDRAW_GROUP, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(background_gobj, sc1PTrainingRemixMenuDrawProcBackground, SC1PTRAININGREMIXMENUDRAW_BACKGROUND_DL_LINK,
                     GOBJ_PRIORITY_DEFAULT, ~0);
}

static void sc1PTrainingRemixMenuDrawMakePageGlyphs(void)
{
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_R, SC1PTRAININGREMIXMENUDRAW_GROUP, SC1PTRAININGREMIXMENUDRAW_SPRITE_DL_LINK,
                      SC1PTRAININGREMIXMENUDRAW_PAGE_R_X, SC1PTRAININGREMIXMENUDRAW_PAGE_R_Y, SC1PTRAININGREMIXMENUDRAW_PAGE_GLYPH_TINT,
                      SC1PTRAININGREMIXMENUDRAW_PAGE_GLYPH_PALETTE, SC1PTRAININGREMIXMENUDRAW_PAGE_SCALE);
    ifRemixSpriteMake(IFREMIX_SPRITE_BUTTON_Z, SC1PTRAININGREMIXMENUDRAW_GROUP, SC1PTRAININGREMIXMENUDRAW_SPRITE_DL_LINK,
                      SC1PTRAININGREMIXMENUDRAW_PAGE_Z_X, SC1PTRAININGREMIXMENUDRAW_PAGE_Z_Y, SC1PTRAININGREMIXMENUDRAW_PAGE_GLYPH_TINT,
                      SC1PTRAININGREMIXMENUDRAW_PAGE_GLYPH_PALETTE, SC1PTRAININGREMIXMENUDRAW_PAGE_SCALE);
}

void sc1PTrainingRemixMenuDrawMake(void)
{
    sc1PTrainingRemixMenuDrawMakeBackground();
    sc1PTrainingModeMakeModalFrame(SC1PTRAININGREMIXMENUDRAW_GROUP);
    sc1PTrainingRemixMenuDrawMakePageGlyphs();
    gmRemixGroupDisplaySet(SC1PTRAININGREMIXMENUDRAW_GROUP, 1);
}

/**
 * Menu.draw_ 676-704: room left for the value of a row. The label is squeezed to 150 pixels; the value may reach from 10 pixels past
 * the start of the last label character to the right edge.
 */
static f32 sc1PTrainingRemixMenuDrawGetValueMaxWidth(const char *title)
{
    char prefix[REMIX_HUD_TEXT_LENGTH];
    size_t prefix_length = strlen(title) - 1;
    f32 label_width = remix_hud_text_measure(title, SC1PTRAININGREMIXMENUDRAW_SCALE);
    f32 squeeze = ((label_width > SC1PTRAININGREMIXMENUDRAW_LABEL_MAX_WIDTH) && (label_width > 0.0F))
                      ? (SC1PTRAININGREMIXMENUDRAW_LABEL_MAX_WIDTH / label_width)
                      : 1.0F;
    f32 last_char_x;
    f32 max_width;

    if (prefix_length >= sizeof(prefix))
    {
        prefix_length = sizeof(prefix) - 1;
    }
    memcpy(prefix, title, prefix_length);
    prefix[prefix_length] = '\0';

    last_char_x = SC1PTRAININGREMIXMENUDRAW_LABEL_X + (remix_hud_text_measure(prefix, SC1PTRAININGREMIXMENUDRAW_SCALE) * squeeze);
    max_width = SC1PTRAININGREMIXMENUDRAW_VALUE_X - (last_char_x + SC1PTRAININGREMIXMENUDRAW_VALUE_GAP);

    return (max_width > 1.0F) ? max_width : 1.0F;
}

static void sc1PTrainingRemixMenuDrawValue(s32 row, s32 slot, f32 y)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);
    s32 value = sc1PTrainingRemixMenuGetValue(slot);
    const char *text = sc1PTrainingMenuEntriesGetText(entry, value);
    s32 text_slot = SC1PTRAININGREMIXMENUDRAW_VALUE_SLOT_FIRST + row;

    if (entry->kind == SC1PTRAININGMENU_KIND_TITLE)
    {
        remix_hud_text_hide(text_slot);
        return;
    }
    if (text == NULL)
    {
        remix_hud_text_putf(text_slot, SC1PTRAININGREMIXMENUDRAW_VALUE_X, y, SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR,
                            SC1PTRAININGREMIXMENUDRAW_SCALE, REMIX_HUD_ALIGN_RIGHT, "%d", (int)value);
        return;
    }
    remix_hud_text_put_fit(text_slot, text, SC1PTRAININGREMIXMENUDRAW_VALUE_X, y, SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR,
                           SC1PTRAININGREMIXMENUDRAW_SCALE, REMIX_HUD_ALIGN_RIGHT,
                           sc1PTrainingRemixMenuDrawGetValueMaxWidth(entry->title));
}

static void sc1PTrainingRemixMenuDrawRow(s32 row, s32 slot)
{
    const SC1PTrainingMenuEntry *entry;
    f32 y = (f32)(SC1PTRAININGREMIXMENUDRAW_MENU_Y + (row * SC1PTRAININGMENURULES_ROW_HEIGHT));

    if (slot >= SC1PTRAININGMENU_SLOT_COUNT)
    {
        remix_hud_text_hide(SC1PTRAININGREMIXMENUDRAW_LABEL_SLOT_FIRST + row);
        remix_hud_text_hide(SC1PTRAININGREMIXMENUDRAW_VALUE_SLOT_FIRST + row);
        return;
    }
    entry = sc1PTrainingMenuEntriesGet(slot);

    remix_hud_text_put_fit(SC1PTRAININGREMIXMENUDRAW_LABEL_SLOT_FIRST + row, entry->title, SC1PTRAININGREMIXMENUDRAW_LABEL_X, y,
                           SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR, SC1PTRAININGREMIXMENUDRAW_SCALE, REMIX_HUD_ALIGN_LEFT,
                           SC1PTRAININGREMIXMENUDRAW_LABEL_MAX_WIDTH);
    sc1PTrainingRemixMenuDrawValue(row, slot, y);
}

static void sc1PTrainingRemixMenuDrawPagination(void)
{
    remix_hud_text_put(SC1PTRAININGREMIXMENUDRAW_PAGE_SLASH_SLOT, "/", SC1PTRAININGREMIXMENUDRAW_PAGE_SLASH_X,
                       SC1PTRAININGREMIXMENUDRAW_PAGE_SLASH_Y, SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR,
                       SC1PTRAININGREMIXMENUDRAW_PAGE_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_put(SC1PTRAININGREMIXMENUDRAW_PAGE_NEXT_SLOT, "Next/", SC1PTRAININGREMIXMENUDRAW_PAGE_NEXT_X,
                       SC1PTRAININGREMIXMENUDRAW_PAGE_NEXT_Y, SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR,
                       SC1PTRAININGREMIXMENUDRAW_PAGE_SCALE, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_put(SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_SLOT, "Prev", SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_X,
                       SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_Y, SC1PTRAININGREMIXMENUDRAW_TEXT_COLOR,
                       SC1PTRAININGREMIXMENUDRAW_PAGE_SCALE, REMIX_HUD_ALIGN_LEFT);
}

static void sc1PTrainingRemixMenuDrawCursor(s32 selection)
{
    f32 row_y = (f32)(SC1PTRAININGREMIXMENUDRAW_MENU_Y +
                      (sc1PTrainingMenuRulesGetRow(selection) * SC1PTRAININGMENURULES_ROW_HEIGHT));

    remix_hud_rect_put(SC1PTRAININGREMIXMENUDRAW_CURSOR_RECT_SLOT, SC1PTRAININGREMIXMENUDRAW_CURSOR_X,
                       row_y + SC1PTRAININGREMIXMENUDRAW_CURSOR_OFFSET_Y, SC1PTRAININGREMIXMENUDRAW_CURSOR_SIZE,
                       SC1PTRAININGREMIXMENUDRAW_CURSOR_SIZE, SC1PTRAININGREMIXMENUDRAW_CURSOR_COLOR);
    remix_hud_rect_put(SC1PTRAININGREMIXMENUDRAW_UNDERLINE_RECT_SLOT, SC1PTRAININGREMIXMENUDRAW_CURSOR_X,
                       row_y + SC1PTRAININGREMIXMENUDRAW_UNDERLINE_OFFSET_Y, SC1PTRAININGREMIXMENUDRAW_UNDERLINE_WIDTH, 1.0F,
                       SC1PTRAININGREMIXMENUDRAW_CURSOR_COLOR);
}

/** Takes every label and the cursor off the screen the frame the menu closes (a label would stay one frame more). */
static void sc1PTrainingRemixMenuDrawHide(void)
{
    s32 slot;

    for (slot = SC1PTRAININGREMIXMENUDRAW_LABEL_SLOT_FIRST; slot <= SC1PTRAININGREMIXMENUDRAW_PAGE_PREV_SLOT; slot++)
    {
        remix_hud_text_hide(slot);
    }
    remix_hud_rect_put(SC1PTRAININGREMIXMENUDRAW_CURSOR_RECT_SLOT, 0.0F, 0.0F, 0.0F, 0.0F, 0);
    remix_hud_rect_put(SC1PTRAININGREMIXMENUDRAW_UNDERLINE_RECT_SLOT, 0.0F, 0.0F, 0.0F, 0.0F, 0);
}

void sc1PTrainingRemixMenuDrawUpdate(sb32 is_open)
{
    s32 selection = sc1PTrainingRemixMenuGetSelection();
    s32 first_slot = sc1PTrainingMenuRulesGetPage(selection) * SC1PTRAININGMENURULES_ROWS_PER_PAGE;
    s32 row;

    gmRemixGroupDisplaySet(SC1PTRAININGREMIXMENUDRAW_GROUP, (is_open != FALSE) ? 0 : 1);

    if (is_open == FALSE)
    {
        if (sSC1PTrainingRemixMenuDrawWasOpen != FALSE)
        {
            sc1PTrainingRemixMenuDrawHide();
        }
        sSC1PTrainingRemixMenuDrawWasOpen = FALSE;
        return;
    }
    sSC1PTrainingRemixMenuDrawWasOpen = TRUE;
    for (row = 0; row < SC1PTRAININGMENURULES_ROWS_PER_PAGE; row++)
    {
        sc1PTrainingRemixMenuDrawRow(row, first_slot + row);
    }
    sc1PTrainingRemixMenuDrawPagination();
    sc1PTrainingRemixMenuDrawCursor(selection);
}

#endif
