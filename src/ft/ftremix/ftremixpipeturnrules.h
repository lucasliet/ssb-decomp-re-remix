#ifndef _FTREMIXPIPETURNRULES_H_
#define _FTREMIXPIPETURNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the Character.pipe_turn table (Character.asm:3079, filled in marioshared.asm:157-199 and DrLuigi.asm:150): the
 * fighters that turn like Mario when they enter and leave a warp pipe. Only int, so the headless unit tests (tests/remix) can run it.
 */

/**
 * TRUE when the Remix fighter id has a set Character.pipe_turn entry (NDRM, NWARIO, NCONKER, JMARIO, JLUIGI, MLUIGI, WARIO, GOEMON,
 * EBI, DRM, CONKER, DRL). Mario and the vanilla Luigi are handled by the vanilla check, not by the table.
 */
extern int ftRemixPipeTurnRulesHasEntry(int fkind);

#endif

#endif
