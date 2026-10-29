#ifndef _FTREMIXAICOMMANDS_H_
#define _FTREMIXAICOMMANDS_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Smash Remix CPU input scripts with ids >= 0x31 (AI.asm add_cpu_input_routine) and the
 * extended script commands they use: CUSTOM(n) 0xFE (CUSTOM_COMMANDS) and stick X codes
 * 0x81 to 0x84 (extend_stick_x_commands). REGION_REMIX only.
 *
 * The routines are byte streams in the Remix RAM image, read through AI.command_table
 * (RAM 0x805192D0). The image holds native u32 words, so each script is copied byte by byte
 * into a host buffer at its first use.
 */

#include <ft/fighter.h>
#include "ft/ftremix/ftremixaicommandsrules.h"

#define FTREMIX_AI_COMMAND_CUSTOM 0xFE

/**
 * TRUE for a custom input id (0x31 to 0xFE) whose routine is in the Remix image and ends with the END byte.
 * Vanilla ids (below 0x31) are never custom and return FALSE.
 */
sb32 ftRemixAiIsCustomInputSupported(s32 input_kind);

/**
 * Host copy of the input script of `input_kind`, or NULL for a vanilla id (below 0x31) so the vanilla table is used.
 * A custom id whose routine is missing returns a script holding only the END byte.
 */
u8 *ftRemixAiGetInputScript(s32 input_kind);

/**
 * Runs the custom command whose index byte follows 0xFE at `p_command` (0 to 4: JUMPSQUAT_WAIT, PRESS_C, UNPRESS_C,
 * STICK_TO_TARGET, TURNAROUND_WAIT). Returns the script position to continue from. A waiting command sets
 * the input wait to one frame and returns the position of its 0xFE byte, so it is read again next frame.
 */
u8 *ftRemixAiRunCustomCommand(FTStruct *fp, u8 *p_command);

/**
 * Stick X value of the extended stick code `code` (0x81 to 0x84) for `fp`, see ftRemixAiRulesStickXFromCode.
 */
s32 ftRemixAiStickXFromCode(FTStruct *fp, u8 code);

#endif

#endif
