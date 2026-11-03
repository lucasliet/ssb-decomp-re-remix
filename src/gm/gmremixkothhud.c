/**
 * gmremixkothhud.c - score meters of Remix King of the Hill (KingOfTheHill.asm update_indicators_, setup_ colours and
 * positions). Each active port has a frame sprite (the KOTH GFX file, a native sprite on the interface display link, group 0xB)
 * and a bar plus a white flash rectangle drawn by the port overlay over it.
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
#include "remix/remix_hud_text.h"

#define GMREMIX_KOTH_BAR_WIDTH 40.0F
#define GMREMIX_KOTH_BAR_HEIGHT 5.0F
#define GMREMIX_KOTH_FRAME_X_OFFSET (-3.0F)
#define GMREMIX_KOTH_FRAME_SHADOW 0x000000FF
#define GMREMIX_KOTH_GROUP 0x0B
#define GMREMIX_KOTH_DL_LINK 23
#define GMREMIX_KOTH_FLASH_STEP 5
#define GMREMIX_KOTH_FLASH_MAX 100
#define GMREMIX_KOTH_BAR_SLOT_FIRST 0

/* KingOfTheHill.colors_bar (free-for-all, the fifth is the CPU gray) and team_colors_bar. */
static const u32 dGMRemixKothBarColors[] = { 0xC96A6AFF, 0x7E7EC9FF, 0xB2B244FF, 0x4EAD4EFF, 0x888888FF };
static const u32 dGMRemixKothTeamBarColors[] = { 0xC96A6AFF, 0x7E7EC9FF, 0x4EAD4EFF, 0xB2B244FF };

static s32 sGMRemixKothFlashAlpha;
static s32 sGMRemixKothFlashDelta;
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

static u32 gmRemixKothHudGetColor(s32 port)
{
    if (gSCManagerBattleState->is_team_battle != FALSE)
    {
        return dGMRemixKothTeamBarColors[gSCManagerBattleState->players[port].team];
    }
    return (gSCManagerBattleState->players[port].pkind == nFTPlayerKindCom) ? dGMRemixKothBarColors[4] : dGMRemixKothBarColors[port];
}

static sb32 gmRemixKothHudIsDragonKingStage(void)
{
    if (gSCManagerSceneData.gkind == nGRRemixStage_DRAGONKING)
    {
        return TRUE;
    }
    return (gSCManagerSceneData.gkind == nGRRemixStage_DRAGONKING_REMIX) ? TRUE : FALSE;
}

void gmRemixKothHudSetup(void)
{
    f32 frame_y;
    s32 port;

    sGMRemixKothFlashAlpha = 0;
    sGMRemixKothFlashDelta = GMREMIX_KOTH_FLASH_STEP;
    gmRemixKothRulesMeterY(gmRemixToggle(dragon_king_hud), gmRemixKothHudIsDragonKingStage(), &frame_y, &sGMRemixKothBarY);

    for (port = 0; port < GMCOMMON_PLAYERS_MAX; port++)
    {
        if (gmRemixKothHudIsPortActive(port) == FALSE)
        {
            continue;
        }
        ifRemixSpriteMake(IFREMIX_SPRITE_KOTH_METER, GMREMIX_KOTH_GROUP, GMREMIX_KOTH_DL_LINK,
                          gmRemixKothRulesMeterX(port) + GMREMIX_KOTH_FRAME_X_OFFSET, frame_y, gmRemixKothHudGetColor(port),
                          GMREMIX_KOTH_FRAME_SHADOW, 1.0F);
    }
}

static void gmRemixKothHudStepFlash(void)
{
    sGMRemixKothFlashAlpha += sGMRemixKothFlashDelta;

    if ((sGMRemixKothFlashAlpha >= GMREMIX_KOTH_FLASH_MAX) || (sGMRemixKothFlashAlpha <= 0))
    {
        sGMRemixKothFlashDelta = -sGMRemixKothFlashDelta;
    }
}

void gmRemixKothHudUpdate(void)
{
    s32 target = gmRemixKothGetTargetFrames();
    s32 port;

    gmRemixKothHudStepFlash();

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
        flash = (fraction >= 1.0F) ? ((f32)sGMRemixKothFlashAlpha / 255.0F) : 0.0F;

        remix_hud_bar_put(GMREMIX_KOTH_BAR_SLOT_FIRST + port, gmRemixKothRulesMeterX(port), sGMRemixKothBarY, GMREMIX_KOTH_BAR_WIDTH,
                          GMREMIX_KOTH_BAR_HEIGHT, fraction, gmRemixKothHudGetColor(port), flash);
    }
}

#endif
