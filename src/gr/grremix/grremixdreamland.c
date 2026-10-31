/**
 * grremixdreamland.c - Whispy Woods wind and downtime of Dream Land for the Remix Whispy Mode. See grremixdreamland.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gm/gmremixtoggle.h"
#include "grremixdreamland.h"
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

/** Toggles.entry_whispy_mode (Whispy Mode setting). */
static s32 grRemixDreamLandWhispyMode(void)
{
    return gmRemixToggle(whispy_mode);
}

/**
 * REMIX-TODO(screen id): asm `Global.current_screen == 1P`; mapped here to the 1P game scene, not yet verified. Global.asm
 * line 60 defines screen 0x01 as TITLE_AND_1P (title, 1P game over, 1P battle, Remix 1P battle), and the decomp SCKind
 * index 1 is nSCKindTitle, so the numbers do not map one to one.
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

#endif
