#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaiteachingrules.h"

/** Training, advanced CPUs and the improved AI all need a screen that is not excluded, except training itself. */
int ftRemixAiTechUsesImprovedRoll(int is_training, int is_advanced_cpu, int is_classic, int improved_ai,
                                  int is_excluded_screen)
{
    if (is_training != 0)
    {
        return 1;
    }
    if ((is_advanced_cpu == 0) && ((is_classic != 0) || (improved_ai == 0)))
    {
        return 0;
    }
    return is_excluded_screen == 0;
}

/** A, B, A+C and B+C (1, 2, 4, 5) block the tech. */
int ftRemixAiTechSingleButtonAllows(int single_button_mode)
{
    return (single_button_mode == 0) || (single_button_mode == 3) || (single_button_mode == 6);
}

/** Forward and backward take the first 60 rolls, in place the next 30, the rest fail. */
int ftRemixAiTechRollOutcome(int roll, int is_off_stage)
{
    if (is_off_stage != 0)
    {
        return (roll < FTREMIX_AI_TECH_CHANCE_FORWARD + FTREMIX_AI_TECH_CHANCE_IN_PLACE + FTREMIX_AI_TECH_CHANCE_BACKWARD)
                   ? FTREMIX_AI_TECH_OUTCOME_IN_PLACE
                   : FTREMIX_AI_TECH_OUTCOME_FAIL;
    }
    if (roll < FTREMIX_AI_TECH_CHANCE_FORWARD)
    {
        return FTREMIX_AI_TECH_OUTCOME_FORWARD;
    }
    if (roll < FTREMIX_AI_TECH_CHANCE_FORWARD + FTREMIX_AI_TECH_CHANCE_BACKWARD)
    {
        return FTREMIX_AI_TECH_OUTCOME_BACKWARD;
    }
    if (roll < FTREMIX_AI_TECH_CHANCE_FORWARD + FTREMIX_AI_TECH_CHANCE_BACKWARD + FTREMIX_AI_TECH_CHANCE_IN_PLACE)
    {
        return FTREMIX_AI_TECH_OUTCOME_IN_PLACE;
    }
    return FTREMIX_AI_TECH_OUTCOME_FAIL;
}

/** Every gate of the Z-cancel roll must pass before the roll is made. */
int ftRemixAiZCancelGateOpen(int improved_path, int is_excluded_screen, int single_button_allows, int is_cpu,
                             int is_classic)
{
    return (improved_path != 0) && (is_excluded_screen == 0) && (single_button_allows != 0) && (is_cpu != 0) &&
           (is_classic == 0);
}

/** The asm rolls 0 to 99 and succeeds below 95. */
int ftRemixAiZCancelRollSucceeds(int roll)
{
    return roll < FTREMIX_AI_ZCANCEL_CHANCE;
}

#endif
