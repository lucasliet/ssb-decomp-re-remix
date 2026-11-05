#ifndef _SC1PTRAININGREMIX_H_
#define _SC1PTRAININGREMIX_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/*
 * Remix additions of the Training mode (Training.asm advance_frame_): the D-pad of any controller toggles the freeze (up), advances one
 * frame (right), cycles the model display (down) and resets the match (left), and the reset count is drawn on screen.
 */

/** Starts a training scene: the match is not frozen, and the reset count restarts unless the scene was loaded by a reset. */
extern void sc1PTrainingRemixStartScene(sb32 is_reset);

/** Counts a reset of the match (the D-pad shortcut and the pause menu option). */
extern void sc1PTrainingRemixOnReset(void);

/**
 * Reads the D-pad of every controller for one frame of the match. Up and right change the freeze, down changes the model display.
 *
 * @return TRUE when the D-pad asks for a reset (left), which the training scene carries out.
 */
extern sb32 sc1PTrainingRemixPollDpad(void);

/** Starts the track of the Music entry (Training.asm play_custom_music_: bgm_table[entry_music]; entry 0 is the Training Mode theme). */
extern void sc1PTrainingRemixStartMusic(void);

/** Plays the Music entry now, even when the track did not change (Training.asm play_bgm_, the A button on the Music entry of the Z menu). */
extern void sc1PTrainingRemixPlayMusic(void);

/**
 * Plays the Music entry when its value changed since the last call (play_bgm_), and keeps it as the stage track so it resumes after
 * a hammer or star. A hammer or star in play keeps its own theme.
 */
extern void sc1PTrainingRemixPollMusic(void);

/**
 * Builds the four ports of the match from the per-port Training options (Training.asm tail_px: Type, Character, Costume), on top of
 * what the character select chose. Disabled ports get no fighter.
 *
 * @param menu_port Port of the human that owns the Training menu.
 * @param select_cpu_port Port the character select gave to the CPU.
 * @return The port the dummy statistics and behavior follow.
 */
extern s32 sc1PTrainingRemixSetupPorts(s32 menu_port, s32 select_cpu_port);

/**
 * Fighter `port` starts the match with: the Character option of the per-port chain, or the one the character select gave the port.
 *
 * @param port Port 0 to 3.
 */
extern s32 sc1PTrainingRemixGetPortFighter(s32 port);

/** Port whose spawn point `port` starts on (the Spawn entry of the chain). */
extern s32 sc1PTrainingRemixSpawnPort(s32 port);

/** TRUE when this frame of the match must not run (frozen, and no frame advance was asked for). Consumes a frame advance. */
extern sb32 sc1PTrainingRemixIsFrameHeld(void);

/** Draws the reset count. */
extern void sc1PTrainingRemixDrawHud(void);

/** Remix standard items the Item row of the pause menu adds after the vanilla ones (Item.asm extend_training_mode_item_menu_). */
#define SC1PTRAININGREMIX_ITEM_COUNT 13

/**
 * Item kind that a menu entry past the vanilla ones spawns (_get_selected_item_id: the entry number plus 0x1C).
 *
 * @param entry Entry number counted from the first custom one, 0 to SC1PTRAININGREMIX_ITEM_COUNT - 1.
 */
extern s32 sc1PTrainingRemixGetItemKind(s32 entry);

/** Offsets in the Training sprite file (0xFE) of the name image tables Remix appends (Item.asm TRAINING_MENU_IMAGE_TABLE and TRAINING_HUD_IMAGE_TABLE). */
#define SC1PTRAININGREMIX_ITEM_MENU_TABLE 0x02A0
#define SC1PTRAININGREMIX_ITEM_HUD_TABLE 0x0330

/*
 * The custom Z menu (Training.asm run_, setup_, entries 3899-3913, tail_px 3658) is sc1ptrainingremixmenu.h; its entries, rules, drawing
 * and custom spawn are sc1ptrainingmenuentries.h, sc1ptrainingmenurules.h, sc1ptrainingremixmenudraw.h and sc1ptrainingremixspawn.h.
 *
 * Not ported from the Z menu setup (no Remix text path needed, they depend on features the port does not have yet):
 *  - setup_ 1597-1613 "(Hold --)" and the hold-A bar, Toggles.entry_hold_to_exit_training, and hold_to_exit_ 751: the Exit option of
 *    the vanilla pause menu would need a held-frames counter on the exit input of sc1PTrainingModeUpdateExitOption.
 *  - setup_ 1634-1637 and run_ 1954-1981 the L "Show Action" legend and the action and frame strings (create_action_strings_ 1475,
 *    update_frame_counts_ 2097, update_actions_ 2156): they need the action names of every status and a frame counter per port.
 */

#endif

#endif
