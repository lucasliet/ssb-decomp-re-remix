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

/**
 * Plays the Music entry when its value changed since the last call (play_bgm_), and keeps it as the stage track so it resumes after
 * a hammer or star. A hammer or star in play keeps its own theme.
 */
extern void sc1PTrainingRemixPollMusic(void);

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

/** Draws the name of a custom item entry at the Item row of the pause menu. @param entry As in sc1PTrainingRemixGetItemKind. @return Width of the text. */
extern s32 sc1PTrainingRemixDrawItemName(s32 entry);

/*
 * REMIX-TODO(Training) step 8: the name of the item in hand in the Training HUD (Item.asm extend_training_mode_hud_, file 0xFE
 * TRAINING_HUD_IMAGE_TABLE). The port has no held-item name draw for the custom kinds (0x2D+); missing: a port symbol for the
 * file 0xFE name image table (no REMIX_SYM_ entry) and the vanilla held-item HUD draw routine to hook.
 */

/*
 * REMIX-TODO(Training) steps 10 to 13: the custom Z menu (Training.asm run_, setup_, the entries at :3899-3913 and the per-port
 * chain tail_px at :3658). Missing: the menu state byte Training.toggle_menu (RAM 0x80190979, shared with the vanilla pause menu),
 * the Remix text path (Render.draw_string / draw_number) in the decomp training scene, and the per-port training struct
 * (Training.struct port_1..4). Step 14 guards (fix_sector_z_crashes_, fix_pokemon_crashes_, skip_crowd_cheer_) are not checked yet.
 */

#endif

#endif
