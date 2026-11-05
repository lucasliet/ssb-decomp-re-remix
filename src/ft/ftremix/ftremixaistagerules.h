#ifndef _FTREMIXAISTAGERULES_H_
#define _FTREMIXAISTAGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure stage rules of the Remix AI (Hazards.asm planet_zebes_acid_ _ai_fix), so the headless unit tests (tests/remix) can run
 * them without game state. Numbers in, numbers out. */

/**
 * TRUE when the CPU should avoid a rising acid on this stage: a stage whose Stages.function_table entry is the Planet Zebes
 * maker (Planet Zebes, Norfair, Zebes DL, Zebes Omega, Norfair Remix) and the hazards are on. With the hazards off the acid does
 * not exist, so the CPU must not flee it.
 */
extern int ftRemixAiStageHasZebesAcid(int gkind, int hazards_on);

#endif

#endif
