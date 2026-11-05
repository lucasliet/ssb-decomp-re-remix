/**
 * ftremixdpad.c - Smash Remix DpadControl.asm: the controller transform of the D-pad Control panel option (Stick Swap,
 * Stickless and their J variants), the R macro it changes (A+L instead of A+Z) and the D-pad stick values of
 * DpadFunctions.asm dpad_macro_check_ (_dpad_macro_move_setup .. _dpad_move_end).
 *
 * ftRemixDpadControlController (ftMainProcUpdateInterrupt, after ftRemixInputDelayController) returns the controller
 * sample the human fighter reads this frame. Not ported here: the jump type and dash attack checks of the J variants
 * (check_jump_type_idle_, check_jump_type_running_, dash_attack_check_) and the shield mask override
 * (set_shield_mask_).
 *
 * Fighter fields behind the asm player struct offsets: 0x08 FTStruct.fkind, 0x0D FTStruct.player, 0x1C
 * FTStruct.status_total_tics, 0x24 FTStruct.status_id, 0x14C FTStruct.ga, 0xF0 FTStruct.coll_data.floor_dist (the asm
 * reads its high half, which is zero only for 0.0F), 0xF6 low half of FTStruct.coll_data.floor_flags, 0x1C2
 * FTStruct.input.pl.stick_range (the sample stick, which the caller copies there).
 *
 * Known difference: the asm reads A and B from a3 after Single Button Mode filtered it (the filter runs at the
 * controller struct write, before dpad_macro_check_). The port filters after this transform, so A and B here are the
 * unfiltered buttons.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmdef.h>
#include <gm/gmremixpanel.h>
#include <mp/mpdef.h>
#include <PR/os.h>
#include <sys/controller.h>
#include "remix/remix_roster_data.h"

#define FTREMIX_DPAD_CONTROL_STICK_SWAP 1
#define FTREMIX_DPAD_CONTROL_STICKLESS 2
#define FTREMIX_DPAD_CONTROL_STICK_SWAP_J 3
#define FTREMIX_DPAD_CONTROL_STICKLESS_J 4

#define FTREMIX_DPAD_BUTTONS (U_JPAD | D_JPAD | L_JPAD | R_JPAD)
#define FTREMIX_DPAD_SWAP_DEADZONE_X 40
#define FTREMIX_DPAD_SWAP_DEADZONE_Y 30

#define FTREMIX_DPAD_STICK_HALF_MAX 0x28
#define FTREMIX_DPAD_STICK_MAX 0x50
#define FTREMIX_DPAD_STICK_HALF_MIN 0xD8
#define FTREMIX_DPAD_STICK_MIN 0xB0
#define FTREMIX_DPAD_TAP_WINDOW 10
#define FTREMIX_DPAD_TAP_SYNCED 9
#define FTREMIX_DPAD_WALK_FAST_CLEAR_TICS 16
#define FTREMIX_DPAD_WALK_MIDDLE_PARTIAL_TICS 3
#define FTREMIX_DPAD_TURN_CENTER_TICS 10
#define FTREMIX_DPAD_UP_DOWN_BUTTONS (U_JPAD | D_JPAD)
#define FTREMIX_DPAD_LEFT_RIGHT_BUTTONS (L_JPAD | R_JPAD)

/* Remix action ids that the vanilla status enums do not name (Mewtwo.asm:426-429, Sheik.asm:114-117,
 * Goemon.asm:251-253, Ebi.asm:204). */
#define FTREMIX_DPAD_MTWO_TELEPORT_START 0xEA
#define FTREMIX_DPAD_MTWO_TELEPORT_START_AIR 0xED
#define FTREMIX_DPAD_SHEIK_USP_GROUND_BEGIN 0xE4
#define FTREMIX_DPAD_SHEIK_USP_AIR_BEGIN 0xE7
#define FTREMIX_DPAD_NINJA_USP 0xDF
#define FTREMIX_DPAD_GOEMON_USP_ATTACK 0xE0

/* The asm compares action ids against Remix constants (Action.asm:16-42, 158, 721-732, 1898-1903). */
_Static_assert(nFTCommonStatusWalkMiddle == 0x0C, "Action.Walk2");
_Static_assert(nFTCommonStatusWalkFast == 0x0D, "Action.Walk3");
_Static_assert(nFTCommonStatusTurn == 0x12, "Action.Turn");
_Static_assert(nFTCommonStatusSquatWait == 0x1D, "Action.CrouchIdle");
_Static_assert(nFTCommonStatusDamageHi1 == 0x25, "Action.DamageHigh1");
_Static_assert(nFTCommonStatusGuard == 0x99, "Action.Shield");
_Static_assert(nFTCommonStatusSpecialStart == 0xDC, "first character-specific action id");
_Static_assert(nFTFoxStatusSpecialAirN == 0xE2, "Action.FOX.LaserAir");
_Static_assert(nFTFoxStatusSpecialHiStart == 0xE3, "Action.FOX.FireFoxStart");
_Static_assert(nFTFoxStatusSpecialAirHiHold == 0xE6, "Action.FOX.ReadyingFireFoxAir");
_Static_assert(nFTPikachuStatusSpecialHiStart == 0xE8, "Action.PIKACHU.QuickAttackStart");
_Static_assert(nFTPikachuStatusSpecialAirHiEnd == 0xED, "Action.PIKACHU.QuickAttackEndAir");

typedef enum FTRemixDpadVerticalMode
{
    nFTRemixDpadVerticalNormal,
    nFTRemixDpadVerticalSpecial,
    nFTRemixDpadVerticalUpMax

} FTRemixDpadVerticalMode;

/* dpad_move_timer: frames left of the double tap window, one per port. */
static s32 sFTRemixDpadMoveTimers[GMCOMMON_PLAYERS_MAX];

/* One synthetic sample per port (asm writes the swapped values into the controller struct). */
static SYController sFTRemixDpadControlSamples[GMCOMMON_PLAYERS_MAX];

/**
 * Returns TRUE for the modes that exchange the stick and the D-pad (Stick Swap and Stick Swap J).
 */
static sb32 ftRemixDpadControlSwapsStick(s32 mode)
{
    return (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP) || (mode == FTREMIX_DPAD_CONTROL_STICK_SWAP_J);
}

/**
 * Returns the D-pad bits a stick position presses. An axis presses a direction once its magnitude reaches the deadzone
 * (40 on X, 30 on Y, as in the asm).
 */
static u16 ftRemixDpadFromStick(Vec2b stick)
{
    u16 dpad = 0;

    if (stick.x >= FTREMIX_DPAD_SWAP_DEADZONE_X)
    {
        dpad |= R_JPAD;
    }
    if (stick.x <= -FTREMIX_DPAD_SWAP_DEADZONE_X)
    {
        dpad |= L_JPAD;
    }
    if (stick.y >= FTREMIX_DPAD_SWAP_DEADZONE_Y)
    {
        dpad |= U_JPAD;
    }
    if (stick.y <= -FTREMIX_DPAD_SWAP_DEADZONE_Y)
    {
        dpad |= D_JPAD;
    }
    return dpad;
}

/**
 * Returns the held buttons with R replaced by A+L (set_R_macro). R is cleared so ftMainProcUpdateInterrupt does not
 * expand it again to A+Z.
 */
static u16 ftRemixDpadControlRMacro(u16 hold)
{
    if (!(hold & R_TRIG))
    {
        return hold;
    }
    return (hold & ~R_TRIG) | A_BUTTON | L_TRIG;
}

/**
 * Registers a D-pad press on the double tap timer of a port and returns TRUE when it is a double tap, so the stick gets
 * full values (DpadFunctions.asm 281-293 _no_move_countdown). A timer of 1..9 means the previous press ended less than
 * 10 frames ago; any other value (0 or the 10 of a press still held) is a first press and gets partial values.
 */
static sb32 ftRemixDpadRegisterTap(s32 player)
{
    s32 timer = sFTRemixDpadMoveTimers[player];
    sb32 is_double_tap = (timer > 0) && (timer < FTREMIX_DPAD_TAP_WINDOW);

    sFTRemixDpadMoveTimers[player] = is_double_tap ? FTREMIX_DPAD_TAP_WINDOW - 1 : FTREMIX_DPAD_TAP_WINDOW;
    return is_double_tap;
}

/**
 * Counts the double tap timer of a port down while no D-pad direction is held (DpadFunctions.asm 269-279). The timer
 * stops at 0.
 */
static void ftRemixDpadCountDownTap(s32 player)
{
    if (sFTRemixDpadMoveTimers[player] > 0)
    {
        sFTRemixDpadMoveTimers[player]--;
    }
}

/**
 * Returns TRUE and clears the timer of a port when the fighter has been in Walk3 for 16 frames, so a held D-pad keeps
 * full values without a stale tap window (DpadFunctions.asm 295-304).
 */
static sb32 ftRemixDpadClearTapOnLongWalk(FTStruct *fp)
{
    if (fp->status_id != nFTCommonStatusWalkFast)
    {
        return FALSE;
    }
    if (fp->status_total_tics < FTREMIX_DPAD_WALK_FAST_CLEAR_TICS)
    {
        return FALSE;
    }
    sFTRemixDpadMoveTimers[fp->player] = 0;
    return TRUE;
}

/**
 * Returns TRUE for the fox firefox charge and start actions that the Spacies share (DpadFunctions.asm 339-355).
 */
static sb32 ftRemixDpadIsFireFoxStatus(s32 status_id)
{
    return (status_id >= nFTFoxStatusSpecialHiStart) && (status_id <= nFTFoxStatusSpecialAirHiHold);
}

/**
 * Returns the vertical stick mode of the Spacies (Fox, JFox, Falco, Wolf, Peppy): only diagonals during firefox use the
 * special values (DpadFunctions.asm 339-355).
 */
static FTRemixDpadVerticalMode ftRemixDpadSpacieMode(FTStruct *fp, u16 buttons)
{
    if (!(buttons & FTREMIX_DPAD_LEFT_RIGHT_BUTTONS))
    {
        return nFTRemixDpadVerticalNormal;
    }
    return ftRemixDpadIsFireFoxStatus(fp->status_id) ? nFTRemixDpadVerticalSpecial : nFTRemixDpadVerticalNormal;
}

/**
 * Returns the vertical stick mode of Pikachu, JPika and EPika: quick attack actions use the special values on diagonals
 * and the maximum values otherwise (DpadFunctions.asm 356-367).
 */
static FTRemixDpadVerticalMode ftRemixDpadPikachuMode(FTStruct *fp, u16 buttons)
{
    if ((fp->status_id < nFTPikachuStatusSpecialHiStart) || (fp->status_id > nFTPikachuStatusSpecialAirHiEnd))
    {
        return nFTRemixDpadVerticalNormal;
    }
    return (buttons & FTREMIX_DPAD_LEFT_RIGHT_BUTTONS) ? nFTRemixDpadVerticalSpecial : nFTRemixDpadVerticalUpMax;
}

/**
 * Returns the vertical stick mode of Mewtwo: the teleport start actions use the special values
 * (DpadFunctions.asm 368-376).
 */
static FTRemixDpadVerticalMode ftRemixDpadMewtwoMode(FTStruct *fp)
{
    if ((fp->status_id == FTREMIX_DPAD_MTWO_TELEPORT_START) || (fp->status_id == FTREMIX_DPAD_MTWO_TELEPORT_START_AIR))
    {
        return nFTRemixDpadVerticalSpecial;
    }
    return nFTRemixDpadVerticalNormal;
}

/**
 * Returns the vertical stick mode of Sheik: the up special start actions use the special values
 * (DpadFunctions.asm 377-385).
 */
static FTRemixDpadVerticalMode ftRemixDpadSheikMode(FTStruct *fp)
{
    if ((fp->status_id == FTREMIX_DPAD_SHEIK_USP_GROUND_BEGIN) || (fp->status_id == FTREMIX_DPAD_SHEIK_USP_AIR_BEGIN))
    {
        return nFTRemixDpadVerticalSpecial;
    }
    return nFTRemixDpadVerticalNormal;
}

/**
 * Returns the vertical stick mode of Goemon and Ebisumaru: the up special actions use the maximum values
 * (DpadFunctions.asm 386-394). Ebisumaru has no USPAttack action, but the asm compares both ids for both fighters.
 */
static FTRemixDpadVerticalMode ftRemixDpadNinjaMode(FTStruct *fp)
{
    if ((fp->status_id == FTREMIX_DPAD_NINJA_USP) || (fp->status_id == FTREMIX_DPAD_GOEMON_USP_ATTACK))
    {
        return nFTRemixDpadVerticalUpMax;
    }
    return nFTRemixDpadVerticalNormal;
}

/**
 * Returns how the fighter reads the vertical stick value for this frame (DpadFunctions.asm 311-394
 * _dpad_move_check_special_character).
 */
static FTRemixDpadVerticalMode ftRemixDpadVerticalMode(FTStruct *fp, u16 buttons)
{
    switch (fp->fkind)
    {
    case REMIX_ID_FOX:
    case REMIX_ID_JFOX:
    case REMIX_ID_FALCO:
    case REMIX_ID_WOLF:
    case REMIX_ID_PEPPY:
        return ftRemixDpadSpacieMode(fp, buttons);

    case REMIX_ID_MTWO:
        return ftRemixDpadMewtwoMode(fp);

    case REMIX_ID_SHEIK:
        return ftRemixDpadSheikMode(fp);

    case REMIX_ID_PIKACHU:
    case REMIX_ID_JPIKA:
    case REMIX_ID_EPIKA:
        return ftRemixDpadPikachuMode(fp, buttons);

    case REMIX_ID_GOEMON:
    case REMIX_ID_EBI:
        return ftRemixDpadNinjaMode(fp);

    default:
        return nFTRemixDpadVerticalNormal;
    }
}

/**
 * Returns the vertical byte of the special modes: B held keeps a half range so the special can be steered, otherwise
 * full range (DpadFunctions.asm 422-438). Up wins over down when both are held.
 */
static u16 ftRemixDpadSpecialStickY(u16 buttons)
{
    sb32 is_holding_b = (buttons & B_BUTTON) != 0;

    if (buttons & U_JPAD)
    {
        return is_holding_b ? FTREMIX_DPAD_STICK_HALF_MAX : FTREMIX_DPAD_STICK_MAX;
    }
    return is_holding_b ? FTREMIX_DPAD_STICK_HALF_MIN : FTREMIX_DPAD_STICK_MIN;
}

/**
 * Returns the vertical byte of an up press: full range on the ground, half range in the air so aerial up inputs do not
 * reach the up special threshold (DpadFunctions.asm 396-404).
 */
static u16 ftRemixDpadUpStickY(FTStruct *fp)
{
    if (fp->ga != nMPKineticsGround)
    {
        return FTREMIX_DPAD_STICK_HALF_MAX;
    }
    return FTREMIX_DPAD_STICK_MAX;
}

/**
 * Turns a soft platform under the fighter into a solid one for the current frame so a down press can crouch
 * (DpadFunctions.asm 440-458 _check_for_platform). The asm overwrites the low half of the floor flags with 0x8000.
 */
static void ftRemixDpadSolidifySoftPlatform(FTStruct *fp)
{
    if (fp->status_id == nFTCommonStatusSquatWait)
    {
        return;
    }
    if ((u32)fp->status_id >= nFTCommonStatusDamageHi1)
    {
        return;
    }
    if (fp->coll_data.floor_dist != 0.0F)
    {
        return;
    }
    if (!(fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS))
    {
        return;
    }
    fp->coll_data.floor_flags = (fp->coll_data.floor_flags & ~0xFFFF) | MAP_VERTEX_COLL_CLIFF;
}

/**
 * Returns the vertical byte of the stick word for the held up and down directions, or 0 with no vertical direction. The
 * special modes return before the down press and the platform check (DpadFunctions.asm 307-309, 396-438). Up and down
 * together add up to 0x100, which carries into the horizontal byte exactly as the asm sum does.
 */
static u16 ftRemixDpadVerticalStick(FTStruct *fp, u16 buttons, sb32 is_double_tap)
{
    FTRemixDpadVerticalMode mode;
    u16 stick_y = 0;

    if (!(buttons & FTREMIX_DPAD_UP_DOWN_BUTTONS))
    {
        return 0;
    }
    mode = ftRemixDpadVerticalMode(fp, buttons);

    if (mode == nFTRemixDpadVerticalSpecial)
    {
        return ftRemixDpadSpecialStickY(buttons);
    }
    if (buttons & U_JPAD)
    {
        stick_y += (mode == nFTRemixDpadVerticalUpMax) ? FTREMIX_DPAD_STICK_MAX : ftRemixDpadUpStickY(fp);
    }
    if (buttons & D_JPAD)
    {
        stick_y += FTREMIX_DPAD_STICK_MIN;
    }
    if (!is_double_tap)
    {
        ftRemixDpadSolidifySoftPlatform(fp);
    }
    return stick_y;
}

/**
 * Returns TRUE when the horizontal stick gets full values (DpadFunctions.asm 460-493): on a double tap, with A held,
 * in any character-specific action or shield, and once Walk2 passed its third frame. The last case re-arms the tap
 * timer so the hold keeps full values after the walk reaches Walk3.
 */
static sb32 ftRemixDpadUsesFullRange(FTStruct *fp, u16 buttons, sb32 is_double_tap)
{
    if (is_double_tap || (buttons & A_BUTTON))
    {
        return TRUE;
    }
    if (fp->status_id >= nFTCommonStatusSpecialStart)
    {
        return TRUE;
    }
    if (fp->status_id == nFTCommonStatusGuard)
    {
        return TRUE;
    }
    if (fp->status_id != nFTCommonStatusWalkMiddle)
    {
        return FALSE;
    }
    if (fp->status_total_tics < FTREMIX_DPAD_WALK_MIDDLE_PARTIAL_TICS)
    {
        return FALSE;
    }
    sFTRemixDpadMoveTimers[fp->player] = FTREMIX_DPAD_TAP_SYNCED;
    return TRUE;
}

/**
 * Returns the horizontal bytes of the stick word (DpadFunctions.asm 495-512). Left and right together cancel out
 * through the 16 bit wrap of the sum.
 */
static u16 ftRemixDpadHorizontalStick(u16 buttons, sb32 is_full_range)
{
    u16 stick_x = 0;
    u16 left = is_full_range ? FTREMIX_DPAD_STICK_MIN : FTREMIX_DPAD_STICK_HALF_MIN;
    u16 right = is_full_range ? FTREMIX_DPAD_STICK_MAX : FTREMIX_DPAD_STICK_HALF_MAX;

    if (buttons & L_JPAD)
    {
        stick_x += left << 8;
    }
    if (buttons & R_JPAD)
    {
        stick_x += right << 8;
    }
    return stick_x;
}

/**
 * Returns TRUE on frame 10 of a Turn without A or B held, where the asm centres the horizontal stick so the double tap
 * dash is buffered instead of eaten by the turn ("Turn Tap", DpadFunctions.asm 514-523).
 */
static sb32 ftRemixDpadCentersOnTurn(FTStruct *fp, u16 buttons)
{
    if (buttons & (A_BUTTON | B_BUTTON))
    {
        return FALSE;
    }
    if (fp->status_id != nFTCommonStatusTurn)
    {
        return FALSE;
    }
    return fp->status_total_tics == FTREMIX_DPAD_TURN_CENTER_TICS;
}

/**
 * Returns the stick word (X byte high, Y byte low) of a frame with at least one D-pad direction held. buttons holds the
 * non-D-pad buttons of the sample plus the physical D-pad (the asm a0).
 */
static u16 ftRemixDpadMoveStickWord(FTStruct *fp, u16 buttons)
{
    sb32 is_double_tap = ftRemixDpadRegisterTap(fp->player);
    sb32 is_full_range;
    u16 stick_word;

    if (ftRemixDpadClearTapOnLongWalk(fp) != FALSE)
    {
        is_double_tap = TRUE;
    }
    stick_word = ftRemixDpadVerticalStick(fp, buttons, is_double_tap);
    is_full_range = ftRemixDpadUsesFullRange(fp, buttons, is_double_tap);
    stick_word += ftRemixDpadHorizontalStick(buttons, is_full_range);

    if ((is_full_range != FALSE) && (ftRemixDpadCentersOnTurn(fp, buttons) != FALSE))
    {
        stick_word &= 0x00FF;
    }
    return stick_word;
}

/**
 * Writes the stick of the sample from the physical D-pad (DpadFunctions.asm 265-279 _dpad_macro_move). With no
 * direction held the stick is 0: Stick Swap moved the physical stick to the D-pad bits and Stickless cleared it
 * (DpadControl.asm 183-191), and the asm returns before touching it.
 */
static void ftRemixDpadWriteStick(FTStruct *fp, SYController *sample, u16 buttons)
{
    u16 stick_word = 0;

    if (buttons & FTREMIX_DPAD_BUTTONS)
    {
        stick_word = ftRemixDpadMoveStickWord(fp, buttons);
    }
    else
    {
        ftRemixDpadCountDownTap(fp->player);
    }
    sample->stick_range.x = (s8)(stick_word >> 8);
    sample->stick_range.y = (s8)(stick_word & 0x00FF);
}

/**
 * Returns the controller sample a human fighter reads this frame with DpadControl applied. Returns controller unchanged
 * when the port has the option off or the port is invalid. Called from ftMainProcUpdateInterrupt after the InputDelay
 * sample, so the swap works on the delayed input.
 */
SYController *ftRemixDpadControlController(FTStruct *fp, SYController *controller)
{
    s32 player = fp->player;
    s32 mode;
    u16 dpad;
    u16 hold;
    SYController *sample;

    if (player >= GMCOMMON_PLAYERS_MAX)
    {
        return controller;
    }
    mode = gmRemixPanel(DPAD_CONTROL, player);
    if (mode == 0)
    {
        return controller;
    }

    sample = &sFTRemixDpadControlSamples[player];
    *sample = *controller;
    dpad = controller->button_hold & FTREMIX_DPAD_BUTTONS;
    hold = controller->button_hold;

    if (ftRemixDpadControlSwapsStick(mode))
    {
        hold = (hold & ~FTREMIX_DPAD_BUTTONS) | ftRemixDpadFromStick(controller->stick_range);
    }
    sample->button_hold = ftRemixDpadControlRMacro(hold);
    ftRemixDpadWriteStick(fp, sample, (sample->button_hold & ~FTREMIX_DPAD_BUTTONS) | dpad);
    return sample;
}

#endif
