/*
 * ftremixairoll.c - level 10 routines of the Remix AI (AI.asm fix_multi_doublejump_usage, skip_repeated_input_check,
 * skip_repeated_input_skip, shield_drop_, stop_roll_spam_). Pure rules live in ftremixairollrules.c.
 * REMIX-TODO(AI): stop_roll_spam_ per-character branches (AI.asm 6049-6307: Ness, Yoshi, Lucas, Fox family, Kirby, Puff)
 * are not ported. The Remix build never fills Character.close_quarter_combat.table: the table patches at AI.asm 6309-6360
 * are commented out, so every entry is `_continue` and those branches are unreachable. Porting them needs the patches
 * re-enabled first (lead decision).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftcomputer.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <sys/utils.h>
#include "remix/remix_roster_data.h"
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
 * shield_drop_ (AI.asm): the platform drop routine. An idle level 10 CPU sends the custom SHIELD_DROP routine (id 0x3C),
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

/**
 * stop_roll_spam_ (AI.asm) for a CPU in roll range: draws the advantage or neutral table once, after the target check.
 * The random draw is made once in both branches, as the asm does.
 */
static s32 ftRemixAiRollSpamRandomCommand(FTStruct *fp, s32 roll_command)
{
    FTComputer *com = &fp->computer;
    FTStruct *target_fp = ftGetComTargetFighter(com);
    s32 random = syUtilsRandIntRange(FTREMIX_AI_ROLL_SPAM_RANDOM_RANGE);

    if ((target_fp != NULL) && ftRemixAiRollSpamIsAdvantageAction(target_fp->status_id))
    {
        return ftRemixAiRollSpamAdvantageCommand(roll_command, random);
    }
    return ftRemixAiRollSpamNeutralCommand(roll_command, random);
}

/**
 * Command chosen by stop_roll_spam_ (AI.asm): the vanilla roll for CPUs outside the gate, the stick reset while moving,
 * otherwise a random alternative. FTREMIX_AI_ROLL_SPAM_SKIP when the routine declines to act.
 */
static s32 ftRemixAiRollSpamCommand(FTStruct *fp, s32 roll_command)
{
    if (ftRemixAiRollSpamApplies(fp->fkind == REMIX_ID_GBOWSER, fp->level) == FALSE)
    {
        return roll_command;
    }
    if (ftRemixAiRollSpamResetsStick(fp->status_id))
    {
        return FTREMIX_AI_ROLL_SPAM_RESET_STICK_INPUT;
    }
    return ftRemixAiRollSpamRandomCommand(fp, roll_command);
}

/**
 * stop_roll_spam_ (AI.asm P8/P9): replaces the vanilla roll issued by ftComputerFollowObjectiveAttack. Issues the chosen
 * command and returns TRUE, or returns FALSE when the routine skips so the CPU looks for other attacks.
 */
sb32 ftRemixAiStopRollSpam(FTStruct *fp, s32 roll_command)
{
    s32 command = ftRemixAiRollSpamCommand(fp, roll_command);

    if (command == FTREMIX_AI_ROLL_SPAM_SKIP)
    {
        return FALSE;
    }
    ftComputerSetCommandWaitShort(fp, command);
    return TRUE;
}

#endif
