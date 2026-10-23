/**
 * gmremixkothhud.c - score meters of Remix King of the Hill (KingOfTheHill.asm update_indicators_, setup_ colours and
 * positions), drawn by the port overlay.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include "gm/gmremixkoth.h"
#include "gm/gmremixkothdraw.h"
#include "remix/remix_hud_text.h"

#define GMREMIX_KOTH_BAR_X 40.0F
#define GMREMIX_KOTH_BAR_X_STEP 70.0F
#define GMREMIX_KOTH_BAR_Y 180.0F
#define GMREMIX_KOTH_BAR_WIDTH 40.0F
#define GMREMIX_KOTH_BAR_HEIGHT 5.0F
#define GMREMIX_KOTH_FLASH_STEP 5
#define GMREMIX_KOTH_FLASH_MAX 100
#define GMREMIX_KOTH_BAR_SLOT_FIRST 0

/* KingOfTheHill.colors_bar (free-for-all, the fifth is the CPU gray) and team_colors_bar. */
static const u32 dGMRemixKothBarColors[] = { 0xC96A6AFF, 0x7E7EC9FF, 0xB2B244FF, 0x4EAD4EFF, 0x888888FF };
static const u32 dGMRemixKothTeamBarColors[] = { 0xC96A6AFF, 0x7E7EC9FF, 0x4EAD4EFF, 0xB2B244FF };

static s32 sGMRemixKothFlashAlpha;
static s32 sGMRemixKothFlashDelta;

void gmRemixKothHudSetup(void)
{
    sGMRemixKothFlashAlpha = 0;
    sGMRemixKothFlashDelta = GMREMIX_KOTH_FLASH_STEP;
}

static void gmRemixKothHudStepFlash(void)
{
    sGMRemixKothFlashAlpha += sGMRemixKothFlashDelta;

    if ((sGMRemixKothFlashAlpha >= GMREMIX_KOTH_FLASH_MAX) || (sGMRemixKothFlashAlpha <= 0))
    {
        sGMRemixKothFlashDelta = -sGMRemixKothFlashDelta;
    }
}

static u32 gmRemixKothHudGetColor(s32 key)
{
    if (gSCManagerBattleState->is_team_battle != FALSE)
    {
        return dGMRemixKothTeamBarColors[key];
    }
    return (gSCManagerBattleState->players[key].pkind == nFTPlayerKindCom) ? dGMRemixKothBarColors[4] : dGMRemixKothBarColors[key];
}

/* The meter of a team exists when one of its players is in the match; in free-for-all each port has its own. */
static sb32 gmRemixKothHudHasKey(s32 key)
{
    s32 i;

    if (gSCManagerBattleState->is_team_battle == FALSE)
    {
        return (gSCManagerBattleState->players[key].pkind != nFTPlayerKindNot) ? TRUE : FALSE;
    }
    for (i = 0; i < GMCOMMON_PLAYERS_MAX; i++)
    {
        if ((gSCManagerBattleState->players[i].pkind != nFTPlayerKindNot) && (gSCManagerBattleState->players[i].team == key))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void gmRemixKothHudUpdate(void)
{
    s32 target = gmRemixKothGetTargetFrames();
    s32 key;
    s32 slot = GMREMIX_KOTH_BAR_SLOT_FIRST;

    gmRemixKothHudStepFlash();

    for (key = 0; key < GMCOMMON_PLAYERS_MAX; key++)
    {
        s32 score = gmRemixKothGetScoreOf(key);
        f32 fraction;
        f32 flash;

        if (gmRemixKothHudHasKey(key) == FALSE)
        {
            continue;
        }
        fraction = (target > 0) ? ((f32)score / (f32)target) : 1.0F;
        flash = (fraction >= 1.0F) ? ((f32)sGMRemixKothFlashAlpha / 255.0F) : 0.0F;

        remix_hud_bar_put(slot, GMREMIX_KOTH_BAR_X + (GMREMIX_KOTH_BAR_X_STEP * slot), GMREMIX_KOTH_BAR_Y, GMREMIX_KOTH_BAR_WIDTH,
                          GMREMIX_KOTH_BAR_HEIGHT, fraction, gmRemixKothHudGetColor(key), flash);
        slot++;
    }
}

#endif
