#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixairollrules.h"

/** Only the exact level 10 gets the advanced routines. */
int ftRemixAiRollIsLevelTen(int level)
{
    return level == FTREMIX_AI_ROLL_LEVEL_TEN;
}

/** The proc test is a level 10 only extra: the status test is vanilla and applies at every level. */
int ftRemixAiRollCanJumpWhileFalling(int level, int proc_is_aerial_jump, int status_is_fall_special)
{
    if (status_is_fall_special)
    {
        return 1;
    }
    return ftRemixAiRollIsLevelTen(level) && proc_is_aerial_jump;
}

/** Idle level 10 CPUs shield drop, every other CPU does the normal platform drop. */
int ftRemixAiRollShieldDrops(int level, int is_idle)
{
    return ftRemixAiRollIsLevelTen(level) && is_idle;
}

/** Giga Bowser uses the routine at any level, every other CPU from level 10. */
int ftRemixAiRollSpamApplies(int is_giga_bowser, int level)
{
    return is_giga_bowser || (level >= FTREMIX_AI_ROLL_LEVEL_TEN);
}

/** The asm compares the action as signed, so every action above TurnRun resets the stick. */
int ftRemixAiRollSpamResetsStick(int action)
{
    return action > FTREMIX_AI_ROLL_SPAM_ACTION_TURN_RUN;
}

/** Damage actions are a range, the down, stun and sleep actions are single values. */
int ftRemixAiRollSpamIsAdvantageAction(int action)
{
    if ((action >= FTREMIX_AI_ROLL_SPAM_ACTION_DAMAGE_HIGH1) && (action <= FTREMIX_AI_ROLL_SPAM_ACTION_DAMAGE_FLY_ROLL))
    {
        return 1;
    }
    return (action == FTREMIX_AI_ROLL_SPAM_ACTION_DOWN_BOUNCE_D) || (action == FTREMIX_AI_ROLL_SPAM_ACTION_DOWN_BOUNCE_U) ||
           (action == FTREMIX_AI_ROLL_SPAM_ACTION_STUN) || (action == FTREMIX_AI_ROLL_SPAM_ACTION_SLEEP);
}

/** Draws 0 and 1 choose a short hop, every other draw skips the routine. */
int ftRemixAiRollSpamAdvantageCommand(int roll_command, int random)
{
    switch (random)
    {
        case 0:
            return FTREMIX_AI_ROLL_SPAM_SHORT_HOP_IN_PLACE_INPUT;

        case 1:
            return FTREMIX_AI_ROLL_SPAM_SHORT_HOP_TOWARDS_INPUT;

        default:
            return FTREMIX_AI_ROLL_SPAM_SKIP;
    }
}

/** Draw 0 keeps the vanilla roll, draws 1 to 5 choose a hop or pivot, every other draw skips the routine. */
int ftRemixAiRollSpamNeutralCommand(int roll_command, int random)
{
    switch (random)
    {
        case 0:
            return roll_command;

        case 1:
            return FTREMIX_AI_ROLL_SPAM_SHORT_HOP_IN_PLACE_INPUT;

        case 2:
            return FTREMIX_AI_ROLL_SPAM_SHORT_HOP_TOWARDS_INPUT;

        case 3:
            return FTREMIX_AI_ROLL_SPAM_SHORT_HOP_AWAY_INPUT;

        case 4:
        case 5:
            return FTREMIX_AI_ROLL_SPAM_PIVOT_AWAY_INPUT;

        default:
            return FTREMIX_AI_ROLL_SPAM_SKIP;
    }
}

#endif
