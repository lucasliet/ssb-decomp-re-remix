/**
 * ftremixsinglebutton.c - Smash Remix SingleButtonMode.asm: Single Button Mode restricts a player to one button
 * (A, B or R), optionally with C buttons (A+C, B+C, R+C) for jumping.
 *
 * ftRemixSingleButtonModeFilter is the input filter of single_button_mode (called from the DpadControl hook, which
 * computes the gate from the screen and pause state). ftRemixSingleButtonCpu* are the CPU input hooks of
 * handle_cpu_inputs_ (AI input path).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include <PR/os.h>

#define FTREMIX_SBM_OFF 0
#define FTREMIX_SBM_A 1
#define FTREMIX_SBM_B 2
#define FTREMIX_SBM_R 3
#define FTREMIX_SBM_FIRST_C_MODE 4
#define FTREMIX_SBM_C_MODE_OFFSET 3

#define FTREMIX_SBM_DPAD 0x0F00
#define FTREMIX_SBM_C_BUTTONS 0x000F
#define FTREMIX_SBM_BUTTON_Z 0x2000

/**
 * Returns the single button of a mode without the C buttons (1 = A, 2 = B, 3 = R), 0 for OFF or an unknown mode.
 */
static s32 ftRemixSingleButtonBaseMode(s32 mode)
{
    if (mode >= FTREMIX_SBM_FIRST_C_MODE)
    {
        return mode - FTREMIX_SBM_C_MODE_OFFSET;
    }
    return mode;
}

/**
 * Returns the N64 button mask of a base mode, or 0 when the base mode has no button.
 */
static u16 ftRemixSingleButtonMainMask(s32 base_mode)
{
    switch (base_mode)
    {
        case FTREMIX_SBM_A:
            return CONT_A;

        case FTREMIX_SBM_B:
            return CONT_B;

        case FTREMIX_SBM_R:
            return CONT_R;

        default:
            return 0;
    }
}

/**
 * Returns the inputs a player keeps in Single Button Mode: the single button, Start and the Dpad, plus the C buttons
 * in the C modes (SingleButtonMode.asm _modify_input).
 */
static u16 ftRemixSingleButtonKeepMask(s32 mode, u16 main_mask)
{
    u16 keep_mask = main_mask | CONT_START | FTREMIX_SBM_DPAD;

    if (mode >= FTREMIX_SBM_FIRST_C_MODE)
    {
        keep_mask |= FTREMIX_SBM_C_BUTTONS;
    }
    return keep_mask;
}

/**
 * Removes the inputs of an open Training menu that could be buffered for the game. Only Start and B (which close
 * the menu) are kept, and B only when it is the single button (SingleButtonMode.asm _check_pause_training).
 */
static void ftRemixSingleButtonFilterTrainingMenu(s32 base_mode, u16 pressed, u16 *held)
{
    u16 keep_mask;

    if ((pressed & (CONT_START | CONT_B)) == 0)
    {
        return;
    }
    keep_mask = CONT_START;
    if (base_mode == FTREMIX_SBM_B)
    {
        keep_mask |= CONT_B;
    }
    *held &= keep_mask;
}

/**
 * Filters the held and pressed inputs of a player for Single Button Mode. gate is one of FTREMIX_SBM_GATE_*, computed
 * by the caller from the screen and pause state (SingleButtonMode.asm _check_screen, _check_pause).
 */
void ftRemixSingleButtonModeFilter(u16 *held, u16 *pressed, s32 gate)
{
    s32 mode = gmRemixToggle(single_button_mode);
    s32 base_mode = ftRemixSingleButtonBaseMode(mode);
    u16 main_mask;

    if (mode == FTREMIX_SBM_OFF)
    {
        return;
    }
    if (gate == FTREMIX_SBM_GATE_CLOSED)
    {
        return;
    }
    if (gate == FTREMIX_SBM_GATE_TRAINING_MENU)
    {
        ftRemixSingleButtonFilterTrainingMenu(base_mode, *pressed, held);
        return;
    }
    main_mask = ftRemixSingleButtonMainMask(base_mode);
    if (main_mask == 0)
    {
        return;
    }
    *held &= ftRemixSingleButtonKeepMask(mode, main_mask);
    *pressed &= ftRemixSingleButtonKeepMask(mode, main_mask);
}

/**
 * Returns the press a CPU sends for its A or B input (cpu_button) in Single Button Mode. The press becomes the single
 * button; R is sent as A+Z (SingleButtonMode.asm handle_cpu_inputs_._a_b).
 */
u16 ftRemixSingleButtonCpuAOrBPress(s32 mode, u16 cpu_button)
{
    switch (ftRemixSingleButtonBaseMode(mode))
    {
        case FTREMIX_SBM_A:
            return CONT_A;

        case FTREMIX_SBM_B:
            return CONT_B;

        case FTREMIX_SBM_R:
            return CONT_A | FTREMIX_SBM_BUTTON_Z;

        default:
            return cpu_button;
    }
}

/**
 * Returns the press a CPU sends for its Z or L input in Single Button Mode: none while the mode is on
 * (SingleButtonMode.asm handle_cpu_inputs_._z_l).
 */
u16 ftRemixSingleButtonCpuZOrLPress(s32 mode, u16 cpu_button)
{
    if (mode != FTREMIX_SBM_OFF)
    {
        return 0;
    }
    return cpu_button;
}

#endif
