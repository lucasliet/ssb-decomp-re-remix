/**
 * ifremixinputdisplay.c - sprites of the Remix Input Display, see ifremixinputdisplay.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include <gm/gmremixpanel.h>
#include "if/ifcommon.h"
#include "if/ifremixdragonkinghud.h"
#include "if/ifremixinputdisplay.h"
#include "if/ifremixinputdisplayrules.h"
#include "if/ifremixsprite.h"
#include "sc/sc1pmode/sc1premixmode.h"

#define IFREMIX_INPUTDISPLAY_GROUP 0x0B
#define IFREMIX_INPUTDISPLAY_DL_LINK 23
#define IFREMIX_INPUTDISPLAY_PORTS 4
#define IFREMIX_INPUTDISPLAY_PANEL_BOTTOM 2

#define IFREMIX_INPUTDISPLAY_BUTTON_A 0x8000
#define IFREMIX_INPUTDISPLAY_BUTTON_B 0x4000
#define IFREMIX_INPUTDISPLAY_BUTTON_Z 0x2000
#define IFREMIX_INPUTDISPLAY_BUTTON_DU 0x0800
#define IFREMIX_INPUTDISPLAY_BUTTON_DD 0x0400
#define IFREMIX_INPUTDISPLAY_BUTTON_DL 0x0200
#define IFREMIX_INPUTDISPLAY_BUTTON_DR 0x0100
#define IFREMIX_INPUTDISPLAY_BUTTON_L 0x0020
#define IFREMIX_INPUTDISPLAY_BUTTON_R 0x0010
#define IFREMIX_INPUTDISPLAY_BUTTON_CU 0x0008
#define IFREMIX_INPUTDISPLAY_BUTTON_CD 0x0004
#define IFREMIX_INPUTDISPLAY_BUTTON_CL 0x0002
#define IFREMIX_INPUTDISPLAY_BUTTON_CR 0x0001

#define IFREMIX_INPUTDISPLAY_GRAY_OFF 0x8484844FU
#define IFREMIX_INPUTDISPLAY_GRAY_ON 0x848484FFU
#define IFREMIX_INPUTDISPLAY_GRAY_PALETTE 0x303030FFU
#define IFREMIX_INPUTDISPLAY_C_OFF 0xC0CC004FU
#define IFREMIX_INPUTDISPLAY_C_ON 0xC0CC00FFU

/* One sprite of the block: X offset from the series logo of the percent HUD, Y at the top and at the bottom edge. */
typedef struct IFRemixInputDisplayButton
{
    s32 sprite_id;
    f32 offset_x;
    f32 top_y;
    f32 bottom_y;
    f32 scale;
    u32 off_tint;
    u32 on_tint;
    u32 palette;
    u16 mask;

} IFRemixInputDisplayButton;

/* create_input_display_objects_: the order of the objects (L is the control object), their hex float arguments decoded. */
static const IFRemixInputDisplayButton sIFRemixInputDisplayButtons[] =
{
    { IFREMIX_SPRITE_BUTTON_L, -18.0F, 21.0F, 162.0F, 0.625F, IFREMIX_INPUTDISPLAY_GRAY_OFF, IFREMIX_INPUTDISPLAY_GRAY_ON, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, IFREMIX_INPUTDISPLAY_BUTTON_L },
    { IFREMIX_SPRITE_BUTTON_R, -6.0F, 21.0F, 162.0F, 0.625F, IFREMIX_INPUTDISPLAY_GRAY_OFF, IFREMIX_INPUTDISPLAY_GRAY_ON, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, IFREMIX_INPUTDISPLAY_BUTTON_R },
    { IFREMIX_SPRITE_BUTTON_A, 22.0F, 33.0F, 174.0F, 0.625F, 0x50A8FF4FU, 0x50A8FFFFU, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, IFREMIX_INPUTDISPLAY_BUTTON_A },
    { IFREMIX_SPRITE_BUTTON_B, 16.0F, 27.0F, 168.0F, 0.625F, 0x00D0404FU, 0x00D040FFU, 0x003000FFU, IFREMIX_INPUTDISPLAY_BUTTON_B },
    { IFREMIX_SPRITE_BUTTON_Z, -6.0F, 30.0F, 171.0F, 0.625F, IFREMIX_INPUTDISPLAY_GRAY_OFF, IFREMIX_INPUTDISPLAY_GRAY_ON, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, IFREMIX_INPUTDISPLAY_BUTTON_Z },
    { IFREMIX_SPRITE_C_ARROW_A, 27.0F, 20.0F, 161.0F, 0.5F, IFREMIX_INPUTDISPLAY_C_OFF, IFREMIX_INPUTDISPLAY_C_ON, 0x000000FFU, IFREMIX_INPUTDISPLAY_BUTTON_CU },
    { IFREMIX_SPRITE_C_ARROW_B, 27.0F, 28.0F, 169.0F, 0.5F, IFREMIX_INPUTDISPLAY_C_OFF, IFREMIX_INPUTDISPLAY_C_ON, 0x000000FFU, IFREMIX_INPUTDISPLAY_BUTTON_CD },
    { IFREMIX_SPRITE_C_ARROW_C, 23.0F, 24.0F, 165.0F, 0.5F, IFREMIX_INPUTDISPLAY_C_OFF, IFREMIX_INPUTDISPLAY_C_ON, 0x000000FFU, IFREMIX_INPUTDISPLAY_BUTTON_CL },
    { IFREMIX_SPRITE_C_ARROW_D, 31.0F, 24.0F, 165.0F, 0.5F, IFREMIX_INPUTDISPLAY_C_OFF, IFREMIX_INPUTDISPLAY_C_ON, 0x000000FFU, IFREMIX_INPUTDISPLAY_BUTTON_CR },
    { IFREMIX_SPRITE_INPUT_DPAD, -18.0F, 29.0F, 170.0F, 0.625F, IFREMIX_INPUTDISPLAY_GRAY_OFF, IFREMIX_INPUTDISPLAY_GRAY_OFF, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, 0 },
    { IFREMIX_SPRITE_C_ARROW_A, -15.1875F, 29.25F, 170.25F, 0.328125F, 0x00000000U, IFREMIX_INPUTDISPLAY_GRAY_ON, 0x00000000U, IFREMIX_INPUTDISPLAY_BUTTON_DU },
    { IFREMIX_SPRITE_C_ARROW_B, -15.1875F, 34.5F, 175.5F, 0.328125F, 0x00000000U, IFREMIX_INPUTDISPLAY_GRAY_ON, 0x00000000U, IFREMIX_INPUTDISPLAY_BUTTON_DD },
    { IFREMIX_SPRITE_C_ARROW_C, -17.875F, 31.875F, 172.875F, 0.328125F, 0x00000000U, IFREMIX_INPUTDISPLAY_GRAY_ON, 0x00000000U, IFREMIX_INPUTDISPLAY_BUTTON_DL },
    { IFREMIX_SPRITE_C_ARROW_D, -12.5F, 31.875F, 172.875F, 0.328125F, 0x00000000U, IFREMIX_INPUTDISPLAY_GRAY_ON, 0x00000000U, IFREMIX_INPUTDISPLAY_BUTTON_DR }
};

#define IFREMIX_INPUTDISPLAY_BUTTON_COUNT ((s32)(sizeof(sIFRemixInputDisplayButtons) / sizeof(sIFRemixInputDisplayButtons[0])))

/* Joystick gate and joystick (the last two objects): X offset, Y at the top and the bottom edge, scale, colour, palette. */
#define IFREMIX_INPUTDISPLAY_GATE_X 1.125F
#define IFREMIX_INPUTDISPLAY_GATE_TOP_Y 25.75F
#define IFREMIX_INPUTDISPLAY_GATE_BOTTOM_Y 166.75F
#define IFREMIX_INPUTDISPLAY_GATE_SCALE 0.5F
#define IFREMIX_INPUTDISPLAY_STICK_X 5.0F
#define IFREMIX_INPUTDISPLAY_STICK_TOP_Y 30.75F
#define IFREMIX_INPUTDISPLAY_STICK_BOTTOM_Y 171.75F
#define IFREMIX_INPUTDISPLAY_STICK_SCALE 0.53125F
#define IFREMIX_INPUTDISPLAY_STICK_TINT 0xD4D4D4FFU

typedef struct IFRemixInputDisplayBlock
{
    sb32 is_made;
    SObj *buttons[sizeof(sIFRemixInputDisplayButtons) / sizeof(sIFRemixInputDisplayButtons[0])];
    SObj *gate;
    SObj *stick;
    f32 stick_center_x;
    f32 stick_center_y;
    s32 cooldown;

} IFRemixInputDisplayBlock;

static IFRemixInputDisplayBlock sIFRemixInputDisplayBlocks[IFREMIX_INPUTDISPLAY_PORTS];

/** Screen class of the running scene for the placement rules. */
static IFRemixInputDisplayScreen ifRemixInputDisplayGetScreen(void)
{
    switch (gSCManagerSceneData.scene_curr)
    {
    case nSCKindVSBattle:
        return IFREMIX_INPUTDISPLAY_SCREEN_VS;

    case nSCKind1PTrainingMode:
        return IFREMIX_INPUTDISPLAY_SCREEN_TRAINING;

    case nSCKind1PBonusStage:
        return IFREMIX_INPUTDISPLAY_SCREEN_BONUS;

    default:
        return IFREMIX_INPUTDISPLAY_SCREEN_OTHER;
    }
}

/** Single player mode class; the mode flag only matters in a 1P game run. */
static IFRemixInputDisplayMode ifRemixInputDisplayGetMode(void)
{
    if (gSCManagerSceneData.scene_curr != nSCKind1PGame)
    {
        return IFREMIX_INPUTDISPLAY_MODE_OTHER;
    }
    switch (sc1PRemixModeGet())
    {
    case SC1PREMIX_MODE_MULTIMAN:
    case SC1PREMIX_MODE_CRUEL:
        return IFREMIX_INPUTDISPLAY_MODE_MULTIMAN;

    case SC1PREMIX_MODE_ALLSTAR:
        return IFREMIX_INPUTDISPLAY_MODE_ALLSTAR;

    case SC1PREMIX_MODE_STANDARD:
    case SC1PREMIX_MODE_REMIX_1P:
        return IFREMIX_INPUTDISPLAY_MODE_SINGLE;

    default:
        return IFREMIX_INPUTDISPLAY_MODE_OTHER;
    }
}

/** Applies a 0xRRGGBBAA colour and palette to a sprite. */
static void ifRemixInputDisplaySetColors(SObj *sobj, u32 tint, u32 palette)
{
    u8 alpha;

    ifRemixSpriteRulesUnpackColor(tint, &sobj->sprite.red, &sobj->sprite.green, &sobj->sprite.blue, &sobj->sprite.alpha);
    ifRemixSpriteRulesUnpackColor(palette, &sobj->envcolor.r, &sobj->envcolor.g, &sobj->envcolor.b, &alpha);
    sobj->envcolor.a = alpha;
}

/** Makes one sprite of the block and returns its SObj, or NULL when the sprite file is missing. */
static SObj* ifRemixInputDisplayMakeSprite(s32 sprite_id, f32 x, f32 y, u32 tint, u32 palette, f32 scale)
{
    GObj *gobj = ifRemixSpriteMake(sprite_id, IFREMIX_INPUTDISPLAY_GROUP, IFREMIX_INPUTDISPLAY_DL_LINK, x, y, tint, palette, scale);

    return (gobj != NULL) ? SObjGetStruct(gobj) : NULL;
}

/** Makes the block of one port at the series logo X and the layout of the match. */
static void ifRemixInputDisplayMakeBlock(s32 player, f32 logo_x, const IFRemixInputDisplayLayout *layout)
{
    IFRemixInputDisplayBlock *block = &sIFRemixInputDisplayBlocks[player];
    s32 index;

    for (index = 0; index < IFREMIX_INPUTDISPLAY_BUTTON_COUNT; index++)
    {
        const IFRemixInputDisplayButton *button = &sIFRemixInputDisplayButtons[index];
        f32 y = ((layout->is_bottom != 0) ? button->bottom_y : button->top_y) + layout->offset_y;

        block->buttons[index] = ifRemixInputDisplayMakeSprite(button->sprite_id, logo_x + button->offset_x, y, button->off_tint,
                                                              button->palette, button->scale);
    }
    block->gate = ifRemixInputDisplayMakeSprite(IFREMIX_SPRITE_INPUT_GATE, logo_x + IFREMIX_INPUTDISPLAY_GATE_X,
                                                ((layout->is_bottom != 0) ? IFREMIX_INPUTDISPLAY_GATE_BOTTOM_Y : IFREMIX_INPUTDISPLAY_GATE_TOP_Y) + layout->offset_y,
                                                IFREMIX_INPUTDISPLAY_GRAY_OFF, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, IFREMIX_INPUTDISPLAY_GATE_SCALE);
    block->stick_center_x = logo_x + IFREMIX_INPUTDISPLAY_STICK_X;
    block->stick_center_y = ((layout->is_bottom != 0) ? IFREMIX_INPUTDISPLAY_STICK_BOTTOM_Y : IFREMIX_INPUTDISPLAY_STICK_TOP_Y) + layout->offset_y;
    block->stick = ifRemixInputDisplayMakeSprite(IFREMIX_SPRITE_INPUT_STICK, block->stick_center_x, block->stick_center_y,
                                                 IFREMIX_INPUTDISPLAY_STICK_TINT, IFREMIX_INPUTDISPLAY_GRAY_PALETTE, IFREMIX_INPUTDISPLAY_STICK_SCALE);
    block->cooldown = 0;
    block->is_made = TRUE;
}

void ifRemixInputDisplayClear(void)
{
    s32 player;

    for (player = 0; player < IFREMIX_INPUTDISPLAY_PORTS; player++)
    {
        sIFRemixInputDisplayBlocks[player].is_made = FALSE;
    }
}

void ifRemixInputDisplayMake(void)
{
    IFRemixInputDisplayLayoutInput input;
    IFRemixInputDisplayLayout layout;
    s32 player;

    ifRemixInputDisplayClear();
    input.screen = ifRemixInputDisplayGetScreen();
    input.mode = ifRemixInputDisplayGetMode();
    input.stage = gSCManagerSceneData.spgame_stage;
    input.is_dragon_king_hud = (ifRemixDragonKingHudIsOn() != FALSE) ? 1 : 0;

    for (player = 0; player < IFREMIX_INPUTDISPLAY_PORTS; player++)
    {
        s32 panel_value = gmRemixPanel(INPUT_DISPLAY, player);
        f32 logo_x;

        if ((panel_value == 0) || (ifCommonPlayerDamageGetLogoX(player, &logo_x) == FALSE))
        {
            continue;
        }
        input.is_bottom_requested = (panel_value == IFREMIX_INPUTDISPLAY_PANEL_BOTTOM) ? 1 : 0;
        ifRemixInputDisplayRulesResolveLayout(&input, &layout);
        ifRemixInputDisplayMakeBlock(player, logo_x, &layout);
    }
}

/** Lights or dims every button sprite of a block for the pressed mask. */
static void ifRemixInputDisplayApplyButtons(IFRemixInputDisplayBlock *block, u16 buttons)
{
    s32 index;

    for (index = 0; index < IFREMIX_INPUTDISPLAY_BUTTON_COUNT; index++)
    {
        const IFRemixInputDisplayButton *button = &sIFRemixInputDisplayButtons[index];

        if ((block->buttons[index] == NULL) || (button->mask == 0))
        {
            continue;
        }
        ifRemixInputDisplaySetColors(block->buttons[index], ((buttons & button->mask) != 0) ? button->on_tint : button->off_tint, button->palette);
    }
}

/** Moves the joystick sprite and sets the alpha of the stick and of its gate. */
static void ifRemixInputDisplayApplyStick(IFRemixInputDisplayBlock *block, s32 stick_x, s32 stick_y)
{
    IFRemixInputDisplayStick stick;

    if ((block->stick == NULL) || (block->gate == NULL))
    {
        return;
    }
    ifRemixInputDisplayRulesUpdateStick(stick_x, stick_y, block->cooldown, &stick);

    block->stick->pos.x = block->stick_center_x + stick.offset_x;
    block->stick->pos.y = block->stick_center_y + stick.offset_y;
    block->stick->sprite.alpha = stick.stick_alpha;
    block->gate->sprite.alpha = stick.gate_alpha;
    block->cooldown = stick.cooldown;
}

void ifRemixInputDisplayApply(s32 player, u16 buttons, s32 stick_x, s32 stick_y)
{
    IFRemixInputDisplayBlock *block;

    if ((player < 0) || (player >= IFREMIX_INPUTDISPLAY_PORTS))
    {
        return;
    }
    block = &sIFRemixInputDisplayBlocks[player];

    if (block->is_made == FALSE)
    {
        return;
    }
    ifRemixInputDisplayApplyButtons(block, buttons);
    ifRemixInputDisplayApplyStick(block, stick_x, stick_y);
}

#endif
