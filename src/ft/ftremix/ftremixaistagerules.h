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

/**
 * TRUE when the CPU must avoid the Hyrule tornado on this stage (Hazards.asm ai_extend_hyrule_tornado_check): Hyrule Castle and
 * the three stages whose Stages.function_table entry is the Hyrule maker (Hyrule Castle DL, Omega, Remix).
 */
extern int ftRemixAiStageHasHyruleTornado(int gkind);

/**
 * TRUE when a CPU standing on a platform line at joint_y must switch to the Recover objective because the platform drops
 * (Hazards.asm mk_remix_hardcoding_9): below y -100 on Mushroom Kingdom, MK Remix, Big Boo's Haunt and Yoshi's Island II, below
 * y -1000 on Great Bay (turtle platform). The platform-line test belongs to the caller.
 */
extern int ftRemixAiStageFleesDroppingPlatform(int gkind, float joint_y);

#endif

#endif
