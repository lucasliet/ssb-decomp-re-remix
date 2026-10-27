/**
 * gmremixtwelvecbhud.c - overlay text of the 12-Character Battle, see gmremixtwelvecbhud.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include "gm/gmremixtwelvecb.h"
#include "gm/gmremixtwelvecbhud.h"
#include "remix/remix_hud_text.h"
#include "remix/remix_modes.h"

#define GMREMIX_TWELVECB_HUD_SLOT_TITLE 24
#define GMREMIX_TWELVECB_HUD_SLOT_PLAYER 25
#define GMREMIX_TWELVECB_HUD_TITLE_X 160.0F
#define GMREMIX_TWELVECB_HUD_TITLE_Y 236.0F
#define GMREMIX_TWELVECB_HUD_TITLE_HEIGHT 10.0F
#define GMREMIX_TWELVECB_HUD_PLAYER_X 80.0F
#define GMREMIX_TWELVECB_HUD_PLAYER_X_STEP 160.0F
#define GMREMIX_TWELVECB_HUD_PLAYER_Y 227.0F
#define GMREMIX_TWELVECB_HUD_PLAYER_HEIGHT 7.0F
#define GMREMIX_TWELVECB_HUD_WHITE 0xFFFFFFFF

/* Lighter than the asm string colours of the stock counters (p1 0xB00000FF, p2 0x4040C0FF), which sink into the CSS background. */
static const u32 dGMRemixTwelveCBHudPlayerColors[GMREMIX_TWELVECB_PLAYERS] = { 0xE04040FF, 0x6060E0FF };

static void gmRemixTwelveCBHudDrawTitle(const GMRemixTwelveCBState *state, s32 game_number)
{
    if (state == NULL)
    {
        remix_hud_text_put(GMREMIX_TWELVECB_HUD_SLOT_TITLE, "12-Char. Battle", GMREMIX_TWELVECB_HUD_TITLE_X,
                           GMREMIX_TWELVECB_HUD_TITLE_Y, GMREMIX_TWELVECB_HUD_WHITE, GMREMIX_TWELVECB_HUD_TITLE_HEIGHT);
        return;
    }
    remix_hud_text_putf(GMREMIX_TWELVECB_HUD_SLOT_TITLE, GMREMIX_TWELVECB_HUD_TITLE_X, GMREMIX_TWELVECB_HUD_TITLE_Y,
                        GMREMIX_TWELVECB_HUD_WHITE, GMREMIX_TWELVECB_HUD_TITLE_HEIGHT, "12-Char. Battle  Game %d",
                        (int)game_number);
}

static void gmRemixTwelveCBHudDrawPlayer(const GMRemixTwelveCBState *state, s32 port)
{
    const GMRemixTwelveCBPlayer *player = &state->players[port];
    const char *name = remix_modes_fighter_name(player->best_fkind);
    f32 x = GMREMIX_TWELVECB_HUD_PLAYER_X + (GMREMIX_TWELVECB_HUD_PLAYER_X_STEP * port);

    if (name == NULL)
    {
        remix_hud_text_putf(GMREMIX_TWELVECB_HUD_SLOT_PLAYER + port, x, GMREMIX_TWELVECB_HUD_PLAYER_Y,
                            dGMRemixTwelveCBHudPlayerColors[port], GMREMIX_TWELVECB_HUD_PLAYER_HEIGHT,
                            "P%d Stocks %d", (int)(port + 1), (int)player->stocks_remaining);
        return;
    }
    remix_hud_text_putf(GMREMIX_TWELVECB_HUD_SLOT_PLAYER + port, x, GMREMIX_TWELVECB_HUD_PLAYER_Y,
                        dGMRemixTwelveCBHudPlayerColors[port], GMREMIX_TWELVECB_HUD_PLAYER_HEIGHT,
                        "P%d Stocks %d  Best %s %d TKOs", (int)(port + 1), (int)player->stocks_remaining, name,
                        (int)player->best_tkos_for);
}

void gmRemixTwelveCBHudDrawCSS(void)
{
    const GMRemixTwelveCBState *state;
    s32 port;

    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return;
    }
    state = gmRemixTwelveCBGetState();
    gmRemixTwelveCBHudDrawTitle(state, (state != NULL) ? (state->current_game + 2) : 1);

    if (state == NULL)
    {
        return;
    }
    for (port = 0; port < GMREMIX_TWELVECB_PLAYERS; port++)
    {
        gmRemixTwelveCBHudDrawPlayer(state, port);
    }
}

void gmRemixTwelveCBHudDrawResults(void)
{
    const GMRemixTwelveCBState *state;

    if (gmRemixTwelveCBIsActive() == FALSE)
    {
        return;
    }
    state = gmRemixTwelveCBGetState();
    gmRemixTwelveCBHudDrawTitle(state, (state != NULL) ? (state->current_game + 1) : 1);
}

#endif
