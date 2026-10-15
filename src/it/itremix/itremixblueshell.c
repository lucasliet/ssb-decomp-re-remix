#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixblueshell.h"
#include <it/item.h>
#include <it/itmain.h>
#include <it/itmap.h>
#include <it/itprocess.h>
#include <it/itcommon/itrshell.h>
#include <it/itcommon/ittomato.h>
#include <ef/efmanager.h>
#include <if/ifcommon.h>

#define ITREMIX_BLUESHELL_KIND 0x30
#define ITREMIX_BLUESHELL_FILE_OFFSET 0xE30
#define ITREMIX_BLUESHELL_LIFETIME 0x3C0 /* 16 seconds */
#define ITREMIX_BLUESHELL_HURTBOX_DAMAGE 30
#define ITREMIX_BLUESHELL_BASE_KNOCKBACK 100
#define ITREMIX_BLUESHELL_KNOCKBACK_GROWTH 0x100
#define ITREMIX_BLUESHELL_KNOCKBACK_ANGLE 90
#define ITREMIX_BLUESHELL_HURTBOX_FGM 0x02B7
#define ITREMIX_BLUESHELL_ELECTRIC_ELEMENT 3
#define ITREMIX_BLUESHELL_GRAVITY 1.2F
#define ITREMIX_BLUESHELL_TVEL 100.0F
#define ITREMIX_BLUESHELL_REBOUND_COMMON 0.25F
#define ITREMIX_BLUESHELL_REBOUND_GROUND 0.5F
#define ITREMIX_BLUESHELL_THROW_TIMER 0x10
#define ITREMIX_BLUESHELL_THROW_TIMER_RESET 0xFF
#define ITREMIX_BLUESHELL_GFX_INTERVAL 8
#define ITREMIX_BLUESHELL_DAMAGE_OWNER_WAIT 16
#define ITREMIX_BLUESHELL_STOP_VEL_X 8.0F
#define ITREMIX_BLUESHELL_HURTBOX_VEL_MUL 10.0F
#define ITREMIX_BLUESHELL_RECOIL_VEL_X (-8.0F)
#define ITREMIX_BLUESHELL_RECOIL_MUL_X 0.7F /* constant at 0x8019CDA8, UNCERTAIN: taken from ITRSHELL_RECOIL_MUL_X */
#define ITREMIX_BLUESHELL_SHIELD_VEL_MUL 0.5F
#define ITREMIX_BLUESHELL_LAND_VEL_LR (-8.0F)
#define ITREMIX_BLUESHELL_LAND_VEL_BASE (-10.0F)
#define ITREMIX_BLUESHELL_WALL_MASK 0x0021
#define ITREMIX_BLUESHELL_SHIELD_FGM 0x0038
#define ITREMIX_BLUESHELL_EXPLODE_FGM 0x0001

enum itRemixBlueShellStatus
{
    nITRemixBlueShellStatusWait,
    nITRemixBlueShellStatusIdle,
    nITRemixBlueShellStatusHeld,
    nITRemixBlueShellStatusThrown,
    nITRemixBlueShellStatusFall,
    nITRemixBlueShellStatusGrounded,
    nITRemixBlueShellStatusAerial,
    nITRemixBlueShellStatusEnumCount
};

extern void *func_800269C0_275C0(u16 id);

static sb32 itRemixBlueShellRestingProcMap(GObj *item_gobj);
static void itRemixBlueShellGroundToAir(GObj *item_gobj);
static sb32 itRemixBlueShellGroundedCheckProcMap(GObj *item_gobj);
static sb32 itRemixBlueShellIdleGroundProcMap(GObj *item_gobj);
static void itRemixBlueShellIdleGroundedInitial(GObj *item_gobj);
static void itRemixBlueShellThrowSetStatus(GObj *item_gobj);
static sb32 itRemixBlueShellAirToGroundProcMap(GObj *item_gobj);
static sb32 itRemixBlueShellAerialProcUpdate(GObj *item_gobj);
static sb32 itRemixBlueShellCollideWithPlayer(GObj *item_gobj);
static sb32 itRemixBlueShellCollideWithShield(GObj *item_gobj);
static void itRemixBlueShellSetGrounded(GObj *item_gobj);
static void itRemixBlueShellSetAerial(GObj *item_gobj);
static sb32 itRemixBlueShellGroundedProcUpdate(GObj *item_gobj);
static void itRemixBlueShellGroundedGfx(GObj *item_gobj);
static sb32 itRemixBlueShellGroundedHurtboxCollide(GObj *item_gobj);
static sb32 itRemixBlueShellIdleHurtboxCollide(GObj *item_gobj);
static void itRemixBlueShellHitGfx(GObj *item_gobj);
static void itRemixBlueShellHoldProc(GObj *item_gobj);

static ITStatusDesc sITRemixBlueShellStatusDescs[nITRemixBlueShellStatusEnumCount] =
{
    /* Status 0 (wait) */
    {
        NULL,                                   /* Proc Update */
        itRemixBlueShellRestingProcMap,         /* Proc Map */
        NULL,                                   /* Proc Hit */
        NULL,                                   /* Proc Shield */
        NULL,                                   /* Proc Hop */
        NULL,                                   /* Proc Set-Off */
        NULL,                                   /* Proc Reflector */
        itRemixBlueShellIdleHurtboxCollide      /* Proc Damage */
    },

    /* Status 1 (idle in the air, uses the grounded-idle map proc) */
    {
        itRemixBlueShellAerialProcUpdate,
        itRemixBlueShellIdleGroundProcMap,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Status 2 (pickup, held) */
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },

    /* Status 3 (thrown) */
    {
        itRemixBlueShellAerialProcUpdate,
        itRemixBlueShellAirToGroundProcMap,
        itRemixBlueShellCollideWithPlayer,
        itRShellCommonProcShield,
        itMainCommonProcHop,
        itRShellCommonProcShield,
        itRShellCommonProcReflector,
        itRemixBlueShellIdleHurtboxCollide
    },

    /* Status 4 (falling) */
    {
        itRemixBlueShellAerialProcUpdate,
        itRemixBlueShellAirToGroundProcMap,
        itRemixBlueShellCollideWithPlayer,
        itRShellCommonProcShield,
        itMainCommonProcHop,
        itRShellCommonProcShield,
        itRShellCommonProcReflector,
        itRemixBlueShellIdleHurtboxCollide
    },

    /* Status 5 (active grounded) */
    {
        itRemixBlueShellGroundedProcUpdate,
        itRemixBlueShellGroundedCheckProcMap,
        itRemixBlueShellCollideWithPlayer,
        itRemixBlueShellCollideWithShield,
        NULL,
        NULL,
        itRShellCommonProcReflector,
        itRemixBlueShellGroundedHurtboxCollide
    },

    /* Status 6 (active aerial) */
    {
        itRemixBlueShellAerialProcUpdate,
        itRemixBlueShellAirToGroundProcMap,
        itRemixBlueShellCollideWithPlayer,
        itRemixBlueShellCollideWithPlayer,
        NULL,
        NULL,
        itRShellCommonProcReflector,
        itRemixBlueShellIdleHurtboxCollide
    }
};

static ITDesc sITRemixBlueShellItemInfo =
{
    ITREMIX_BLUESHELL_KIND,                 /* Item.BlueShell.id */
    &gITManagerCommonData,                  /* item file 0xFB, see notes REMIX-TODO(file) */
    ITREMIX_BLUESHELL_FILE_OFFSET,          /* item footer offset */

    {
        nGCMatrixKindNull,                  /* item_info_array 0x0C = 0: the spawn adds the red shell XObjs itself */
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itRemixBlueShellAerialProcUpdate,
    itRemixBlueShellIdleGroundProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    itRemixBlueShellIdleHurtboxCollide
};

const ITRemixItemDesc dITRemixBlueShellItemDesc =
{
    ITREMIX_BLUESHELL_KIND,
    itRemixBlueShellMakeItem,
    TRUE,
    itRemixBlueShellHoldProc,               /* prepickup_main_ */
    NULL,                                   /* PICKUP_ITEM_MAIN(0) */
    itTomatoDroppedSetStatus,               /* DROP_ITEM 0x801745FC */
    itRemixBlueShellThrowSetStatus,         /* throw_initial_ */
    NULL                                    /* PLAYER_COLLISION(0) */
};

/* spawn_custom_item_based_on_red_shell_: DObj setup copied from the red shell spawn */
static void itRemixBlueShellSetupDObj(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    Vec3f translate = dobj->translate.vec.f;

    dobj->rotate.vec.f.y = F_CST_DTOR32(90.0F);
    gcAddXObjForDObjFixed(dobj, nGCMatrixKindTraRotRpyR, 0);
    gcAddXObjForDObjFixed(dobj, 0x48, 0);
    dobj->translate.vec.f = translate;
    dobj->mobj->palette_id = 0.0F;
}

/* spawn_custom_item_based_on_red_shell_: Blue Shell hurtbox and knockback values */
static void itRemixBlueShellSetupAttack(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->attack_coll.can_rehit_shield = TRUE;
    ip->attack_coll.damage = ITREMIX_BLUESHELL_HURTBOX_DAMAGE;
    ip->attack_coll.knockback_base = ITREMIX_BLUESHELL_BASE_KNOCKBACK;
    ip->attack_coll.knockback_scale = ITREMIX_BLUESHELL_KNOCKBACK_GROWTH;
    ip->attack_coll.angle = ITREMIX_BLUESHELL_KNOCKBACK_ANGLE;
    ip->attack_coll.fgm_id = ITREMIX_BLUESHELL_HURTBOX_FGM;
    ip->attack_coll.element = ITREMIX_BLUESHELL_ELECTRIC_ELEMENT;
    ip->lifetime = ITREMIX_BLUESHELL_LIFETIME;
}

/* spawn_custom_item_based_on_red_shell_: red shell state and arrow setup, then the Blue Shell overrides */
static void itRemixBlueShellSetupState(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->is_unused_item_bool = TRUE;
    ip->item_vars.shell.health = 1;
    ip->item_vars.shell.is_setup_vars = FALSE;
    ip->item_vars.shell.is_damage = FALSE;
    ip->item_vars.shell.damage_all_delay = ITREMIX_BLUESHELL_THROW_TIMER_RESET;
    ip->item_vars.shell.vel_x = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);
}

/* spawn_custom_item_based_on_red_shell_. Remix SPAWN_ITEM (parent, pos, vel, flags) */
GObj* itRemixBlueShellMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixBlueShellItemInfo, pos, vel, flags);

    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixBlueShellSetupDObj(item_gobj);
    itRemixBlueShellSetupAttack(item_gobj);
    itRemixBlueShellSetupState(item_gobj);

    return item_gobj;
}

/* prepickup_main_ (PICKUP_ITEM_INIT). The asm zeroes DObj+0x34, taken as rotate.vec.f.y (UNCERTAIN) */
static void itRemixBlueShellHoldProc(GObj *item_gobj)
{
    DObjGetStruct(item_gobj)->rotate.vec.f.y = 0.0F;
    itMainSetStatus(item_gobj, sITRemixBlueShellStatusDescs, nITRemixBlueShellStatusHeld);
}

/* resting_main_ (state 0 map) */
static sb32 itRemixBlueShellRestingProcMap(GObj *item_gobj)
{
    itMapCheckLRWallProcNoFloor(item_gobj, itRemixBlueShellGroundToAir);

    return FALSE;
}

/* ground_to_air_ */
static void itRemixBlueShellGroundToAir(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->is_allow_pickup = FALSE;
    itMapSetAir(ip);
    itMainSetStatus(item_gobj, sITRemixBlueShellStatusDescs, nITRemixBlueShellStatusIdle);
}

/* grounded_check_collision_ (state 5 map) */
static sb32 itRemixBlueShellGroundedCheckProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (itMapCheckLRWallProcNoFloor(item_gobj, itRemixBlueShellSetAerial) == FALSE)
    {
        return FALSE;
    }
    if ((ip->coll_data.mask_curr & ITREMIX_BLUESHELL_WALL_MASK) == 0)
    {
        return FALSE;
    }
    ip->physics.vel_air.x = -ip->physics.vel_air.x;
    itMainSetSpinVelLR(item_gobj);
    itMainClearOwnerStats(item_gobj);
    ip->item_vars.shell.vel_x = -ip->item_vars.shell.vel_x;

    return FALSE;
}

/* idle_ground_collision_ (state 1 map). The asm uses the same proc for the ITDesc map */
static sb32 itRemixBlueShellIdleGroundProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (ip->item_vars.shell.health != 0)
    {
        itMapCheckDestroyDropped(item_gobj, ITREMIX_BLUESHELL_REBOUND_COMMON, ITREMIX_BLUESHELL_REBOUND_GROUND, itRemixBlueShellIdleGroundedInitial);
        return FALSE;
    }
    return itMapCheckDestroyLanding(item_gobj, ITREMIX_BLUESHELL_REBOUND_COMMON);
}

/* idle_grounded_initial_ and idle_grounded_initial_2_ (the asm wrapper is merged into this body) */
static void itRemixBlueShellIdleGroundedInitial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMapSetGround(ip);
    if (ABSF(ip->physics.vel_air.x) >= ITREMIX_BLUESHELL_STOP_VEL_X && ip->item_vars.shell.is_damage != FALSE)
    {
        ip->attack_coll.attack_state = nGMAttackStateNew;
        itProcessUpdateAttackPositions(item_gobj);
        itRemixBlueShellSetGrounded(item_gobj);
        return;
    }
    itMainSetGroundAllowPickup(item_gobj);
    ip->item_vars.shell.is_damage = FALSE;
    ip->physics.vel_air.x = 0.0F;
    itMainClearOwnerStats(item_gobj);
    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    itRShellCommonClearAnim(item_gobj);
    itMainSetStatus(item_gobj, sITRemixBlueShellStatusDescs, nITRemixBlueShellStatusWait);
}

/* throw_initial_ (THROW_ITEM) */
static void itRemixBlueShellThrowSetStatus(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->item_vars.shell.health = 1;
    ip->item_vars.shell.is_damage = TRUE;
    ip->item_vars.shell.damage_all_delay = ITREMIX_BLUESHELL_THROW_TIMER;
    ip->times_thrown = 0;
    itMapSetAir(ip);
    itMainSetStatus(item_gobj, sITRemixBlueShellStatusDescs, nITRemixBlueShellStatusThrown);
}

/* air_to_ground_check_ (thrown, falling and active aerial map). Landing with a fixed bounce */
static sb32 itRemixBlueShellAirToGroundProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    if (itMapCheckLanding(item_gobj, ITREMIX_BLUESHELL_REBOUND_COMMON, ITREMIX_BLUESHELL_REBOUND_GROUND, itRemixBlueShellSetGrounded) == FALSE)
    {
        return FALSE;
    }
    if (ip->physics.vel_air.x < 0.0F)
    {
        ip->lr = -1;
    }
    else
    {
        ip->lr = +1;
    }
    ip->physics.vel_air.x = ((ip->lr * ITREMIX_BLUESHELL_LAND_VEL_LR) + ITREMIX_BLUESHELL_LAND_VEL_BASE) * ITREMIX_BLUESHELL_RECOIL_MUL_X;

    return FALSE;
}

/* aerial_main_ (aerial update). Also counts down the throw timer (damage_all_delay) */
static sb32 itRemixBlueShellAerialProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itMainApplyGravityClampTVel(ip, ITREMIX_BLUESHELL_GRAVITY, ITREMIX_BLUESHELL_TVEL);
    if (ip->item_vars.shell.damage_all_delay == 0)
    {
        itMainClearOwnerStats(item_gobj);
        ip->item_vars.shell.damage_all_delay = ITREMIX_BLUESHELL_THROW_TIMER_RESET;
    }
    ip->item_vars.shell.damage_all_delay--;

    return FALSE;
}

/* collide_with_player_. Only the destroy path is reachable: the asm HP check is commented out, so the bounce path is dropped */
static sb32 itRemixBlueShellCollideWithPlayer(GObj *item_gobj)
{
    itRemixBlueShellHitGfx(item_gobj);
    func_800269C0_275C0(ITREMIX_BLUESHELL_EXPLODE_FGM);

    return TRUE;
}

/* collide_with_shield_ (state 5 shield). Always bounces; the asm destroy block is commented out */
static sb32 itRemixBlueShellCollideWithShield(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    func_800269C0_275C0(ITREMIX_BLUESHELL_SHIELD_FGM);
    ip->damage_coll.hitstatus = nGMHitStatusNormal;
    ip->physics.vel_air.x = ((ip->physics.vel_air.x * ITREMIX_BLUESHELL_SHIELD_VEL_MUL) + (ITREMIX_BLUESHELL_RECOIL_VEL_X * ip->hit_lr)) * ITREMIX_BLUESHELL_RECOIL_MUL_X;
    ip->item_vars.shell.health = 0; /* UNCERTAIN: asm stores v0 left over from FGM.play_ */
    itRShellCommonClearAnim(item_gobj);
    if (ip->ga == nMPKineticsGround)
    {
        itRemixBlueShellSetGrounded(item_gobj);
    }
    else
    {
        itRemixBlueShellSetAerial(item_gobj);
    }

    return FALSE;
}

/* set_grounded */
static void itRemixBlueShellSetGrounded(GObj *item_gobj)
{
    itRShellSpinInitVars(item_gobj);
    itMainSetStatus(item_gobj, sITRemixBlueShellStatusDescs, nITRemixBlueShellStatusGrounded);
}

/* set_aerial */
static void itRemixBlueShellSetAerial(GObj *item_gobj)
{
    itRShellSpinAirInitVars(item_gobj);
    itMainSetStatus(item_gobj, sITRemixBlueShellStatusDescs, nITRemixBlueShellStatusAerial);
}

/* grounded_active_ (state 5 update). Homing toward the player in first is REMIX-TODO(player-in-first) */
static sb32 itRemixBlueShellGroundedProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itRemixBlueShellGroundedGfx(item_gobj);
    itRShellSpinCheckCollisionEdge(item_gobj);
    if (ip->lifetime == 0)
    {
        return TRUE;
    }
    ip->lifetime--;

    return FALSE;
}

/* grounded_gfx_. Every 8 frames: clears the owner and spawns the dust effect */
static void itRemixBlueShellGroundedGfx(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);
    Vec3f pos;

    if (ip->item_vars.shell.dust_effect_int != 0)
    {
        ip->item_vars.shell.dust_effect_int--;
        return;
    }
    ip->owner_gobj = NULL;
    pos = dobj->translate.vec.f;
    pos.y += ip->attr->map_coll_bottom;
    /* REMIX-TODO(gfx): asm spawns the custom pink smoke (custom_smoke_, particle 0x8A); the closest vanilla dust effect is used */
    efManagerDustLightMakeEffect(&pos, ip->lr, 1.0F);
    ip->item_vars.shell.dust_effect_int = ITREMIX_BLUESHELL_GFX_INTERVAL - 1;
}

/* grounded_collide_with_hurtbox_ (state 5 damage). Hit by another shell while rolling */
static sb32 itRemixBlueShellGroundedHurtboxCollide(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->item_vars.shell.interact--;
    if (ip->item_vars.shell.interact == 0)
    {
        return TRUE;
    }
    ip->physics.vel_air.x += (ip->damage_queue * 2.0F) * -ip->damage_lr;
    if (ABSF(ip->physics.vel_air.x) > ITREMIX_BLUESHELL_STOP_VEL_X)
    {
        ip->attack_coll.attack_state = nGMAttackStateNew;
        itProcessUpdateAttackPositions(item_gobj);
        itMainCopyDamageStats(item_gobj);
        itRemixBlueShellSetGrounded(item_gobj);
    }
    else
    {
        ip->attack_coll.attack_state = nGMAttackStateOff;
    }

    return FALSE;
}

/* idle_hurtbox_collision_ (damage in the idle, thrown, falling and aerial states) */
static sb32 itRemixBlueShellIdleHurtboxCollide(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x = (ip->damage_queue * ITREMIX_BLUESHELL_HURTBOX_VEL_MUL) * -ip->damage_lr;
    if (ABSF(ip->physics.vel_air.x) > ITREMIX_BLUESHELL_STOP_VEL_X)
    {
        ip->item_vars.shell.is_damage = TRUE;
        ip->attack_coll.attack_state = nGMAttackStateNew;
        itProcessUpdateAttackPositions(item_gobj);
        itMainCopyDamageStats(item_gobj);
        if (ip->ga == nMPKineticsGround)
        {
            itRemixBlueShellSetGrounded(item_gobj);
        }
        else
        {
            itRemixBlueShellSetAerial(item_gobj);
        }
    }
    else
    {
        ip->physics.vel_air.x = 0.0F;
        ip->attack_coll.attack_state = nGMAttackStateOff;
    }
    ip->owner_gobj = ip->damage_gobj;
    ip->item_vars.shell.dust_effect_int = ITREMIX_BLUESHELL_DAMAGE_OWNER_WAIT;
    /* REMIX-TODO(player-in-first): asm calls get_player_in_first_ here; not ported */

    return FALSE;
}

/* hit_gfx_: white spark effect at the item position */
static void itRemixBlueShellHitGfx(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    Vec3f pos = DObjGetStruct(item_gobj)->translate.vec.f;

    pos.y += ip->attr->map_coll_bottom;
    efManagerShieldBreakMakeEffect(&pos);
    ip->item_vars.shell.dust_effect_int = ITREMIX_BLUESHELL_GFX_INTERVAL; /* UNCERTAIN: asm stores a leftover register */
}

#endif
