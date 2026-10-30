/*
 * ftremixairoll.c - level 10 routines of the Remix AI (AI.asm fix_multi_doublejump_usage, skip_repeated_input_check,
 * skip_repeated_input_skip, shield_drop_). Pure rules live in ftremixairollrules.c.
 * REMIX-TODO(AI): stop_roll_spam_ (AI.asm P8/P9, Giga Bowser at any level and level 10) is not ported. Its per-character
 * close-quarter branches (about 230 asm lines) need their own slice; the vanilla roll spam stays until then.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include "ft/ftremix/ftremixairoll.h"
#include "ft/ftremix/ftremixairollrules.h"

/**
 * fix_multi_doublejump_usage (AI.asm): the status test is the vanilla FallSpecial check, level 10 also accepts the
 * aerial jump proc so the CPU does not wait in Fall before its extra jumps.
 */
sb32 ftRemixAiCanJumpWhileFalling(FTStruct *fp)
{
    int proc_is_aerial_jump = (fp->proc_update == ftCommonJumpAerialProcUpdate);

    return ftRemixAiRollCanJumpWhileFalling(fp->level, proc_is_aerial_jump, fp->status_id == nFTCommonStatusFallSpecial);
}

/**
 * skip_repeated_input_check (AI.asm): level 10 skips the recently-used-input range adjustment, so the attack ranges
 * stay at their base values.
 */
sb32 ftRemixAiSkipRepeatedInputCheck(FTStruct *fp)
{
    return ftRemixAiRollIsLevelTen(fp->level);
}

/**
 * skip_repeated_input_skip (AI.asm): level 10 never reports the four-in-a-row limit. It resets the counter instead,
 * as the asm store does.
 */
sb32 ftRemixAiInputRepeatLimitHit(FTStruct *fp, FTComputer *com)
{
    if (ftRemixAiRollIsLevelTen(fp->level))
    {
        com->input_repeat_count = 0;
        return FALSE;
    }
    return com->input_repeat_count >= 4;
}

/**
 * shield_drop_ (AI.asm): the platform drop routine. An idle level 10 CPU sends the custom SHIELD_DROP routine (id 0x37),
 * every other CPU sends the vanilla STICK_DOWN drop (nFTComputerInputStickND1MoveAutoSmashLw, AI.asm STICK_DOWN 0x27).
 */
s32 ftRemixAiDownCommand(FTStruct *fp)
{
    if (ftRemixAiRollShieldDrops(fp->level, fp->status_id == nFTCommonStatusWait))
    {
        return FTREMIX_AI_ROLL_SHIELD_DROP_INPUT;
    }
    return nFTComputerInputStickND1MoveAutoSmashLw;
}

#endif
