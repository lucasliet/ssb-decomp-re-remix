#ifndef _SC1PREMIXHIGHSCORES_H_
#define _SC1PREMIXHIGHSCORES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * SinglePlayer.high_score_enabled: the switch the 1P, Bonus and All-Star character selects keep in step with the panel options, the
 * Gameplay and Pokemon toggles and the Single Player Enemy option. It starts on and survives the change of scene, like the asm
 * variable, so the record writers of the 1P menu modes read it after the match.
 */

/** Sets the switch (the character select panel menu, every frame of the 1P, Bonus and All-Star select). */
extern void sc1PRemixHighScoresSetEnabled(sb32 is_enabled);

/** Current value of the switch. */
extern sb32 sc1PRemixHighScoresIsEnabled(void);

/** TRUE when a 1P game, Bonus or Home-Run Contest record may be saved: the switch is on and the CPU plays the enemy. */
extern sb32 sc1PRemixHighScoresCanSaveRecord(void);

/** TRUE when the Multi-Man or Cruel Multi-Man KO count may be saved: the switch is on. */
extern sb32 sc1PRemixHighScoresCanSaveKoRecord(void);

#endif

#endif
