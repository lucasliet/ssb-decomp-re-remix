/**
 * gmremixtwelvecbhud.c - overlay text of the 12-Character Battle, see gmremixtwelvecbhud.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "gm/gmremixtwelvecb.h"
#include "gm/gmremixtwelvecbhud.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_modes.h"

/* TwelveCharBattle.asm setup_ (TCB:5093-5121): Render.draw_string arguments, Kabel, centred */
#define GMREMIX_TWELVECB_HUD_SLOT_REMAINING_TITLE 32
#define GMREMIX_TWELVECB_HUD_SLOT_POOL 33
#define GMREMIX_TWELVECB_HUD_SLOT_BEST_TITLE 35
#define GMREMIX_TWELVECB_HUD_SLOT_BEST_NAME 36
#define GMREMIX_TWELVECB_HUD_SLOT_TKOS 38
#define GMREMIX_TWELVECB_HUD_SLOT_GAME 40
#define GMREMIX_TWELVECB_HUD_CENTER_X 160.0F
#define GMREMIX_TWELVECB_HUD_PLAYER_X 124.0F
#define GMREMIX_TWELVECB_HUD_PLAYER_X_STEP 68.0F
#define GMREMIX_TWELVECB_HUD_REMAINING_Y 126.0F
#define GMREMIX_TWELVECB_HUD_POOL_Y 140.0F
#define GMREMIX_TWELVECB_HUD_BEST_TITLE_Y 160.0F
#define GMREMIX_TWELVECB_HUD_BEST_NAME_Y 174.0F
#define GMREMIX_TWELVECB_HUD_TKOS_Y 187.0F
#define GMREMIX_TWELVECB_HUD_BEST_NAME_MAX_WIDTH 72.0F
#define GMREMIX_TWELVECB_HUD_TITLE_SCALE 0.875F
#define GMREMIX_TWELVECB_HUD_BEST_SCALE 0.78125F
#define GMREMIX_TWELVECB_HUD_WHITE 0xFFFFFFFF
#define GMREMIX_TWELVECB_HUD_GAME_X 32.0F
#define GMREMIX_TWELVECB_HUD_GAME_Y 46.0F

/* TCB p1_color 0xB00000FF and p2_color 0x4040C0FF */
static const u32 dGMRemixTwelveCBHudPlayerColors[GMREMIX_TWELVECB_PLAYERS] = { 0xB00000FF, 0x4040C0FF };

static f32 gmRemixTwelveCBHudPlayerX(s32 port)
{
    return GMREMIX_TWELVECB_HUD_PLAYER_X + (GMREMIX_TWELVECB_HUD_PLAYER_X_STEP * port);
}

/** Stocks Remaining and the pool of each player, always shown (asm items 1-3). */
static void gmRemixTwelveCBHudDrawPools(const GMRemixTwelveCBState *state)
{
    s32 port;

    remix_hud_text_put(GMREMIX_TWELVECB_HUD_SLOT_REMAINING_TITLE, "Stocks Remaining", GMREMIX_TWELVECB_HUD_CENTER_X,
                       GMREMIX_TWELVECB_HUD_REMAINING_Y, GMREMIX_TWELVECB_HUD_WHITE, GMREMIX_TWELVECB_HUD_TITLE_SCALE,
                       REMIX_HUD_ALIGN_CENTER);
    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        remix_hud_text_putf(GMREMIX_TWELVECB_HUD_SLOT_POOL + port, gmRemixTwelveCBHudPlayerX(port), GMREMIX_TWELVECB_HUD_POOL_Y,
                            dGMRemixTwelveCBHudPlayerColors[port], GMREMIX_TWELVECB_HUD_TITLE_SCALE, REMIX_HUD_ALIGN_CENTER, "%d",
                            (int)state->players[port].stocks_remaining);
    }
}

/** Best Character, its name and its TKOs once the battle has started (asm items 8-10). */
static void gmRemixTwelveCBHudDrawBest(const GMRemixTwelveCBState *state)
{
    s32 port;

    if (state->status == GMREMIX_TWELVECB_STATUS_NOT_STARTED)
    {
        return;
    }
    remix_hud_text_put(GMREMIX_TWELVECB_HUD_SLOT_BEST_TITLE, "Best Character", GMREMIX_TWELVECB_HUD_CENTER_X,
                       GMREMIX_TWELVECB_HUD_BEST_TITLE_Y, GMREMIX_TWELVECB_HUD_WHITE, GMREMIX_TWELVECB_HUD_TITLE_SCALE,
                       REMIX_HUD_ALIGN_CENTER);
    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        const char *name = remix_modes_fighter_name(state->players[port].best_fkind);

        if (name != NULL)
        {
            remix_hud_text_put_fit(GMREMIX_TWELVECB_HUD_SLOT_BEST_NAME + port, name, gmRemixTwelveCBHudPlayerX(port),
                                   GMREMIX_TWELVECB_HUD_BEST_NAME_Y, dGMRemixTwelveCBHudPlayerColors[port],
                                   GMREMIX_TWELVECB_HUD_BEST_SCALE, REMIX_HUD_ALIGN_CENTER,
                                   GMREMIX_TWELVECB_HUD_BEST_NAME_MAX_WIDTH);
        }
        remix_hud_text_putf(GMREMIX_TWELVECB_HUD_SLOT_TKOS + port, gmRemixTwelveCBHudPlayerX(port), GMREMIX_TWELVECB_HUD_TKOS_Y,
                            dGMRemixTwelveCBHudPlayerColors[port], GMREMIX_TWELVECB_HUD_BEST_SCALE, REMIX_HUD_ALIGN_CENTER, "%d",
                            (int)state->players[port].best_tkos_for);
    }
}

void gmRemixTwelveCBHudDrawCSS(void)
{
    const GMRemixTwelveCBState *state;

    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return;
    }
    state = gmRemixTwelveCBGetState();
    if (state == NULL)
    {
        return;
    }
    gmRemixTwelveCBHudDrawPools(state);
    gmRemixTwelveCBHudDrawBest(state);
}

void gmRemixTwelveCBHudDrawResults(void)
{
    const GMRemixTwelveCBState *state;

    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return;
    }
    state = gmRemixTwelveCBGetState();
    remix_hud_text_putf(GMREMIX_TWELVECB_HUD_SLOT_GAME, GMREMIX_TWELVECB_HUD_GAME_X, GMREMIX_TWELVECB_HUD_GAME_Y,
                        GMREMIX_TWELVECB_HUD_WHITE, GMREMIX_TWELVECB_HUD_TITLE_SCALE, REMIX_HUD_ALIGN_LEFT, "Game %02d",
                        (int)((state != NULL) ? (state->current_game + 1) : 1));
}

#endif
