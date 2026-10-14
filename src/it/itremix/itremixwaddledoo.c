/**
 * itremixwaddledoo.c - Smash Remix Waddle Doo (Item.WaddleDoo.id 0x47), the
 * laser minion from Dedede's down special. Port of items/WaddleDoo.asm.
 *
 * Routines ported from WaddleDoo.asm: waddle_doo_attack_initial,
 * waddle_doo_attack_main, star_stage_setting, beam_main. create_beam_star is
 * a stub (REMIX-TODO); see the notes file for what is still missing.
 */

#ifdef PORT

#include "it/itremix/itremixwaddledoo.h"
#include "it/itremix/itremixwaddledee.h"
#include <math.h>
#include <ef/efmanager.h>
#include <it/item.h>
#include <it/itcommon/itstarrod.h>
#include <it/itfighter/itlinkbomb.h>
#include <it/itmain.h>
#include <it/itmap.h>
#include <wp/weapon.h>
#include <reloc_data.h>
#include <sys/objman.h>

extern alSoundEffect* func_800269C0_275C0(u16 id);


#define ITREMIX_WADDLEDOO_KIND 0x47

#define ITREMIX_WADDLEDOO_STATE_ATTACK 4
#define ITREMIX_WADDLEDOO_ATTACK_ANIM_FRAME 6
#define ITREMIX_WADDLEDOO_FGM_BEAM_INITIAL 0xDC
#define ITREMIX_WADDLEDOO_FGM_BEAM_SHOOT 0x40
#define ITREMIX_WADDLEDOO_COLANIM_FLASH 0x07
#define ITREMIX_WADDLEDOO_COLANIM_BEAM_FLASH 0x2A
#define ITREMIX_WADDLEDOO_BEAM_FLASH_FRAMES 0x14

#define ITREMIX_WADDLEDOO_BEAM_CHARGE_TIME 20
#define ITREMIX_WADDLEDOO_BEAM_SHOOT_TIME 40
#define ITREMIX_WADDLEDOO_BEAM_STAR_COUNT 4
#define ITREMIX_WADDLEDOO_BEAM_STAR_DAMAGE 2
#define ITREMIX_WADDLEDOO_BEAM_START_UP 10
#define ITREMIX_WADDLEDOO_BEAM_DURATION 10
#define ITREMIX_WADDLEDOO_BEAM_DISTANCE 256.0F
#define ITREMIX_WADDLEDOO_BEAM_INITIAL_ANGLE 1.125F            /* 0x3F900000 */
#define ITREMIX_WADDLEDOO_BEAM_ROTATION_AMOUNT 0.26171875F     /* 0x3E860000, per 2 frames */
#define ITREMIX_WADDLEDOO_BEAM_CHILD_ANGLE_OFFSET 0.40625F     /* 0x3ED00000 */
#define ITREMIX_WADDLEDOO_BEAM_HIT_FGM 0x16                      /* FGM.hit.ELECTRIC_S */
#define ITREMIX_WADDLEDOO_BEAM_WPKIND 0x27                       /* Projectile.id.DEDEDE_WADDLEDOO_BEAM (8th add_projectile, after SONIC_SPRING 0x20) */

/* Remix item special struct fields beyond the vanilla ITStruct union. Offsets are the N64 ones. */
typedef struct ITRemixWaddleDooVars
{
    ITRemixWaddleDeeVars minion;                /* spawned by WaddleDee.stage_setting_, so it shares the Waddle Dee layout */
    GObj *last_star_gobj;                       /* 0x0350: most recent beam star */
} ITRemixWaddleDooVars;

/* Remix projectile fields beyond the vanilla WPStruct weapon_vars union. */
typedef struct ITRemixWaddleDooBeamVars
{
    s16 startup_timer;                          /* 0x029A: frames of spark gfx before the star moves */
    s16 draw_toggle;                            /* 0x02A8: draw/hitbox toggle, 1 after creation */
    f32 angle;                                  /* 0x02A4: angle around the parent, in radians */
    GObj *parent_gobj;                          /* 0x02A0: Waddle Doo that spawned the star */
} ITRemixWaddleDooBeamVars;

_Static_assert(sizeof(ITRemixWaddleDooVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixWaddleDooVars must fit in ITStruct.item_vars");
_Static_assert(sizeof(ITRemixWaddleDooBeamVars) <= sizeof(((WPStruct*)0)->weapon_vars), "ITRemixWaddleDooBeamVars must fit in WPStruct.weapon_vars");

// // // // // // // // // // // //
//                               //
//       INITIALIZED DATA        //
//                               //
// // // // // // // // // // // //

/* item_info_array. p_file is set from the owner fighter (Character.DEDEDE_file_6_ptr = p_file_special1) at spawn. */
static ITDesc sITRemixWaddleDooItemDesc =
{
    ITREMIX_WADDLEDOO_KIND,
    NULL,
    0xB0,                                       /* item footer offset, used as o_attributes */
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixWaddleDeeMinionNullMain,  /* 0x14: item_info_array runs into item_state_table, so the desc starts in state 0 */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static sb32 itRemixWaddleDooAttackProcUpdate(GObj *item_gobj);
static sb32 itRemixWaddleDooBeamProcUpdate(GObj *weapon_gobj);

/* beam_info_array (WaddleDoo.asm 801-815): the star rod's weapon data with the
 * beam projectile id and beam_main as update proc; every collision proc is 0.
 * o_attributes is 0x4D4, the star rod weapon attributes in the common item
 * file (llITCommonDataStarRodWeaponAttributes), which is also what the Remix
 * file pointer 0x8018D040 + offset 0x4D4 resolves to. */
static WPDesc sITRemixWaddleDooBeamWeaponDesc =
{
    0,                                          /* render flags */
    ITREMIX_WADDLEDOO_BEAM_WPKIND,              /* Projectile.id.DEDEDE_WADDLEDOO_BEAM */
    &gITManagerCommonData,                      /* Remix common item file 0xFB base (asm 0x8018D040) */
    llITCommonDataStarRodWeaponAttributes,      /* 0x4D4: star rod weapon attributes */
    {
        nGCMatrixKindTraRotRpyRSca,             /* asm render index 0x1C000000 */
        nGCMatrixKindNull,
        0
    },
    itRemixWaddleDooBeamProcUpdate,             /* beam_main */
    NULL,                                       /* map */
    NULL,                                       /* hit */
    NULL,                                       /* shield */
    NULL,                                       /* hop */
    NULL,                                       /* set-off */
    NULL,                                       /* reflector */
    NULL                                        /* absorb */
};

/*
 * item_state_table, read by block index (the asm block labels disagree with each other).
 * Index 4 is the attack state used by waddle_doo_attack_initial (a2 = 4).
 * Entry order: update, map, hit, shield, hop, setoff, reflector, damage.
 */
static ITStatusDesc sITRemixWaddleDooStatusDescs[] =
{
    // Status 0 (null)
    {
        itRemixWaddleDeeMinionNullMain,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    // Status 1 (main/aerial)
    {
        itRemixWaddleDeeWaddleDeeAerialMain,
        itRemixWaddleDeeWaddleDeeCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixWaddleDeeMinionHitboxCollision
    },

    // Status 2 (landing)
    {
        itRemixWaddleDeeWaddleDeeLandingMain,
        itRemixWaddleDeeWaddleDeeRestingCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itRemixWaddleDeeMinionHurtboxCollision,
        itMainCommonProcHop,
        NULL,
        itMainCommonProcReflector,
        itRemixWaddleDeeMinionHitboxCollision
    },

    // Status 3 (walking around)
    {
        itRemixWaddleDeeMinionWalkMain,
        itRemixWaddleDeeWaddleGroundCollision,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixWaddleDeeMinionHitboxCollision
    },

    // Status 4 (attack, beam)
    {
        itRemixWaddleDooAttackProcUpdate,
        itRemixWaddleDeeWaddleGroundCollision,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixWaddleDeeMinionHitboxCollision
    },

    // Status 5 (attack, copy of status 4 in asm)
    {
        itRemixWaddleDooAttackProcUpdate,
        itRemixWaddleDeeWaddleGroundCollision,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixWaddleDeeMinionHitboxCollision
    }
};

/* Item.WaddleDoo.SPAWN_ITEM: Dedede passes Doo's item_info_array to WaddleDee.stage_setting_. */
GObj* itRemixWaddleDooMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    return itRemixWaddleDeeSpawnMinion(parent_gobj, &sITRemixWaddleDooItemDesc, pos, vel);
}

/* Descriptor for the Remix item table. */
const ITRemixItemDesc dITRemixWaddleDooItemDesc =
{
    ITREMIX_WADDLEDOO_KIND,                     /* Item.WaddleDoo.id */
    itRemixWaddleDooMakeItem,                   /* SPAWN_ITEM: WaddleDee.stage_setting_ with Doo's item_info_array */
    FALSE,                                      /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                                       /* PICKUP_ITEM_INIT */
    NULL,                                       /* PICKUP_ITEM_MAIN */
    NULL,                                       /* DROP_ITEM */
    NULL,                                       /* THROW_ITEM */
    NULL                                        /* PLAYER_COLLISION */
};

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

/** Returns the Waddle Doo overlay on the item's ITStruct.item_vars. */
static ITRemixWaddleDooVars* itRemixWaddleDooGetVars(GObj *item_gobj)
{
    return (ITRemixWaddleDooVars*)&itGetStruct(item_gobj)->item_vars;
}

/** Returns the beam star overlay on the projectile's WPStruct.weapon_vars. */
static ITRemixWaddleDooBeamVars* itRemixWaddleDooGetBeamVars(GObj *star_gobj)
{
    return (ITRemixWaddleDooBeamVars*)&wpGetStruct(star_gobj)->weapon_vars;
}

/**
 * Creates one beam star for Waddle Doo's laser. Port of create_beam_star: the
 * asm tail-jumps into itStarRodWeaponStarMakeWeapon right at its
 * wpManagerMakeWeapon call (0x801784A8) with the beam desc in a1, the fighter
 * parent in a0 and WEAPON_FLAG_COLLPROJECT (lui 0x8000) in a3, then runs the
 * star rod tail (gcAddXObjForDObjFixed 0x2E, translate = pos, z = 0) before
 * returning to star_stage_setting, which overwrites the velocity and lifetime.
 */
static GObj* itRemixWaddleDooCreateBeamStar(GObj *item_gobj, Vec3f *pos)
{
    GObj *star_gobj = wpManagerMakeWeapon(itGetStruct(item_gobj)->owner_gobj, &sITRemixWaddleDooBeamWeaponDesc, pos, WEAPON_FLAG_COLLPROJECT);
    DObj *dobj;

    if (star_gobj == NULL)
    {
        return NULL;
    }
    dobj = DObjGetStruct(star_gobj);
    gcAddXObjForDObjFixed(dobj, 0x2E, 0);
    dobj->translate.vec.f = *pos;
    dobj->translate.vec.f.z = 0.0F;

    return star_gobj;
}

/** Sets a beam star's stats and initial position. Port of the star_stage_setting setup block. */
static void itRemixWaddleDooBeamInit(GObj *star_gobj, GObj *item_gobj, Vec3f *pos, f32 angle, s32 duration, s32 startup_timer)
{
    ITStruct *ip = itGetStruct(item_gobj);
    WPStruct *wp = wpGetStruct(star_gobj);
    ITRemixWaddleDooBeamVars *beam = itRemixWaddleDooGetBeamVars(star_gobj);
    DObj *dobj = DObjGetStruct(star_gobj);
    f32 x = pos->x + ((f32)ip->lr * ITREMIX_WADDLEDOO_BEAM_DISTANCE);
    f32 y = pos->y + (sinf(angle) * ITREMIX_WADDLEDOO_BEAM_DISTANCE);

    wp->team = ip->team;
    wp->owner_gobj = ip->owner_gobj;
    wp->physics.vel_air.x = 0.0F;
    wp->lr = ip->lr;
    /* The delegated star rod proc (itStarRodWeaponStarProcUpdate) expires the
     * star through weapon_vars.star.lifetime (N64 0x029C), so the duration goes
     * there, not the WPStruct lifetime field. */
    wp->weapon_vars.star.lifetime = duration;
    wp->attack_coll.damage = ITREMIX_WADDLEDOO_BEAM_STAR_DAMAGE;
    wp->attack_coll.element = nGMHitElementElectric;
    wp->attack_coll.fgm_id = ITREMIX_WADDLEDOO_BEAM_HIT_FGM;
    wp->attack_coll.attack_state = nGMAttackStateOff;

    beam->startup_timer = startup_timer;
    beam->draw_toggle = 1;
    beam->angle = angle;
    beam->parent_gobj = item_gobj;

    /* star_stage_setting_ "overwrite visibility": the star spawns with its
     * draw flag set and beam_main toggles it (see the toggle in the update). */
    star_gobj->flags = GOBJ_FLAG_HIDDEN;

    dobj->translate.vec.f.x = x;
    dobj->translate.vec.f.y = y;
    wp->coll_data.pos_prev.x = x;
    wp->coll_data.pos_prev.y = y;
}

/**
 * Creates the beam star for one step of the beam. Port of star_stage_setting.
 * Returns the new star, or NULL when no star could be created.
 */
static GObj* itRemixWaddleDooStarStageSetting(GObj *item_gobj, Vec3f *pos)
{
    GObj *star_gobj = itRemixWaddleDooCreateBeamStar(item_gobj, pos);

    if (star_gobj == NULL)
    {
        return NULL;
    }
    itRemixWaddleDooBeamInit(star_gobj, item_gobj, pos, ITREMIX_WADDLEDOO_BEAM_INITIAL_ANGLE, ITREMIX_WADDLEDOO_BEAM_DURATION, ITREMIX_WADDLEDOO_BEAM_START_UP);

    return star_gobj;
}

/** Ends the attack when the lifetime runs out. Returns TRUE when the item was freed. */
static sb32 itRemixWaddleDooCheckDuration(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (ip->lifetime == 0)
    {
        itRemixWaddleDeeMinionFree(item_gobj);
        return TRUE;
    }
    ip->lifetime--;

    return FALSE;
}

/** Sets up the first beam star's frame, hurtbox and sound. Port of the _spawn_beam_initial block. */
static void itRemixWaddleDooBeamInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = 0;              /* asm 0x0248 = hurtbox disabled; mapping uncertain, see notes */
    func_800269C0_275C0(ITREMIX_WADDLEDOO_FGM_BEAM_SHOOT);
    /* REMIX-TODO(image): asm sets the image struct frame to ITREMIX_WADDLEDOO_ATTACK_ANIM_FRAME (jumping image) */
}

/** Creates the star(s) for this frame of the charge phase. Port of the star branch of waddle_doo_attack_main. */
static void itRemixWaddleDooAttackCreateStars(GObj *item_gobj, ITRemixWaddleDooVars *vars)
{
    Vec3f *pos;

    if (itGetStruct(item_gobj)->multi == (ITREMIX_WADDLEDOO_BEAM_CHARGE_TIME - ITREMIX_WADDLEDOO_BEAM_STAR_COUNT + 1))
    {
        itRemixWaddleDooBeamInitial(item_gobj);
        pos = &DObjGetStruct(item_gobj)->translate.vec.f;
    }
    else
    {
        if (vars->last_star_gobj == NULL)
        {
            return;
        }
        pos = &DObjGetStruct(vars->last_star_gobj)->translate.vec.f;
    }
    vars->last_star_gobj = itRemixWaddleDooStarStageSetting(item_gobj, pos);
    itMainCheckSetColAnimID(item_gobj, ITREMIX_WADDLEDOO_COLANIM_BEAM_FLASH, ITREMIX_WADDLEDOO_BEAM_FLASH_FRAMES);
}

/** Shoot phase, after the charge. Port of the _shooting block. Returns the same as the attack update. */
static sb32 itRemixWaddleDooAttackShoot(GObj *item_gobj, ITRemixWaddleDooVars *vars)
{
    if (itGetStruct(item_gobj)->multi >= (ITREMIX_WADDLEDOO_BEAM_CHARGE_TIME + ITREMIX_WADDLEDOO_BEAM_SHOOT_TIME))
    {
        /* REMIX-TODO(hitbox-flag): asm clears the hitbox collision flag at 0x01D4 on every exit path of waddle_doo_attack_main */
        itRemixWaddleDeeMinionWalkInitial(item_gobj);
        return FALSE;
    }
    return itRemixWaddleDooCheckDuration(item_gobj);
}

/**
 * Waddle Doo beam update. Port of waddle_doo_attack_main (state 3 and 4 main).
 * Returns FALSE to keep the item alive, TRUE when it was freed.
 */
static sb32 itRemixWaddleDooAttackProcUpdate(GObj *item_gobj)
{
    ITRemixWaddleDooVars *vars = itRemixWaddleDooGetVars(item_gobj);

    itGetStruct(item_gobj)->multi++;
    if (itGetStruct(item_gobj)->multi > ITREMIX_WADDLEDOO_BEAM_CHARGE_TIME)
    {
        return itRemixWaddleDooAttackShoot(item_gobj, vars);
    }
    if (itGetStruct(item_gobj)->multi >= (ITREMIX_WADDLEDOO_BEAM_CHARGE_TIME - ITREMIX_WADDLEDOO_BEAM_STAR_COUNT + 1))
    {
        itRemixWaddleDooAttackCreateStars(item_gobj, vars);
    }

    return itRemixWaddleDooCheckDuration(item_gobj);
}

/**
 * Starts the laser attack. Port of waddle_doo_attack_initial. Called by the WaddleDee
 * and ScuttleMinion ports.
 */
void itRemixWaddleDooAttackInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixWaddleDooVars *vars = itRemixWaddleDooGetVars(item_gobj);

    DObjGetStruct(item_gobj)->rotate.vec.f.z = 0.0F;
    /* REMIX-TODO(image): asm sets the image frame to 5 (jump squat) and clears the animation pointers at 0x0090/0x0094 */
    ip->attack_coll.attack_state = 0;           /* asm 0x010C: hitbox disabled */
    ip->times_landed = 0;                       /* asm 0x02CE: andi 0xCF */
    ip->multi = 0;                              /* asm 0x033E */
    itMapSetGround(ip);                         /* asm 0x80173F54: sets kinetic state and x speed multiplier */
    itLinkBombCommonSetHitStatusNormal(item_gobj);  /* asm 0x80185CD4: sets the unknown value to 1 */
    func_800269C0_275C0(ITREMIX_WADDLEDOO_FGM_BEAM_INITIAL);
    itMainCheckSetColAnimID(item_gobj, ITREMIX_WADDLEDOO_COLANIM_FLASH, ITREMIX_WADDLEDOO_BEAM_CHARGE_TIME);
    itMainSetStatus(item_gobj, sITRemixWaddleDooStatusDescs, ITREMIX_WADDLEDOO_STATE_ATTACK);
}

/**
 * Beam star update. Port of beam_main, the Remix Waddle Doo beam projectile proc.
 * Only referenced once the beam WPDesc is ported (see the REMIX-TODO in itRemixWaddleDooCreateBeamStar).
 */
static sb32 itRemixWaddleDooBeamProcUpdate(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    ITRemixWaddleDooBeamVars *beam = itRemixWaddleDooGetBeamVars(weapon_gobj);
    DObj *dobj = DObjGetStruct(weapon_gobj);
    GObj *parent_gobj = beam->parent_gobj;
    ITStruct *parent_ip;
    DObj *parent_dobj;
    f32 x;
    f32 y;

    if (beam->startup_timer != 0)
    {
        if (beam->startup_timer == ITREMIX_WADDLEDOO_BEAM_START_UP)
        {
            efManagerSparkleWhiteScaleMakeEffect(&dobj->translate.vec.f, 1.0F);
        }
        beam->startup_timer--;
        return FALSE;
    }
    if (itStarRodWeaponStarProcUpdate(weapon_gobj) != FALSE)
    {
        return TRUE;
    }
    /* beam_main draw/hitbox toggle (asm 688-707): when the GObj draw flag and
     * the 0x02A8 copy (draw_toggle) match, flip the draw flag and the hitbox
     * update interval (attack_state 3 <-> off); otherwise re-sync the copy.
     * This blinks the star every 2 frames while it orbits the Waddle Doo. */
    if (weapon_gobj->flags == (u32)beam->draw_toggle)
    {
        weapon_gobj->flags = (weapon_gobj->flags != FALSE) ? GOBJ_FLAG_NONE : GOBJ_FLAG_HIDDEN;
        wp->attack_coll.attack_state = (wp->attack_coll.attack_state == nGMAttackStateInterpolate) ? nGMAttackStateOff : nGMAttackStateInterpolate;
    }
    else beam->draw_toggle = weapon_gobj->flags;

    if ((wp->weapon_vars.star.lifetime & 1) != 0)
    {
        return FALSE;
    }
    beam->angle -= ITREMIX_WADDLEDOO_BEAM_ROTATION_AMOUNT;
    if (parent_gobj == NULL)
    {
        return FALSE;
    }
    parent_ip = itGetStruct(parent_gobj);
    if ((parent_ip == NULL) || (parent_ip->kind == 0))
    {
        beam->parent_gobj = NULL;
        return FALSE;
    }
    parent_dobj = DObjGetStruct(parent_gobj);
    if (parent_dobj == NULL)
    {
        beam->parent_gobj = NULL;
        return FALSE;
    }
    x = parent_dobj->translate.vec.f.x + ((f32)wp->lr * ITREMIX_WADDLEDOO_BEAM_DISTANCE);
    y = parent_dobj->translate.vec.f.y + (sinf(beam->angle) * ITREMIX_WADDLEDOO_BEAM_DISTANCE);
    dobj->translate.vec.f.x = x;
    dobj->translate.vec.f.y = y;
    wp->coll_data.pos_prev.x = x;
    wp->coll_data.pos_prev.y = y;
    /* REMIX-TODO(beam-multihit): asm clears the already-hit player at 0x0214 so the star can rehit */

    return FALSE;
}

#endif
