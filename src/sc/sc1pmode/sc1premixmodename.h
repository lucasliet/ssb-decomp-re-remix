#ifndef _SC1PREMIXMODENAME_H_
#define _SC1PREMIXMODENAME_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Remix 1P modes (SinglePlayerModes.asm singleplayer_mode_flag ids, ROADMAP T6.2). The ids are the asm values so the saved
 * records and the tables of the Remix keep their meaning.
 */
#define SC1PREMIX_MODE_STANDARD 0
#define SC1PREMIX_MODE_BONUS3 1
#define SC1PREMIX_MODE_MULTIMAN 2
#define SC1PREMIX_MODE_CRUEL 3
#define SC1PREMIX_MODE_REMIX_1P 4
#define SC1PREMIX_MODE_ALLSTAR 5
#define SC1PREMIX_MODE_HRC 6
#define SC1PREMIX_MODE_COUNT 7

/**
 * Mode named by `name` (standard, bonus3, multiman, cruel, remix1p, allstar, hrc), the spelling of SSB64_REMIX_1P_MODE.
 *
 * @param name Mode name.
 * @return The SC1PREMIX_MODE_* id, or -1 when the name is NULL or not a mode.
 */
extern int sc1PRemixModeFromName(const char *name);

#endif

#endif
