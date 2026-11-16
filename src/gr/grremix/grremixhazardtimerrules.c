#include "gr/grremix/grremixhazardtimerrules.h"

int32_t grRemixRainbowRulesSfxCue(int32_t timer)
{
    int32_t i;

    if (timer == GRREMIX_RAINBOW_SFX_FIRST_TIMER)
    {
        return GRREMIX_RAINBOW_FGM_SFX_FIRST;
    }
    for (i = 1; i <= GRREMIX_RAINBOW_SFX_LOOP_COUNT; i++)
    {
        if (timer == GRREMIX_RAINBOW_SFX_FIRST_TIMER + GRREMIX_RAINBOW_SFX_INTERVAL * i)
        {
            return GRREMIX_RAINBOW_FGM_SFX_LOOP;
        }
    }
    if (timer == GRREMIX_RAINBOW_SFX_LAST_TIMER)
    {
        return GRREMIX_RAINBOW_FGM_SFX_LAST;
    }
    return GRREMIX_RAINBOW_FGM_SFX_NONE;
}

int32_t grRemixRainbowRulesGrowlWindowIsOpen(uint8_t frame_draw_last)
{
    return frame_draw_last < GRREMIX_RAINBOW_CHOMP_SOUND_FRAMES;
}

uint32_t grRemixMetallicRulesShrinkTimerKept(uint32_t timer, int32_t is_mushroom_active)
{
    if (is_mushroom_active != 0)
    {
        return 0;
    }
    return timer;
}

GRRemixCacodemonFactoryAction grRemixCacodemonRulesFactoryAction(uint32_t frame, int32_t roll, int32_t is_spawn_prevented)
{
    if ((roll != GRREMIX_CACODEMON_ROLL_HIT) && (frame != GRREMIX_CACODEMON_FORCE_FRAME))
    {
        return nGRRemixCacodemonFactoryActionNone;
    }
    if (is_spawn_prevented != 0)
    {
        return nGRRemixCacodemonFactoryActionRestart;
    }
    return nGRRemixCacodemonFactoryActionSpawn;
}
