#ifndef _FTREMIXSONICCLASSIC_H_
#define _FTREMIXSONICCLASSIC_H_

#include <ft/fighter.h>

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Classic Sonic, match side (Sonic/Sonic.asm). The per-port flag lives in ftremixsonic.c (ftRemixSonicIsClassic,
 * ftRemixSonicSetClassic = Sonic.classic_table); the CSS side that writes it is mn/mnplayers/mnremixsonicclassic.c.
 */

extern sb32 ftRemixSonicIsClassic(s32 player);
extern void ftRemixSonicSetClassic(s32 player, sb32 is_classic);

/**
 * Main file of the fighter being made (Sonic.asm load_classic_model_, 0x800D7FDC): Sonic of a port with the classic flag set
 * uses the classic main file (File.CSONIC_MAIN, the fighter's special 2 file), every other case the regular main file. Like
 * the asm it clears the select animation frame of the port, and clears the classic flag when the fighter is not Sonic.
 *
 * @param data FTData of the fighter being made.
 * @param fkind Fighter kind being made.
 * @param player Port the fighter is made for.
 * @return Main file whose attributes the fighter uses.
 */
extern void* ftRemixSonicClassicGetMainFile(const FTData *data, s32 fkind, s32 player);

/** FGM of the 0x38 "play FGM" moveset command (classic_fgm_patch_1_). */
extern s32 ftRemixSonicClassicPlayFgm(const FTStruct *fp, s32 fgm_id);

/** FGM of the 0x3C "play FGM once" moveset command (classic_fgm_patch_2_). */
extern s32 ftRemixSonicClassicPlayLoopFgm(const FTStruct *fp, s32 fgm_id);

/** TRUE when the 0x44 / 0x48 voice commands must be skipped (classic_voice_patch_1_, classic_voice_patch_2_). */
extern sb32 ftRemixSonicClassicIsVoiceMuted(const FTStruct *fp);

/** Offset added to the costume id when the Shield option reads Character.costume_shield_color (Shield.asm color_fix_). */
extern s32 ftRemixSonicClassicShieldCostumeOffset(const FTStruct *fp);

/** Frame the CSS selection animation of `player` continues from (Sonic.select_anim_frame). */
extern f32 ftRemixSonicClassicGetSelectFrame(s32 player);

/** Stores the frame the CSS selection animation of `player` continues from. */
extern void ftRemixSonicClassicSetSelectFrame(s32 player, f32 frame);

/**
 * Start frame for a fighter status change (Sonic.asm handle_select_anim_): Sonic entering the CSS selection status
 * (0x10004) starts on the frame stored for its port, every other case on frame 0.
 */
extern f32 ftRemixSonicClassicGetStatusFrame(const FTStruct *fp, s32 status_id);

/**
 * Menu motion of a classic Sonic (TwelveCharBattle.asm defeated_action_override_ + Sonic.asm classic_sonic_anim_swap_): on the
 * 1P intro and the VS results the 1P pose and the second victory pose are replaced by the classic animations, with no moveset
 * commands and no animation flags.
 *
 * @param fp Fighter struct.
 * @param motion_desc Motion row the status selected.
 * @return `motion_desc`, or a static replacement row valid until the next call.
 */
extern FTMotionDesc* ftRemixSonicClassicSwapMenuMotion(const FTStruct *fp, FTMotionDesc *motion_desc);

#endif

#endif
