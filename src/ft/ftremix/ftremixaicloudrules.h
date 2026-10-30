#ifndef _FTREMIXAICLOUDRULES_H_
#define _FTREMIXAICLOUDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure Magic Cloud rules of the Remix AI (AI.asm usp_check_, recovery_stick_y_fix), so the headless unit tests
 * (tests/remix) can run them without game state. Numbers in, numbers out. */

#define FTREMIX_AI_CLOUD_ACTION_RIDE 0xDF
#define FTREMIX_AI_CLOUD_ACTION_ATTACK 0xE0
#define FTREMIX_AI_CLOUD_ACTION_JUMP 0xE1
#define FTREMIX_AI_CLOUD_ACTION_ESCAPE 0xE2
#define FTREMIX_AI_CLOUD_STICK_UP 0x50
#define FTREMIX_AI_CLOUD_BUTTON_A 0x80
#define FTREMIX_AI_CLOUD_BUTTON_Z 0x20

/** TRUE for the Goemon USP actions that take the air branch of the recovery check (usp_check_). */
extern int ftRemixAiCloudIsGoemonUspAction(int action);

/** TRUE for the Ebisumaru USP action (Magic Cloud Ride) that takes the air branch of the recovery check (usp_check_). */
extern int ftRemixAiCloudIsEbisumaruUspAction(int action);

/** TRUE for the Goemon actions that hold the stick up while recovering (Magic Cloud Ride and Attack). */
extern int ftRemixAiCloudIsGoemonStickUpAction(int action);

/**
 * Replaces the high byte of a 16-bit button word with high_byte and keeps the low byte. The asm writes one byte at the
 * start of the button halfword, so the other high-byte buttons are cleared too.
 */
extern int ftRemixAiCloudReplaceButtonHighByte(int button_inputs, int high_byte);

#endif

#endif
