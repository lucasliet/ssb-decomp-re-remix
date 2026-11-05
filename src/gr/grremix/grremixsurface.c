/**
 * grremixsurface.c - Remix surface frictions for the vanilla ground physics. See ft/ftremix/ftremixhooks.h.
 */

#ifdef PORT

#include <ssb_types.h>
#include <ft/fighter.h>
#include <mp/map.h>
#include <gr/grtypes.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include "gr/grremix/grremixcasino.h"
#include "gr/grremix/grremixhazards.h"
#include "gr/grremix/grremixids.h"
#include "gr/grremix/grremixsurface.h"
#include "gr/grremix/grremixsurfacerules.h"

extern void *func_800269C0_275C0(u16 id);

#define GRREMIX_SURFACE_VANILLA_COUNT 16
#define GRREMIX_SURFACE_FRICTION_DEFAULT 4.0F
#define GRREMIX_SURFACE_CONVEYOR_FIRST 0x1F
#define GRREMIX_SURFACE_CONVEYOR_LAST 0x22
#define GRREMIX_SURFACE_FLOOR_LINE_NONE -1
#define GRREMIX_SURFACE_FLOOR_LINE_UNKNOWN -2
#define GRREMIX_SURFACE_DAMAGEFLOOR_WAIT 16
#define GRREMIX_SURFACE_BOUNCE_VEL_Y 130.0F /* Surface.asm BOUNCE_Y_VELOCITY 0x4302 as the high halfword of a float */
#define GRREMIX_SURFACE_BOUNCE_SFX 0x5C2

/* Surface.asm add_surface friction values, in the order of the new surface ids 0x10..0x24. */
static const f32 sGRRemixSurfaceFrictions[] =
{
    4.0F, /* 0x10 big_blue_surface_1 */
    0.3F, /* 0x11 cool_cool_surface_1 */
    4.0F, /* 0x12 failed_z_cancel */
    4.0F, /* 0x13 big_blue_surface_2 */
    4.0F, /* 0x14 corneria_surface_1 */
    4.0F, /* 0x15 corneria_surface_2 */
    4.0F, /* 0x16 mute */
    4.0F, /* 0x17 casino_left_diagonal */
    4.0F, /* 0x18 casino_left_top */
    4.0F, /* 0x19 casino_left_side */
    4.0F, /* 0x1A casino_right_diagonal */
    4.0F, /* 0x1B casino_right_top */
    4.0F, /* 0x1C casino_right_side */
    4.0F, /* 0x1D toadsturnpike_car */
    4.0F, /* 0x1E failed_z_cancel_lava */
    4.0F, /* 0x1F push_right_2 */
    4.0F, /* 0x20 push_left_2 */
    4.0F, /* 0x21 push_right_1 */
    4.0F, /* 0x22 push_left_1 */
    4.0F, /* 0x23 acid */
    4.0F, /* 0x24 bounce */
};

/* Surface.asm conveyor_speed in the order of the materials 0x1F..0x22: RTTF right, RTTF left, Time Twister right and left. */
static const f32 sGRRemixSurfaceConveyorSpeeds[] = { 20.5F, -24.0F, 20.0F, -20.0F };

/**
 * Ground friction of a material id. Ids 0x00..0x0F read the vanilla table, 0x10..0x24 the Remix surfaces;
 * any other id gets the default friction instead of reading past the tables.
 */
f32 grRemixSurfaceFriction(u32 material)
{
    u32 remix_index = material - GRREMIX_SURFACE_VANILLA_COUNT;

    if (material < GRREMIX_SURFACE_VANILLA_COUNT)
    {
        return dMPCollisionMaterialFrictions[material];
    }
    if (remix_index >= (sizeof(sGRRemixSurfaceFrictions) / sizeof(sGRRemixSurfaceFrictions[0])))
    {
        return GRREMIX_SURFACE_FRICTION_DEFAULT;
    }
    return sGRRemixSurfaceFrictions[remix_index];
}

/**
 * Surface.asm fighter/item_apply_conveyor_surface: horizontal push of the conveyor material under a grounded object, written
 * to speed. Returns FALSE for any other material, and off Road to the Final Frontier while Hazard Mode turns movement off.
 */
sb32 grRemixSurfaceConveyorPush(u32 material, f32 *speed)
{
    if ((material < GRREMIX_SURFACE_CONVEYOR_FIRST) || (material > GRREMIX_SURFACE_CONVEYOR_LAST))
    {
        return FALSE;
    }
    if ((gSCManagerBattleState->gkind != nGRRemixStage_REMIX_RTTF) && (grRemixMovementIsOn() == FALSE))
    {
        return FALSE;
    }
    *speed = sGRRemixSurfaceConveyorSpeeds[material - GRREMIX_SURFACE_CONVEYOR_FIRST];
    return TRUE;
}

/* One GRAttackColl per knockback row; the hit log keeps the pointer until the fighter processes its hits in the same frame. */
static GRAttackColl sGRRemixSurfaceAttackColls[GRREMIX_SURFACE_RULES_ROW_COUNT];

/**
 * Surface.asm _bounce: the fighter jumps off the surface. Runs the damage or trap callback first, clears the color
 * animation, enters JumpF and sets the launch velocity. The asm byte mask 0x07 at N64 0x18D also clears is_absorb,
 * absorb_lr and is_goto_attack100 together with the fast fall flag.
 */
static void grRemixSurfaceBounce(FTStruct *fp)
{
    GObj *fighter_gobj = fp->fighter_gobj;

    if (fp->proc_damage != NULL)
    {
        fp->proc_damage(fighter_gobj);
    }
    else if (fp->proc_trap != NULL)
    {
        fp->proc_trap(fighter_gobj);
    }
    ftParamResetFighterColAnim(fighter_gobj);
    ftMainSetStatus(fighter_gobj, nFTCommonStatusJumpF, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
    ftMainPlayAnimEventsAll(fighter_gobj);

    fp->physics.vel_air.y = GRREMIX_SURFACE_BOUNCE_VEL_Y;
    fp->physics.vel_damage_air.y = 0.0F;
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
    fp->ga = nMPKineticsAir;
    fp->jumps_used = 1;

    func_800269C0_275C0(GRREMIX_SURFACE_BOUNCE_SFX);
}

/**
 * Material the knockback lookup reads: the Casino bumper wall (Hazards.asm casino_night_side_bumper_knockback) or the
 * floor under a grounded fighter on a real line. Returns FALSE when there is none.
 */
static sb32 grRemixSurfaceGetHitMaterial(FTStruct *fp, u32 *material)
{
    if (grRemixCasinoGetBumperMaterial(fp, material) != FALSE)
    {
        return TRUE;
    }
    if (fp->ga != nMPKineticsGround)
    {
        return FALSE;
    }
    if ((fp->coll_data.floor_line_id == GRREMIX_SURFACE_FLOOR_LINE_NONE) || (fp->coll_data.floor_line_id == GRREMIX_SURFACE_FLOOR_LINE_UNKNOWN))
    {
        return FALSE;
    }
    *material = fp->coll_data.floor_flags & MAP_VERTEX_MAT_MASK;

    return TRUE;
}

/** Copies a knockback row into the GRAttackColl of that row. */
static GRAttackColl* grRemixSurfaceMakeAttackColl(s32 row)
{
    const GRRemixSurfaceKnockback *knockback = grRemixSurfaceRulesGetRow(row);
    GRAttackColl *gr_attack_coll = &sGRRemixSurfaceAttackColls[row];

    gr_attack_coll->kind = knockback->kind;
    gr_attack_coll->damage = knockback->damage;
    gr_attack_coll->angle = knockback->angle;
    gr_attack_coll->knockback_scale = knockback->growth;
    gr_attack_coll->knockback_weight = knockback->fixed;
    gr_attack_coll->knockback_base = knockback->base;
    gr_attack_coll->element = knockback->effect;

    return gr_attack_coll;
}

sb32 grRemixSurfaceGetGroundHit(FTStruct *fp, GRAttackColl **p_gr_attack_coll)
{
    u32 material;
    s32 row;

    if (fp->damagefloor_wait != 0)
    {
        return FALSE;
    }
    if (grRemixSurfaceGetHitMaterial(fp, &material) == FALSE)
    {
        return FALSE;
    }
    if (material == GRREMIX_SURFACE_RULES_BOUNCE_MATERIAL)
    {
        grRemixSurfaceBounce(fp);

        return FALSE;
    }
    row = grRemixSurfaceRulesFindRow(material);

    if (row < 0)
    {
        return FALSE;
    }
    if (grRemixSurfaceRulesIsBlockedByHazardMode(grRemixSurfaceRulesGetRow(row), grRemixHazardsAreOn()))
    {
        return FALSE;
    }
    *p_gr_attack_coll = grRemixSurfaceMakeAttackColl(row);

    return TRUE;
}

sb32 grRemixSurfaceApplyFgmOverride(FTStruct *fp, const GRAttackColl *gr_attack_coll)
{
    const GRRemixSurfaceKnockback *knockback;
    s32 row;

    if ((gr_attack_coll < &sGRRemixSurfaceAttackColls[0]) || (gr_attack_coll > &sGRRemixSurfaceAttackColls[GRREMIX_SURFACE_RULES_ROW_COUNT - 1]))
    {
        return FALSE;
    }
    row = gr_attack_coll - &sGRRemixSurfaceAttackColls[0];
    knockback = grRemixSurfaceRulesGetRow(row);

    if (knockback->fgm_id < 0)
    {
        return FALSE;
    }
    fp->damagefloor_wait = GRREMIX_SURFACE_DAMAGEFLOOR_WAIT;
    func_800269C0_275C0(knockback->fgm_id);

    return TRUE;
}

#endif
