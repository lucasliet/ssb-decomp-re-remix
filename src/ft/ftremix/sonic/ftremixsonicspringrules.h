#ifndef _FTREMIXSONICSPRINGRULES_H_
#define _FTREMIXSONICSPRINGRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure decision of Sonic's spring for the actions every fighter shares (SonicSpecial.asm check_spring_bounce_,
 * _check_action), free of game globals so the headless unit tests (tests/remix) can run it.
 */

/* Remix action ids (Action.asm) the shared check reads. */
#define FTREMIX_SONIC_SPRING_ACTION_JUMPF 0x016
#define FTREMIX_SONIC_SPRING_ACTION_PASS 0x021
#define FTREMIX_SONIC_SPRING_ACTION_TUMBLE 0x039
#define FTREMIX_SONIC_SPRING_ACTION_FALLSPECIAL 0x03A
#define FTREMIX_SONIC_SPRING_ACTION_SHIELDBREAK 0x09E
#define FTREMIX_SONIC_SPRING_ACTION_SHIELDBREAKFALL 0x09F
#define FTREMIX_SONIC_SPRING_ACTION_INHALEPULLED 0x0AD

/* Outcome of the shared check. */
enum
{
    nFTRemixSonicSpringRulesByCharacter,   /* _keep_action: the per-character checks decide */
    nFTRemixSonicSpringRulesChangeAction,  /* _change_action: the fighter goes to JumpF */
    nFTRemixSonicSpringRulesIgnore         /* _next: the spring ignores the fighter */
};

/**
 * Shared part of the spring's action check.
 *
 * @param action Current action (status) id of the fighter on the spring.
 * @return nFTRemixSonicSpringRulesChangeAction for JumpF..Pass, Tumble, ShieldBreak and ShieldBreakFall;
 *         nFTRemixSonicSpringRulesIgnore for InhalePulled; nFTRemixSonicSpringRulesByCharacter otherwise.
 */
int ftRemixSonicSpringRulesCommonAction(int action);

#endif

#endif
