/**
 * ftremixsonicusp.c - C ports of Smash Remix Sonic Spring Jump (up special)
 * routines (.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm, scope
 * SonicUSP, and the spring projectile data in the same file).
 */

#ifdef PORT

#include "ft/ftremix/ftremix.h"
#include "remix/remix_symbols.h"
#include <ef/efmanager.h>
#include <ft/ftmain.h>
#include <ft/ftphysics.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <gm/gmcollision.h>
#include <gm/gmrumble.h>
#include <mp/mpcommon.h>
#include <sys/objman.h>
#include <sys/utils.h>
#include <wp/weapon.h>
#include <wp/wpdef.h>
#include <wp/wpmain.h>
#include <wp/wpmanager.h>

extern void *func_800269C0_275C0(u16 id);
extern sb32 ftRemixSonicIsClassic(s32 player);          /* classic-table reader owned by ftremixsonic.c */
extern sb32 wpSamusBombProcMap(GObj *weapon_gobj);
extern sb32 wpFoxBlasterProcReflector(GObj *weapon_gobj);

enum
{
    nFTRemixSonicStatusSpring = 0xE4
};

/* Remix action ids from Action.asm and the character files. */
enum
{
    nFTRemixSonicUSPJumpF = 0x016,
    nFTRemixSonicUSPPass = 0x021,
    nFTRemixSonicUSPTumble = 0x039,
    nFTRemixSonicUSPFallSpecial = 0x03A,
    nFTRemixSonicUSPShieldBreak = 0x09E,
    nFTRemixSonicUSPShieldBreakFall = 0x09F,
    nFTRemixSonicUSPInhalePulled = 0x0AD,
    nFTRemixSonicUSPFoxFireAir = 0x0E8,
    nFTRemixSonicUSPReflectorStartAir = 0x0F1,
    nFTRemixSonicUSPReflectorTurnAir = 0x0F5,
    nFTRemixSonicUSPKirbyFinalCutter = 0x100,
    nFTRemixSonicUSPKirbyFinalCutterAir = 0x102,
    nFTRemixSonicUSPKirbyFinalCutterFall = 0x103,
    nFTRemixSonicUSPKirbyStoneFall = 0x109,
    nFTRemixSonicUSPYoshiGroundPoundDrop = 0x0E3,
    nFTRemixSonicUSPBowserBombDrop = 0x0E3,
    nFTRemixSonicUSPBowserForwardThrow1 = 0x0E5,
    nFTRemixSonicUSPBowserForwardThrow2 = 0x0E6,
    nFTRemixSonicUSPBowserForwardThrow3 = 0x0E8,
    nFTRemixSonicUSPNessPsiMagnetStartAir = 0x0F1,
    nFTRemixSonicUSPNessPsiMagnetEndAir = 0x0F4,
    nFTRemixSonicUSPNessPKThunderStartAir = 0x0E8,
    nFTRemixSonicUSPNessPKTAAir = 0x0EC,
    nFTRemixSonicUSPMarthDSPGA = 0x0F1,
    nFTRemixSonicUSPMarthDSPGAAttack = 0x0F2,
    nFTRemixSonicUSPConkerHelicopterAir = 0x0E4,
    nFTRemixSonicUSPConkerHelicopterDescent = 0x0E6
};

/* What the spring does to a player it touches (asm _check_action). */
typedef enum FTRemixSonicSpringBounce
{
    nFTRemixSonicSpringBounceSkip,
    nFTRemixSonicSpringBounceSmoke,
    nFTRemixSonicSpringBounceChangeAction
} FTRemixSonicSpringBounce;

/* Per-player spring state. Remix keeps the spring GObj pointer in temp variable 3
 * and its enable flag in the GObj at 0x40; LP64 cannot hold the pointer in
 * flag2, so both live here. */
/* Per-spring state, kept in the spring's own WPStruct.weapon_vars so an older spring stays valid after a new up special. */
typedef struct FTRemixSonicUSPSpring
{
    s32 fgm_id;
    sb32 is_bounce_enabled;
} FTRemixSonicUSPSpring;

_Static_assert(sizeof(FTRemixSonicUSPSpring) <= sizeof(((WPStruct*)0)->weapon_vars), "FTRemixSonicUSPSpring must fit in WPStruct.weapon_vars");

#define SONIC_USP_Y_SPEED 129.0F                       /* 0x43010000 */
#define SONIC_USP_SPRING_LIFT 144.0F                   /* 0x43100000 */
#define SONIC_USP_SPRING_DROP_Y_OFFSET -228.0F         /* 0xC3640000 */
#define SONIC_USP_SPRING_LAUNCH_Y_SPEED 130.0F         /* 0x43020000 */
#define SONIC_USP_SPRING_DURATION 240
#define SONIC_USP_SPRING_MAX_SPEED 100.0F
#define SONIC_USP_SPRING_GRAVITY 3.0F
#define SONIC_USP_SPRING_ROTATION_SPEED 0.1875F        /* 0x3E400000 */
#define SONIC_USP_SPRING_RUMBLE_LENGTH 5
#define SONIC_USP_SPRING_FGM 0x3D7                     /* FGM.SPRING */
#define SONIC_USP_SPRING_FGM_CLASSIC 0x3DF             /* FGM.CLASSIC_SPRING, classic Sonic's spring */
#define SONIC_USP_SPRING_IMAGE_COILED 1                /* spring coiled image and palette */
#define SONIC_USP_SPRING_IMAGE_COILED_CLASSIC 3        /* classic spring coiled image and palette */
#define SONIC_USP_SPRING_JOINT 4                       /* FTStruct.joints[4] at N64 0x08F8, the body part */
#define SONIC_USP_SPRING_TRANSFORM_TK2 0x47
/* Remix Projectile.id.SONIC_SPRING. Beyond the vanilla nWPKind enum (ends 0x1F).
 * wp->kind is only stored; nothing in decomp/src/wp indexes a table by it. */
/* SonicSpecial.asm change_spring_room_ (0x80165C2C): the spring is displayed in room 0x0B instead of the weapon room 0x0E, so it is z-buffered correctly */
#define SONIC_USP_SPRING_DL_LINK 0x0B
#define SONIC_USP_SPRING_WPKIND 0x20

/* Remix keeps these at 0x0B18 (movement flag) and 0x0B1C (grounded flag). They
 * are only read by the USP, which resets both in air_initial_, so per-player
 * statics match the Remix behaviour. */
static s32 sFTRemixSonicUSPMovement[MAXCONTROLLERS];
static s32 sFTRemixSonicUSPGrounded[MAXCONTROLLERS];
static GObj *sFTRemixSonicUSPSpringGObjs[MAXCONTROLLERS];

/* Up special bool at 0x0ADC, which is the passive_vars union in the decomp.
 * Remix shares it with the SSonic counter, so it stays in fighter memory. */
static s32 *ftRemixSonicUSPUpSpecialBool(FTStruct *fp)
{
    return (s32 *)&fp->passive_vars;
}

/* Read by the vanilla action check patch (SonicUSP.action_check_patch_). */
sb32 ftRemixSonicUSPIsUpSpecialUsed(FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_SONIC)
    {
        return FALSE;
    }
    return *ftRemixSonicUSPUpSpecialBool(fp) != FALSE;
}

/* Clears the air flags that the Remix spring code writes to byte 0x018D (asm `andi 0x0007`). */
static void ftRemixSonicUSPClearAirFlags(FTStruct *fp)
{
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* SonicUSP.air_initial_: sets the Spring status and resets the USP state. */
static void ftRemixSonicUSPAirInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftMainSetStatus(fighter_gobj, nFTRemixSonicStatusSpring, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp->motion_vars.flags.flag0 = 0;
    fp->motion_vars.flags.flag1 = 0;
    fp->motion_vars.flags.flag2 = 1;
    sFTRemixSonicUSPMovement[fp->player] = FALSE;
    sFTRemixSonicUSPGrounded[fp->player] = FALSE;
    sFTRemixSonicUSPSpringGObjs[fp->player] = NULL;

    fp->physics.vel_air.x = 0.0F;
    fp->physics.vel_air.y = 0.0F;

    ftRemixSonicUSPClearAirFlags(fp);
}

/* SonicUSP.ground_initial_: lifts Sonic onto the spring, then starts the aerial state. */
static void ftRemixSonicUSPGroundInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    DObj *top_joint;
    s32 floor_line_id;

    mpCommonSetFighterAir(fp);

    top_joint = DObjGetStruct(fighter_gobj);
    top_joint->translate.vec.f.y += SONIC_USP_SPRING_LIFT;
    ftRemixSonicUSPAirInitial(fighter_gobj);

    sFTRemixSonicUSPGrounded[fp->player] = TRUE;

    /* Byte 0x186 is the floor line ID and byte 0x187 is 1 when the spring starts
     * in the air (no floor line ID). The Remix spring creation reads both. */
    floor_line_id = fp->coll_data.floor_line_id;
    fp->motion_vars.flags.flag2 = ((u32)(floor_line_id & 0xFF) << 8) | (floor_line_id < 0);
}

/* SonicUSP._begin_movement: starts the upward movement once the spring has uncoiled. */
static void ftRemixSonicUSPBeginMovement(FTStruct *fp)
{
    fp->physics.vel_air.y = SONIC_USP_Y_SPEED;
    fp->motion_vars.flags.flag0 = 0;
    sFTRemixSonicUSPMovement[fp->player] = TRUE;
    *ftRemixSonicUSPUpSpecialBool(fp) = TRUE;

    if (sFTRemixSonicUSPGrounded[fp->player] == FALSE)
    {
        fp->jumps_used = fp->attr->jumps_max;
    }
}

/* Returns the spring state for a spring GObj, or NULL if the player's slot holds another spring. */
static FTRemixSonicUSPSpring *ftRemixSonicUSPSpringOfGObj(GObj *spring_gobj)
{
    return (FTRemixSonicUSPSpring*)&wpGetStruct(spring_gobj)->weapon_vars;
}

/* Walks the live weapon list, so a spring that was destroyed by a hit or a wall is never dereferenced. */
static sb32 ftRemixSonicUSPSpringIsAlive(GObj *spring_gobj)
{
    GObj *weapon_gobj = gGCCommonLinks[nGCCommonLinkIDWeapon];

    while (weapon_gobj != NULL)
    {
        if (weapon_gobj == spring_gobj)
        {
            return TRUE;
        }
        weapon_gobj = weapon_gobj->link_next;
    }
    return FALSE;
}

/* Sets the spring's image index and the palette, which Remix writes as the same value. */
static void ftRemixSonicUSPSetSpringImage(MObj *image, s32 index)
{
    image->texture_id_curr = (u16)index;
    image->palette_id = (f32)index;
}

/* Switches a coiled spring (odd image index) to uncoiled. Returns TRUE if it was coiled. */
static sb32 ftRemixSonicUSPUncoilSpring(GObj *spring_gobj)
{
    MObj *image = DObjGetStruct(spring_gobj)->mobj;
    s32 index = image->texture_id_curr;

    if ((index & 1) == 0)
    {
        return FALSE;
    }
    ftRemixSonicUSPSetSpringImage(image, index - 1);
    return TRUE;
}

/* Switches an uncoiled spring (even image index) to coiled. Returns TRUE if it was uncoiled. */
static sb32 ftRemixSonicUSPCoilSpring(GObj *spring_gobj)
{
    MObj *image = DObjGetStruct(spring_gobj)->mobj;
    s32 index = image->texture_id_curr;

    if ((index & 1) != 0)
    {
        return FALSE;
    }
    ftRemixSonicUSPSetSpringImage(image, index + 1);
    return TRUE;
}

/* Clears the spring's hit records. Remix writes the top byte (0xE0): hurt, shield and reflect set, the rest zero. */
static void ftRemixSonicUSPSpringClearHitRecords(WPStruct *wp)
{
    s32 record_index;
    GMAttackRecord *record;

    for (record_index = 0; record_index < GMATTACKREC_NUM_MAX; record_index++)
    {
        record = &wp->attack_coll.attack_records[record_index];
        record->victim_gobj = NULL;
        record->victim_flags.is_interact_hurt = TRUE;
        record->victim_flags.is_interact_shield = TRUE;
        record->victim_flags.is_interact_reflect = TRUE;
        record->victim_flags.is_interact_absorb = FALSE;
        record->victim_flags.group_id = 0;
    }
}

/* SonicUSP.spring_main_ expiry: smoke at the spring position. The manager destroys the GObj. */
static void ftRemixSonicUSPSpringExpire(GObj *spring_gobj)
{
    efManagerDustExpandSmallMakeEffect(&DObjGetStruct(spring_gobj)->translate.vec.f, 1.0F);
}

/* SonicUSP.spring_main_, grounded branch: flattens the spring, removes its hitbox and enables bounce unless it was coiled. */
static void ftRemixSonicUSPSpringSettle(GObj *spring_gobj, FTRemixSonicUSPSpring *spring)
{
    WPStruct *wp = wpGetStruct(spring_gobj);
    sb32 was_coiled;

    DObjGetStruct(spring_gobj)->rotate.vec.f.x = 0.0F;
    wp->attack_coll.attack_count = 0;
    ftRemixSonicUSPSpringClearHitRecords(wp);

    spring->is_bounce_enabled = TRUE;
    was_coiled = ftRemixSonicUSPUncoilSpring(spring_gobj);
    if (was_coiled)
    {
        spring->is_bounce_enabled = FALSE;
    }
}

/* SonicUSP.spring_main_, after the first 4 frames: hitbox on, then spin in the air or settle on the ground. */
static void ftRemixSonicUSPSpringFlightStep(GObj *spring_gobj, FTRemixSonicUSPSpring *spring)
{
    WPStruct *wp = wpGetStruct(spring_gobj);

    wp->attack_coll.attack_count = 1;
    if (wp->ga != FALSE)
    {
        DObjGetStruct(spring_gobj)->rotate.vec.f.x += SONIC_USP_SPRING_ROTATION_SPEED;
        return;
    }
    ftRemixSonicUSPSpringSettle(spring_gobj, spring);
}

/* True when `action` lies in the inclusive range [first, last]. */
static sb32 ftRemixSonicUSPSpringActionInRange(s32 action, s32 first, s32 last)
{
    return (action >= first) && (action <= last);
}

/* Fox, JFox and Falco: fire-air and reflector actions. */
static s32 ftRemixSonicUSPSpringFoxFalcoBounce(s32 action)
{
    if ((action == nFTRemixSonicUSPFoxFireAir) ||
        ftRemixSonicUSPSpringActionInRange(action, nFTRemixSonicUSPReflectorStartAir, nFTRemixSonicUSPReflectorTurnAir))
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    return nFTRemixSonicSpringBounceSmoke;
}

/* SonicUSP._psi_magnet (SonicSpecial.asm 1887-1896): clears the fighter words the Remix
 * psi magnet overlay uses, so the glow and loop SFX stop when the bounce changes
 * the action to JumpF. The asm clears raw offsets 0x0A20-0x0A30, the 0x018C
 * halfword and 0x0A88; in the port those are p_loop_sfx/loop_sfx_id,
 * colanim.cs[0] script state, the first 16 flags after motion_vars and the
 * colanim use-flags. */
static void ftRemixSonicUSPClearPsiMagnetOverlay(FTStruct *fp)
{
    fp->p_loop_sfx = NULL;
    fp->loop_sfx_id = 0;

    fp->colanim.cs[0].p_script = NULL;
    fp->colanim.cs[0].p_subroutine[0] = NULL;

    fp->is_attack_active = FALSE;
    fp->is_hitstatus_nodamage = FALSE;
    fp->is_damage_coll_modify = FALSE;
    fp->is_modelpart_modify = FALSE;
    fp->is_texturepart_modify = FALSE;
    fp->is_reflect = FALSE;
    fp->reflect_lr = 0;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
    fp->is_magnify_show = FALSE;
    fp->is_limit_map_bounds = FALSE;
    fp->is_invisible = FALSE;

    fp->colanim.is_use_color1 = FALSE;
    fp->colanim.is_use_light = FALSE;
    fp->colanim.is_use_color2 = FALSE;
    fp->colanim.skeleton_id = 0;
}

/* Ness, JNess and Lucas: psi magnet and PK thunder actions. */
static s32 ftRemixSonicUSPSpringNessLucasBounce(FTStruct *fp, s32 action)
{
    if (ftRemixSonicUSPSpringActionInRange(action, nFTRemixSonicUSPNessPsiMagnetStartAir, nFTRemixSonicUSPNessPsiMagnetEndAir))
    {
        ftRemixSonicUSPClearPsiMagnetOverlay(fp);
        return nFTRemixSonicSpringBounceChangeAction;
    }
    if (ftRemixSonicUSPSpringActionInRange(action, nFTRemixSonicUSPNessPKThunderStartAir, nFTRemixSonicUSPNessPKTAAir))
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    return nFTRemixSonicSpringBounceSmoke;
}

/* Kirby and JKirby: Final Cutter and Stone fall actions. */
static s32 ftRemixSonicUSPSpringKirbyBounce(s32 action)
{
    if ((action == nFTRemixSonicUSPKirbyFinalCutter) || (action == nFTRemixSonicUSPKirbyFinalCutterAir) ||
        (action == nFTRemixSonicUSPKirbyFinalCutterFall) || (action == nFTRemixSonicUSPKirbyStoneFall))
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    return nFTRemixSonicSpringBounceSmoke;
}

/* Bowser and Giga Bowser: bomb drop changes action, the forward throws skip the spring. */
static s32 ftRemixSonicUSPSpringBowserBounce(s32 action)
{
    if (action == nFTRemixSonicUSPBowserBombDrop)
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    if ((action == nFTRemixSonicUSPBowserForwardThrow1) || (action == nFTRemixSonicUSPBowserForwardThrow2) ||
        (action == nFTRemixSonicUSPBowserForwardThrow3))
    {
        return nFTRemixSonicSpringBounceSkip;
    }
    return nFTRemixSonicSpringBounceSmoke;
}

/* Character-specific part of the bounce check (asm `_keep_action`). */
static s32 ftRemixSonicUSPSpringCharacterBounce(FTStruct *fp, s32 action)
{
    switch (fp->fkind)
    {
        case nFTKindFox:
        case REMIX_ID_JFOX:
        case REMIX_ID_FALCO:
            return ftRemixSonicUSPSpringFoxFalcoBounce(action);
        case REMIX_ID_PEPPY:
            return (action == nFTRemixSonicUSPFoxFireAir) ? nFTRemixSonicSpringBounceChangeAction : nFTRemixSonicSpringBounceSmoke;
        case nFTKindKirby:
        case REMIX_ID_JKIRBY:
            return ftRemixSonicUSPSpringKirbyBounce(action);
        case nFTKindYoshi:
        case REMIX_ID_JYOSHI:
            return (action == nFTRemixSonicUSPYoshiGroundPoundDrop) ? nFTRemixSonicSpringBounceChangeAction : nFTRemixSonicSpringBounceSmoke;
        case REMIX_ID_BOWSER:
        case REMIX_ID_GBOWSER:
            return ftRemixSonicUSPSpringBowserBounce(action);
        case nFTKindNess:
        case REMIX_ID_JNESS:
        case REMIX_ID_LUCAS:
            return ftRemixSonicUSPSpringNessLucasBounce(fp, action);
        case REMIX_ID_MARTH:
            return ((action == nFTRemixSonicUSPMarthDSPGA) || (action == nFTRemixSonicUSPMarthDSPGAAttack)) ?
                nFTRemixSonicSpringBounceChangeAction : nFTRemixSonicSpringBounceSmoke;
        case REMIX_ID_CONKER:
            return ((action == nFTRemixSonicUSPConkerHelicopterAir) || (action == nFTRemixSonicUSPConkerHelicopterDescent)) ?
                nFTRemixSonicSpringBounceChangeAction : nFTRemixSonicSpringBounceSmoke;
        default:
            return nFTRemixSonicSpringBounceSmoke;
    }
}

/* Asm `_check_action`: the action decides whether the spring changes the action, adds smoke, or is skipped. */
static s32 ftRemixSonicUSPSpringBounceKind(FTStruct *fp)
{
    s32 action = fp->status_id;

    if (ftRemixSonicUSPSpringActionInRange(action, nFTRemixSonicUSPJumpF, nFTRemixSonicUSPPass))
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    if (ftRemixSonicUSPSpringActionInRange(action, nFTRemixSonicUSPTumble, nFTRemixSonicUSPFallSpecial))
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    if ((action == nFTRemixSonicUSPShieldBreak) || (action == nFTRemixSonicUSPShieldBreakFall))
    {
        return nFTRemixSonicSpringBounceChangeAction;
    }
    if (action == nFTRemixSonicUSPInhalePulled)
    {
        return nFTRemixSonicSpringBounceSkip;
    }
    return ftRemixSonicUSPSpringCharacterBounce(fp, action);
}

/* Sets the player to JumpF and plays its animation events (asm `_change_action`). */
static void ftRemixSonicUSPSpringChangeToJumpF(GObj *player_gobj)
{
    ftMainSetStatus(player_gobj, nFTRemixSonicUSPJumpF, 0.0F, 1.0F, 0);
    ftMainPlayAnimEventsAll(player_gobj);
}

/* Jump smoke on the player's top joint (asm `_draw_smoke_gfx`). */
static void ftRemixSonicUSPSpringJumpSmoke(DObj *player_dobj)
{
    efManagerDustHeavyDoubleMakeEffect(&player_dobj->translate.vec.f, 1, 1.0F);
}

/* Rumble for human players only (asm `_end_rumble`). */
static void ftRemixSonicUSPSpringRumble(FTStruct *fp)
{
    if (fp->pkind != nFTPlayerKindMan)
    {
        return;
    }
    gmRumbleSetPlayerRumbleParams(fp->player, 0, SONIC_USP_SPRING_RUMBLE_LENGTH);
}

/* Clears the up special bool for Sonic, Super Sonic and Ebisumaru (asm `_next` restore). */
static void ftRemixSonicUSPSpringRestoreSpecial(FTStruct *fp)
{
    if ((fp->fkind == REMIX_ID_SONIC) || (fp->fkind == REMIX_ID_SSONIC) || (fp->fkind == REMIX_ID_EBI))
    {
        *ftRemixSonicUSPUpSpecialBool(fp) = FALSE;
    }
}

/* Sets the launch velocity and plays the spring sound, the rumble and the restore (asm `_set_y_velocity`). */
static void ftRemixSonicUSPSpringLaunch(GObj *player_gobj, FTRemixSonicUSPSpring *spring)
{
    FTStruct *fp = ftGetStruct(player_gobj);

    fp->physics.vel_air.y = SONIC_USP_SPRING_LAUNCH_Y_SPEED;
    fp->physics.vel_damage_air.y = 0.0F;
    ftRemixSonicUSPClearAirFlags(fp);

    func_800269C0_275C0((u16)spring->fgm_id);
    ftRemixSonicUSPSpringRumble(fp);
    ftRemixSonicUSPSpringRestoreSpecial(fp);
}

/* Snaps the player onto the spring, coils it and applies the bounce for the player's action (asm `_check_action` onward). */
static void ftRemixSonicUSPSpringBounce(GObj *player_gobj, GObj *spring_gobj, FTRemixSonicUSPSpring *spring)
{
    FTStruct *fp = ftGetStruct(player_gobj);
    DObj *player_dobj = DObjGetStruct(player_gobj);
    s32 kind;

    player_dobj->translate.vec.f.y = DObjGetStruct(spring_gobj)->translate.vec.f.y;
    ftRemixSonicUSPCoilSpring(spring_gobj);

    kind = ftRemixSonicUSPSpringBounceKind(fp);
    if (kind == nFTRemixSonicSpringBounceSkip)
    {
        return;
    }
    if (kind == nFTRemixSonicSpringBounceChangeAction)
    {
        ftRemixSonicUSPSpringChangeToJumpF(player_gobj);
    }
    else
    {
        ftRemixSonicUSPSpringJumpSmoke(player_dobj);
    }
    ftRemixSonicUSPSpringLaunch(player_gobj, spring);
}

/* True when the player is airborne, over the spring's X range and within its Y band while falling (asm check_spring_bounce_). */
static sb32 ftRemixSonicUSPSpringTriggeredBy(GObj *player_gobj, GObj *spring_gobj)
{
    FTStruct *fp = ftGetStruct(player_gobj);
    DObj *player_dobj = DObjGetStruct(player_gobj);
    DObj *spring_dobj = DObjGetStruct(spring_gobj);
    WPStruct *wp = wpGetStruct(spring_gobj);
    f32 dx;
    f32 spring_top_y;

    if (fp->fkind == REMIX_ID_BOSS)
    {
        return FALSE;
    }
    if (fp->ga == FALSE)
    {
        return FALSE;
    }
    if (player_dobj->translate.vec.f.z != 0.0F)
    {
        return FALSE;
    }
    /* asm 0x0090 is Y velocity; the port's FTStruct field at that offset is coll_data.pos_diff.y. */
    if (fp->coll_data.pos_diff.y >= 0.0F)
    {
        return FALSE;
    }
    dx = player_dobj->translate.vec.f.x - spring_dobj->translate.vec.f.x;
    if (ABS(dx) > wp->coll_data.map_coll.width)
    {
        return FALSE;
    }
    if (player_dobj->translate.vec.f.y < spring_dobj->translate.vec.f.y)
    {
        return FALSE;
    }
    spring_top_y = spring_dobj->translate.vec.f.y + wp->coll_data.map_coll.top;
    return player_dobj->translate.vec.f.y <= spring_top_y;
}

/* Remix check_spring_bounce_: launches every player that lands on the spring. */
static void ftRemixSonicUSPSpringCheckBounce(GObj *spring_gobj, FTRemixSonicUSPSpring *spring)
{
    GObj *player_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    while (player_gobj != NULL)
    {
        if (ftRemixSonicUSPSpringTriggeredBy(player_gobj, spring_gobj))
        {
            ftRemixSonicUSPSpringBounce(player_gobj, spring_gobj, spring);
        }
        player_gobj = player_gobj->link_next;
    }
}

/* SonicUSP.spring_main_: spring proc_update. Returns TRUE to destroy the spring. */
static sb32 ftRemixSonicUSPSpringMain(GObj *spring_gobj)
{
    WPStruct *wp = wpGetStruct(spring_gobj);
    FTRemixSonicUSPSpring *spring = ftRemixSonicUSPSpringOfGObj(spring_gobj);
    f32 gravity = 0.0F;

    if (wpMainDecLifeCheckExpire(wp) != FALSE)
    {
        ftRemixSonicUSPSpringExpire(spring_gobj);
        return TRUE;
    }
    if (wp->lifetime < SONIC_USP_SPRING_DURATION - 4)
    {
        gravity = SONIC_USP_SPRING_GRAVITY;
        ftRemixSonicUSPSpringFlightStep(spring_gobj, spring);
    }
    wpMainApplyGravityClampTVel(wp, gravity, SONIC_USP_SPRING_MAX_SPEED);

    if (spring->is_bounce_enabled != FALSE)
    {
        ftRemixSonicUSPSpringCheckBounce(spring_gobj, spring);
    }
    return FALSE;
}

/* Builds the Remix spring WPDesc (asm spring_projectile_struct) on the stack for one spawn. */
static void ftRemixSonicUSPSpringFillDesc(WPDesc *spring_desc, FTStruct *fp)
{
    spring_desc->flags = 0;
    spring_desc->kind = SONIC_USP_SPRING_WPKIND;
    spring_desc->p_weapon = ftRemixPowerData(fp)->p_file_special1;
    spring_desc->o_attributes = 0;
    spring_desc->transform_types.tk1 = nGCMatrixKindTra;
    spring_desc->transform_types.tk2 = SONIC_USP_SPRING_TRANSFORM_TK2;
    spring_desc->transform_types.tk3 = 0;
    spring_desc->proc_update = ftRemixSonicUSPSpringMain;
    spring_desc->proc_map = wpSamusBombProcMap;
    spring_desc->proc_hit = wpSamusBombProcMap;
    spring_desc->proc_shield = NULL;
    spring_desc->proc_hop = NULL;
    spring_desc->proc_setoff = wpFoxBlasterProcReflector;
    spring_desc->proc_reflector = NULL;
    spring_desc->proc_absorb = NULL;
}

/* Asm `ground_initial_` flags: byte 0x186 is the saved floor line ID and byte 0x187 the in-air flag. */
static sb32 ftRemixSonicUSPSpringStartsAerial(FTStruct *fp)
{
    u32 flag2 = fp->motion_vars.flags.flag2;
    s32 saved_floor_line_id = (s8)((flag2 >> 8) & 0xFF);

    if ((flag2 & 0xFF) != 0)
    {
        return TRUE;
    }
    if (saved_floor_line_id != fp->coll_data.floor_line_id)
    {
        return TRUE;
    }
    return fp->coll_data.floor_line_id < 0;
}

/* SonicUSP.spring_stage_setting_: spawns the spring at spawn_pos and records it for the player. */
static void ftRemixSonicUSPSpringStageSetting(GObj *fighter_gobj, Vec3f *spawn_pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    FTRemixSonicUSPSpring *spring;
    WPDesc spring_desc;
    GObj *spring_gobj;
    WPStruct *wp;
    sb32 is_aerial;

    ftRemixSonicUSPSpringFillDesc(&spring_desc, fp);
    spring_gobj = wpManagerMakeWeapon(fighter_gobj, &spring_desc, spawn_pos, WEAPON_FLAG_COLLPROJECT);
    if (spring_gobj == NULL)
    {
        return;
    }

    gcMoveGObjDL(spring_gobj, SONIC_USP_SPRING_DL_LINK, GOBJ_PRIORITY_DEFAULT);

    wp = wpGetStruct(spring_gobj);
    wp->lifetime = SONIC_USP_SPRING_DURATION;
    wp->attack_coll.attack_count = 0;
    is_aerial = ftRemixSonicUSPSpringStartsAerial(fp);
    wp->ga = is_aerial;
    if (!is_aerial)
    {
        wp->coll_data.floor_line_id = fp->coll_data.floor_line_id;
    }
    if (ftRemixSonicIsClassic(fp->player))
    {
        /* Classic Sonic's spring: coiled image/palette 3 and FGM 0x3DF (spring_stage_setting_). */
        ftRemixSonicUSPSetSpringImage(DObjGetStruct(spring_gobj)->mobj, SONIC_USP_SPRING_IMAGE_COILED_CLASSIC);
    }
    else
    {
        ftRemixSonicUSPSetSpringImage(DObjGetStruct(spring_gobj)->mobj, SONIC_USP_SPRING_IMAGE_COILED);
    }

    sFTRemixSonicUSPSpringGObjs[fp->player] = spring_gobj;
    spring = ftRemixSonicUSPSpringOfGObj(spring_gobj);
    spring->fgm_id = ftRemixSonicIsClassic(fp->player) ? SONIC_USP_SPRING_FGM_CLASSIC : SONIC_USP_SPRING_FGM;
    spring->is_bounce_enabled = FALSE;

    func_800269C0_275C0((u16)spring->fgm_id);
    wpMainVelSetModelPitch(spring_gobj);
}

/* Creates the spring above the body part (asm `_create_spring`). */
static void ftRemixSonicUSPCreateSpring(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    Vec3f spawn_pos;

    gmCollisionGetFighterPartsWorldPosition(fp->joints[SONIC_USP_SPRING_JOINT], &spawn_pos);
    spawn_pos.y += SONIC_USP_SPRING_DROP_Y_OFFSET;
    spawn_pos.z = 0.0F;
    ftRemixSonicUSPSpringStageSetting(fighter_gobj, &spawn_pos);
}

/* SonicUSP.main_air_: creates the spring, starts the movement and waits for the animation to end. */
static void ftRemixSonicUSPMainAir(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    GObj *spring_gobj = sFTRemixSonicUSPSpringGObjs[fp->player];

    if (spring_gobj == NULL)
    {
        ftRemixSonicUSPCreateSpring(fighter_gobj);
    }
    else if (fp->motion_vars.flags.flag0 != 0)
    {
        if (ftRemixSonicUSPSpringIsAlive(spring_gobj))
        {
            ftRemixSonicUSPUncoilSpring(spring_gobj);
        }
        ftRemixSonicUSPBeginMovement(fp);
    }
    if (fighter_gobj->anim_frame <= 0.0F)
    {
        mpCommonSetFighterWaitOrFall(fighter_gobj);
    }
}

/* SonicUSP.interrupt_: allows aerial attacks and jumps once temp variable 2 is set. */
static void ftRemixSonicUSPInterrupt(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->motion_vars.flags.flag1 == 0)
    {
        return;
    }
    if (ftCommonAttackAirCheckInterruptCommon(fighter_gobj))
    {
        return;
    }
    ftCommonJumpAerialCheckInterruptCommon(fighter_gobj);
}

/* SonicUSP.air_physics_: player air control only once the movement has started. */
static void ftRemixSonicUSPAirPhysics(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (sFTRemixSonicUSPMovement[fp->player] != FALSE)
    {
        ftPhysicsApplyAirVelDrift(fighter_gobj);
    }
}

/* Remix address -> port routine for every SonicUSP routine in the worklist. */
const FTRemixProc dFTRemixSonicUSPProcs[] =
{
    { REMIX_SYM_SonicUSP__ground_initial_, ftRemixSonicUSPGroundInitial },
    { REMIX_SYM_SonicUSP__air_initial_, ftRemixSonicUSPAirInitial },
    { REMIX_SYM_SonicUSP__main_air_, ftRemixSonicUSPMainAir },
    { REMIX_SYM_SonicUSP__interrupt_, ftRemixSonicUSPInterrupt },
    { REMIX_SYM_SonicUSP__air_physics_, ftRemixSonicUSPAirPhysics }
};

const s32 dFTRemixSonicUSPProcsNum = ARRAY_COUNT(dFTRemixSonicUSPProcs);

#endif
