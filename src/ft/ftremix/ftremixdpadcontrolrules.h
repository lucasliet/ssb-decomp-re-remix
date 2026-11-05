#ifndef _FTREMIXDPADCONTROLRULES_H_
#define _FTREMIXDPADCONTROLRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/* Pure rules of the Remix CSS panel D-pad Control option that act on actions rather than on the controller
 * (css/DpadControl.asm check_jump_type_idle_, check_jump_type_running_, dash_attack_check_, set_shield_mask_), so the
 * headless unit tests (tests/remix) can run them without game state.
 *
 * The panel values are 0 Off, 1 Stick Swap, 2 Stickless, 3 Stick Swap J and 4 Stickless J. The J modes keep the vanilla
 * stick jump and dash attack; modes 1 and 2 do not. */

#define FTREMIX_DPAD_CONTROL_OFF 0
#define FTREMIX_DPAD_CONTROL_STICK_SWAP 1
#define FTREMIX_DPAD_CONTROL_STICKLESS 2
#define FTREMIX_DPAD_CONTROL_STICK_SWAP_J 3
#define FTREMIX_DPAD_CONTROL_STICKLESS_J 4

#define FTREMIX_DPAD_CONTROL_BUTTON_A 0x8000
#define FTREMIX_DPAD_CONTROL_BUTTON_B 0x4000
#define FTREMIX_DPAD_CONTROL_BUTTON_L 0x0020
#define FTREMIX_DPAD_CONTROL_BUTTON_Z 0x2000

/** What a stick pointed up does while a fighter dashes or runs under the D-pad Control modes 1 and 2. */
typedef enum FTRemixDpadControlRunUpAction
{
    nFTRemixDpadControlRunUpVanilla,    /* the mode keeps the vanilla stick jump */
    nFTRemixDpadControlRunUpNone,       /* no stick jump, nothing else happens */
    nFTRemixDpadControlRunUpSmash,      /* up smash (A held and not pressed this frame) */
    nFTRemixDpadControlRunUpSpecial     /* ground up special (B held and not pressed this frame) */

} FTRemixDpadControlRunUpAction;

/** TRUE for the modes without the J: Stick Swap and Stickless (the asm test `value < 3` with a value other than 0). */
extern int ftRemixDpadControlIsStandardMode(int mode);

/**
 * Action of a stick pointed up while dashing or running (check_jump_type_running_ _check_up_special .. _up_smash). B held
 * and not pressed this frame starts the up special; otherwise A held and not pressed this frame starts the up smash;
 * otherwise nothing. Modes 0, 3 and 4 return nFTRemixDpadControlRunUpVanilla.
 *
 * @param mode D-pad Control panel value.
 * @param button_tap Buttons pressed this frame.
 * @param button_hold Buttons held this frame.
 */
extern int ftRemixDpadControlRunUpAction(int mode, unsigned int button_tap, unsigned int button_hold);

/**
 * TRUE when a dash attack is replaced by nothing (dash_attack_check_): modes 1 and 2 with the stick pointed up.
 *
 * @param mode D-pad Control panel value.
 * @param stick_y Vertical stick of the fighter.
 */
extern int ftRemixDpadControlSkipsDashAttack(int mode, int stick_y);

/**
 * Shield button mask of a human fighter (set_shield_mask_): L with the D-pad Control on (L and Z when the Taunt Button
 * option moved the taunt off L), Z otherwise.
 *
 * @param mode D-pad Control panel value.
 * @param is_taunt_button_custom TRUE when the Taunt Button panel value is not Default.
 */
extern unsigned int ftRemixDpadControlShieldMask(int mode, int is_taunt_button_custom);

/**
 * Taunt button mask of a human fighter that D-pad Control changed (set_shield_mask_): Z when the control is on and the
 * Taunt Button option is Default, otherwise the mask unchanged.
 *
 * @param mode D-pad Control panel value.
 * @param is_taunt_button_custom TRUE when the Taunt Button panel value is not Default.
 * @param taunt_mask Taunt mask before the control.
 */
extern unsigned int ftRemixDpadControlTauntMask(int mode, int is_taunt_button_custom, unsigned int taunt_mask);

#endif

#endif
