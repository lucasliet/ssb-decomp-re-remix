/**
 * gmremixkothhud.c - score meters of Remix King of the Hill (KingOfTheHill.asm update_indicators_, setup_ colours and
 * positions). Each active port has a frame sprite (the KOTH GFX file, a native sprite on the interface display link, group 0xB)
 * and a bar plus a white flash rectangle drawn over it by a text layer of the same room.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <gr/grremix/grremixids.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothdraw.h"
#include "gm/gmremixkothrules.h"
#include "gm/gmremixtoggle.h"
#include "if/ifremixsprite.h"
#include "if/ifremixtextlayer.h"
#include "remix/remix_hud_text.h"

#define GMREMIX_KOTH_BAR_WIDTH 40.0F
#define GMREMIX_KOTH_BAR_HEIGHT 5.0F
#define GMREMIX_KOTH_FRAME_X_OFFSET (-3.0F)
#define GMREMIX_KOTH_FRAME_SHADOW 0x000000FF
#define GMREMIX_KOTH_GROUP 0x0B
#define GMREMIX_KOTH_DL_LINK 23
#define GMREMIX_KOTH_BAR_SLOT_FIRST 0

/* update_indicators_: every meter has its own flash rectangle, alpha 0 and delta +5 at setup_. */
static int sGMRemixKothFlashAlpha[GMCOMMON_PLAYERS_MAX];
static int sGMRemixKothFlashDelta[GMCOMMON_PLAYERS_MAX];
static f32 sGMRemixKothBarY;

static sb32 gmRemixKothHudIsPortActive(s32 port)
{
    return (gSCManagerBattleState->players[port].pkind != nFTPlayerKindNot) ? TRUE : FALSE;
}

/* The score a port counts for: its team in a team battle, itself otherwise. */
static s32 gmRemixKothHudGetKey(s32 port)
{
    return (gSCManagerBattleState->is_team_battle != FALSE) ? gSCManagerBattleState->players[port].team : port;
}

/* Colour tag of a port's meter (setup_): the team colour in a team battle, the port colour or the CPU gray otherwise. */
static s32 gmRemixKothHudGetTag(s32 port)
{
    sb32 is_cpu = (gSCManagerBattleState->players[port].pkind == nFTPlayerKindCom) ? TRUE : FALSE;

    return gmRemixKothRulesTag(gmRemixKothHudGetKey(port), gSCManagerBattleState->is_team_battle, is_cpu);
}

static sb32 gmRemixKothHudIsDragonKingStage(void)
{
    if (gSCManagerSceneData.gkind == nGRRemixStage_DRAGONKING)
    {
        return TRUE;
    }
    return (gSCManagerSceneData.gkind == nGRRemixStage_DRAGONKING_REMIX) ? TRUE : FALSE;
}

/**
 * Makes the layer that draws the bars and the flash of the meters in the interface group and room, after the frame sprites and
 * the rest of the HUD, so the frames, the item arrows made later and the hidden HUD (pause, Disable HUD) sort as in the asm.
 */
static void gmRemixKothHudMakeLayer(void)
{
    GObj *layer_gobj = ifRemixTextLayerMake(GMREMIX_KOTH_GROUP, GMREMIX_KOTH_DL_LINK, GOBJ_PRIORITY_DEFAULT);

    ifRemixTextLayerBindBars(layer_gobj, GMREMIX_KOTH_BAR_SLOT_FIRST, GMCOMMON_PLAYERS_MAX);
}

void gmRemixKothHudSetup(void)
{
    f32 frame_y;
    s32 port;

    gmRemixKothRulesMeterY(gmRemixToggle(dragon_king_hud), gmRemixKothHudIsDragonKingStage(), &frame_y, &sGMRemixKothBarY);

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        sGMRemixKothFlashAlpha[port] = 0;
        sGMRemixKothFlashDelta[port] = GMREMIX_KOTHRULES_FLASH_STEP;

        if (gmRemixKothHudIsPortActive(port) == FALSE)
        {
            continue;
        }
        ifRemixSpriteMake(IFREMIX_SPRITE_KOTH_METER, GMREMIX_KOTH_GROUP, GMREMIX_KOTH_DL_LINK,
                          gmRemixKothRulesMeterX(port) + GMREMIX_KOTH_FRAME_X_OFFSET, frame_y,
                          gmRemixKothRulesMeterColor(gmRemixKothHudGetTag(port)),
                          GMREMIX_KOTH_FRAME_SHADOW, 1.0F);
    }
    gmRemixKothHudMakeLayer();
}

void gmRemixKothHudUpdate(void)
{
    s32 target = gmRemixKothGetTargetFrames();
    s32 port;

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        s32 score;
        f32 fraction;
        f32 flash;

        if (gmRemixKothHudIsPortActive(port) == FALSE)
        {
            continue;
        }
        score = gmRemixKothGetScoreOf(gmRemixKothHudGetKey(port));
        fraction = (target > 0) ? ((f32)score / (f32)target) : 1.0F;
        flash = 0.0F;

        if (fraction >= 1.0F)
        {
            gmRemixKothRulesFlashStep(&sGMRemixKothFlashAlpha[port], &sGMRemixKothFlashDelta[port]);
            flash = (f32)sGMRemixKothFlashAlpha[port] / 255.0F;
        }

        remix_hud_bar_put(GMREMIX_KOTH_BAR_SLOT_FIRST + port, gmRemixKothRulesMeterX(port), sGMRemixKothBarY, GMREMIX_KOTH_BAR_WIDTH,
                          GMREMIX_KOTH_BAR_HEIGHT, fraction, gmRemixKothRulesBarColor(gmRemixKothHudGetTag(port)), flash);
    }
}

#endif
