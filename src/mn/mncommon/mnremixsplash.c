/**
 * mnremixsplash.c - Remix splash image after the N64 logo (Boot.asm splash_), see mnremixsplash.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <mn/mncommon/mnremixsplash.h>
#include <mn/mncommon/mnremixsplashrules.h>
#include <sc/scene.h>
#include <sys/audio.h>
#include <sys/controller.h>

/* Boot.asm splash_._fgm: `lli a1, 0x7800` for BGM.set_volume_ and `lli a0, 0x7800` for FGM.set_volume_ */
#define MN_REMIX_SPLASH_VOLUME 0x7800

static sb32 sMNRemixSplashIsActive;
static s32 sMNRemixSplashFramesLeft;

void mnRemixSplashStartScene(s32 scene_prev)
{
    sMNRemixSplashIsActive = (scene_prev == nSCKindStartup) ? TRUE : FALSE;
    sMNRemixSplashFramesLeft = MN_REMIX_SPLASH_FRAMES;
}

sb32 mnRemixSplashIsActive(void)
{
    return sMNRemixSplashIsActive;
}

/** Number of controllers that hold L this frame (Boot.asm _fgm adds the L bit of P1 to P4). */
static s32 mnRemixSplashCountHeldL(void)
{
    s32 player;
    s32 count = 0;

    for (player = 0; player < MN_REMIX_SPLASH_PLAYER_COUNT; player++)
    {
        if (gSYControllerDevices[player].button_hold & L_TRIG)
        {
            count++;
        }
    }
    return count;
}

void mnRemixSplashPlayJingle(void)
{
    syAudioPlayFGM(mnRemixSplashRulesGetJingleFgm(mnRemixSplashCountHeldL()));
    syAudioSetBGMVolume(0, MN_REMIX_SPLASH_VOLUME);
    func_80020E64(MN_REMIX_SPLASH_VOLUME);
}

sb32 mnRemixSplashTickCountdown(void)
{
    return (mnRemixSplashRulesTickCountdown(&sMNRemixSplashFramesLeft) != 0) ? TRUE : FALSE;
}

#endif
