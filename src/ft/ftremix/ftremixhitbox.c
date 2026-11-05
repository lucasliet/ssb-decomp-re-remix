/**
 * ftremixhitbox.c - Smash Remix Hitbox.asm: force grab immunity, and the display-list builders of the hitbox display
 * (Advanced Hurtbox colours, outlined hurtboxes in hitbox+model, reflect and absorb hitbox).
 *
 * Not ported: the Kirby Cube (Hitbox.asm kirby_cube_enabled, state 4 of hitbox_mode_, and the cube branches of
 * make_hurtboxes_transparent_, fix_hitbox_position_, skip_rendering_hitboxes_ 0x800F336C and render_ecb_over_hurtboxes_
 * 0x800F3648). kirby_cube_enabled is set by Hitbox.setup_ only when the text of Toggles.entry_player_tags_7 (the 8th
 * Player Tag, a free text the user edits) reads "cube" (0x63756265), and the cube model is the Remix file 0x124F
 * (File.KIRBY_CUBE). The port has no editable Player Tag text (its Player Tag panel is a number, remix_player_panel.cpp),
 * so kirby_cube_enabled is always FALSE. With it FALSE those four patches are no-ops: skip_rendering_hitboxes_ and
 * render_ecb_over_hurtboxes_ only branch when the display state is 4 or 0x800F3648 is reached with Kirby or J Kirby, and
 * state 4 is only written by hitbox_mode_ under kirby_cube_enabled.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "ft/ftremix/ftremixhitbox.h"
#include "ft/ftremix/ftremixreflect.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_roster_data.h"
#include <ft/fighter.h>
#include <sys/malloc.h>
#include <sys/matrix.h>
#include <sys/taskman.h>
#include "sc/sc1pmode/sc1premixmode.h"
#include "sc/sc1pmode/sc1premixmodename.h"

/* Crash.asm Action.DSPWait and Action.DSPTurn, same values as nFTRemixCrashStatusDSPWait/Turn (ftremixcrashdsp.c) */
#define FTREMIX_HITBOX_CRASH_DSP_WAIT 0xE6
#define FTREMIX_HITBOX_CRASH_DSP_TURN 0xE7

/* Hitbox.asm blend color alpha of the outlined hitboxes (setblendcolor 0x000000E0) */
#define FTREMIX_HITBOX_BLEND_ALPHA 0xE0

/* Hitbox.asm add_reflect_absorb_hitboxes colours (RGBA32): reflect FF8040FF, absorb FF4080FF */
#define FTREMIX_HITBOX_REFLECT_R 0xFF
#define FTREMIX_HITBOX_REFLECT_G 0x80
#define FTREMIX_HITBOX_REFLECT_B 0x40
#define FTREMIX_HITBOX_ABSORB_R 0xFF
#define FTREMIX_HITBOX_ABSORB_G 0x40
#define FTREMIX_HITBOX_ABSORB_B 0x80

/* Reflect.asm custom reflect id 0 is the Franklin Badge, which reflects; the other ids (Marina, Dedede) absorb */
#define FTREMIX_HITBOX_REFLECT_ID_FRANKLIN_BADGE 0

extern SYMallocRegion gSYTaskmanGraphicsHeap;

/**
 * Returns TRUE when the fighter cannot be grabbed for reasons Hitbox.asm force_grab_immunity_ forces: Giga Bowser
 * while the Remix 1P mode is active (singleplayer_mode_flag == 4, any controller, as the asm has no player type check),
 * and Crash while in Dragon Shot Pursuit wait or turn. Called from ftMainSearchFighterCatch for the victim.
 *
 * @param fp The victim fighter struct.
 * @return TRUE when the grab must not connect.
 */
sb32 ftRemixForceGrabImmunity(FTStruct *fp)
{
    if (fp->fkind == REMIX_ID_GBOWSER)
    {
        return sc1PRemixModeIs(SC1PREMIX_MODE_REMIX_1P);
    }
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

/**
 * Hitbox.asm grab_immunity_and_armor_frames_ (RAM 0x800F279C, the Normal case of the hurtbox colour switch). With the
 * Advanced Hurtbox Display toggle on, the env colour is cyan 00FFFF while the fighter reflects or absorbs (bits 0x0480 of
 * the flag halfword at FTStruct 0x018C), else gray 606060 while the knockback resist word (FTStruct 0x07E8) is non-zero,
 * else dark cyan 00D0D0 when the hurtbox cannot be grabbed (FTDamageColl 0x10 is zero). Otherwise it stays D0D000.
 *
 * @param fp The fighter struct.
 * @param damage_coll The hurtbox being drawn.
 */
void ftRemixHitboxDisplayNormalHurtboxColor(FTStruct *fp, FTDamageColl *damage_coll)
{
    u8 red = 0xD0;
    u8 green = 0xD0;
    u8 blue = 0x00;

    if (gmRemixToggle(advanced_hurtbox) != FALSE)
    {
        if ((fp->is_reflect != FALSE) || (fp->is_absorb != FALSE))
        {
            red = 0x00;
            green = 0xFF;
            blue = 0xFF;
        }
        else if (fp->knockback_resist_status != 0.0F)
        {
            red = green = blue = 0x60;
        }
        else if (damage_coll->is_grabbable == FALSE)
        {
            red = 0x00;
            green = 0xD0;
            blue = 0xD0;
        }
    }
    gDPSetEnvColor(gSYTaskmanDLHeads[0]++, red, green, blue, 0xFF);
}

/**
 * Hitbox.asm make_hurtboxes_transparent_ (RAM 0x800F2830, 0x8017193C). In hitbox+model the hurtbox is drawn as an outline:
 * the prim color command just emitted takes the env color, a blend color with alpha E0 is added, and the display list is
 * the hitbox cube (RAM 0x8012C310, the outline-only list) instead of the hurtbox cuboid (RAM 0x8012C058). The prim and
 * env color commands must be the last two commands of display list head 0, as at both asm hooks. Any other mode draws
 * the cuboid.
 */
void ftRemixHitboxDisplayDrawHurtbox(void)
{
    Gfx *list_end;

    if (ftRemixHitboxDisplayIsOverlay() == FALSE)
    {
        gSPDisplayList(gSYTaskmanDLHeads[0]++, dFTDisplayMainHurtCollisionCuboidDL);
        return;
    }
    list_end = gSYTaskmanDLHeads[0];
    list_end[-2].words.w1 = list_end[-1].words.w1;
    gDPSetBlendColor(gSYTaskmanDLHeads[0]++, 0x00, 0x00, 0x00, FTREMIX_HITBOX_BLEND_ALPHA);
    gSPDisplayList(gSYTaskmanDLHeads[0]++, dFTDisplayMainHitCollisionCubeDL);
}

/**
 * Returns TRUE when the reflect hitbox is drawn with the absorb color: the fighter absorbs, or its reflect hitbox is a
 * Reflect.asm custom one that absorbs (Marina and Dedede, the "hacky fix" of add_reflect_absorb_hitboxes).
 *
 * @param fp The fighter struct, with a special collision.
 * @return TRUE for the absorb color.
 */
static sb32 ftRemixHitboxDisplaySpecialCollIsAbsorb(FTStruct *fp)
{
    u32 custom_id = (u32)fp->special_coll->kind >> 16;

    if (fp->is_absorb != FALSE)
    {
        return TRUE;
    }
    return ftRemixReflectIsCustom(fp) && (custom_id != FTREMIX_HITBOX_REFLECT_ID_FRANKLIN_BADGE);
}

/**
 * Emits the colors and the hitbox cube of the reflect or absorb hitbox: pipe sync, blend color alpha E0, prim color of
 * the reflect or absorb, env color white, and the outline-only hitbox cube list (RAM 0x8012C310).
 *
 * @param fp The fighter struct, with a special collision.
 */
static void ftRemixHitboxDisplayEmitSpecialColl(FTStruct *fp)
{
    gDPPipeSync(gSYTaskmanDLHeads[0]++);
    gDPSetBlendColor(gSYTaskmanDLHeads[0]++, 0x00, 0x00, 0x00, FTREMIX_HITBOX_BLEND_ALPHA);

    if (ftRemixHitboxDisplaySpecialCollIsAbsorb(fp) != FALSE)
    {
        gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, FTREMIX_HITBOX_ABSORB_R, FTREMIX_HITBOX_ABSORB_G, FTREMIX_HITBOX_ABSORB_B, 0xFF);
    }
    else gDPSetPrimColor(gSYTaskmanDLHeads[0]++, 0, 0, FTREMIX_HITBOX_REFLECT_R, FTREMIX_HITBOX_REFLECT_G, FTREMIX_HITBOX_REFLECT_B, 0xFF);

    gDPSetEnvColor(gSYTaskmanDLHeads[0]++, 0xFF, 0xFF, 0xFF, 0xFF);
    gSPDisplayList(gSYTaskmanDLHeads[0]++, dFTDisplayMainHitCollisionCubeDL);
}

/**
 * Hitbox.asm add_reflect_absorb_hitboxes (RAM 0x800F3358). While the fighter reflects or absorbs (flag bits 0x04800000 at
 * FTStruct 0x018C) the hitbox of its special collision (FTStruct 0x0850) is drawn: at the offset of the special collision
 * from its joint, scaled by size / 15 (the asm divides by 0x41700000, 15.0). The asm builds the joint matrix chain by
 * hand after the parts; here it is drawn while ftDisplayMainDrawParts has the joint matrix on the stack, like a hurtbox.
 *
 * @param fp The fighter struct.
 * @param dobj The joint being traversed.
 */
void ftRemixHitboxDisplayDrawSpecialColl(FTStruct *fp, DObj *dobj)
{
    FTSpecialColl *special_coll = fp->special_coll;
    Mtx *m;

    if (special_coll == NULL)
    {
        return;
    }
    if ((fp->is_reflect == FALSE) && (fp->is_absorb == FALSE))
    {
        return;
    }
    if ((special_coll->joint_id < 0) || (special_coll->joint_id >= FTPARTS_JOINT_NUM_MAX))
    {
        return;
    }
    if (fp->joints[special_coll->joint_id] != dobj)
    {
        return;
    }
    syMatrixAdvanceW(m, gSYTaskmanGraphicsHeap);

    syMatrixTra(m, special_coll->offset.x, special_coll->offset.y, special_coll->offset.z);

    gSPMatrix(gSYTaskmanDLHeads[0]++, m, G_MTX_PUSH | G_MTX_MUL | G_MTX_MODELVIEW);

    syMatrixAdvanceW(m, gSYTaskmanGraphicsHeap);

    syMatrixSca(m, special_coll->size.x / 15.0F, special_coll->size.y / 15.0F, special_coll->size.z / 15.0F);

    gSPMatrix(gSYTaskmanDLHeads[0]++, m, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_MODELVIEW);

    ftRemixHitboxDisplayEmitSpecialColl(fp);

    gSPPopMatrix(gSYTaskmanDLHeads[0]++, G_MTX_MODELVIEW);
}

#endif
