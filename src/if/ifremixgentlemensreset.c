/**
 * ifremixgentlemensreset.c - Gentlemen's Reset request of the 12-Character Battle pause, see ifremixgentlemensreset.h.
 *
 * Layout from TwelveCharBattle.asm 3580-3610: two centred lines, "Accept:" with the reset combo drawn by the pause decal
 * sprites moved by (-4, -39), "Decline:" and the START button. Sprites join the Remix group 0xE of the pause legends.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/objman.h>
#include "gm/gmremixtwelvecb.h"
#include "gm/gmremixtwelvecbrules.h"
#include "gm/gmremixgroupdisplay.h"
#include "if/ifremixgentlemensreset.h"
#include "if/ifremixpauselegend.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"

#define IFREMIX_GENTLEMENS_DL_LINK 24
#define IFREMIX_GENTLEMENS_GROUP 0x0E
#define IFREMIX_GENTLEMENS_SPRITES_MAX 8

/* remix_hud_text slots 19 to 22: free in battle (see the slot map in remix_hud_text.h) */
#define IFREMIX_GENTLEMENS_SLOT_LINE_1 19
#define IFREMIX_GENTLEMENS_SLOT_LINE_2 20
#define IFREMIX_GENTLEMENS_SLOT_ACCEPT 21
#define IFREMIX_GENTLEMENS_SLOT_DECLINE 22
#define IFREMIX_GENTLEMENS_COLOR 0xFFFFFFFF
#define IFREMIX_GENTLEMENS_CENTER_X 160.0F
#define IFREMIX_GENTLEMENS_LINE_1_Y 126.0F
#define IFREMIX_GENTLEMENS_LINE_2_Y 144.0F
#define IFREMIX_GENTLEMENS_ACCEPT_X 40.0F
#define IFREMIX_GENTLEMENS_DECLINE_X 188.0F
#define IFREMIX_GENTLEMENS_BUTTONS_Y 164.0F
#define IFREMIX_GENTLEMENS_START_X 244.0F
#define IFREMIX_GENTLEMENS_START_TINT 0xFF0000FF
#define IFREMIX_GENTLEMENS_START_PALETTE 0x303030FF
#define IFREMIX_GENTLEMENS_REQUESTER_INDEX 24

typedef struct IFRemixGentlemensSprite
{
    s32 sprite_id;
    f32 x;
    f32 y;
    u32 tint;
    u32 palette;
} IFRemixGentlemensSprite;

/* The pause decals 1 to 7 (dIFCommonBattlePauseDecalsSpriteData: A, B, Z, R and three plus signs) at their position of the pause
 * line, moved by (-4, -39) as the loop of the asm does after each native call. */
static const IFRemixGentlemensSprite sIFRemixGentlemensComboSprites[] =
{
    { IFREMIX_SPRITE_BUTTON_A, 95.0F, 164.0F, 0x0095FFFF, 0x0005C7FF },
    { IFREMIX_SPRITE_BUTTON_B, 118.0F, 164.0F, 0x36BF00FF, 0x003000FF },
    { IFREMIX_SPRITE_BUTTON_Z, 141.0F, 163.0F, 0x808080FF, 0x212121FF },
    { IFREMIX_SPRITE_BUTTON_R, 160.0F, 164.0F, 0x808080FF, 0x212121FF },
    { IFREMIX_SPRITE_BUTTON_PLUS, 109.0F, 167.0F, 0xFFFFFFFF, 0x000000FF },
    { IFREMIX_SPRITE_BUTTON_PLUS, 132.0F, 167.0F, 0xFFFFFFFF, 0x000000FF },
    { IFREMIX_SPRITE_BUTTON_PLUS, 151.0F, 167.0F, 0xFFFFFFFF, 0x000000FF }
};

static sb32 sIFRemixGentlemensIsRequested;
static s32 sIFRemixGentlemensRequester;
static GObj *sIFRemixGentlemensSprites[IFREMIX_GENTLEMENS_SPRITES_MAX];
static s32 sIFRemixGentlemensSpriteCount;

static void ifRemixGentlemensMakeSprite(s32 sprite_id, f32 x, f32 y, u32 tint, u32 palette)
{
    GObj *gobj = ifRemixSpriteMake(sprite_id, IFREMIX_GENTLEMENS_GROUP, IFREMIX_GENTLEMENS_DL_LINK, x, y, tint, palette, 1.0F);

    if ((gobj != NULL) && (sIFRemixGentlemensSpriteCount < IFREMIX_GENTLEMENS_SPRITES_MAX))
    {
        sIFRemixGentlemensSprites[sIFRemixGentlemensSpriteCount++] = gobj;
    }
}

static void ifRemixGentlemensMakeSprites(void)
{
    s32 index;

    for (index = 0; index < (s32)(sizeof(sIFRemixGentlemensComboSprites) / sizeof(sIFRemixGentlemensComboSprites[0])); index++)
    {
        const IFRemixGentlemensSprite *sprite = &sIFRemixGentlemensComboSprites[index];

        ifRemixGentlemensMakeSprite(sprite->sprite_id, sprite->x, sprite->y, sprite->tint, sprite->palette);
    }
    ifRemixGentlemensMakeSprite(IFREMIX_SPRITE_START_BUTTON, IFREMIX_GENTLEMENS_START_X, IFREMIX_GENTLEMENS_BUTTONS_Y,
                                IFREMIX_GENTLEMENS_START_TINT, IFREMIX_GENTLEMENS_START_PALETTE);
}

static void ifRemixGentlemensEjectSprites(void)
{
    s32 index;

    for (index = 0; index < sIFRemixGentlemensSpriteCount; index++)
    {
        gcEjectGObj(sIFRemixGentlemensSprites[index]);
        sIFRemixGentlemensSprites[index] = NULL;
    }
    sIFRemixGentlemensSpriteCount = 0;
}

static s32 ifRemixGentlemensGetControllerPort(s32 pausing_port)
{
    s32 opponent = pausing_port ^ 1;
    sb32 opponent_is_human = (gSCManagerBattleState->players[opponent].pkind == nFTPlayerKindMan) ? TRUE : FALSE;

    return gmRemixTwelveCBResetControllerPort(pausing_port, opponent_is_human);
}

sb32 ifRemixGentlemensResetHandleCombo(u8 *pause_player)
{
    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return FALSE;
    }
    if (sIFRemixGentlemensIsRequested != FALSE)
    {
        ifRemixGentlemensResetEnd();
        gmRemixTwelveCBUndoCurrentGame();
        return FALSE;
    }
    sIFRemixGentlemensIsRequested = TRUE;
    sIFRemixGentlemensRequester = *pause_player;
    ifRemixGentlemensMakeSprites();
    ifRemixPauseLegendHideMusicAndCamera();
    *pause_player = (u8)ifRemixGentlemensGetControllerPort(*pause_player);
    return TRUE;
}

void ifRemixGentlemensResetUpdate(void)
{
    char line_1[] = "Game reset requested by  P.";

    if (sIFRemixGentlemensIsRequested == FALSE)
    {
        return;
    }
    line_1[IFREMIX_GENTLEMENS_REQUESTER_INDEX] = gmRemixTwelveCBResetRequesterDigit(sIFRemixGentlemensRequester);
    remix_hud_text_put(IFREMIX_GENTLEMENS_SLOT_LINE_1, line_1, IFREMIX_GENTLEMENS_CENTER_X, IFREMIX_GENTLEMENS_LINE_1_Y,
                       IFREMIX_GENTLEMENS_COLOR, 1.0F, REMIX_HUD_ALIGN_CENTER);
    remix_hud_text_put(IFREMIX_GENTLEMENS_SLOT_LINE_2, "If accepted, stock counts will be reset.", IFREMIX_GENTLEMENS_CENTER_X,
                       IFREMIX_GENTLEMENS_LINE_2_Y, IFREMIX_GENTLEMENS_COLOR, 1.0F, REMIX_HUD_ALIGN_CENTER);
    remix_hud_text_put(IFREMIX_GENTLEMENS_SLOT_ACCEPT, "Accept:", IFREMIX_GENTLEMENS_ACCEPT_X, IFREMIX_GENTLEMENS_BUTTONS_Y,
                       IFREMIX_GENTLEMENS_COLOR, 1.0F, REMIX_HUD_ALIGN_LEFT);
    remix_hud_text_put(IFREMIX_GENTLEMENS_SLOT_DECLINE, "Decline:", IFREMIX_GENTLEMENS_DECLINE_X, IFREMIX_GENTLEMENS_BUTTONS_Y,
                       IFREMIX_GENTLEMENS_COLOR, 1.0F, REMIX_HUD_ALIGN_LEFT);
}

sb32 ifRemixGentlemensResetIsRequested(void)
{
    return sIFRemixGentlemensIsRequested;
}

void ifRemixGentlemensResetEnd(void)
{
    sIFRemixGentlemensIsRequested = FALSE;
    ifRemixGentlemensEjectSprites();
    remix_hud_text_hide(IFREMIX_GENTLEMENS_SLOT_LINE_1);
    remix_hud_text_hide(IFREMIX_GENTLEMENS_SLOT_LINE_2);
    remix_hud_text_hide(IFREMIX_GENTLEMENS_SLOT_ACCEPT);
    remix_hud_text_hide(IFREMIX_GENTLEMENS_SLOT_DECLINE);
}

#endif
