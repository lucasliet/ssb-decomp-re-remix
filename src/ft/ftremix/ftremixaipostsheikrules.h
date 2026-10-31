#ifndef _FTREMIXAIPOSTSHEIKRULES_H_
#define _FTREMIXAIPOSTSHEIKRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix AI Sheik post process (AI.asm cpu_post_process, Sheik/AI/Attacks.asm). Numbers in, numbers out,
 * so the headless unit tests (tests/remix) can run them without game state. */

/** Stick Y written while the teleport goes down (Sheik/AI/Attacks.asm teleport_down, 0xFFB0 = -80). */
#define FTREMIX_AI_SHEIK_TELEPORT_STICK_Y (-80)

/** Radius in world units inside which the DSP control presses B (Sheik/AI/Attacks.asm dsp_control, 600.0F). */
#define FTREMIX_AI_SHEIK_DSP_PRESS_RANGE 600.0F

/**
 * TRUE for the Sheik USP actions the teleport steers down: USPG_BEGIN, USPG_MOVE, USPA_BEGIN and USPA_MOVE
 * (Sheik/Sheik.asm 0xE4, 0xE5, 0xE7, 0xE8).
 */
extern int ftRemixAiSheikIsTeleportAction(int action);

/** TRUE for the Sheik DSP begin action (Sheik/Sheik.asm DSP_BEGIN 0xF0), which runs dsp_control. */
extern int ftRemixAiSheikIsDspAction(int action);

/**
 * TRUE when the teleport goes down, which is when ground is below the fighter. floor_line_id is -1 when there is no ground
 * below (Sheik/AI/Attacks.asm teleport_down, compares the clipping below with -1).
 */
extern int ftRemixAiSheikTeleportHasGroundBelow(int floor_line_id);

/** TRUE when a DSP target at `distance` is close enough to press B (Sheik/AI/Attacks.asm dsp_control). */
extern int ftRemixAiSheikDspIsInRange(float distance);

#endif

#endif
