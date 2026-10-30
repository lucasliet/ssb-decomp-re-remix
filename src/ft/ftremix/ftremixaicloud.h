#ifndef _FTREMIXAICLOUD_H_
#define _FTREMIXAICLOUD_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* Magic Cloud hooks of the Remix AI (AI.asm usp_check_, recovery_stick_y_fix). */

/** usp_check_: TRUE for the fighters that take the Fox USP branch of the air check (Fox, JFox, Peppy, Falco). */
sb32 ftRemixAiUspIsFoxFamily(FTStruct *fp);

/** usp_check_: TRUE when a Goemon or Ebisumaru fighter is in a Magic Cloud action that skips the air stick zeroing. */
sb32 ftRemixAiUspIsCloudAction(FTStruct *fp);

/** recovery_stick_y_fix: holds the stick up and presses Z or A for Goemon and Ebisumaru while they recover. */
void ftRemixAiCloudRecoveryInput(FTStruct *fp);

#endif

#endif
