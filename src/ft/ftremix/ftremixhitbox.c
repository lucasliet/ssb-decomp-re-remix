/**
 * ftremixhitbox.c - Smash Remix Hitbox.asm: force grab immunity.
 *
 * The hitbox display patches of Hitbox.asm are not ported (see the REMIX-TODO list below).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "remix/remix_roster_data.h"
#include <ft/fighter.h>

/* Crash.asm Action.DSPWait and Action.DSPTurn, same values as nFTRemixCrashStatusDSPWait/Turn (ftremixcrashdsp.c) */
#define FTREMIX_HITBOX_CRASH_DSP_WAIT 0xE6
#define FTREMIX_HITBOX_CRASH_DSP_TURN 0xE7

/*
 * REMIX-TODO(singleplayer): Hitbox.asm force_grab_immunity_ also makes Giga Bowser (REMIX_ID_GBOWSER) not
 * grabbable while SinglePlayerModes.singleplayer_mode_flag == REMIX_1P_ID (4). The Remix 1P mode flag is not
 * ported yet, so the Giga Bowser branch is skipped. It stays FALSE until that flag exists.
 *
 * REMIX-TODO(hitbox display): these Hitbox.asm patches are not ported. They are debug display code, and
 * the display-list builders they edit are not ported yet. Default Remix toggles (Special Model Display and
 * Advanced Hurtbox Display, both FALSE) make most of them no-ops, except the display state writes.
 *   projectile_ (0x80167538), grab_immunity_and_armor_frames_ (0x800F279C, Advanced Hurtbox),
 *   make_hurtboxes_transparent_ (0x800F2830, 0x8017193C), make_hitboxes_transparent_ (0x800F33B0),
 *   add_reflect_absorb_hitboxes (0x800F3358, and the dw buffer sizes at 0x8013B9C8, 0x80138CC8,
 *   0x80135B70, 0x80135B80), render_hitbox_plus_model_ (0x800F2FD0), render_hitbox_plus_projectile_
 *   (0x80167578, 0x801675AC), render_hitbox_plus_item_ (9 hooks in itdisplay.c and itmonster/), fix_hitbox_position_
 *   (0x800F3310), star KO display fix (0x800F29A0), skip_rendering_hitboxes_ (0x800F336C), render_ecb_over_hurtboxes_
 *   (0x800F3648), hitbox_mode_ (0x800F2BFC, and the item display state loop).
 */

/**
 * Returns TRUE when the fighter cannot be grabbed for reasons Hitbox.asm force_grab_immunity_ forces:
 * Crash while in Dragon Shot Pursuit wait or turn. Called from ftMainSearchFighterCatch for the victim.
 */
sb32 ftRemixForceGrabImmunity(FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_CRASH)
    {
        return FALSE;
    }
    if (fp->status_id == FTREMIX_HITBOX_CRASH_DSP_WAIT || fp->status_id == FTREMIX_HITBOX_CRASH_DSP_TURN)
    {
        return TRUE;
    }
    return FALSE;
}

#endif
