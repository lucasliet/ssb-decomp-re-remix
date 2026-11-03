/**
 * ifremixpauselegend.c - Pause legends of Smash Remix (Pause.asm show_legend_helper_, check_toggle_hud_press_,
 * check_legend_toggle_press_ and disable_hud_, Render groups 0xE, 0xF and 0x10).
 *
 * The glyphs (buttons, D-pad, C stick arrows, the "Legend" word) are native sprites on the pause display link, one GObj each in
 * the Remix group (GObj link) they belong to, so the group display of I1 shows and hides them as the asm does. The text is
 * Kabel text of the port overlay, put every frame its group is visible. The sprites are made when a pause starts and ejected
 * when it ends. Reference: the real Remix pause in the reference emulator (docs/remix/notes/i8_reference_emulator.md).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <PR/os.h>
#include <ssb_types.h>
#include <mp/map.h>
#include <sys/objman.h>
#include "gm/gmremixgroupdisplay.h"
#include "gm/gmremixtoggle.h"
#include "if/ifremixpauselegend.h"
#include "if/ifremixsprite.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_music_title.h"

/* Pause.asm: the display link (room 0x18) of the pause menu, and the Render groups the legends use as GObj links. */
#define IFREMIX_PAUSE_LEGEND_DL_LINK 24
#define IFREMIX_PAUSE_LEGEND_GROUP_R 0x0E
#define IFREMIX_PAUSE_LEGEND_GROUP_MUSIC 0x0F
#define IFREMIX_PAUSE_LEGEND_GROUP_CAMERA 0x10

/* remix_hud_text slots 51 to 70: free in battle (see the slot map in remix_hud_text.h) */
#define IFREMIX_PAUSE_LEGEND_SLOT_FIRST 51
#define IFREMIX_PAUSE_LEGEND_SLOT_TITLE 69
#define IFREMIX_PAUSE_LEGEND_SLOT_GAME 70
#define IFREMIX_PAUSE_LEGEND_COLOR 0xFFFFFFFF
#define IFREMIX_PAUSE_LEGEND_GAME_SCALE 0.75F
#define IFREMIX_PAUSE_LEGEND_TITLE_MAX_WIDTH 228.0F
#define IFREMIX_PAUSE_LEGEND_TITLE_X 52.0F
#define IFREMIX_PAUSE_LEGEND_TITLE_Y 74.0F
#define IFREMIX_PAUSE_LEGEND_GAME_Y 90.0F
#define IFREMIX_PAUSE_LEGEND_ITEMS_MAX 40

typedef enum IFRemixPauseLegendKind
{
    IFREMIX_PAUSE_LEGEND_SPRITE,
    IFREMIX_PAUSE_LEGEND_TEXT,
    IFREMIX_PAUSE_LEGEND_RECT
} IFRemixPauseLegendKind;

typedef struct IFRemixPauseLegendItem
{
    IFRemixPauseLegendKind kind;
    s32 group;
    s32 sprite_id;      /* sprite items */
    const char *text;   /* text items */
    f32 x;
    f32 y;
    f32 scale;          /* sprite and text items */
    u32 tint;           /* sprite items */
    u32 palette;        /* sprite items */
    f32 width;          /* rect items */
    f32 height;
    u32 rgba;
} IFRemixPauseLegendItem;

#define SPRITE(group, id, x, y, scale, tint, palette) { IFREMIX_PAUSE_LEGEND_SPRITE, group, id, NULL, x, y, scale, tint, palette, 0.0F, 0.0F, 0 }
#define TEXT(group, text, x, y) { IFREMIX_PAUSE_LEGEND_TEXT, group, 0, text, x, y, 1.0F, 0, 0, 0.0F, 0.0F, 0 }
#define RECT(group, x, y, rgba) { IFREMIX_PAUSE_LEGEND_RECT, group, 0, NULL, x, y, 1.0F, 0, 0, 2.0F, 2.0F, rgba }

#define COLOR_BUTTON_A 0x50A8FFFF
#define PALETTE_BUTTON_A 0x303030FF
#define COLOR_BUTTON_B 0x00D040FF
#define PALETTE_BUTTON_B 0x003000FF
#define COLOR_GREY 0x848484FF
#define PALETTE_GREY 0x303030FF
#define COLOR_C_STICK 0xC0CC00FF
#define PALETTE_C_STICK 0x000000FF
#define COLOR_YELLOW 0xFFFF00FF

/* Pause.asm 181-182 (R legend) and 279-340 (music and camera legends): Render.draw_texture_at_offset, draw_string and
 * draw_rectangle arguments, decoded to screen units. The track title and the game of the music block are live strings, put
 * apart. */
static const IFRemixPauseLegendItem sIFRemixPauseLegendItems[] =
{
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_R, IFREMIX_SPRITE_BUTTON_R, 226.0F, 203.0F, 1.0F, 0x808080FF, 0x212121FF),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_R, IFREMIX_SPRITE_LEGEND_WORD, 243.0F, 205.0F, 1.0F, 0xFFFFFFFF, 0x00000000),

    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, "\x80", 40.0F, 74.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, IFREMIX_SPRITE_DPAD, 40.0F, 105.0F, 1.0F, COLOR_GREY, PALETTE_GREY),
    RECT(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, 51.0F, 113.0F, COLOR_YELLOW),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, ": Next Music Track", 60.0F, 106.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, IFREMIX_SPRITE_DPAD, 40.0F, 121.0F, 1.0F, COLOR_GREY, PALETTE_GREY),
    RECT(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, 47.0F, 132.0F, COLOR_YELLOW),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, ": Random Music Track", 60.0F, 122.0F),

    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_BUTTON_A, 40.0F, 140.0F, 0.9375F, COLOR_BUTTON_A, PALETTE_BUTTON_A),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, "/", 52.0F, 140.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_BUTTON_B, 58.0F, 140.0F, 0.9375F, COLOR_BUTTON_B, PALETTE_BUTTON_B),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, ": Zoom Camera", 72.0F, 140.0F),

    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_C_ARROW_A, 40.0F, 156.0F, 0.75F, COLOR_C_STICK, PALETTE_C_STICK),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, "/", 50.0F, 154.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_C_ARROW_B, 54.0F, 156.0F, 0.75F, COLOR_C_STICK, PALETTE_C_STICK),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, "/", 64.0F, 154.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_C_ARROW_C, 68.0F, 156.0F, 0.75F, COLOR_C_STICK, PALETTE_C_STICK),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, "/", 78.0F, 154.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_C_ARROW_D, 82.0F, 156.0F, 0.75F, COLOR_C_STICK, PALETTE_C_STICK),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, ": Pan Camera", 94.0F, 154.0F),

    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_BUTTON_Z, 41.0F, 170.0F, 1.0F, COLOR_GREY, PALETTE_GREY),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_BUTTON_PLUS, 53.0F, 173.0F, 0.9375F, 0xFFFFFFFF, PALETTE_GREY),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_BUTTON_A, 62.0F, 170.0F, 0.9375F, COLOR_BUTTON_A, PALETTE_BUTTON_A),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, "/", 74.0F, 170.0F),
    SPRITE(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, IFREMIX_SPRITE_BUTTON_B, 80.0F, 170.0F, 0.9375F, COLOR_BUTTON_B, PALETTE_BUTTON_B),
    TEXT(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, ": Field of View", 94.0F, 170.0F),
};

#define IFREMIX_PAUSE_LEGEND_ITEM_COUNT ((s32)(sizeof(sIFRemixPauseLegendItems) / sizeof(sIFRemixPauseLegendItems[0])))

/* Pause.asm groups 0xE (R legend) and 0xF and 0x10 (music and camera legends): their show state */
static sb32 sIFRemixPauseLegendHudShown;
static sb32 sIFRemixPauseLegendExtraShown;
static GObj *sIFRemixPauseLegendSprites[IFREMIX_PAUSE_LEGEND_ITEMS_MAX];
static s32 sIFRemixPauseLegendSpriteCount;

static void ifRemixPauseLegendMakeSprites(void)
{
    s32 index;

    sIFRemixPauseLegendSpriteCount = 0;

    for (index = 0; index < IFREMIX_PAUSE_LEGEND_ITEM_COUNT; index++)
    {
        const IFRemixPauseLegendItem *item = &sIFRemixPauseLegendItems[index];
        GObj *gobj;

        if (item->kind != IFREMIX_PAUSE_LEGEND_SPRITE)
        {
            continue;
        }
        gobj = ifRemixSpriteMake(item->sprite_id, item->group, IFREMIX_PAUSE_LEGEND_DL_LINK, item->x, item->y, item->tint,
                                 item->palette, item->scale);
        sIFRemixPauseLegendSprites[sIFRemixPauseLegendSpriteCount++] = gobj;
    }
}

/** Shows and hides the sprite groups of the current state (Render.toggle_group_display_ of the asm). */
static void ifRemixPauseLegendApplyGroups(void)
{
    sb32 hud_visible = (gmRemixToggle(disable_hud) == 0) && sIFRemixPauseLegendHudShown;

    gmRemixGroupDisplaySet(IFREMIX_PAUSE_LEGEND_GROUP_R, hud_visible ? 0 : 1);
    gmRemixGroupDisplaySet(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC, (hud_visible && sIFRemixPauseLegendExtraShown) ? 0 : 1);
    gmRemixGroupDisplaySet(IFREMIX_PAUSE_LEGEND_GROUP_CAMERA, (hud_visible && sIFRemixPauseLegendExtraShown) ? 0 : 1);
}

void ifRemixPauseLegendReset(void)
{
    sIFRemixPauseLegendHudShown = TRUE;
    sIFRemixPauseLegendExtraShown = FALSE;
    ifRemixPauseLegendMakeSprites();
    ifRemixPauseLegendApplyGroups();
}

void ifRemixPauseLegendEnd(void)
{
    s32 index;

    for (index = 0; index < sIFRemixPauseLegendSpriteCount; index++)
    {
        if (sIFRemixPauseLegendSprites[index] != NULL)
        {
            gcEjectGObj(sIFRemixPauseLegendSprites[index]);
            sIFRemixPauseLegendSprites[index] = NULL;
        }
    }
    sIFRemixPauseLegendSpriteCount = 0;
}

/** Toggles the R legend with Dpad Left while the Disable HUD toggle is OFF (check_toggle_hud_press_); hiding it hides the rest. */
static void ifRemixPauseLegendToggleHudOnDpadLeft(u16 button_tap)
{
    if ((gmRemixToggle(disable_hud) != 0) || (button_tap != L_JPAD))
    {
        return;
    }
    sIFRemixPauseLegendHudShown = !sIFRemixPauseLegendHudShown;
    sIFRemixPauseLegendExtraShown = FALSE;
}

/** Toggles the music and camera legends on a solo R press while the R legend is drawn (check_legend_toggle_press_). */
static void ifRemixPauseLegendToggleOnRPress(u16 button_tap, u16 button_hold)
{
    if ((button_tap != R_TRIG) || (button_hold != R_TRIG))
    {
        return;
    }
    if ((gmRemixToggle(disable_hud) != 0) || !sIFRemixPauseLegendHudShown)
    {
        return;
    }
    sIFRemixPauseLegendExtraShown = !sIFRemixPauseLegendExtraShown;
}

static sb32 ifRemixPauseLegendIsGroupShown(s32 group)
{
    if ((gmRemixToggle(disable_hud) != 0) || !sIFRemixPauseLegendHudShown)
    {
        return FALSE;
    }
    return (group == IFREMIX_PAUSE_LEGEND_GROUP_R) ? TRUE : sIFRemixPauseLegendExtraShown;
}

/** Track title and game of the music block, read live from the stage song (Pause.asm render_live_music_string_). */
static void ifRemixPauseLegendPutMusicTitle(void)
{
    const char *track;
    const char *game;

    if (remix_music_title_get((s32)gMPCollisionBGMDefault, &track, &game) == 0)
    {
        return;
    }
    remix_hud_text_put_fit(IFREMIX_PAUSE_LEGEND_SLOT_TITLE, track, IFREMIX_PAUSE_LEGEND_TITLE_X, IFREMIX_PAUSE_LEGEND_TITLE_Y,
                           IFREMIX_PAUSE_LEGEND_COLOR, 1.0F, REMIX_HUD_ALIGN_LEFT, IFREMIX_PAUSE_LEGEND_TITLE_MAX_WIDTH);
    remix_hud_text_put_fit(IFREMIX_PAUSE_LEGEND_SLOT_GAME, game, IFREMIX_PAUSE_LEGEND_TITLE_X, IFREMIX_PAUSE_LEGEND_GAME_Y,
                           IFREMIX_PAUSE_LEGEND_COLOR, IFREMIX_PAUSE_LEGEND_GAME_SCALE, REMIX_HUD_ALIGN_LEFT,
                           IFREMIX_PAUSE_LEGEND_TITLE_MAX_WIDTH);
}

/** Puts the text and the squares of the groups that are shown; the ones not put stop being drawn on their own. */
static void ifRemixPauseLegendDraw(void)
{
    s32 index;
    s32 slot = IFREMIX_PAUSE_LEGEND_SLOT_FIRST;
    s32 rect_slot = 0;

    for (index = 0; index < IFREMIX_PAUSE_LEGEND_ITEM_COUNT; index++)
    {
        const IFRemixPauseLegendItem *item = &sIFRemixPauseLegendItems[index];

        if ((item->kind == IFREMIX_PAUSE_LEGEND_SPRITE) || !ifRemixPauseLegendIsGroupShown(item->group))
        {
            continue;
        }
        if (item->kind == IFREMIX_PAUSE_LEGEND_TEXT)
        {
            remix_hud_text_put(slot++, item->text, item->x, item->y, IFREMIX_PAUSE_LEGEND_COLOR, item->scale, REMIX_HUD_ALIGN_LEFT);
        }
        else
        {
            remix_hud_rect_put(rect_slot++, item->x, item->y, item->width, item->height, item->rgba);
        }
    }
    if (ifRemixPauseLegendIsGroupShown(IFREMIX_PAUSE_LEGEND_GROUP_MUSIC))
    {
        ifRemixPauseLegendPutMusicTitle();
    }
}

void ifRemixPauseLegendUpdate(u16 button_tap, u16 button_hold)
{
    ifRemixPauseLegendToggleHudOnDpadLeft(button_tap);
    ifRemixPauseLegendToggleOnRPress(button_tap, button_hold);
    ifRemixPauseLegendApplyGroups();
    ifRemixPauseLegendDraw();
}

#endif
