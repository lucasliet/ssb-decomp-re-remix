#include <mn/menu.h>
#include <it/item.h>
#include <sc/scene.h>
#include <gm/gmsound.h>
#include <sys/controller.h>
#include <sys/video.h>
#include <sys/rdp.h>
#include <reloc_data.h>
#ifdef PORT
extern void *func_800269C0_275C0(u16 id);
#endif
#ifdef REGION_REMIX
#include "it/itremix/itremixspawn.h"
#endif

// // // // // // // // // // // //
//                               //
//             MACROS            //
//                               //
// // // // // // // // // // // //

#define mnVSItemSwitchCheckGetOptionButtonInput(is_button, mask) \
mnCommonCheckGetOptionButtonInput(sMNVSItemSwitchOptionChangeWait, is_button, mask)

#define mnVSItemSwitchCheckGetOptionStickInputUD(stick_range, min, b) \
mnCommonCheckGetOptionStickInputUD(sMNVSItemSwitchOptionChangeWait, stick_range, min, b)

#define mnVSItemSwitchCheckGetOptionStickInputLR(stick_range, min, b) \
mnCommonCheckGetOptionStickInputLR(sMNVSItemSwitchOptionChangeWait, stick_range, min, b)

#define mnVSItemSwitchSetOptionChangeWaitP(is_button, stick_range, div) \
mnCommonSetOptionChangeWaitP(sMNVSItemSwitchOptionChangeWait, is_button, stick_range, div)

#define mnVSItemSwitchSetOptionChangeWaitN(is_button, stick_range, div) \
mnCommonSetOptionChangeWaitN(sMNVSItemSwitchOptionChangeWait, is_button, stick_range, div)

// // // // // // // // // // // //
//                               //
//       INITIALIZED DATA        //
//                               //
// // // // // // // // // // // //


// 0x80133210
s32 dMNVSItemSwitchTogglesItemKinds[/* */] =
{
    0,
    nITKindSword,
    nITKindBat,
    nITKindHammer,
    nITKindHarisen,
    nITKindMSBomb,
    nITKindBombHei,
    nITKindNBumper,
    nITKindGShell,
    nITKindMBall,
    nITKindLGun,
    nITKindFFlower,
    nITKindStarRod,
    nITKindTomato,
    nITKindHeart,
    nITKindStar
};

// 0x80133250
#ifdef PORT
u32 dMNVSItemSwitchFileIDs[/* */] = { llMNVSItemSwitchFileID };
#else
u32 dMNVSItemSwitchFileIDs[/* */] = { &llMNVSItemSwitchFileID };
#endif

// 0x80133258
Lights1 dMNVSItemSwitchLights1 = gdSPDefLights1(0x20, 0x20, 0x20, 0xFF, 0xFF, 0xFF, 0x3C, 0x3C, 0x3C);

// 0x80133270
Gfx dMNVSItemSwitchDisplayList[/* */] =
{
    gsSPSetGeometryMode(G_LIGHTING),
    gsSPSetLights1(dMNVSItemSwitchLights1),
    gsSPEndDisplayList()
};

// // // // // // // // // // // //
//                               //
//   GLOBAL / STATIC VARIABLES   //
//                               //
// // // // // // // // // // // //

// 0x801333D0
s32 sMNVSItemSwitchPad0x801333D0[2];

// 0x801333D8
s32 sMNVSItemSwitchOptionSelectID;

// 0x801333DC
GObj *sMNVSItemSwitchUnkGObj;

// 0x801333E0
GObj *sMNVSItemSwitchOptionGObjs[(nITKindUtilityEnd - nITKindUtilityStart) + 1];

// 0x80133420
sb32 sMNVSItemSwitchOptionStatuses[(nITKindUtilityEnd - nITKindUtilityStart) + 1];

// 0x80133460
GObj *sMNVSItemSwitchCursorGObj;

// 0x80133464
s32 sMNVSItemSwitchOptionChangeWait;

// 0x80133468
s32 sMNVSItemSwitchTotalTimeTics;

// 0x8013346C
s32 sMNVSItemSwitchReturnTic;

// 0x80133470
LBFileNode sMNVSItemSwitchStatusBuffer[24];

// 0x80133530
void *sMNVSItemSwitchFiles[ARRAY_COUNT(dMNVSItemSwitchFileIDs)];

#ifdef REGION_REMIX
// Remix Item.asm item_switch_cursor_ free-space equivalents:
// ITEM_SWITCH_PAGE(0x80133470), CUSTOM_ITEM_IMAGE(0x80133474),
// CUSTOM_ON_OFF_ARRAY(0x80133478). Page 0 = vanilla items, page 1 = custom.
s32 sMNVSItemSwitchPage;

// Vanilla item grid GObj, ejected/recreated on page flip (Remix hides it
// with a display flag; the port has no per-GObj hide, so we rebuild it).
GObj *sMNVSItemSwitchItemListGObj;

// Custom icon strip GObj (page 1), likewise rebuilt on each flip to page 1.
GObj *sMNVSItemSwitchCustomItemListGObj;

// ON/OFF values of the 13 Remix standard items (page-1 toggle rows 1..13).
sb32 sMNVSItemSwitchCustomOptionStatuses[ITREMIX_STANDARD_ITEMS_NUM];
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80131B00
void mnVSItemSwitchFuncLights(Gfx **dls)
{
    gSPDisplayList(dls[0]++, dMNVSItemSwitchDisplayList);
}

// 0x80131B24
void mnVSItemSwitchSetToggleSpriteColors(GObj *gobj, s32 status)
{
    SObj *sobj = SObjGetStruct(gobj);
    
    if (status != nMNOptionTabStatusOff)
    {
        sobj->sprite.red = 0xFF;
        sobj->sprite.green = 0x00;
        sobj->sprite.blue = 0x28;
        
        sobj->next->sprite.red = 0x32;
        sobj->next->sprite.green = 0x32;
        sobj->next->sprite.blue = 0x32;
    }
    else
    {
        sobj->sprite.red = 0x32;
        sobj->sprite.green = 0x32;
        sobj->sprite.blue = 0x32;
        
        sobj->next->sprite.red = 0xFF;
        sobj->next->sprite.green = 0x00;
        sobj->next->sprite.blue = 0x28;
    }
}

// 0x80131B98
void mnVSItemSwitchMakeToggle(GObj *gobj, f32 pos_x, f32 pos_y)
{
#ifdef PORT
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchToggleOnSprite));
#else
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchToggleOnSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->pos.x = pos_x;
    sobj->pos.y = pos_y;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchToggleOffSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchToggleOffSprite));
#endif

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->pos.x = pos_x + 21.0F + 5.0F;
    sobj->pos.y = pos_y;

#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchToggleSlashSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchToggleSlashSprite));
#endif

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->pos.x = pos_x + 21.0F;
    sobj->pos.y = pos_y;

    sobj->sprite.red = 0x32;
    sobj->sprite.green = 0x32;
    sobj->sprite.blue = 0x32;
}

// 0x80131CA4
void mnVSItemSwitchLabelsProcDisplay(GObj *gobj)
{
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    gDPSetCombineMode(gSYTaskmanDLHeads[0]++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
    gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, 0x80, 0x80, 0x80, 0xFF);
    gDPFillRectangle(gSYTaskmanDLHeads[0]++, 79, 34, 310, 39);
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetRenderMode(gSYTaskmanDLHeads[0]++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCycleType(gSYTaskmanDLHeads[0]++, G_CYC_1CYCLE);
    
    lbCommonClearExternSpriteParams();
    lbCommonDrawSObjAttr(gobj);
}

// 0x80131DE8
void mnVSItemSwitchMakeLabels(void)
{
    GObj *gobj;
    SObj *sobj;

    gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, mnVSItemSwitchLabelsProcDisplay, 1, GOBJ_PRIORITY_DEFAULT, ~0);
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchLabelVSOptionsSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchLabelVSOptionsSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->envcolor.r = 0x00;
    sobj->envcolor.g = 0x00;
    sobj->envcolor.b = 0x00;
    
    sobj->sprite.red = 0xF2;
    sobj->sprite.green = 0xC7;
    sobj->sprite.blue = 0x0D;
    
    sobj->pos.x = 84.0F;
    sobj->pos.y = 24.0F;
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchLabelItemSwitchSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchLabelItemSwitchSprite));
#endif

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xFF;
    sobj->sprite.blue = 0xFF;
    
    sobj->pos.x = 222.0F;
    sobj->pos.y = 30.0F;
}

// 0x80131EFC
void mnItemSwitchSetSubtitleSpriteColors(SObj *sobj)
{
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->envcolor.r = 0x00;
    sobj->envcolor.g = 0x00;
    sobj->envcolor.b = 0x00;
    
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xFF;
    sobj->sprite.blue = 0xFF;
}

// 0x80131F30
void mnItemSwitchMakeSubtitle(s32 option_id)
{
    GObj *gobj;
    SObj *sobj;
    
    // 0x80133298
    intptr_t offsets1[/* */] =
    {
#ifdef PORT
        llMNVSItemSwitchAppearancePercentTextJapSprite,
        llMNVSItemSwitchBeamSwordTextJapSprite,
        llMNVSItemSwitchHomerunBatTextJapSprite,
        llMNVSItemSwitchHammerTextJapSprite,
        llMNVSItemSwitchHarisenTextJapSprite,
        llMNVSItemSwitchMotionSensorBombTextJapSprite,
        llMNVSItemSwitchBombTrooperTextJapSprite,
        llMNVSItemSwitchBumperTextJapSprite,
        llMNVSItemSwitchShellTextJapSprite,
        llMNVSItemSwitchMonsterBallTextJapSprite,
        llMNVSItemSwitchLayGunTextJapSprite,
        llMNVSItemSwitchFireFlowerTextJapSprite,
        llMNVSItemSwitchStarRodTextJapSprite,
        llMNVSItemSwitchMaximTomatoTextJapSprite,
        llMNVSItemSwitchHeartTextJapSprite,
        llMNVSItemSwitchStarTextJapSprite
#else
        &llMNVSItemSwitchAppearancePercentTextJapSprite,
        &llMNVSItemSwitchBeamSwordTextJapSprite,
        &llMNVSItemSwitchHomerunBatTextJapSprite,
        &llMNVSItemSwitchHammerTextJapSprite,
        &llMNVSItemSwitchHarisenTextJapSprite,
        &llMNVSItemSwitchMotionSensorBombTextJapSprite,
        &llMNVSItemSwitchBombTrooperTextJapSprite,
        &llMNVSItemSwitchBumperTextJapSprite,
        &llMNVSItemSwitchShellTextJapSprite,
        &llMNVSItemSwitchMonsterBallTextJapSprite,
        &llMNVSItemSwitchLayGunTextJapSprite,
        &llMNVSItemSwitchFireFlowerTextJapSprite,
        &llMNVSItemSwitchStarRodTextJapSprite,
        &llMNVSItemSwitchMaximTomatoTextJapSprite,
        &llMNVSItemSwitchHeartTextJapSprite,
        &llMNVSItemSwitchStarTextJapSprite
#endif
    };

    // 0x801332D8
    intptr_t offsets2[/* */] =
    {
#ifdef PORT
        llMNVSItemSwitchNothingTextJapSprite,
        llMNVSItemSwitchVeryLowTextJapSprite,
        llMNVSItemSwitchLowTextJapSprite,
        llMNVSItemSwitchMiddleTextJapSprite,
        llMNVSItemSwitchHighTextJapSprite,
        llMNVSItemSwitchVeryHighTextJapSprite
#else
        &llMNVSItemSwitchNothingTextJapSprite,
        &llMNVSItemSwitchVeryLowTextJapSprite,
        &llMNVSItemSwitchLowTextJapSprite,
        &llMNVSItemSwitchMiddleTextJapSprite,
        &llMNVSItemSwitchHighTextJapSprite,
        &llMNVSItemSwitchVeryHighTextJapSprite
#endif
    };
    
#if defined(REGION_US)
    s32 unused2[2];
#endif
    
    sMNVSItemSwitchUnkGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);

#if defined(REGION_JP)
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);

#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchFrameSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchFrameSprite));
#endif

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->pos.x = 27.0F;
    sobj->pos.y = 186.0F;
    
    mnItemSwitchSetSubtitleSpriteColors(sobj);

    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], offsets1[option_id]));
            
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    switch (option_id)
    {
        case 0:
            sobj->pos.x = 31.0F;
            sobj->pos.y = 188.0F;
            break;
        case 5:
            sobj->pos.x = 31.0F;
            sobj->pos.y = 188.0F;
            break;
        default:
            sobj->pos.x = 31.0F;
            sobj->pos.y = 193.0F;
    }

    mnItemSwitchSetSubtitleSpriteColors(sobj);

    if (option_id == 0)
    {
            sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], offsets2[sMNVSItemSwitchOptionStatuses[option_id]]));
                
            sobj->sprite.attr &= ~SP_FASTCOPY;
            sobj->sprite.attr |= SP_TRANSPARENT;
            
            sobj->pos.x = 31.0F;
            sobj->pos.y = 199.0F;
        
            mnItemSwitchSetSubtitleSpriteColors(sobj);
    }
    
#endif
}

// 0x80131FDC
void mnVSItemSwitchMakeDecal(void)
{
    GObj *gobj;
    SObj *sobj;
    
    gobj = gcMakeGObjSPAfter(0, NULL, 6, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 4, GOBJ_PRIORITY_DEFAULT, ~0);
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchDecalButtonSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchDecalButtonSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0x48;
    sobj->sprite.green = 0x2A;
    sobj->sprite.blue = 0x23;
    
    sobj->pos.x = 10.0F;
    sobj->pos.y = 10.0F;
}

// 0x80132084
void mnVSItemSwitchMakeItemList(void)
{
    GObj *gobj;
    SObj *sobj;

#ifdef REGION_REMIX
    sMNVSItemSwitchItemListGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);
#else
    gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);
#endif
    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchItemListSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchItemListSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xFF;
    sobj->sprite.blue = 0xFF;
    
    sobj->pos.x = 125.0F;
    sobj->pos.y = 48.0F;
}

#ifdef REGION_REMIX
// Offset 0xBD70 of the decompressed Remix MNVSItemSwitch file (file_id 8,
// decompressed size 0xBDC0 vs vanilla 0x98A0): the pre-rendered icon+name
// strip for the 13 standard custom items appended by Smash Remix
// (Item.asm item_switch_setup_, Render.draw_texture_at_offset ... 0xBD70).
// Verified against the extracted baserom.remix.z64 file: it is a complete
// 68-byte Sprite, identical in shape to llMNVSItemSwitchItemListSprite
// (112x165, scale 1.0f, 3 bitmaps of 112x(72,72,23), texel data appended at
// 0x98A8/0xA870/0xB838, every pointer slot a regular reloc chain node), so
// it draws through the same SObj path as the vanilla grid.
#define dMNVSItemSwitchCustomItemListOffset ((intptr_t) 0xBD70)
#endif

// 0x8013212C
void mnVSItemSwitchSetCursorPosition(GObj *gobj, s32 option_id)
{
    SObj *sobj = SObjGetStruct(gobj);
    
    sobj->pos.x = 115.0F;
    
    if (option_id == 0)
    {
        sobj->pos.y = 47.0F;
    }
    else sobj->pos.y = (option_id * 10) + 51;
}

// 0x80132178
void mnVSItemSwitchMakeCursor(s32 off_y)
{
    GObj *gobj;
    SObj *sobj;

    sMNVSItemSwitchCursorGObj = gobj = gcMakeGObjSPAfter(0, NULL, 5, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 3, GOBJ_PRIORITY_DEFAULT, ~0);
#ifdef PORT
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], llMNVSItemSwitchCursorSprite));
#else
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], &llMNVSItemSwitchCursorSprite));
#endif
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xDE;
    sobj->sprite.blue = 0x00;
    
    mnVSItemSwitchSetCursorPosition(gobj, off_y);
}

#ifdef REGION_REMIX
// Draw the Remix custom item strip (page 1) with the same SObj path and at
// the same position as the vanilla grid (Item.asm item_switch_setup_).
void mnVSItemSwitchMakeCustomItemList(void)
{
    GObj *gobj;
    SObj *sobj;

    sMNVSItemSwitchCustomItemListGObj = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);

    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], dMNVSItemSwitchCustomItemListOffset));

    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;

    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0xFF;
    sobj->sprite.blue = 0xFF;

    sobj->pos.x = 125.0F;
    sobj->pos.y = 48.0F;
}

// Page-aware status accessors: Remix Item.asm handle_custom_item_values_
// redirects the row-N ON/OFF read/write to CUSTOM_ON_OFF_ARRAY[N-1] when
// the custom page is active. Row 0 (appearance rate) stays vanilla on both
// pages.
sb32 mnVSItemSwitchGetPageOptionStatus(s32 option_id)
{
    if ((sMNVSItemSwitchPage != 0) && (option_id != 0))
    {
        return sMNVSItemSwitchCustomOptionStatuses[option_id - 1];
    }
    return sMNVSItemSwitchOptionStatuses[option_id];
}

void mnVSItemSwitchSetPageOptionStatus(s32 option_id, sb32 status)
{
    if ((sMNVSItemSwitchPage != 0) && (option_id != 0))
    {
        sMNVSItemSwitchCustomOptionStatuses[option_id - 1] = status;
    }
    else sMNVSItemSwitchOptionStatuses[option_id] = status;
}

// Recreate one toggle row GObj (used when flipping back to page 0, which
// ejected rows 14-15 because the custom page only has 13 items).
void mnVSItemSwitchMakeToggleRow(s32 option_id)
{
    GObj *gobj;

    sMNVSItemSwitchOptionGObjs[option_id] = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    mnVSItemSwitchMakeToggle(gobj, 244.0F, option_id * 10 + 54);
    mnVSItemSwitchSetToggleSpriteColors(gobj, mnVSItemSwitchGetPageOptionStatus(option_id));
}

// Highest selectable row on the current page (Item.asm restrict_cursor_:
// 15 vanilla rows on page 0, 13 custom rows on page 1).
s32 mnVSItemSwitchGetPageLastOptionID(void)
{
    if (sMNVSItemSwitchPage != 0)
    {
        return ITREMIX_STANDARD_ITEMS_NUM;
    }
    return ARRAY_COUNT(sMNVSItemSwitchOptionGObjs) - 1;
}

sb32 mnVSItemSwitchCheckAnyCustomToggleOn(void)
{
    s32 i;

    for (i = 0; i < ITREMIX_STANDARD_ITEMS_NUM; i++)
    {
        if (sMNVSItemSwitchCustomOptionStatuses[i] != nMNOptionTabStatusOff)
        {
            return TRUE;
        }
    }
    return FALSE;
}
#endif

// 0x80132224
void mnVSItemSwitchMakeAppearance(s32 rate)
{
    GObj *gobj;
    SObj *sobj;

    // 0x801332F0
    s32 pos_x[/* */] =
    {
        242,
        240,
        254,
        244,
        252,
        238
    };

    // 0x80133308
    intptr_t rate_offsets[/* */] =
    {
#ifdef PORT
        llMNVSItemSwitchAppearanceNoneSprite,
        llMNVSItemSwitchAppearanceVeryLowSprite,
        llMNVSItemSwitchAppearanceLowSprite,
        llMNVSItemSwitchAppearanceMiddleSprite,
        llMNVSItemSwitchAppearanceHighSprite,
        llMNVSItemSwitchAppearanceVeryHighSprite
#else
        &llMNVSItemSwitchAppearanceNoneSprite,
        &llMNVSItemSwitchAppearanceVeryLowSprite,
        &llMNVSItemSwitchAppearanceLowSprite,
        &llMNVSItemSwitchAppearanceMiddleSprite,
        &llMNVSItemSwitchAppearanceHighSprite,
        &llMNVSItemSwitchAppearanceVeryHighSprite
#endif
    };
        
    sMNVSItemSwitchOptionGObjs[0] = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
    
    sobj = lbCommonMakeSObjForGObj(gobj, lbRelocGetFileData(Sprite*, sMNVSItemSwitchFiles[0], rate_offsets[rate]));
    
    sobj->sprite.attr &= ~SP_FASTCOPY;
    sobj->sprite.attr |= SP_TRANSPARENT;
    
    sobj->pos.x = pos_x[rate];
    sobj->pos.y = 49.0F;

    sobj->sprite.red = 0xFF;
    sobj->sprite.green = 0x00;
    sobj->sprite.blue = 0x00;
}

// 0x80132368
void mnVSItemSwitchInitToggles(void)
{
    s32 i;
    GObj *gobj;
    
    mnVSItemSwitchMakeAppearance(sMNVSItemSwitchOptionStatuses[0]);
    
    for (i = 1; i < (ARRAY_COUNT(sMNVSItemSwitchOptionGObjs) + ARRAY_COUNT(sMNVSItemSwitchOptionStatuses)) / 2; i++)
    {
        sMNVSItemSwitchOptionGObjs[i] = gobj = gcMakeGObjSPAfter(0, NULL, 3, GOBJ_PRIORITY_DEFAULT);

        gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, 1, GOBJ_PRIORITY_DEFAULT, ~0);
        mnVSItemSwitchMakeToggle(gobj, 244.0F, i * 10 + 54);
        mnVSItemSwitchSetToggleSpriteColors(gobj, sMNVSItemSwitchOptionStatuses[i]);
    }
}

// 0x80132468
void mnVSItemSwitchMakeCursorCamera(void)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            1,
            NULL,
            1,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            30,
            COBJ_MASK_DLLINK(3),
            ~0,
            FALSE,
            nGCProcessKindFunc,
            NULL,
            1,
            FALSE
        )
    );
    syRdpSetViewport(&cobj->viewport, 10.0F, 10.0F, 310.0F, 230.0F);
}

// 0x80132508
void mnVSItemSwitchMakeLabelsCamera(void)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            1,
            NULL,
            1,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            40,
            COBJ_MASK_DLLINK(1),
            ~0,
            FALSE,
            nGCProcessKindFunc,
            NULL,
            1,
            FALSE
        )
    );
    syRdpSetViewport(&cobj->viewport, 10.0F, 10.0F, 310.0F, 230.0F);
}

// 0x801325A8 - unused?
void func_ovl21_801325A8(void)
{
    return;
}

// 0x801325B0
void mnVSItemSwitchMakeDecalCamera(void)
{
    CObj *cobj = CObjGetStruct
    (
        gcMakeCameraGObj
        (
            1,
            NULL,
            1,
            GOBJ_PRIORITY_DEFAULT,
            lbCommonDrawSprite,
            60,
            COBJ_MASK_DLLINK(4),
            ~0,
            FALSE,
            nGCProcessKindFunc,
            NULL,
            1,
            FALSE
        )
    );
    syRdpSetViewport(&cobj->viewport, 10.0F, 10.0F, 310.0F, 230.0F);
}

// 0x80132650
void mnVSItemSwitchGetItemSettings(void)
{
    s32 i;

    sMNVSItemSwitchOptionStatuses[0] = gSCManagerTransferBattleState.item_appearance_rate;

    for (i = 1; i < (ARRAY_COUNT(dMNVSItemSwitchTogglesItemKinds) + ARRAY_COUNT(sMNVSItemSwitchOptionStatuses)) / 2; i++)
    {
        if ((1 << dMNVSItemSwitchTogglesItemKinds[i]) & gSCManagerTransferBattleState.item_toggles)
        {
            sMNVSItemSwitchOptionStatuses[i] = nMNOptionTabStatusOn;
        }
        else sMNVSItemSwitchOptionStatuses[i] = nMNOptionTabStatusOff;
    }

#ifdef REGION_REMIX
    // Item.asm extend_item_switch_bitmask_.initialize_: custom item n reads
    // item_toggles bit nITKindCommonEnd+1+n (n < 12, mask is full past that)
    // or gITRemixExtendedToggles bit n-12.
    for (i = 0; i < ITREMIX_STANDARD_ITEMS_NUM; i++)
    {
        u32 toggle_mask;
        u32 toggles;

        if (i < ITREMIX_VANILLA_TOGGLE_ITEMS_MAX)
        {
            toggle_mask = 1U << (nITKindCommonEnd + 1 + i);
            toggles = gSCManagerTransferBattleState.item_toggles;
        }
        else
        {
            toggle_mask = 1U << (i - ITREMIX_VANILLA_TOGGLE_ITEMS_MAX);
            toggles = gITRemixExtendedToggles;
        }

        if (toggles & toggle_mask)
        {
            sMNVSItemSwitchCustomOptionStatuses[i] = nMNOptionTabStatusOn;
        }
        else sMNVSItemSwitchCustomOptionStatuses[i] = nMNOptionTabStatusOff;
    }
#endif
}

// 0x801327B8
void mnVSItemSwitchSetItemSettings(void)
{
    s32 i;

    gSCManagerTransferBattleState.item_appearance_rate = sMNVSItemSwitchOptionStatuses[0];

    for (i = 1; i < (ARRAY_COUNT(dMNVSItemSwitchTogglesItemKinds) + ARRAY_COUNT(sMNVSItemSwitchOptionStatuses)) / 2; i++)
    {
        if (sMNVSItemSwitchOptionStatuses[i] != nMNOptionTabStatusOff)
        {
            gSCManagerTransferBattleState.item_toggles |= (1 << dMNVSItemSwitchTogglesItemKinds[i]);

            if (dMNVSItemSwitchTogglesItemKinds[i] == nITKindGShell)
            {
                gSCManagerTransferBattleState.item_toggles |= (1 << nITKindRShell);
            }
        }
        else
        {
            gSCManagerTransferBattleState.item_toggles &= ~(1 << dMNVSItemSwitchTogglesItemKinds[i]);

            if (dMNVSItemSwitchTogglesItemKinds[i] == nITKindGShell)
            {
                gSCManagerTransferBattleState.item_toggles &= ~(1 << nITKindRShell);
            }
        }
    }

#ifdef REGION_REMIX
    // Item.asm extend_item_switch_bitmask_.save_: write the custom item
    // ON/OFF values back to item_toggles bits nITKindCommonEnd+1+n and to
    // gITRemixExtendedToggles bit n-12.
    {
        sb32 is_any_custom_on = FALSE;

        for (i = 0; i < ITREMIX_STANDARD_ITEMS_NUM; i++)
        {
            u32 toggle_mask;
            u32 *toggles;

            if (i < ITREMIX_VANILLA_TOGGLE_ITEMS_MAX)
            {
                toggle_mask = 1U << (nITKindCommonEnd + 1 + i);
                toggles = &gSCManagerTransferBattleState.item_toggles;
            }
            else
            {
                toggle_mask = 1U << (i - ITREMIX_VANILLA_TOGGLE_ITEMS_MAX);
                toggles = &gITRemixExtendedToggles;
            }

            if (sMNVSItemSwitchCustomOptionStatuses[i] != nMNOptionTabStatusOff)
            {
                is_any_custom_on = TRUE;
                *toggles |= toggle_mask;
            }
            else *toggles &= ~toggle_mask;
        }

        // Same rule as vanilla "any item on": custom items being on forces
        // the containers (Item.asm save_ ORs 0xF into the bitmask).
        if (is_any_custom_on)
        {
            gSCManagerTransferBattleState.item_toggles |= ((1 << nITKindEgg) | (1 << nITKindCapsule) | (1 << nITKindTaru) | (1 << nITKindBox));
        }

        // Item.asm save_ stores the extended mask twice: live (+0x00) and
        // saved (+0x04, restored at the CSS scene start).
        itRemixSaveExtendedToggles();
    }
#endif
}

// 0x80132948
void mnVSItemSwitchInitVars(void)
{
    sMNVSItemSwitchOptionSelectID = 0;

#ifdef REGION_REMIX
    sMNVSItemSwitchPage = 0;
#endif

    mnVSItemSwitchGetItemSettings();
    
    sMNVSItemSwitchOptionChangeWait = 0;
    sMNVSItemSwitchTotalTimeTics = 0;
    
    sMNVSItemSwitchReturnTic = sMNVSItemSwitchTotalTimeTics + I_MIN_TO_TICS(5);
}

// 0x80132988
sb32 mnVSItemSwitchCheckAllTogglesOff(void)
{
    s32 i;
    
    for (i = 1; i < ARRAY_COUNT(sMNVSItemSwitchOptionStatuses); i++)
    {
        if (sMNVSItemSwitchOptionStatuses[i] != nMNOptionTabStatusOff)
        {
            return FALSE;
        }
    }
    return TRUE;
}

// 0x80132A44
void mnVSItemSwitchSetItemToggles(void)
{
#ifdef REGION_REMIX
    // Item.asm always_update_item_frequency_: custom items count as "any
    // item on" for the container/rate forcing, and the rate is saved even
    // when every toggle is off (vanilla lazily keeps the previous rate).
    if ((mnVSItemSwitchCheckAllTogglesOff() != FALSE) && (mnVSItemSwitchCheckAnyCustomToggleOn() == FALSE))
    {
        gSCManagerTransferBattleState.item_toggles = 0;
        gSCManagerTransferBattleState.item_appearance_rate = sMNVSItemSwitchOptionStatuses[0];
    }
    else
    {
        mnVSItemSwitchSetItemSettings();

        gSCManagerTransferBattleState.item_toggles |= ((1 << nITKindEgg) | (1 << nITKindCapsule) | (1 << nITKindTaru) | (1 << nITKindBox));
    }
#else
    if (mnVSItemSwitchCheckAllTogglesOff() != FALSE)
    {
        gSCManagerTransferBattleState.item_toggles = 0;
    }
    else
    {
        mnVSItemSwitchSetItemSettings();

        gSCManagerTransferBattleState.item_toggles |= ((1 << nITKindEgg) | (1 << nITKindCapsule) | (1 << nITKindTaru) | (1 << nITKindBox));
    }
#endif
}

// 0x80132A94
void mnVSItemSwitchUpdateOption(s32 option_id, s32 rate)
{
    if (option_id == 0)
    {
        gcEjectGObj(sMNVSItemSwitchOptionGObjs[option_id]);
        mnVSItemSwitchMakeAppearance(rate);
    }
    else mnVSItemSwitchSetToggleSpriteColors(sMNVSItemSwitchOptionGObjs[option_id], rate);
}

#ifdef REGION_REMIX
// Item.asm item_switch_cursor_.handle_r_l_._toggle_items_on_off (physical
// L): flip the first toggle row of the current page and write it to every
// toggle row of that page, then refresh the ON/OFF sprites.
void mnVSItemSwitchToggleAllPageItems(void)
{
    s32 i;
    s32 last = mnVSItemSwitchGetPageLastOptionID();
    sb32 status = mnVSItemSwitchGetPageOptionStatus(1);

    if (status == nMNOptionTabStatusOff)
    {
        status = nMNOptionTabStatusOn;
    }
    else status = nMNOptionTabStatusOff;

    for (i = 1; i <= last; i++)
    {
        mnVSItemSwitchSetPageOptionStatus(i, status);
        mnVSItemSwitchUpdateOption(i, status);
    }
    func_800269C0_275C0(nSYAudioFGMMenuScroll1);
}

// Item.asm item_switch_cursor_.handle_r_l_ (physical R): flip between the
// vanilla grid (page 0) and the custom item strip (page 1), clamp the
// cursor to the page's row count and refresh the toggle sprites.
void mnVSItemSwitchFlipPage(void)
{
    s32 i;
    s32 last;

    sMNVSItemSwitchPage ^= 1;

    if (sMNVSItemSwitchPage != 0)
    {
        // Page 1: hide the vanilla item grid, show the custom strip.
        // Remix flips a display flag; the port rebuilds the GObjs instead
        // (gcEjectGObj(NULL) is not a no-op, so guard every eject).
        if (sMNVSItemSwitchItemListGObj != NULL)
        {
            gcEjectGObj(sMNVSItemSwitchItemListGObj);
            sMNVSItemSwitchItemListGObj = NULL;
        }
        mnVSItemSwitchMakeCustomItemList();

        // Rows 14-15 have no custom counterpart; eject until we flip back.
        for (i = ITREMIX_STANDARD_ITEMS_NUM + 1; i < ARRAY_COUNT(sMNVSItemSwitchOptionGObjs); i++)
        {
            if (sMNVSItemSwitchOptionGObjs[i] != NULL)
            {
                gcEjectGObj(sMNVSItemSwitchOptionGObjs[i]);
                sMNVSItemSwitchOptionGObjs[i] = NULL;
            }
        }
    }
    else
    {
        // Page 0: restore the vanilla grid and toggle rows 14-15.
        if (sMNVSItemSwitchCustomItemListGObj != NULL)
        {
            gcEjectGObj(sMNVSItemSwitchCustomItemListGObj);
            sMNVSItemSwitchCustomItemListGObj = NULL;
        }
        mnVSItemSwitchMakeItemList();

        for (i = ITREMIX_STANDARD_ITEMS_NUM + 1; i < ARRAY_COUNT(sMNVSItemSwitchOptionGObjs); i++)
        {
            mnVSItemSwitchMakeToggleRow(i);
        }
    }

    last = mnVSItemSwitchGetPageLastOptionID();

    if (sMNVSItemSwitchOptionSelectID > last)
    {
        sMNVSItemSwitchOptionSelectID = last;
    }
    for (i = 1; i <= last; i++)
    {
        mnVSItemSwitchUpdateOption(i, mnVSItemSwitchGetPageOptionStatus(i));
    }
    mnVSItemSwitchSetCursorPosition(sMNVSItemSwitchCursorGObj, sMNVSItemSwitchOptionSelectID);

    func_800269C0_275C0(nSYAudioFGMMenuScroll2);
}
#endif

// 0x80132AF0
void mnVSItemSwitchFuncRun(GObj *gobj)
{
    s32 unused;
    s32 stick_range;
    sb32 is_button;
    
    sMNVSItemSwitchTotalTimeTics++;
    
    if (sMNVSItemSwitchTotalTimeTics >= 10)
    {
        if (sMNVSItemSwitchOptionChangeWait != 0)
        {
            sMNVSItemSwitchOptionChangeWait--;
        }
        if
        (
            (scSubsysControllerGetPlayerStickInRangeLR(-20, 20) != FALSE)                                         &&
            (scSubsysControllerGetPlayerStickInRangeUD(-20, 20) != FALSE)                                         &&
            (scSubsysControllerGetPlayerHoldButtons(U_JPAD | R_JPAD | R_TRIG | U_CBUTTONS | R_CBUTTONS) == FALSE) &&
            (scSubsysControllerGetPlayerHoldButtons(D_JPAD | L_JPAD | L_TRIG | D_CBUTTONS | L_CBUTTONS) == FALSE)
        )
        {
            sMNVSItemSwitchOptionChangeWait = 0;
        }
        if (scSubsysControllerGetPlayerTapButtons(B_BUTTON) != FALSE)
        {
            gSCManagerSceneData.scene_prev = gSCManagerSceneData.scene_curr;
            gSCManagerSceneData.scene_curr = nSCKindVSOptions;
            
            mnVSItemSwitchSetItemToggles();
            syTaskmanSetLoadScene();
        }
        if
        (
            mnVSItemSwitchCheckGetOptionButtonInput(is_button, U_JPAD | U_CBUTTONS) ||
            mnVSItemSwitchCheckGetOptionStickInputUD(stick_range, 20, 1)
        )
        {
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
                
            mnVSItemSwitchSetOptionChangeWaitP(is_button, stick_range, 7);
            
            if (sMNVSItemSwitchOptionSelectID == 0)
            {
#ifdef REGION_REMIX
                sMNVSItemSwitchOptionSelectID = mnVSItemSwitchGetPageLastOptionID();
#else
                sMNVSItemSwitchOptionSelectID = (ARRAY_COUNT(sMNVSItemSwitchOptionGObjs) - 1);
#endif
            }
            else sMNVSItemSwitchOptionSelectID--;
    
            if (sMNVSItemSwitchOptionSelectID == 0)
            {
                sMNVSItemSwitchOptionChangeWait += 8;
            }
            mnVSItemSwitchSetCursorPosition(sMNVSItemSwitchCursorGObj, sMNVSItemSwitchOptionSelectID);
            
            gcEjectGObj(sMNVSItemSwitchUnkGObj);
            mnItemSwitchMakeSubtitle(sMNVSItemSwitchOptionSelectID);
        }
        if
        (
            mnVSItemSwitchCheckGetOptionButtonInput(is_button, D_JPAD | D_CBUTTONS) ||
            mnVSItemSwitchCheckGetOptionStickInputUD(stick_range, -20, 0)
        )
        {
            func_800269C0_275C0(nSYAudioFGMMenuScroll2);
                
            mnVSItemSwitchSetOptionChangeWaitN(is_button, stick_range, 7);
            
#ifdef REGION_REMIX
            if (sMNVSItemSwitchOptionSelectID == mnVSItemSwitchGetPageLastOptionID())
#else
            if (sMNVSItemSwitchOptionSelectID == (ARRAY_COUNT(sMNVSItemSwitchOptionGObjs) - 1))
#endif
            {
                sMNVSItemSwitchOptionSelectID = 0;
            }
            else sMNVSItemSwitchOptionSelectID++;
    
            if (sMNVSItemSwitchOptionSelectID == (ARRAY_COUNT(sMNVSItemSwitchOptionGObjs) - 1))
            {
                sMNVSItemSwitchOptionChangeWait += 8;
            }
            mnVSItemSwitchSetCursorPosition(sMNVSItemSwitchCursorGObj, sMNVSItemSwitchOptionSelectID);
            
            gcEjectGObj(sMNVSItemSwitchUnkGObj);
            mnItemSwitchMakeSubtitle(sMNVSItemSwitchOptionSelectID);
        }
        if
        (
#ifdef REGION_REMIX
            mnVSItemSwitchCheckGetOptionButtonInput(is_button, L_JPAD | L_CBUTTONS) ||
#else
            mnVSItemSwitchCheckGetOptionButtonInput(is_button, L_JPAD | L_TRIG | L_CBUTTONS) ||
#endif
            mnVSItemSwitchCheckGetOptionStickInputLR(stick_range, -20, 0)
        )
        {
            if (sMNVSItemSwitchOptionSelectID == 0)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                
                if (sMNVSItemSwitchOptionStatuses[0] > nSCBattleItemSwitchNone)
                {
                    sMNVSItemSwitchOptionStatuses[0]--;
                }
                else sMNVSItemSwitchOptionStatuses[0] = nSCBattleItemSwitchVeryHigh;
                
                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, sMNVSItemSwitchOptionStatuses[0]);
                
                gcEjectGObj(sMNVSItemSwitchUnkGObj);
                mnItemSwitchMakeSubtitle(sMNVSItemSwitchOptionSelectID);
            }
#ifdef REGION_REMIX
            else if (mnVSItemSwitchGetPageOptionStatus(sMNVSItemSwitchOptionSelectID) == nMNOptionTabStatusOff)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);

                mnVSItemSwitchSetPageOptionStatus(sMNVSItemSwitchOptionSelectID, nMNOptionTabStatusOn);

                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, nMNOptionTabStatusOn);
            }
#else
            else if (sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] == nMNOptionTabStatusOff)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                
                sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] = nMNOptionTabStatusOn;
                
                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID]);
            }
#endif
            mnVSItemSwitchSetOptionChangeWaitN(is_button, stick_range, 7);
        }
        if
        (
#ifdef REGION_REMIX
            mnVSItemSwitchCheckGetOptionButtonInput(is_button, R_JPAD | R_CBUTTONS) ||
#else
            mnVSItemSwitchCheckGetOptionButtonInput(is_button, R_JPAD | R_TRIG | R_CBUTTONS) ||
#endif
            mnVSItemSwitchCheckGetOptionStickInputLR(stick_range, 20, 1)
        )
        {
            if (sMNVSItemSwitchOptionSelectID == 0)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                
                if (sMNVSItemSwitchOptionStatuses[0] < nSCBattleItemSwitchVeryHigh)
                {
                    sMNVSItemSwitchOptionStatuses[0]++;
                }
                else sMNVSItemSwitchOptionStatuses[0] = nSCBattleItemSwitchNone;
                
                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, sMNVSItemSwitchOptionStatuses[0]);
                
                gcEjectGObj(sMNVSItemSwitchUnkGObj);
                mnItemSwitchMakeSubtitle(sMNVSItemSwitchOptionSelectID);
            }
#ifdef REGION_REMIX
            else if (mnVSItemSwitchGetPageOptionStatus(sMNVSItemSwitchOptionSelectID) == nMNOptionTabStatusOn)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);

                mnVSItemSwitchSetPageOptionStatus(sMNVSItemSwitchOptionSelectID, nMNOptionTabStatusOff);

                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, nMNOptionTabStatusOff);
            }
#else
            else if (sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] == nMNOptionTabStatusOn)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                
                sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] = nMNOptionTabStatusOff;
                
                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID]);
            }
#endif
            mnVSItemSwitchSetOptionChangeWaitP(is_button, stick_range, 7);
        }
        if (scSubsysControllerGetPlayerTapButtons(A_BUTTON) != FALSE)
        {
            if (sMNVSItemSwitchOptionSelectID == 0)
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);
                
                if (sMNVSItemSwitchOptionStatuses[0] < nSCBattleItemSwitchVeryHigh)
                {
                    sMNVSItemSwitchOptionStatuses[0]++;
                }
                else sMNVSItemSwitchOptionStatuses[0] = nSCBattleItemSwitchNone;
                
                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, sMNVSItemSwitchOptionStatuses[0]);
                
                gcEjectGObj(sMNVSItemSwitchUnkGObj);
                mnItemSwitchMakeSubtitle(sMNVSItemSwitchOptionSelectID);
            }
            else
            {
                func_800269C0_275C0(nSYAudioFGMMenuScroll1);

#ifdef REGION_REMIX
                if (mnVSItemSwitchGetPageOptionStatus(sMNVSItemSwitchOptionSelectID) == nMNOptionTabStatusOff)
                {
                    mnVSItemSwitchSetPageOptionStatus(sMNVSItemSwitchOptionSelectID, nMNOptionTabStatusOn);
                }
                else mnVSItemSwitchSetPageOptionStatus(sMNVSItemSwitchOptionSelectID, nMNOptionTabStatusOff);

                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, mnVSItemSwitchGetPageOptionStatus(sMNVSItemSwitchOptionSelectID));
#else
                if (sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] == nMNOptionTabStatusOff)
                {
                    sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] = nMNOptionTabStatusOn;
                }
                else sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID] = nMNOptionTabStatusOff;
                
                mnVSItemSwitchUpdateOption(sMNVSItemSwitchOptionSelectID, sMNVSItemSwitchOptionStatuses[sMNVSItemSwitchOptionSelectID]);
#endif
            }
        }

#ifdef REGION_REMIX
        // Item.asm item_switch_cursor_.handle_r_l_: with L_TRIG/R_TRIG
        // removed from the option masks above, the physical buttons handle
        // toggle-all (L) and page flip (R).
        if (scSubsysControllerGetPlayerTapButtons(L_TRIG) != FALSE)
        {
            mnVSItemSwitchToggleAllPageItems();
        }
        if (scSubsysControllerGetPlayerTapButtons(R_TRIG) != FALSE)
        {
            mnVSItemSwitchFlipPage();
        }
#endif
    }
}

// 0x80133090
void mnVSItemSwitchFuncStart(void)
{
    LBRelocSetup rl_setup;

    rl_setup.table_addr = (uintptr_t)&lLBRelocTableAddr;
#ifdef PORT
    rl_setup.table_files_num = (u32)llRelocFileCount;
#else
    rl_setup.table_files_num = (u32)&llRelocFileCount;
#endif
    rl_setup.file_heap = NULL;
    rl_setup.file_heap_size = 0;
    rl_setup.status_buffer = sMNVSItemSwitchStatusBuffer;
    rl_setup.status_buffer_size = ARRAY_COUNT(sMNVSItemSwitchStatusBuffer);
    rl_setup.force_status_buffer = NULL;
    rl_setup.force_status_buffer_size = 0;
    
    lbRelocInitSetup(&rl_setup);
    lbRelocLoadFilesListed(dMNVSItemSwitchFileIDs, sMNVSItemSwitchFiles);
    gcMakeGObjSPAfter(0, mnVSItemSwitchFuncRun, 0, GOBJ_PRIORITY_DEFAULT);
    gcMakeDefaultCameraGObj(0, GOBJ_PRIORITY_DEFAULT, 100, COBJ_FLAG_FILLCOLOR | COBJ_FLAG_ZBUFFER, GPACK_RGBA8888(0x00, 0x00, 0x00, 0xFF));
    
    mnVSItemSwitchInitVars();
    mnVSItemSwitchMakeCursorCamera();
    mnVSItemSwitchMakeLabelsCamera();
    mnVSItemSwitchMakeDecalCamera();
    mnVSItemSwitchMakeLabels();
    mnVSItemSwitchMakeDecal();
    mnVSItemSwitchMakeItemList();
    mnVSItemSwitchMakeCursor(sMNVSItemSwitchOptionSelectID);
    mnVSItemSwitchInitToggles();

    mnItemSwitchMakeSubtitle(sMNVSItemSwitchOptionSelectID);
}

// 0x80133320
SYVideoSetup dMNVSItemSwitchVideoSetup = SYVIDEO_SETUP_DEFAULT();

// 0x8013333C
SYTaskmanSetup dMNVSItemSwitchTaskmanSetup =
{
    // Task Manager Buffer Setup
    {
        0,                          // ???
        gcRunAll,              		// Update function
        gcDrawAll,                  // Frame draw function
        &ovl21_BSS_END,             // Allocatable memory pool start
        0,                          // Allocatable memory pool size
        1,                          // ???
        2,                          // Number of contexts?
        sizeof(Gfx) * 2500,         // Display List Buffer 0 Size
        0,                          // Display List Buffer 1 Size
        0,                          // Display List Buffer 2 Size
        0,                          // Display List Buffer 3 Size
        0x8000,                     // Graphics Heap Size
        2,                          // ???
        0xC000,                     // RDP Output Buffer Size
        mnVSItemSwitchFuncLights,   // Pre-render function
        syControllerFuncRead,       // Controller I/O function
    },

    8,                              // Number of GObjThreads
    sizeof(u64) * 192,              // Thread stack size
    0,                              // Number of thread stacks
    0,                              // ???
    0,                              // Number of GObjProcesses
    0,                              // Number of GObjs
    sizeof(GObj),                   // GObj size
    0,                              // Number of XObjs
    NULL,                           // Matrix function list
    NULL,                           // DObjVec eject function
    0,                              // Number of AObjs
    0,                              // Number of MObjs
    0,                              // Number of DObjs
    sizeof(DObj),                   // DObj size
    0,                              // Number of SObjs
    sizeof(SObj),                   // SObj size
    0,                              // Number of CObjs
    sizeof(CObj),                 	// CObj size
    
    mnVSItemSwitchFuncStart         // Task start function
};

// 0x801331B0
void mnVSItemSwitchStartScene(void)
{
    dMNVSItemSwitchVideoSetup.zbuffer = SYVIDEO_ZBUFFER_START(320, 240, 0, 10, u16);
    syVideoInit(&dMNVSItemSwitchVideoSetup);
    
    dMNVSItemSwitchTaskmanSetup.scene_setup.arena_size = (size_t) ((uintptr_t)&ovl1_VRAM - (uintptr_t)&ovl21_BSS_END);
    syTaskmanStartTask(&dMNVSItemSwitchTaskmanSetup);
}
