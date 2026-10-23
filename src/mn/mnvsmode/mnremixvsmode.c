/**
 * mnremixvsmode.c - Remix buttons of the VS Game Mode menu (.toolchain/pkgs/smashremix-src/src/VsRemixMenu.asm).
 *
 * Page 1 gets a "Remix Modes" button (menu_button_table, index 4) and page 2 gets the five Remix modes
 * (remix_menu_button_table). The box of each button is the same as the vanilla buttons of its index, the text is a
 * sprite of the MNVSMode file at the offset given by the asm table.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/menu.h>
#include "mn/mnvsmode/mnvsmode.h"
#include "mn/mnvsmode/mnremixvsmode.h"

#define MNREMIX_VSMODE_BUTTON_WIDTH 17

typedef struct MNRemixVSModeRow
{
    f32 box_x;
    f32 box_y;
    f32 text_x;
    f32 text_y;
    u32 text_offset;
} MNRemixVSModeRow;

s32 gMNVSModeRemixMode = MNREMIX_VSMODE_DEFAULT;

/** Rows 0-4 are the page 2 modes in asm order (12CB, Tag Team, KOTH, Smashketball, Tug of War); row 5 is Remix Modes on page 1. */
static const MNRemixVSModeRow dMNRemixVSModeRows[] =
{
    { 120.0F, 31.0F, 132.0F, 36.0F, 0x00006760 },
    {  97.0F, 70.0F, 133.0F, 73.0F, 0x00007D38 },
    {  74.0F, 109.0F, 85.0F, 112.0F, 0x000084B8 },
    {  51.0F, 148.0F, 71.0F, 151.0F, 0x00008C38 },
    {  28.0F, 187.0F, 56.0F, 190.0F, 0x000093B8 },
    {  28.0F, 187.0F, 46.0F, 190.0F, 0x000075B8 }
};

/** Creates the box and the text of the Remix VS menu button of `row_index`; the caller sets its highlight. */
GObj* mnRemixVSModeMakeButton(s32 row_index, void *file)
{
    const MNRemixVSModeRow *row = &dMNRemixVSModeRows[row_index];
    GObj *button_gobj;
    SObj *text_sobj;

    button_gobj = gcMakeGObjSPAfter(0, NULL, 4, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjDisplay(button_gobj, lbCommonDrawSObjAttr, 2, GOBJ_PRIORITY_DEFAULT, ~0);
    mnVSModeMakeButton(button_gobj, row->box_x, row->box_y, MNREMIX_VSMODE_BUTTON_WIDTH);

    text_sobj = lbCommonMakeSObjForGObj(button_gobj, lbRelocGetFileData(Sprite*, file, row->text_offset));
    text_sobj->sprite.attr &= ~SP_FASTCOPY;
    text_sobj->sprite.attr |= SP_TRANSPARENT;
    text_sobj->sprite.red = 0x00;
    text_sobj->sprite.green = 0x00;
    text_sobj->sprite.blue = 0x00;
    text_sobj->pos.x = row->text_x;
    text_sobj->pos.y = row->text_y;

    return button_gobj;
}

#endif
