/*
 * Smash Remix Practice.asm overlay_colour_ (Toggles entry_practice_overlay, Color Overlays): tints each fighter with the
 * colour of its state. Called from ftMainProcPhysicsMapDefault under REGION_REMIX, before the capture check, as the asm
 * hook at 0x800E260C runs before the vanilla capture check.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include "gm/gmremixtoggle.h"

#define FTREMIX_OVERLAY_PLAYERS 4

/* Practice.asm FAF_LANDING, FAF_HARD_LANDING, FAF_TURN_SLOW and FAF_TURN_FAST: first actionable frames */
#define FTREMIX_OVERLAY_FAF_LANDING 4
#define FTREMIX_OVERLAY_FAF_HARD_LANDING 8
#define FTREMIX_OVERLAY_FAF_TURN_SLOW 5
#define FTREMIX_OVERLAY_FAF_TURN_FAST 3

/* Practice.asm colours, RGBA32 with R in the most significant byte (Color.asm high scope, and the CYAN literal) */
#define FTREMIX_OVERLAY_RGBA_RED 0xFF0000FF
#define FTREMIX_OVERLAY_RGBA_YELLOW 0xFFFF00FF
#define FTREMIX_OVERLAY_RGBA_GREEN 0x00FF00FF
#define FTREMIX_OVERLAY_RGBA_BLUE 0x0000FFFF
#define FTREMIX_OVERLAY_RGBA_CYAN 0x00FFFFFF

/* After subtracting 1 from a hitstatus byte: bit 0x2 is intangible, 0 is vulnerable, 0x1 is invulnerable */
#define FTREMIX_OVERLAY_INTANGIBLE_BIT 0x2

/* flag_overlay of Practice.asm: the colour was set on the previous frame, so its bit is cleared before the new check */
static sb32 sFTRemixOverlayIsSet[FTREMIX_OVERLAY_PLAYERS];

/* flag_perfect_land of Practice.asm: the ground/air kinetics of the previous frame, 0 is ground */
static s32 sFTRemixOverlayWasAir[FTREMIX_OVERLAY_PLAYERS];

/**
 * Clears the colour bit when the overlay set it on the previous frame (_reset_flag). The colour value is kept, as in the asm.
 */
static void ftRemixOverlayResetLastFrame(FTStruct *fp)
{
    if (sFTRemixOverlayIsSet[fp->player] == FALSE)
    {
        return;
    }
    sFTRemixOverlayIsSet[fp->player] = FALSE;
    fp->colanim.is_use_color1 = FALSE;
}

/**
 * Stores rgba as the colour of the fighter and marks it as set for the next frame (_store).
 */
static void ftRemixOverlayStoreColour(FTStruct *fp, u32 rgba)
{
    fp->colanim.color1.r = (u8) (rgba >> 24);
    fp->colanim.color1.g = (u8) (rgba >> 16);
    fp->colanim.color1.b = (u8) (rgba >> 8);
    fp->colanim.color1.a = (u8) rgba;
    fp->colanim.is_use_color1 = TRUE;
    sFTRemixOverlayIsSet[fp->player] = TRUE;
}

/**
 * Returns the hitstatus byte as the asm reads it (lbu of the low byte), minus 1 in 32 bits.
 * A zero byte gives 0xFFFFFFFF, which the asm also treats as a non-vulnerable state.
 */
static u32 ftRemixOverlayHitstatusBits(s32 hitstatus)
{
    return (u32) ((u8) hitstatus) - 1;
}

/**
 * Colour of the vulnerability states (_check_vulnerability): BLUE when intangible, GREEN when invulnerable, and FALSE when
 * all three hitstatus inputs are vulnerable.
 */
static sb32 ftRemixOverlayVulnerabilityColour(FTStruct *fp, u32 *rgba)
{
    u32 bits = ftRemixOverlayHitstatusBits(fp->special_hitstatus) | ftRemixOverlayHitstatusBits(fp->star_hitstatus) | ftRemixOverlayHitstatusBits(fp->hitstatus);

    if (bits == 0)
    {
        return FALSE;
    }
    *rgba = (bits & FTREMIX_OVERLAY_INTANGIBLE_BIT) ? FTREMIX_OVERLAY_RGBA_BLUE : FTREMIX_OVERLAY_RGBA_GREEN;
    return TRUE;
}

/**
 * Colour of the hitstun state (_hitstun): RED when the low halfword of damage.hitstun_tics (0x0B1A) is not zero. The
 * knockback check decides this path, and when the counter is zero the asm stops without checking reflect or action.
 */
static sb32 ftRemixOverlayHitstunColour(FTStruct *fp, u32 *rgba)
{
    if ((u16) fp->status_vars.common.damage.hitstun_tics == 0)
    {
        return FALSE;
    }
    *rgba = FTREMIX_OVERLAY_RGBA_RED;
    return TRUE;
}

/**
 * Returns the fast turn FAF for Samus and the slow one for everyone else (_turn). The asm compares the fast FAF register
 * with the fighter kind for Falcon, so Falcon gets the slow FAF; this matches the Remix build, not its comment.
 */
static s32 ftRemixOverlayTurnFAF(FTStruct *fp)
{
    if (fp->fkind == nFTKindSamus)
    {
        return FTREMIX_OVERLAY_FAF_TURN_FAST;
    }
    return FTREMIX_OVERLAY_FAF_TURN_SLOW;
}

/**
 * YELLOW when the action frame count has reached faf (_neutral_faf). Earlier frames show no overlay.
 */
static sb32 ftRemixOverlayNeutralColour(FTStruct *fp, s32 faf, u32 *rgba)
{
    if (faf > (s32) fp->status_total_tics)
    {
        return FALSE;
    }
    *rgba = FTREMIX_OVERLAY_RGBA_YELLOW;
    return TRUE;
}

/**
 * YELLOW on the standing action, when the fighter was on the ground on the previous frame (_stand). No FAF check applies.
 */
static sb32 ftRemixOverlayStandColour(FTStruct *fp, u32 *rgba)
{
    if (sFTRemixOverlayWasAir[fp->player] != 0)
    {
        return FALSE;
    }
    *rgba = FTREMIX_OVERLAY_RGBA_YELLOW;
    return TRUE;
}

/**
 * Colour of the current action (_check_action): standing, turn, light landing and heavy landing. Other actions show nothing.
 */
static sb32 ftRemixOverlayActionColour(FTStruct *fp, u32 *rgba)
{
    switch (fp->status_id)
    {
    case nFTCommonStatusWait:
        return ftRemixOverlayStandColour(fp, rgba);

    case nFTCommonStatusTurn:
        return ftRemixOverlayNeutralColour(fp, ftRemixOverlayTurnFAF(fp), rgba);

    case nFTCommonStatusLandingLight:
        return ftRemixOverlayNeutralColour(fp, FTREMIX_OVERLAY_FAF_LANDING, rgba);

    case nFTCommonStatusLandingHeavy:
        return ftRemixOverlayNeutralColour(fp, FTREMIX_OVERLAY_FAF_HARD_LANDING, rgba);

    default:
        return FALSE;
    }
}

/**
 * Colour of the reflect or absorb spheres (_check_reflect_absorb): CYAN, from the is_reflect and is_absorb bits that the asm
 * reads as the 0x0480 halfword mask at 0x018C.
 */
static sb32 ftRemixOverlayReflectColour(FTStruct *fp, u32 *rgba)
{
    if (fp->is_reflect == FALSE && fp->is_absorb == FALSE)
    {
        return FALSE;
    }
    *rgba = FTREMIX_OVERLAY_RGBA_CYAN;
    return TRUE;
}

/**
 * Colour of the fighter state, in the priority order of the asm: vulnerability, then knockback (hitstun), then reflect or
 * absorb, then the action. The knockback check uses the f32 at 0x07EC, nonzero as the asm tests it.
 */
static sb32 ftRemixOverlayStateColour(FTStruct *fp, u32 *rgba)
{
    if (ftRemixOverlayVulnerabilityColour(fp, rgba))
    {
        return TRUE;
    }
    if (fp->damage_knockback_stack != 0.0F)
    {
        return ftRemixOverlayHitstunColour(fp, rgba);
    }
    if (ftRemixOverlayReflectColour(fp, rgba))
    {
        return TRUE;
    }
    return ftRemixOverlayActionColour(fp, rgba);
}

/**
 * Applies the Color Overlays colour to a fighter each frame (overlay_colour_). Does nothing when the toggle is off, as the
 * asm guard returns before any state change.
 */
void ftRemixOverlayColour(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    u32 rgba;

    if (gmRemixToggle(practice_overlay) == FALSE)
    {
        return;
    }
    ftRemixOverlayResetLastFrame(fp);
    if (ftRemixOverlayStateColour(fp, &rgba))
    {
        ftRemixOverlayStoreColour(fp, rgba);
    }
    sFTRemixOverlayWasAir[fp->player] = fp->ga;
}

#endif
