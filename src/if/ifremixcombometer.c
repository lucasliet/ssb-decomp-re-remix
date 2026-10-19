/**
 * ifremixcombometer.c - Smash Remix combo meter setup and display (src/ComboMeter.asm setup_, run_, draw_hit_count_).
 *
 * The meter of each port is a GObj with four SObjs (COMBO text, then three digits) from the Remix texture file. The
 * colour set of a port is its team colour when teams are on, else its port colour. In 1v1 the two meters swap their
 * screen positions. Hit state lives in ifremixcombohits.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "if/ifremixcombometer.h"
#include "if/interface.h"
#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <ft/fighter.h>
#include <gr/grremix/grremixids.h>
#include <lb/lbcommon.h>
#include <lb/lbreloc.h>
#include <sc/scmanager.h>
#include <sys/objman.h>
#include <sys/taskman.h>

extern void portFixupBitmapArray(void *bitmaps, unsigned int count);
extern void portFixupSpriteBitmapData(void *sprite, void *bitmaps);

/* File.VS_COMBO_METER_TEXTURES (File.asm) */
#define IFREMIX_COMBO_FILE_ID 0x9A6
#define IFREMIX_COMBO_GOBJ_ID 0x3FE
#define IFREMIX_COMBO_LINK_ID 0x0B
#define IFREMIX_COMBO_DL_LINK_ID 0x18
#define IFREMIX_COMBO_SPRITE_ATTR 0x0201
#define IFREMIX_COMBO_NO_DIGIT (-1)
#define IFREMIX_COMBO_DIGIT_COUNT 10
#define IFREMIX_COMBO_DIGIT_OFFSET 0x0260
#define IFREMIX_COMBO_BITMAP_HEADER 0x0010
#define IFREMIX_COMBO_TEXT_SPRITE_OFFSET 0x0818
#define IFREMIX_COMBO_NUMBER_SPRITE_OFFSET 0x2BF8
#define IFREMIX_COMBO_PLAYER_MAX_PKIND 2
#define IFREMIX_COMBO_DK_HUD_ALWAYS 1
#define IFREMIX_COMBO_DK_HUD_OFF 2
#define IFREMIX_COMBO_X_OFFSET (-45.0F)
#define IFREMIX_COMBO_NUMBERS_X_OFFSET 64.0F
#define IFREMIX_COMBO_NUMBER_X_STEP 9.0F
#define IFREMIX_COMBO_Y_COORD 167.0F
#define IFREMIX_COMBO_DK_Y_COORD 50.0F
#define IFREMIX_COMBO_SHOW_CAMERA_TAG 0xFFFFFFFF

enum IFRemixComboTextureSet
{
    nIFRemixComboSetRed,
    nIFRemixComboSetBlue,
    nIFRemixComboSetYellow,
    nIFRemixComboSetGreen,
    nIFRemixComboSetSilver,
    nIFRemixComboSetCount
};

/* File offsets of the COMBO texts and the 0 digits per texture set, the asm TEXT_OFFSET_X and NUMBER_OFFSET_X */
static const s32 dIFRemixComboTextOffsets[nIFRemixComboSetCount] = { 0x0818, 0x1078, 0x18D8, 0x2138, 0x2998 };
static const s32 dIFRemixComboNumberOffsets[nIFRemixComboSetCount] = { 0x2BF8, 0x43B8, 0x5B78, 0x7338, 0x8AF8 };

/* Texture set of each port without teams (P1 red, P2 blue, P3 yellow, P4 green) */
static const s32 dIFRemixComboPortSets[GMCOMMON_PLAYERS_MAX] =
{
    nIFRemixComboSetRed, nIFRemixComboSetBlue, nIFRemixComboSetYellow, nIFRemixComboSetGreen
};

/* Texture set of each team colour (0 red, 1 blue, 2 green, 3 yellow), the asm team_offset_table */
static const s32 dIFRemixComboTeamSets[GMCOMMON_PLAYERS_MAX] =
{
    nIFRemixComboSetRed, nIFRemixComboSetBlue, nIFRemixComboSetGreen, nIFRemixComboSetYellow
};

typedef struct IFRemixComboPlayerSlot
{
    f32 x_coord;
    IFRemixComboMeter *meter;
    s32 port;
} IFRemixComboPlayerSlot;

static void *sIFRemixComboFile;
static IFRemixComboMeter sIFRemixComboMeters[GMCOMMON_PLAYERS_MAX];
static IFRemixComboMeter *sIFRemixComboSlots[GMCOMMON_PLAYERS_MAX];
static s32 sIFRemixComboColorSets[IFREMIX_COMBO_COLOR_SILVER + 1];
static u32 sIFRemixComboTextTokens[nIFRemixComboSetCount];
static u32 sIFRemixComboNumberTokens[nIFRemixComboSetCount][IFREMIX_COMBO_DIGIT_COUNT];

/** TRUE in the scenes where the asm calls ComboMeter.setup_ (VS battle and 1P game). */
static sb32 ifRemixComboIsSetupScene(void)
{
    u8 scene = gSCManagerSceneData.scene_curr;

    return (scene == nSCKindVSBattle) || (scene == nSCKind1PGame);
}

/** Y coordinate of the meter: the Dragon King HUD setting, and on the default setting the Dragon King stages. */
static f32 ifRemixComboGetYCoord(void)
{
    s32 setting = gmRemixToggle(dragon_king_hud);

    if (setting == IFREMIX_COMBO_DK_HUD_OFF)
    {
        return IFREMIX_COMBO_Y_COORD;
    }
    if (setting == IFREMIX_COMBO_DK_HUD_ALWAYS)
    {
        return IFREMIX_COMBO_DK_Y_COORD;
    }
    if ((gSCManagerSceneData.gkind == nGRRemixStage_DRAGONKING) || (gSCManagerSceneData.gkind == nGRRemixStage_DRAGONKING_REMIX))
    {
        return IFREMIX_COMBO_DK_Y_COORD;
    }
    return IFREMIX_COMBO_Y_COORD;
}

/**
 * Left edge of the meter from the HUD X position of the port. A port without a HUD position gets 0.0F; the asm keeps
 * the value of the previous port in that case.
 */
static f32 ifRemixComboGetXCoord(s32 port)
{
    s32 *positions = gIFCommonPlayerInterface.player_pos_x;

    if ((positions == NULL) || (positions[port] == 0))
    {
        return 0.0F;
    }
    return (f32)positions[port] + IFREMIX_COMBO_X_OFFSET;
}

/** Resets every meter for a new match. The colour and attribution fields are kept, as the asm keeps them. */
static void ifRemixComboInitMeters(void)
{
    s32 port;
    s32 k;
    f32 y_coord = ifRemixComboGetYCoord();

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        IFRemixComboMeter *meter = &sIFRemixComboMeters[port];

        sIFRemixComboSlots[port] = meter;
        meter->fp = NULL;
        meter->meter_gobj = NULL;
        meter->frame_buffer = 0;
        meter->x_coord = ifRemixComboGetXCoord(port);
        meter->y_coord = y_coord;
        meter->max_combo_hits = 0;
        meter->max_combo_damage = 0;
        for (k = 0; k < GMCOMMON_PLAYERS_MAX; k++)
        {
            meter->highest_combo_vs[k] = 0;
        }
    }
}

/** Texture set of every colour index: port colours, or team colours in a VS battle with teams on. */
static void ifRemixComboSetColorSets(void)
{
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        sIFRemixComboColorSets[port] = dIFRemixComboPortSets[port];
    }
    sIFRemixComboColorSets[IFREMIX_COMBO_COLOR_SILVER] = nIFRemixComboSetSilver;
    if ((gSCManagerSceneData.scene_curr != nSCKindVSBattle) || !gSCManagerBattleState->is_team_battle)
    {
        return;
    }
    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        u8 team = gSCManagerBattleState->players[port].team;

        if (team < GMCOMMON_PLAYERS_MAX)
        {
            sIFRemixComboColorSets[port] = dIFRemixComboTeamSets[team];
        }
    }
}

/** TRUE for a port with a man or CPU player (asm port_check: player type 0 or 1). */
static sb32 ifRemixComboIsPlayerPort(s32 port)
{
    return gSCManagerBattleState->players[port].pkind < IFREMIX_COMBO_PLAYER_MAX_PKIND;
}

/** Player struct of a port, or NULL when the port has no fighter. */
static FTStruct* ifRemixComboGetFighter(s32 port)
{
    GObj *fighter_gobj = gSCManagerBattleState->players[port].fighter_gobj;

    if (fighter_gobj == NULL)
    {
        return NULL;
    }
    return ftGetStruct(fighter_gobj);
}

/** 1v1 swap: each meter takes the screen position of the other player, and the slots are exchanged. */
static void ifRemixComboSwapSingles(const IFRemixComboPlayerSlot *first, const IFRemixComboPlayerSlot *second)
{
    second->meter->x_coord = first->x_coord;
    sIFRemixComboSlots[first->port] = second->meter;
    first->meter->x_coord = second->x_coord;
    sIFRemixComboSlots[second->port] = first->meter;
}

/**
 * Attaches the fighters to their meters and returns the number of man or CPU ports. The first two of them are stored in
 * slots for the 1v1 swap.
 */
static s32 ifRemixComboAttachPlayers(IFRemixComboPlayerSlot slots[2])
{
    s32 port;
    s32 player_count = 0;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        IFRemixComboMeter *meter = &sIFRemixComboMeters[port];

        if (!ifRemixComboIsPlayerPort(port))
        {
            continue;
        }
        meter->fp = ifRemixComboGetFighter(port);
        if (player_count < 2)
        {
            slots[player_count].x_coord = meter->x_coord;
            slots[player_count].meter = meter;
            slots[player_count].port = port;
        }
        player_count++;
    }
    return player_count;
}

/** Digit textures of a count, or IFREMIX_COMBO_NO_DIGIT for a digit that is not shown (asm draw_hit_count_). */
static void ifRemixComboPickDigits(s32 count, s32 digits[3])
{
    s32 hundreds = count / 100;
    s32 tens = (count / 10) % 10;
    s32 units = count % 10;

    if (hundreds != 0)
    {
        digits[0] = hundreds;
        digits[1] = tens;
        digits[2] = units;
        return;
    }
    if (tens != 0)
    {
        digits[0] = tens;
        digits[1] = units;
        digits[2] = IFREMIX_COMBO_NO_DIGIT;
        return;
    }
    digits[0] = units;
    digits[1] = IFREMIX_COMBO_NO_DIGIT;
    digits[2] = IFREMIX_COMBO_NO_DIGIT;
}

/**
 * Creates one SObj of the meter from a Remix texture sprite at the given position. The sprite header is copied by
 * lbCommonMakeSObjForGObj; its bitmap is set per frame.
 */
static SObj* ifRemixComboMakeSObj(GObj *gobj, Sprite *sprite, f32 x_coord, f32 y_coord)
{
    SObj *sobj = lbCommonMakeSObjForGObj(gobj, sprite);

    sobj->sprite.attr = IFREMIX_COMBO_SPRITE_ATTR;
    sobj->pos.x = x_coord;
    sobj->pos.y = y_coord;
    return sobj;
}

/** Creates the COMBO text and the three digits of a meter, in the order the draw code reads them. */
static void ifRemixComboMakeSObjs(GObj *gobj, const IFRemixComboMeter *meter)
{
    Sprite *text_sprite = lbRelocGetFileData(Sprite*, sIFRemixComboFile, IFREMIX_COMBO_TEXT_SPRITE_OFFSET);
    Sprite *number_sprite = lbRelocGetFileData(Sprite*, sIFRemixComboFile, IFREMIX_COMBO_NUMBER_SPRITE_OFFSET);
    f32 x_coord = meter->x_coord + IFREMIX_COMBO_NUMBERS_X_OFFSET;
    s32 k;

    ifRemixComboMakeSObj(gobj, text_sprite, meter->x_coord, meter->y_coord);
    for (k = 0; k < 3; k++)
    {
        ifRemixComboMakeSObj(gobj, number_sprite, x_coord, meter->y_coord);
        x_coord += IFREMIX_COMBO_NUMBER_X_STEP;
    }
}

static void ifRemixComboDrawMeter(IFRemixComboMeter *meter, s32 victim);

/** Per-frame process of the combo meters, registered once for all of them. */
static void ifRemixComboRun(GObj *run_gobj)
{
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        ifRemixComboDrawMeter(&sIFRemixComboMeters[port], port);
    }
}

/**
 * Creates the display GObj of a meter. Only the first meter gets the run process (asm s2), the rest pass NULL.
 * The GObj is linked to the pause queue by IFREMIX_COMBO_LINK_ID, which keeps the meter off the screen while paused.
 */
static void ifRemixComboMakeGObj(IFRemixComboMeter *meter, void (*func_run)(GObj*))
{
    GObj *gobj = gcMakeGObjSPAfter(IFREMIX_COMBO_GOBJ_ID, func_run, IFREMIX_COMBO_LINK_ID, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(gobj, lbCommonDrawSObjAttr, IFREMIX_COMBO_DL_LINK_ID, GOBJ_PRIORITY_DEFAULT, IFREMIX_COMBO_SHOW_CAMERA_TAG);
    ifRemixComboMakeSObjs(gobj, meter);
    meter->meter_gobj = gobj;
    gobj->flags = 1;
    gobj->camera_tag = 0;
}

/** Creates the GObjs of every port with a fighter. */
static void ifRemixComboMakeMeters(void)
{
    s32 port;
    sb32 has_run = FALSE;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        IFRemixComboMeter *meter = &sIFRemixComboMeters[port];

        if (meter->fp == NULL)
        {
            continue;
        }
        ifRemixComboMakeGObj(meter, has_run ? NULL : ifRemixComboRun);
        has_run = TRUE;
    }
}

/**
 * Texture token of a Remix bitmap: the 0x10-byte bitmap header lies before the texels, and the token is registered
 * because the port resolves SObj bitmaps through tokens.
 */
static u32 ifRemixComboBitmapToken(Sprite *template_sprite, s32 texture_offset)
{
    Bitmap *header = lbRelocGetFileData(Bitmap*, sIFRemixComboFile, texture_offset - IFREMIX_COMBO_BITMAP_HEADER);

    portFixupBitmapArray(header, 1);
    portFixupSpriteBitmapData(template_sprite, header);
    return portRelocRegisterPointer(header);
}

/** Registers the tokens of every COMBO text and digit texture (asm register_textures). Needs the sprites fixed first. */
static void ifRemixComboLoadTokens(void)
{
    Sprite *text_sprite = lbRelocGetFileData(Sprite*, sIFRemixComboFile, IFREMIX_COMBO_TEXT_SPRITE_OFFSET);
    Sprite *number_sprite = lbRelocGetFileData(Sprite*, sIFRemixComboFile, IFREMIX_COMBO_NUMBER_SPRITE_OFFSET);
    s32 set;
    s32 digit;

    for (set = 0; set < nIFRemixComboSetCount; set++)
    {
        sIFRemixComboTextTokens[set] = ifRemixComboBitmapToken(text_sprite, dIFRemixComboTextOffsets[set]);
        for (digit = 0; digit < IFREMIX_COMBO_DIGIT_COUNT; digit++)
        {
            sIFRemixComboNumberTokens[set][digit] = ifRemixComboBitmapToken(number_sprite, dIFRemixComboNumberOffsets[set] + digit * IFREMIX_COMBO_DIGIT_OFFSET);
        }
    }
}

/** Sets the bitmap of the three digit SObjs of the meter, starting at the first digit. */
static void ifRemixComboSetDigits(SObj *first_digit, s32 set, s32 count)
{
    s32 digits[3];
    SObj *sobj = first_digit;
    s32 k;

    if (count > 999)
    {
        count = 999;
    }
    ifRemixComboPickDigits(count, digits);
    for (k = 0; k < 3; k++)
    {
        sobj->sprite.bitmap = (digits[k] == IFREMIX_COMBO_NO_DIGIT) ? 0 : sIFRemixComboNumberTokens[set][digits[k]];
        sobj = sobj->next;
    }
}

/** Shows the meter with the count: text and digits from the colour set of the meter. */
static void ifRemixComboShowMeter(GObj *gobj, const IFRemixComboMeter *meter, s32 count)
{
    s32 set = sIFRemixComboColorSets[meter->color_display];
    SObj *text_sobj = SObjGetStruct(gobj);

    gobj->flags = 0;
    gobj->camera_tag = IFREMIX_COMBO_SHOW_CAMERA_TAG;
    text_sobj->sprite.bitmap = sIFRemixComboTextTokens[set];
    ifRemixComboSetDigits(text_sobj->next, set, count);
}

/** Draws one port's meter for the frame, as the asm draw_hit_count_ does for the port's struct. */
static void ifRemixComboDrawMeter(IFRemixComboMeter *meter, s32 victim)
{
    s32 count;

    if (meter->fp == NULL)
    {
        return;
    }
    meter->meter_gobj->flags = 1;
    meter->meter_gobj->camera_tag = 0;
    count = ifRemixComboHitsUpdate(sIFRemixComboMeters, victim, gSCManagerBattleState->players[victim].combo_count_foe, gSCManagerBattleState->players[victim].combo_damage_foe);
    if (count < 0)
    {
        return;
    }
    ifRemixComboShowMeter(meter->meter_gobj, meter, count);
}

/**
 * Setup of the combo meters for a match (asm ComboMeter.setup_, called from the scene load). Does nothing when the
 * setting is off or the scene is not a VS or 1P battle.
 */
void ifRemixComboMeterSetup(void)
{
    IFRemixComboPlayerSlot slots[2];
    s32 player_count;

    if (!ifRemixComboIsSetupScene() || !gmRemixToggle(combo_meter))
    {
        return;
    }
    sIFRemixComboFile = lbRelocGetExternHeapFile(IFREMIX_COMBO_FILE_ID, syTaskmanMalloc(lbRelocGetFileSize(IFREMIX_COMBO_FILE_ID), 0x10));
    ifRemixComboInitMeters();
    ifRemixComboSetColorSets();
    player_count = ifRemixComboAttachPlayers(slots);
    if ((player_count == 2) && gmRemixToggle(1v1_combo_meter_swap))
    {
        ifRemixComboSwapSingles(&slots[0], &slots[1]);
    }
    ifRemixComboMakeMeters();
    ifRemixComboLoadTokens();
}

/**
 * Hides the meter of a port that shows a SCORE +/-1 sprite (asm ComboMeter.hide_for_score_sprite_): clears the frame
 * buffer of its slot in a VS battle.
 */
void ifRemixComboMeterHideForScore(s32 port)
{
    IFRemixComboMeter *meter = sIFRemixComboSlots[port];

    if (!gmRemixToggle(combo_meter) || (gSCManagerSceneData.scene_curr != nSCKindVSBattle))
    {
        return;
    }
    if (meter != NULL)
    {
        meter->frame_buffer = 0;
    }
}

#endif
