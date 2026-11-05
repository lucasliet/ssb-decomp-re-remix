/**
 * grremixdreamland.c - Whispy Woods wind and downtime of Dream Land for the Remix Whispy Mode. See grremixdreamland.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtoggle.h"
#include "grremixdreamland.h"
#include "grremixids.h"
#include <gr/ground.h>
#include <sc/scene.h>
#include <sc/scmanager.h>

#define GRREMIX_WHISPY_MODE_NORMAL 0
#define GRREMIX_WHISPY_MODE_JAPANESE 1
#define GRREMIX_WHISPY_MODE_SUPER 2
#define GRREMIX_WHISPY_MODE_HYPER 3

#define GRREMIX_DREAMLAND_HYPER_INITIAL_WAIT 0x100
#define GRREMIX_DREAMLAND_HYPER_STOP_WAIT 0x25
#define GRREMIX_DREAMLAND_HYPER_WIND_SPEED_MAX 30.0F
#define GRREMIX_DREAMLAND_JAPANESE_WIND_DECAY 0.0003F
#define GRREMIX_DREAMLAND_SUPER_WIND_DECAY 0.0006F
#define GRREMIX_DREAMLAND_SUPER_BOX_SCALE 1.09375F
#define GRREMIX_DREAMLAND_HYPER_BOX_SCALE 1.1953125F
#define GRREMIX_DREAM_GREENS_EDGE_RIGHT 3811.0F
#define GRREMIX_DREAM_GREENS_EDGE_LEFT (-3811.0F)
#define GRREMIX_DREAM_GREENS_ORIGIN_X 0.0F

/** Toggles.entry_whispy_mode (Whispy Mode setting). */
static s32 grRemixDreamLandWhispyMode(void)
{
    return gmRemixToggle(whispy_mode);
}

/**
 * TRUE on the 1P battle (asm `Global.current_screen == Global.screen.TITLE_AND_1P`, Global.asm). The asm screen id is
 * shared with the title and the 1P game over screen, but Dream Land only runs in a battle, so the port tests the
 * 1P game scene nSCKind1PGame, the same scene grremixcamera.c uses for its 1P check.
 */
static sb32 grRemixDreamLandIsOneP(void)
{
    return gSCManagerSceneData.scene_curr == nSCKind1PGame;
}

/** TRUE when the Hyper Mode overrides apply: any screen but 1P with Whispy Mode set to Hyper. */
static sb32 grRemixDreamLandIsHyper(void)
{
    if (grRemixDreamLandIsOneP())
    {
        return FALSE;
    }
    return grRemixDreamLandWhispyMode() == GRREMIX_WHISPY_MODE_HYPER;
}

s32 grRemixDreamLandWhispyInitialWait(s32 random_wait)
{
    if (grRemixDreamLandIsHyper())
    {
        return GRREMIX_DREAMLAND_HYPER_INITIAL_WAIT;
    }
    return random_wait;
}

s32 grRemixDreamLandWhispyStopWait(s32 random_wait)
{
    if (grRemixDreamLandIsHyper())
    {
        return GRREMIX_DREAMLAND_HYPER_STOP_WAIT;
    }
    return random_wait;
}

f32 grRemixDreamLandWindSpeedMax(void)
{
    s32 whispy_mode = grRemixDreamLandWhispyMode();

    if ((whispy_mode == GRREMIX_WHISPY_MODE_NORMAL) || (whispy_mode == GRREMIX_WHISPY_MODE_JAPANESE))
    {
        return GRPUPUPU_WHISPY_WIND_VEL_BASE;
    }

    /* dreamland_wind_speed_max_ (Hazards.asm:3094-3125) has no screen check, so the 1P game also gets this value in
       Super and Hyper Mode. The 1P exceptions of the Whispy Mode live in the other four hooks. */
    /* Hazards.asm:3109-3117 compares the Japanese id (t2, never reloaded) with the Super id, so Super never takes the
       20.0 branch and gets the Hyper value 30.0 from the lui at line 3117. Kept as the asm behaves. */
    return GRREMIX_DREAMLAND_HYPER_WIND_SPEED_MAX;
}

f32 grRemixDreamLandWindDistDecay(void)
{
    s32 whispy_mode = grRemixDreamLandWhispyMode();

    if (whispy_mode == GRREMIX_WHISPY_MODE_NORMAL)
    {
        return GRPUPUPU_WHISPY_WIND_DIST_DECAY;
    }
    if (whispy_mode == GRREMIX_WHISPY_MODE_JAPANESE)
    {
        return GRREMIX_DREAMLAND_JAPANESE_WIND_DECAY;
    }
    if (grRemixDreamLandIsOneP())
    {
        return GRPUPUPU_WHISPY_WIND_DIST_DECAY;
    }
    return GRREMIX_DREAMLAND_SUPER_WIND_DECAY;
}

/**
 * Box scale of Super and Hyper Mode (asm 0x3F8C0000 and 0x3F990000), or 1.0 for the 1P game and the other modes
 * (asm `_end` jumps skip the multiplication).
 */
static f32 grRemixDreamLandWindBoxScale(void)
{
    if (grRemixDreamLandIsOneP())
    {
        return 1.0F;
    }
    switch (grRemixDreamLandWhispyMode())
    {
    case GRREMIX_WHISPY_MODE_SUPER:
        return GRREMIX_DREAMLAND_SUPER_BOX_SCALE;

    case GRREMIX_WHISPY_MODE_HYPER:
        return GRREMIX_DREAMLAND_HYPER_BOX_SCALE;

    default:
        return 1.0F;
    }
}

void grRemixDreamLandGetWindBox(GRRemixDreamLandWindBox *wind_box)
{
    f32 scale;

    wind_box->edge_left = GRPUPUPU_WHISPY_WINDBOX_EDGELEFT;
    wind_box->edge_right = GRPUPUPU_WHISPY_WINDBOX_EDGERIGHT;
    wind_box->top = GRPUPUPU_WHISPY_WINDBOX_TOP;
    wind_box->bottom = GRPUPUPU_WHISPY_WINDBOX_BOTTOM;
    wind_box->origin_x = GRPUPUPU_WHISPY_POS_X;

    if (gSCManagerBattleState->gkind == nGRRemixStage_DREAM_LAND_SR)
    {
        wind_box->edge_left = GRREMIX_DREAM_GREENS_EDGE_LEFT;
        wind_box->edge_right = GRREMIX_DREAM_GREENS_EDGE_RIGHT;
        wind_box->origin_x = GRREMIX_DREAM_GREENS_ORIGIN_X;
        return;
    }
    scale = grRemixDreamLandWindBoxScale();
    wind_box->edge_left *= scale;
    wind_box->edge_right *= scale;
    wind_box->top *= scale;
}

#endif
