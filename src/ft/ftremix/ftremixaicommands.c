/**
 * ftremixaicommands.c - Smash Remix CPU input scripts with ids >= 0x31 and their custom commands
 * (.toolchain/pkgs/smashremix-src/src/AI.asm: add_cpu_input_routine, CUSTOM_COMMANDS, extend_stick_x_commands).
 * See ftremixaicommands.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaicommands.h"
#include <ft/fighter.h>
#include <math.h>
#include <port_log.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

#define FTREMIX_AI_VANILLA_INPUT_KINDS 0x31
#define FTREMIX_AI_ROUTINE_TABLE_ENTRIES 0xFF
#define FTREMIX_AI_ROUTINE_COUNT (FTREMIX_AI_ROUTINE_TABLE_ENTRIES - FTREMIX_AI_VANILLA_INPUT_KINDS)
#define FTREMIX_AI_SCRIPT_CAPACITY 0x80
#define FTREMIX_AI_SCRIPT_END 0xFF

/* PRESS_C sets the C-right bit of the CPU button word (AI.asm PRESS_C, `ori 0x0001`). UNPRESS_C clears the four C bits. */
#define FTREMIX_AI_C_BUTTON_BIT 0x0001
#define FTREMIX_AI_C_BUTTONS_MASK 0x000F

/* STICK_TO_TARGET scales the target vector to this magnitude (AI.asm, 80.0). */
#define FTREMIX_AI_STICK_TO_TARGET_MAGNITUDE 80.0F

typedef enum FTRemixAiCustomKind
{
    nFTRemixAiCustomJumpsquatWait,
    nFTRemixAiCustomPressC,
    nFTRemixAiCustomUnpressC,
    nFTRemixAiCustomStickToTarget,
    nFTRemixAiCustomTurnaroundWait

} FTRemixAiCustomKind;

static u8 sFTRemixAiScripts[FTREMIX_AI_ROUTINE_COUNT][FTREMIX_AI_SCRIPT_CAPACITY];
static sb32 sFTRemixAiScriptLoaded[FTREMIX_AI_ROUTINE_COUNT];
static sb32 sFTRemixAiScriptValid[FTREMIX_AI_ROUTINE_COUNT];
static u8 sFTRemixAiEmptyScript[] = { FTREMIX_AI_SCRIPT_END };

/**
 * TRUE for a routine id of AI.command_table that the Remix image adds (0x31 to 0xFE).
 */
static sb32 ftRemixAiIsRoutineKind(s32 input_kind)
{
    return (input_kind >= FTREMIX_AI_VANILLA_INPUT_KINDS) && (input_kind < FTREMIX_AI_ROUTINE_TABLE_ENTRIES);
}

/**
 * Copies the bytes of the routine at `routine_addr` into `script` up to and including its END byte.
 * Returns FALSE when the address is outside the image or no END byte fits in the capacity.
 */
static sb32 ftRemixAiCopyRoutine(u8 *script, u32 routine_addr)
{
    s32 i;

    if (remix_ram_image_resolve(routine_addr) == NULL)
    {
        script[0] = FTREMIX_AI_SCRIPT_END;
        return FALSE;
    }
    for (i = 0; i < FTREMIX_AI_SCRIPT_CAPACITY - 1; i++)
    {
        script[i] = (u8)remix_ram_image_read_byte(routine_addr + i);
        if (script[i] == FTREMIX_AI_SCRIPT_END)
        {
            return TRUE;
        }
    }
    script[FTREMIX_AI_SCRIPT_CAPACITY - 1] = FTREMIX_AI_SCRIPT_END;
    return FALSE;
}

/**
 * Fills the host copy of `input_kind` in `slot` from AI.command_table and logs the first start of that id.
 */
static void ftRemixAiLoadScript(s32 slot, s32 input_kind)
{
    u32 routine_addr = remix_ram_image_read_word(REMIX_SYM_AI__command_table + (input_kind * 4));

    sFTRemixAiScriptValid[slot] = ftRemixAiCopyRoutine(sFTRemixAiScripts[slot], routine_addr);
    sFTRemixAiScriptLoaded[slot] = TRUE;
    port_log("SSB64: Remix AI input 0x%02X routine at 0x%08X %s\n", input_kind, routine_addr,
             sFTRemixAiScriptValid[slot] ? "loaded" : "MISSING or unterminated, END only");
}

/**
 * Returns the slot of a routine id, loading its host copy on the first request.
 */
static s32 ftRemixAiLoadedSlot(s32 input_kind)
{
    s32 slot = input_kind - FTREMIX_AI_VANILLA_INPUT_KINDS;

    if (!sFTRemixAiScriptLoaded[slot])
    {
        ftRemixAiLoadScript(slot, input_kind);
    }
    return slot;
}

sb32 ftRemixAiIsCustomInputSupported(s32 input_kind)
{
    if (!ftRemixAiIsRoutineKind(input_kind))
    {
        return FALSE;
    }
    return sFTRemixAiScriptValid[ftRemixAiLoadedSlot(input_kind)];
}

u8 *ftRemixAiGetInputScript(s32 input_kind)
{
    if (input_kind < FTREMIX_AI_VANILLA_INPUT_KINDS)
    {
        return NULL;
    }
    if (!ftRemixAiIsRoutineKind(input_kind))
    {
        port_log("SSB64: Remix AI input 0x%02X is past the command table (0x%02X entries), END only\n", input_kind,
                 FTREMIX_AI_ROUTINE_TABLE_ENTRIES);
        return sFTRemixAiEmptyScript;
    }
    return sFTRemixAiScripts[ftRemixAiLoadedSlot(input_kind)];
}

/**
 * PRESS_C: sets the C-right bit of the CPU buttons.
 */
static void ftRemixAiPressC(FTStruct *fp)
{
    fp->input.cp.button_inputs |= FTREMIX_AI_C_BUTTON_BIT;
}

/**
 * UNPRESS_C: clears the four C-button bits of the CPU buttons.
 */
static void ftRemixAiUnpressC(FTStruct *fp)
{
    fp->input.cp.button_inputs &= ~FTREMIX_AI_C_BUTTONS_MASK;
}

/**
 * STICK_TO_TARGET: sets the CPU stick to the target direction, scaled to a magnitude of 80. Zero when the target
 * is on the CPU's top joint. Rounds to nearest like the cvt.w.s of the asm.
 */
static void ftRemixAiStickToTarget(FTStruct *fp)
{
    FTComputer *com = &fp->computer;
    f32 dx = com->target_pos.x - fp->joints[nFTPartsJointTopN]->translate.vec.f.x;
    f32 dy = com->target_pos.y - fp->joints[nFTPartsJointTopN]->translate.vec.f.y;
    f32 magnitude = sqrtf((dx * dx) + (dy * dy));
    f32 scale;

    if (magnitude == 0.0F)
    {
        fp->input.cp.stick_range.x = 0;
        fp->input.cp.stick_range.y = 0;
        return;
    }
    scale = FTREMIX_AI_STICK_TO_TARGET_MAGNITUDE / magnitude;
    fp->input.cp.stick_range.x = (s8)lrintf(dx * scale);
    fp->input.cp.stick_range.y = (s8)lrintf(dy * scale);
}

/**
 * Runs custom command `custom_index` and returns TRUE when it waits this frame. Unknown indices are logged and skipped.
 */
static sb32 ftRemixAiRunCustomKind(FTStruct *fp, s32 custom_index)
{
    switch (custom_index)
    {
    case nFTRemixAiCustomJumpsquatWait:
        return fp->status_id == FTREMIX_AI_ACTION_JUMP_SQUAT;

    case nFTRemixAiCustomPressC:
        ftRemixAiPressC(fp);
        return FALSE;

    case nFTRemixAiCustomUnpressC:
        ftRemixAiUnpressC(fp);
        return FALSE;

    case nFTRemixAiCustomStickToTarget:
        ftRemixAiStickToTarget(fp);
        return FALSE;

    case nFTRemixAiCustomTurnaroundWait:
        return ftRemixAiRulesIsTurnAction(fp->status_id);

    default:
        port_log("SSB64: Remix AI custom command index %d is unknown (expected 0 to 4), skipped\n", custom_index);
        return FALSE;
    }
}

u8 *ftRemixAiRunCustomCommand(FTStruct *fp, u8 *p_command)
{
    s32 custom_index = *p_command++;

    if (!ftRemixAiRunCustomKind(fp, custom_index))
    {
        return p_command;
    }
    fp->computer.input_wait = 1;
    return p_command - 2;
}

s32 ftRemixAiStickXFromCode(FTStruct *fp, u8 code)
{
    f32 player_x = fp->joints[nFTPartsJointTopN]->translate.vec.f.x;
    f32 target_x = fp->computer.target_pos.x;

    return ftRemixAiRulesStickXFromCode(code, player_x, target_x, fp->lr);
}

#endif
