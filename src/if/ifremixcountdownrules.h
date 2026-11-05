#ifndef _IFREMIXCOUNTDOWNRULES_H_
#define _IFREMIXCOUNTDOWNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Stoplight countdown sound rules (Hazards.asm stoplight_1, stoplight_2, stoplight_3 and stoplight_go), pure so the headless unit
 * tests (tests/remix) can run them.
 */

#define IFREMIX_COUNTDOWN_NO_FGM 0

typedef enum IFRemixCountdownStep
{
    nIFRemixCountdownStepThree,
    nIFRemixCountdownStepTwo,
    nIFRemixCountdownStepOne,
    nIFRemixCountdownStepGo

} IFRemixCountdownStep;

/** What a stage adds to one step of the countdown. */
typedef struct IFRemixCountdownSounds
{
    int extra_fgm;           /* FGM played besides the vanilla voice, IFREMIX_COUNTDOWN_NO_FGM for none */
    int is_voice_replaced;   /* 1 when the extra FGM takes the place of the vanilla voice */
    int is_extra_first;      /* 1 when the extra FGM plays before the vanilla voice */

} IFRemixCountdownSounds;

/**
 * Sounds the stage adds to a countdown step: Big Blue plays the F-Zero X voice (0x5CB to 0x5CE) in place of the vanilla one, Toad's
 * Turnpike and Rainbow Road play the Mario Kart 64 noise (0x405 for 3, 2 and 1 after the voice, 0x406 for GO before it).
 *
 * @param gkind Stage id.
 * @param step IFRemixCountdownStep.
 * @return The extra FGM and how it combines with the vanilla voice; no extra FGM on any other stage.
 */
extern IFRemixCountdownSounds ifRemixCountdownRulesGetSounds(int gkind, int step);

#endif
#endif
