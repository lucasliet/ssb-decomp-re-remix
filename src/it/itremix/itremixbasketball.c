/**
 * itremixbasketball.c - C port of the Smash Remix Basketball item (id 0x40)
 * (.toolchain/pkgs/smashremix-src/src/items/Basketball.asm), the ball used by Smashketball.
 *
 * Each routine names its Remix label in its doc comment. The asm keeps its state in GObj and
 * ITStruct slots that the decomp structs do not name (and that overlap vanilla fields), so the
 * state lives in one file-local struct instead. Mode globals, vanilla hooks and open items
 * are listed in docs/remix/notes/item_Basketball.md.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "it/itremix/itremix.h"
#include "it/itremix/itremixbasketball.h"
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixsmashketballstate.h"
#include "it/itmain.h"
#include "it/itmap.h"
#include "it/itmanager.h"
#include "it/itvisuals.h"
#include "it/itcommon/itbombhei.h"
#include "ef/efmanager.h"
#include "ft/ftcommon/ftcommonfunctions.h"
#include "if/ifcommon.h"
#include "gr/grcommon/grjungle.h"
#include <lb/lbcommon.h>
#include <math.h>

#define ITREMIX_BASKETBALL_KIND 0x40
#define ITREMIX_BASKETBALL_FILE_OFFSET 0x1170 /* asm FILE_OFFSET: item footer in the common item file */
#define ITREMIX_BASKETBALL_ANIM_FRAMES 20
#define ITREMIX_BASKETBALL_ANIM_SPEED_DIVISOR 3.0F /* 0x40400000; the asm comment says 4 */
#define ITREMIX_BASKETBALL_IMAGE_COUNT 5
#define ITREMIX_BASKETBALL_IMAGE_SOCCER_OFFSET 5
#define ITREMIX_BASKETBALL_IMAGE_BASKETBALL 0
#define ITREMIX_BASKETBALL_BKB 25
#define ITREMIX_BASKETBALL_KBG 100
#define ITREMIX_BASKETBALL_KB_ANGLE 361
#define ITREMIX_BASKETBALL_BASE_DAMAGE 4
#define ITREMIX_BASKETBALL_DAMAGE_TYPE 1
#define ITREMIX_BASKETBALL_SHIELD_DAMAGE 30
#define ITREMIX_BASKETBALL_HIT_FGM 37
#define ITREMIX_BASKETBALL_INVINCIBILITY_TIMER 90
#define ITREMIX_BASKETBALL_INVINCIBILITY_FLASH_TIMER (ITREMIX_BASKETBALL_INVINCIBILITY_TIMER - 5)
#define ITREMIX_BASKETBALL_INVINCIBILITY_END_TIMER 1
#define ITREMIX_BASKETBALL_INVINCIBILITY_COLANIM 0x0A
#define ITREMIX_BASKETBALL_RESPAWN_TIMER 60
#define ITREMIX_BASKETBALL_BARREL_TIMER 90
#define ITREMIX_BASKETBALL_BARREL_SFX_TIMER (ITREMIX_BASKETBALL_BARREL_TIMER - 5)
#define ITREMIX_BASKETBALL_BARREL_REFRESH_TIMER 10
#define ITREMIX_BASKETBALL_BARREL_SPEED -180.0F /* 0xC3340000 */
#define ITREMIX_BASKETBALL_HIT_MULTIPLIER -0.84375F /* 0xBF590000 */
#define ITREMIX_BASKETBALL_HITBOX_REFRESH 16
#define ITREMIX_BASKETBALL_GRAVITY 1.65F /* 0x3FD33333 */
#define ITREMIX_BASKETBALL_MAX_SPEED 270.0F /* 0x43870000; the asm comment says 280 */
#define ITREMIX_BASKETBALL_FALL_SPEED -90.0F /* 0xC2B40000 */
#define ITREMIX_BASKETBALL_GROUND_FRICTION 0.96875F /* 0x3F780000 */
#define ITREMIX_BASKETBALL_GROUND_STOP_SPEED 2.0F /* 0x40000000 */
#define ITREMIX_BASKETBALL_FIRE_BEGIN_SPEED 120.0F /* 0x42F00000 */
#define ITREMIX_BASKETBALL_FIRE_END_SPEED 50.0F /* 0x42480000 */
#define ITREMIX_BASKETBALL_FIRE_END_PERCENT_SCALE 0.5F /* 0x3F000000 */
#define ITREMIX_BASKETBALL_FIRE_END_GFX_TIMER 60
#define ITREMIX_BASKETBALL_BOUNCE 0.85F /* basketball_attributes.BOUNCE */
#define ITREMIX_BASKETBALL_FIRE_BOUNCE 0.94921875F /* 0x3F730000, asm lui-only constant */
#define ITREMIX_BASKETBALL_PERCENT_MAX 300
#define ITREMIX_BASKETBALL_PERCENT_TIMER 210
#define ITREMIX_BASKETBALL_PERCENT_DECAY_SPEED 50.0F
#define ITREMIX_BASKETBALL_PERCENT_SCALE_A 0.06689453125F /* 0x3D890000 */
#define ITREMIX_BASKETBALL_PERCENT_SCALE_B 0.0556640625F /* 0x3D640000 */
#define ITREMIX_BASKETBALL_HIT_MULT_FIRE 1.75F /* 0x3FE00000 */
#define ITREMIX_BASKETBALL_HIT_MULT_NORMAL 1.25F /* 0x3FA00000 */
#define ITREMIX_BASKETBALL_HIT_BASE_ADD 26.0F /* 0x41D00000 */
#define ITREMIX_BASKETBALL_HIT_SPEED_SCALE_FIRE 0.00119781494140625F /* 0x3A9D0000 */
#define ITREMIX_BASKETBALL_HIT_SPEED_SCALE_NORMAL 0.003997802734375F /* 0x3B830000 */
#define ITREMIX_BASKETBALL_FIRE_DAMAGE_BASE 2.0F /* 0x40000000 */
#define ITREMIX_BASKETBALL_FIRE_DAMAGE_SPEED 0.099609375F /* 0x3DCC0000 */
#define ITREMIX_BASKETBALL_FIRE_ANGLE_SCALE 180.0F /* 0x43340000 */
#define ITREMIX_BASKETBALL_PI 3.1415927F /* RAM 0x8013FE60 in the asm; value not verified */
#define ITREMIX_BASKETBALL_FALL_TRIGGER_Y -4480.0F /* 0xC58C0000 */
#define ITREMIX_BASKETBALL_FALL_RESET_X 4224.0F /* 0x45840000 */
#define ITREMIX_BASKETBALL_FALL_RESET_VEL_X 12.5F /* 0x41480000 */
#define ITREMIX_BASKETBALL_FALL_RESET_VEL_Y 134.0F /* 0x43060000 */
#define ITREMIX_BASKETBALL_FALL_COOLDOWN 40
#define ITREMIX_BASKETBALL_COLLISION_MASK 0x0C21
#define ITREMIX_BASKETBALL_GROUND_MASK 0x0800
#define ITREMIX_BASKETBALL_GROUND_LAND_SPEED 5.0F /* 0x40A00000 */
#define ITREMIX_BASKETBALL_FIGHTER_GOBJ_ID 0x3E8 /* asm object type check for a player owner */
#define ITREMIX_BASKETBALL_NO_COMBO_OWNER 4
#define ITREMIX_BASKETBALL_HIT_COLANIM 5
#define ITREMIX_BASKETBALL_FIRE_COLANIM 0x84 /* GFXRoutine.id.BALL_FIRE: 0x55 + position 47 in add_gfx_routine order */
#define ITREMIX_BASKETBALL_SMOKE_COLANIM 0x85 /* GFXRoutine.id.BALL_SMOKE: 0x55 + position 48 */
#define ITREMIX_BASKETBALL_RENDER_ROUTINE 0x2E
#define ITREMIX_BASKETBALL_FGM_BOUNCE 0x42E
#define ITREMIX_BASKETBALL_FGM_FIRE_BOUNCE 0x42F
#define ITREMIX_BASKETBALL_FGM_BUZZER 0x430
#define ITREMIX_BASKETBALL_FGM_FIRE_START 25
#define ITREMIX_BASKETBALL_FGM_FIRE_START_2 183
#define ITREMIX_BASKETBALL_FGM_FIRE_END 275
#define ITREMIX_BASKETBALL_FGM_BARREL_SHOOT 281
#define ITREMIX_BASKETBALL_FGM_WHOOSH 18
#define ITREMIX_BASKETBALL_BOUNCE_SCALE_SPEED 40.0F            /* 0x42200000: bounce_patch_ squash threshold */
#define ITREMIX_BASKETBALL_BOUNCE_HITLAG_SCALE 0.030029296875F /* 0x3CF60000 */
#define ITREMIX_BASKETBALL_GOBJ_KIND 0x3F5                     /* nGCCommonKindItem: asm object type check for an item */

enum itRemixBasketballStatus
{
    nITRemixBasketballStatusMain,
    nITRemixBasketballStatusBarrel,
    nITRemixBasketballStatusRespawn,
    nITRemixBasketballStatusEnumCount
};

/* Remix state of the basketball. The asm keeps these fields in its GObj and ITStruct slots. One
 * static instance is used: Smashketball keeps exactly one basketball alive (basketball_main_ only
 * spawns one when none exists). A second basketball would share this state. */
typedef struct ITRemixBasketballVars
{
    s32 is_scored;             /* asm 0x40 (GObj): hoop scored flag */
    s32 owner_damage_timer;    /* asm 0x44 (GObj) */
    GObj *barrel_gobj;         /* asm 0x48 (GObj): barrel the ball is in */
    s32 is_above_rim;          /* asm 0x50 (GObj) */
    s32 anim_timer;            /* asm 0x54 (GObj): texture animation timer */
    s32 is_soccer;             /* asm 0x58 (GObj): 1 when Smashketball type is soccer */
    s32 is_golden;             /* asm 0x5C (GObj): 1 when the soccer ball spawns golden */
    f32 prev_x;                /* asm 0x60 (GObj) */
    f32 prev_y;                /* asm 0x64 (GObj) */
    s32 is_scoreable;          /* asm 0x68 (GObj) */
    s32 state_timer;           /* asm 0x1CC: invincibility, barrel and respawn timer */
    s32 hitbox_refresh_timer;  /* asm 0x1D0 */
    s32 is_fire;               /* asm 0x1D4 */
    s32 gfx_timer;             /* asm 0x1D8 */
    f32 bounce_x_offset;       /* asm 0x1DC */
    f32 bounce_y_offset;       /* asm 0x1E0 */
    s32 is_bounce_scaled;      /* asm 0x1E4 */
    s32 is_respawn;            /* asm 0x1E8 */
    f32 current_max_speed;     /* asm 0x1C8 */
    s32 percent;               /* asm 0x1EC */
    s32 percent_timer;         /* asm 0x1F0 */
    s32 cooldown_timer;        /* asm 0x1F4 */
} ITRemixBasketballVars;

extern void* func_800269C0_275C0(u16 id);

/* The asm keeps these fields in each ball's GObj and ITStruct (0x40..0x68, 0x1C8..0x1F4); they do not fit in
 * ITStruct.item_vars, so each ball gets a slot of this pool, claimed at spawn (itRemixBasketballSetupVars). */
#define ITREMIX_BASKETBALL_SLOTS 8

typedef struct ITRemixBasketballSlot
{
    GObj *item_gobj;
    ITRemixBasketballVars vars;

} ITRemixBasketballSlot;

static ITRemixBasketballSlot sITRemixBasketballSlots[ITREMIX_BASKETBALL_SLOTS];
static s32 sITRemixBasketballNextSlot;

static void itRemixBasketballSetupVars(GObj *item_gobj);
static void itRemixBasketballSetupImage(GObj *item_gobj);
static void itRemixBasketballSetupAttack(ITStruct *ip);
static void itRemixBasketballSetup(GObj *item_gobj);
static void itRemixBasketballSetGrounded(GObj *item_gobj);
static void itRemixBasketballSetAerial(GObj *item_gobj);
static void itRemixBasketballFireBegin(GObj *item_gobj);
static void itRemixBasketballRestoreBounce(GObj *item_gobj);
static void itRemixBasketballFireEndState(GObj *item_gobj);
static void itRemixBasketballFireGfxStep(GObj *item_gobj);
static void itRemixBasketballSmokeGfxStep(GObj *item_gobj);
static void itRemixBasketballFireStateManager(GObj *item_gobj);
static void itRemixBasketballAdjustHitbox(GObj *item_gobj);
static void itRemixBasketballAnimateImage(GObj *item_gobj);
static void itRemixBasketballUpdateGroundedMovement(GObj *item_gobj);
static void itRemixBasketballUpdateAerialMovement(GObj *item_gobj);
static void itRemixBasketballUpdateAnimation(GObj *item_gobj);
static void itRemixBasketballUpdateTimers(GObj *item_gobj);
static void itRemixBasketballUpdateHitboxRefresh(GObj *item_gobj);
static void itRemixBasketballUpdateInvincibility(GObj *item_gobj);
static void itRemixBasketballCheckFall(GObj *item_gobj);
static void itRemixBasketballCheckScored(GObj *item_gobj);
static void itRemixBasketballSetHitOwner(GObj *item_gobj);
static void itRemixBasketballAddHitPercent(ITStruct *ip);
static f32 itRemixBasketballGetHitKnockback(ITStruct *ip);
static void itRemixBasketballLaunch(GObj *item_gobj, f32 knockback);
static void itRemixBasketballSetScoreable(GObj *item_gobj);
static sb32 itRemixBasketballDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags);
static sb32 itRemixBasketballMainProcUpdate(GObj *item_gobj);
static sb32 itRemixBasketballMainProcMap(GObj *item_gobj);
static sb32 itRemixBasketballCollide(GObj *item_gobj);
static sb32 itRemixBasketballCollideHitbox(GObj *item_gobj);
static sb32 itRemixBasketballBarrelProcUpdate(GObj *item_gobj);
static sb32 itRemixBasketballRespawnProcUpdate(GObj *item_gobj);
static void itRemixBasketballBarrelShoot(GObj *item_gobj);
static void itRemixBasketballBarrelSetup(GObj *item_gobj);
static void itRemixBasketballDecayTimer(s32 *timer);
static f32 itRemixBasketballGetSpeed(ITStruct *ip);
static u8 itRemixBasketballGetComboOwner(GObj *owner_gobj);
static sb32 itRemixBasketballIsSoccer(void);
static sb32 itRemixBasketballIsGolden(void);

static ITStatusDesc sITRemixBasketballStatusDescs[nITRemixBasketballStatusEnumCount] =
{
    {
        itRemixBasketballMainProcUpdate,
        itRemixBasketballMainProcMap,
        itRemixBasketballCollide,
        itRemixBasketballCollide,
        itRemixBasketballCollide,
        NULL,
        itMainCommonProcReflector,
        itRemixBasketballCollideHitbox
    },
    {
        itRemixBasketballBarrelProcUpdate,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    },
    {
        itRemixBasketballRespawnProcUpdate,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    }
};

/* asm item_info_array */
static ITDesc sITRemixBasketballItemDesc =
{
    ITREMIX_BASKETBALL_KIND,
    &gITManagerCommonData,
    ITREMIX_BASKETBALL_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,            /* item_info_array 0x0C = 0x1B000000 */
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    /* item_info_array 0x14..0x30 is item_state_table state 0 (MAIN) */
    itRemixBasketballMainProcUpdate,
    itRemixBasketballMainProcMap,
    itRemixBasketballCollide,
    itRemixBasketballCollide,
    itRemixBasketballCollide,
    NULL,
    itMainCommonProcReflector,
    itRemixBasketballCollideHitbox
};

/**
 * State of one ball: its pool slot, or a new slot (the oldest one) the first time the ball asks.
 * item_gobj: item object.
 * Returns: pointer to the ball's state.
 */
static ITRemixBasketballVars* itRemixBasketballGetVars(GObj *item_gobj)
{
    ITRemixBasketballSlot *slot;
    s32 i;

    for (i = 0; i < ITREMIX_BASKETBALL_SLOTS; i++)
    {
        if (sITRemixBasketballSlots[i].item_gobj == item_gobj)
        {
            return &sITRemixBasketballSlots[i].vars;
        }
    }
    slot = &sITRemixBasketballSlots[sITRemixBasketballNextSlot];
    sITRemixBasketballNextSlot = (sITRemixBasketballNextSlot + 1) % ITREMIX_BASKETBALL_SLOTS;
    slot->item_gobj = item_gobj;
    bzero(&slot->vars, sizeof(slot->vars));

    return &slot->vars;
}

/**
 * Smashketball.type read: TRUE when the mode is soccer.
 */
static sb32 itRemixBasketballIsSoccer(void)
{
    return gmRemixSmashketballIsSoccer();
}

/**
 * Smashketball.spawn_golden read: TRUE when a soccer ball spawns golden.
 */
static sb32 itRemixBasketballIsGolden(void)
{
    return gmRemixSmashketballIsGolden();
}

/**
 * Basketball.decay_timer_: decrements a frame timer, stopping at zero.
 * timer: pointer to the timer to decrement.
 */
static void itRemixBasketballDecayTimer(s32 *timer)
{
    if (*timer != 0)
    {
        *timer -= 1;
    }
}

/**
 * Basketball.get_speed_: magnitude of the item's air velocity.
 * ip: item struct.
 * Returns: sqrt(x^2 + y^2) of vel_air.
 */
static f32 itRemixBasketballGetSpeed(ITStruct *ip)
{
    f32 x = ip->physics.vel_air.x;
    f32 y = ip->physics.vel_air.y;

    return sqrtf((x * x) + (y * y));
}

/**
 * Basketball.set_grounded_: sets the grounded kinetic state and the main status.
 * item_gobj: item object.
 */
static void itRemixBasketballSetGrounded(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->ga = FALSE;
    itMainSetStatus(item_gobj, sITRemixBasketballStatusDescs, nITRemixBasketballStatusMain);
}

/**
 * Basketball.set_aerial_: sets the aerial kinetic state and the main status.
 * item_gobj: item object.
 */
static void itRemixBasketballSetAerial(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->ga = TRUE;
    itMainSetStatus(item_gobj, sITRemixBasketballStatusDescs, nITRemixBasketballStatusMain);
}

/**
 * Basketball.setup_image_: texture index for a basketball or soccer ball.
 * SKIPPED(palette): Basketball.asm lines 139-145 write a palette address into the display list of the first joint for a
 * soccer ball (golden or not). The port has no map from those addresses to a palette_id, so the palette is not swapped.
 * item_gobj: item object.
 */
static void itRemixBasketballSetupImage(GObj *item_gobj)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    s32 image_index = ITREMIX_BASKETBALL_IMAGE_BASKETBALL;

    if (vars->is_soccer)
    {
        image_index = ITREMIX_BASKETBALL_IMAGE_SOCCER_OFFSET;
    }
    DObjGetStruct(item_gobj)->mobj->texture_id_curr = image_index;
}

/**
 * Basketball.setup_vars_: stage_setting_ bookkeeping of the Remix state, before the render setup.
 * item_gobj: item object.
 */
static void itRemixBasketballSetupVars(GObj *item_gobj)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    vars->is_scored = FALSE;
    vars->owner_damage_timer = 0;
    vars->barrel_gobj = NULL;
    vars->is_above_rim = TRUE;
    vars->is_scoreable = TRUE;
    vars->anim_timer = ITREMIX_BASKETBALL_ANIM_FRAMES;
    vars->is_soccer = itRemixBasketballIsSoccer();
    vars->is_golden = itRemixBasketballIsGolden();
    vars->prev_x = joint->translate.vec.f.x;
    vars->prev_y = joint->translate.vec.f.y;
    vars->state_timer = ITREMIX_BASKETBALL_INVINCIBILITY_TIMER;
    vars->hitbox_refresh_timer = 0;
    vars->is_fire = FALSE;
    vars->gfx_timer = 0;
    vars->bounce_x_offset = 0.0F;
    vars->bounce_y_offset = 0.0F;
    vars->is_bounce_scaled = FALSE;
    vars->is_respawn = FALSE;
    vars->percent = 0;
    vars->percent_timer = 0;
    vars->cooldown_timer = 0;
}

/**
 * Basketball.setup_attack_: hit values written by stage_setting_.
 * The asm's "disable clang" clears the can_setoff bit (halfword 0x8000 at 0x158).
 * ip: item struct.
 */
static void itRemixBasketballSetupAttack(ITStruct *ip)
{
    ip->attack_coll.can_setoff = FALSE;
    ip->attack_coll.damage = ITREMIX_BASKETBALL_BASE_DAMAGE;
    ip->attack_coll.knockback_scale = ITREMIX_BASKETBALL_KBG;
    ip->attack_coll.knockback_base = ITREMIX_BASKETBALL_BKB;
    ip->attack_coll.angle = ITREMIX_BASKETBALL_KB_ANGLE;
    ip->attack_coll.shield_damage = ITREMIX_BASKETBALL_SHIELD_DAMAGE;
    ip->attack_coll.fgm_id = ITREMIX_BASKETBALL_HIT_FGM;
    ip->attack_coll.element = ITREMIX_BASKETBALL_DAMAGE_TYPE;
}

/**
 * Basketball.set_golden_ (blast zone, ITStruct proc_dead 0x398): a ball that crosses a blast zone
 * makes the next Smashketball ball golden and is destroyed (asm: set_golden_ returns TRUE).
 * item_gobj: item object.
 * Returns: TRUE (the item is destroyed).
 */
static sb32 itRemixBasketballBlastZone(GObj *item_gobj)
{
    (void) item_gobj;
    gGMRemixSmashketball.spawn_golden = TRUE;
    return TRUE;
}

/**
 * Basketball.stage_setting_ (after itManagerMakeItem): the spawn setup of the asm.
 * item_gobj: item object.
 */
static void itRemixBasketballSetup(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    ip->proc_dead = itRemixBasketballBlastZone;
    itRemixBasketballSetupVars(item_gobj);
    itRemixBasketballSetupImage(item_gobj);
    itMainClearOwnerStats(item_gobj);
    gcAddXObjForDObjFixed(joint, ITREMIX_BASKETBALL_RENDER_ROUTINE, 0);
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    itRemixBasketballSetupAttack(ip);
    ip->vel_scale = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);
    func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_BUZZER);
}

/**
 * Basketball.stage_setting_ (SPAWN_ITEM). The vanilla ABI is (parent, pos, vel, flags).
 * parent_gobj: spawning object or NULL.
 * pos: spawn position.
 * vel: spawn velocity.
 * flags: spawn flags.
 * Returns: the item object, or NULL when no item was spawned.
 */
GObj* itRemixBasketballMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixBasketballItemDesc, pos, vel, flags);

    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixBasketballSetup(item_gobj);

    return item_gobj;
}

/**
 * Basketball.bounce_scale_restore_: undoes a bounce scale and offset before the next update.
 * item_gobj: item object.
 */
static void itRemixBasketballRestoreBounce(GObj *item_gobj)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    vars->is_bounce_scaled = FALSE;
    joint->scale.vec.f.x = 1.0F;
    joint->scale.vec.f.y = 1.0F;
    joint->translate.vec.f.x -= vars->bounce_x_offset;
    joint->translate.vec.f.y -= vars->bounce_y_offset;
    vars->bounce_x_offset = 0.0F;
    vars->bounce_y_offset = 0.0F;
}

/**
 * Basketball.bounce_patch_: applies the Remix bounce squash and reflects the
 * velocity (the asm helper ends by re-running the vanilla lbCommonReflect2D).
 * The squash and offset apply at speed 40 and above; the bounce hitlag is set
 * for every basketball bounce, scaled or not (the asm bc1fl only skips the
 * squash). No-op for items that are not the basketball.
 * item_gobj: item object that bounced.
 * angle: collision angle passed to lbCommonReflect2D.
 * x_scale, y_scale: joint scale of the squash.
 * x_offset, y_offset: translate offset, undone by itRemixBasketballRestoreBounce.
 */
void itRemixBasketballApplyBounce(GObj *item_gobj, Vec3f *angle, f32 x_scale, f32 y_scale, f32 x_offset, f32 y_offset)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);
    f32 speed = itRemixBasketballGetSpeed(ip);

    if (ip->kind == ITREMIX_BASKETBALL_KIND)
    {
        if (speed >= ITREMIX_BASKETBALL_BOUNCE_SCALE_SPEED)
        {
            joint->scale.vec.f.x = x_scale;
            joint->scale.vec.f.y = y_scale;
            vars->bounce_x_offset = x_offset;
            vars->bounce_y_offset = y_offset;
            vars->is_bounce_scaled = TRUE;
            joint->translate.vec.f.x += x_offset;
            joint->translate.vec.f.y += y_offset;
        }
        ip->hitlag_tics = (s32) (speed * ITREMIX_BASKETBALL_BOUNCE_HITLAG_SCALE) + 1;
        if (vars->is_bounce_scaled)
        {
            func_800269C0_275C0(vars->is_fire ? ITREMIX_BASKETBALL_FGM_FIRE_BOUNCE : ITREMIX_BASKETBALL_FGM_BOUNCE);
        }
    }
    lbCommonReflect2D(&ip->physics.vel_air, angle);
}

/**
 * Basketball multiply_player_hitlag_ / multiply_ball_hitlag_hurtbox_ /
 * add_ball_hitlag_hitbox_ checks: TRUE when item_gobj is the fire basketball.
 * item_gobj: item object to test, or NULL.
 */
sb32 itRemixBasketballIsFire(GObj *item_gobj)
{
    if ((item_gobj == NULL) || (item_gobj->id != ITREMIX_BASKETBALL_GOBJ_KIND))
    {
        return FALSE;
    }
    if (itGetStruct(item_gobj)->kind != ITREMIX_BASKETBALL_KIND)
    {
        return FALSE;
    }
    return itRemixBasketballGetVars(item_gobj)->is_fire;
}

/**
 * Basketball bounce_patch_ / prevent_bounce_fgm_ check: TRUE when item_gobj is
 * the basketball with the bounce squash applied.
 * item_gobj: item object to test, or NULL.
 */
sb32 itRemixBasketballIsBounceScaled(GObj *item_gobj)
{
    if ((item_gobj == NULL) || (item_gobj->id != ITREMIX_BASKETBALL_GOBJ_KIND))
    {
        return FALSE;
    }
    if (itGetStruct(item_gobj)->kind != ITREMIX_BASKETBALL_KIND)
    {
        return FALSE;
    }
    return itRemixBasketballGetVars(item_gobj)->is_bounce_scaled;
}

/**
 * Basketball.fire_begin_: the ball turns into fire at speed 120.
 * item_gobj: item object.
 */
static void itRemixBasketballFireBegin(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_FIRE_START);
    func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_FIRE_START_2);
    itMainCheckSetColAnimID(item_gobj, ITREMIX_BASKETBALL_FIRE_COLANIM, 0);
    vars->gfx_timer = 0;
    ip->attack_coll.attack_state = nGMAttackStateNew;
    vars->is_fire = TRUE;
}

/**
 * Basketball.fire_end_: the fire goes out at speed 50. Starts the smoke timer.
 * item_gobj: item object.
 */
static void itRemixBasketballFireEndState(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_FIRE_END);
    itMainCheckSetColAnimID(item_gobj, ITREMIX_BASKETBALL_SMOKE_COLANIM, 0);
    vars->gfx_timer = ITREMIX_BASKETBALL_FIRE_END_GFX_TIMER;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    vars->percent = (s32) ((f32) vars->percent * ITREMIX_BASKETBALL_FIRE_END_PERCENT_SCALE);
    vars->is_fire = FALSE;
}

/**
 * Basketball.smoke_gfx_: the smoke timer counts down after the fire, one smoke every 4 frames.
 * The smoke is efManagerFlameRandgcMakeEffect (asm 0x800FE9B4, called with only a0 = position set).
 * item_gobj: item object.
 */
static void itRemixBasketballSmokeGfxStep(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    s32 gfx_timer = vars->gfx_timer;

    if (gfx_timer == 0)
    {
        return;
    }
    vars->gfx_timer -= 1;
    if ((gfx_timer & 3) == 0)
    {
        efManagerFlameRandgcMakeEffect(ip->coll_data.p_translate);
    }
}

/**
 * Basketball.fire_gfx_ (fire on): every second frame spawns a flame in the direction of travel.
 * Then the hitbox is updated.
 * item_gobj: item object.
 */
static void itRemixBasketballFireGfxStep(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    s32 gfx_timer = vars->gfx_timer;
    s32 lr = (ip->physics.vel_air.x < 0.0F) ? 1 : -1;

    vars->gfx_timer += 1;
    if ((gfx_timer & 1) == 0)
    {
        efManagerFlameLRMakeEffect(ip->coll_data.p_translate, lr);
    }
    itRemixBasketballAdjustHitbox(item_gobj);
}

/**
 * Basketball.fire_state_manager_: switches the fire state and the fire and smoke gfx.
 * item_gobj: item object.
 */
static void itRemixBasketballFireStateManager(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    f32 speed = itRemixBasketballGetSpeed(ip);

    if (vars->cooldown_timer != 0)
    {
        return;
    }
    if (!vars->is_fire)
    {
        if (speed < ITREMIX_BASKETBALL_FIRE_BEGIN_SPEED)
        {
            itRemixBasketballSmokeGfxStep(item_gobj);
            return;
        }
        itRemixBasketballFireBegin(item_gobj);
        itRemixBasketballFireGfxStep(item_gobj);
        return;
    }
    if (speed <= ITREMIX_BASKETBALL_FIRE_END_SPEED)
    {
        itRemixBasketballFireEndState(item_gobj);
        itRemixBasketballSmokeGfxStep(item_gobj);
        return;
    }
    itRemixBasketballFireGfxStep(item_gobj);
}

/**
 * Basketball.adjust_hitbox_: the fire hitbox damage and knockback angle follow the speed.
 * The asm uses cvt.w.s and trunc.w.s; the casts here truncate.
 * item_gobj: item object.
 */
static void itRemixBasketballAdjustHitbox(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    f32 speed = itRemixBasketballGetSpeed(ip);
    f32 angle_rad = atan2f(ip->physics.vel_air.y, fabsf(ip->physics.vel_air.x));
    f32 angle_deg = (angle_rad / ITREMIX_BASKETBALL_PI) * ITREMIX_BASKETBALL_FIRE_ANGLE_SCALE;

    ip->attack_coll.damage = (s32) (ITREMIX_BASKETBALL_FIRE_DAMAGE_BASE + (speed * ITREMIX_BASKETBALL_FIRE_DAMAGE_SPEED));
    ip->attack_coll.angle = (s32) angle_deg;
}

/**
 * Basketball.animate_image_: steps the spin texture one frame, in the direction of travel.
 * item_gobj: item object.
 */
static void itRemixBasketballAnimateImage(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    MObj *mobj = DObjGetStruct(item_gobj)->mobj;
    s32 offset = vars->is_soccer ? ITREMIX_BASKETBALL_IMAGE_SOCCER_OFFSET : 0;
    s32 image_index = mobj->texture_id_curr - offset;
    s32 step = (ip->physics.vel_air.x < 0.0F) ? 1 : -1;
    s32 next_index = image_index + step;

    if (next_index >= ITREMIX_BASKETBALL_IMAGE_COUNT)
    {
        next_index = 0;
    }
    if (next_index < 0)
    {
        next_index = ITREMIX_BASKETBALL_IMAGE_COUNT - 1;
    }
    mobj->texture_id_curr = next_index + offset;
}

/**
 * Basketball.main_ (animate): advances the spin timer by the travel speed, then steps the image.
 * item_gobj: item object.
 */
static void itRemixBasketballUpdateAnimation(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    s32 frames = (s32) (itRemixBasketballGetSpeed(ip) / ITREMIX_BASKETBALL_ANIM_SPEED_DIVISOR) + 1;

    vars->anim_timer -= frames;
    if (vars->anim_timer > 0)
    {
        return;
    }
    vars->anim_timer = ITREMIX_BASKETBALL_ANIM_FRAMES;
    itRemixBasketballAnimateImage(item_gobj);
}

/**
 * Basketball.main_ (grounded): friction, and the percent resets once the ball stops.
 * item_gobj: item object.
 */
static void itRemixBasketballUpdateGroundedMovement(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.x *= ITREMIX_BASKETBALL_GROUND_FRICTION;
    if (fabsf(ip->physics.vel_air.x) < ITREMIX_BASKETBALL_GROUND_STOP_SPEED)
    {
        vars->percent = 0;
        ip->physics.vel_air.x = 0.0F;
        return;
    }
    itRemixBasketballUpdateAnimation(item_gobj);
}

/**
 * Basketball.main_ (aerial): spin, gravity (off while falling fast), spin again.
 * SKIPPED(barrel): check_basketball_barrel_collision_ (Basketball.asm line 909) jumps into the middle of the vanilla
 * grJungleTaruCannCheckGetDamageKind (0x8010A018, grjungle.c) with the ball as its fighter argument. That entry point
 * reads the argument as a fighter, so its result for a ball was not verified. Not ported.
 * item_gobj: item object.
 */
static void itRemixBasketballUpdateAerialMovement(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    f32 gravity = (ip->physics.vel_air.y >= ITREMIX_BASKETBALL_FALL_SPEED) ? ITREMIX_BASKETBALL_GRAVITY : 0.0F;

    itVisualsUpdateSpin(item_gobj);
    itMainApplyGravityClampTVel(ip, gravity, ITREMIX_BASKETBALL_MAX_SPEED);
    itVisualsUpdateSpin(item_gobj);
}

/**
 * Basketball.hitbox_refresh_: clears the hit object records every refresh period.
 * item_gobj: item object.
 */
static void itRemixBasketballUpdateHitboxRefresh(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    s32 i;

    itRemixBasketballDecayTimer(&vars->hitbox_refresh_timer);
    if (vars->hitbox_refresh_timer != 0)
    {
        return;
    }
    for (i = 0; i < ARRAY_COUNT(ip->attack_coll.attack_records); i++)
    {
        ip->attack_coll.attack_records[i].victim_gobj = NULL;
    }
}

/**
 * Basketball.main_ (timers): owner damage, state, cooldown and percent timers.
 * The percent falls by one each frame while the ball is slow and its percent timer has ended.
 * item_gobj: item object.
 */
static void itRemixBasketballUpdateTimers(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    itRemixBasketballDecayTimer(&vars->owner_damage_timer);
    itRemixBasketballDecayTimer(&vars->state_timer);
    itRemixBasketballDecayTimer(&vars->cooldown_timer);
    itRemixBasketballDecayTimer(&vars->percent_timer);
    if (vars->percent_timer != 0)
    {
        return;
    }
    if (itRemixBasketballGetSpeed(ip) > ITREMIX_BASKETBALL_PERCENT_DECAY_SPEED)
    {
        return;
    }
    itRemixBasketballDecayTimer(&vars->percent);
}

/**
 * Basketball.main_ (invincibility): the flash starts at 85 frames and the hurtbox returns at 1.
 * item_gobj: item object.
 */
static void itRemixBasketballUpdateInvincibility(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    if (ip->damage_coll.hitstatus != nGMHitStatusNone)
    {
        return;
    }
    if (vars->state_timer == ITREMIX_BASKETBALL_INVINCIBILITY_FLASH_TIMER)
    {
        itMainCheckSetColAnimID(item_gobj, ITREMIX_BASKETBALL_INVINCIBILITY_COLANIM, 0);
        return;
    }
    if (vars->state_timer == ITREMIX_BASKETBALL_INVINCIBILITY_END_TIMER)
    {
        itMainClearColAnim(item_gobj);
        ip->damage_coll.hitstatus = nGMHitStatusNormal;
    }
}

/**
 * Basketball.check_fall_: a ball below the stage returns to the centre of its side of the stage.
 * item_gobj: item object.
 */
static void itRemixBasketballCheckFall(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);
    f32 side = (joint->translate.vec.f.x < 0.0F) ? -1.0F : 1.0F;

    if (joint->translate.vec.f.y >= ITREMIX_BASKETBALL_FALL_TRIGGER_Y)
    {
        return;
    }
    joint->translate.vec.f.x = side * ITREMIX_BASKETBALL_FALL_RESET_X;
    joint->translate.vec.f.y = ITREMIX_BASKETBALL_FALL_TRIGGER_Y;
    ip->physics.vel_air.x = -side * ITREMIX_BASKETBALL_FALL_RESET_VEL_X;
    ip->physics.vel_air.y = ITREMIX_BASKETBALL_FALL_RESET_VEL_Y;
    vars->cooldown_timer = ITREMIX_BASKETBALL_FALL_COOLDOWN;
    func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_WHOOSH);
}

/**
 * Basketball.check_scored_: a scored ball switches to the respawn status.
 * item_gobj: item object.
 */
static void itRemixBasketballCheckScored(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    if (!vars->is_scored)
    {
        return;
    }
    vars->state_timer = 0;
    vars->hitbox_refresh_timer = 0;
    vars->is_fire = FALSE;
    vars->gfx_timer = 0;
    ip->attack_coll.attack_state = nGMAttackStateOff;
    ip->damage_coll.hitstatus = nGMHitStatusNone;
    vars->is_respawn = TRUE;
    itMainSetStatus(item_gobj, sITRemixBasketballStatusDescs, nITRemixBasketballStatusRespawn);
}

/**
 * Basketball.main_ (proc_update): one frame of the free ball.
 * item_gobj: item object.
 * Returns: FALSE (the ball is never destroyed here).
 */
static sb32 itRemixBasketballMainProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    if (vars->is_bounce_scaled)
    {
        itRemixBasketballRestoreBounce(item_gobj);
    }
    if (ip->ga)
    {
        itRemixBasketballUpdateAerialMovement(item_gobj);
        itRemixBasketballUpdateAnimation(item_gobj);
    }
    else
    {
        itRemixBasketballUpdateGroundedMovement(item_gobj);
    }
    itRemixBasketballUpdateTimers(item_gobj);
    itRemixBasketballUpdateHitboxRefresh(item_gobj);
    itRemixBasketballFireStateManager(item_gobj);
    itRemixBasketballUpdateInvincibility(item_gobj);
    itRemixBasketballCheckFall(item_gobj);
    itRemixBasketballCheckScored(item_gobj);
    vars->prev_x = joint->translate.vec.f.x;
    vars->prev_y = joint->translate.vec.f.y;

    return FALSE;
}

/**
 * Basketball.detect_collision_ (grenade_detect_collision_): wall, ceiling and floor checks.
 * Copy of itRemixConkerGrenadeDetectCollision (itremixconkergrenade.c), which is static there.
 * coll_data: collision data of the ball.
 * item_gobj: item object (unused).
 * coll_flags: collision mask to test.
 * Returns: TRUE when a collision in coll_flags is set.
 */
static sb32 itRemixBasketballDetectCollision(MPCollData *coll_data, GObj *item_gobj, u32 coll_flags)
{
    (void) item_gobj;
    if (mpProcessCheckTestLWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunLWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestRWallCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunRWallCollisionAdjNew(coll_data);
    }
    if (mpProcessCheckTestCeilCollisionAdjNew(coll_data) != FALSE)
    {
        mpProcessRunCeilCollisionAdjNew(coll_data);
        if (coll_data->mask_stat & MAP_FLAG_CEIL)
        {
            mpProcessRunCeilEdgeAdjust(coll_data);
        }
    }
    if (mpProcessRunFloorCollisionAdjNewNULL(coll_data) != FALSE)
    {
        mpProcessSetLandingFloor(coll_data);
    }
    return (coll_data->mask_curr & coll_flags) != 0;
}

/**
 * Basketball.collision_ (proc_map): map collision and the ground check.
 * item_gobj: item object.
 * Returns: FALSE (the ball is never destroyed here).
 */
static sb32 itRemixBasketballMainProcMap(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    f32 bounce = vars->is_fire ? ITREMIX_BASKETBALL_FIRE_BOUNCE : ITREMIX_BASKETBALL_BOUNCE;
    sb32 is_collided;

    if (!ip->ga)
    {
        itMapCheckLRWallProcNoFloor(item_gobj, itRemixBasketballSetAerial);
        return FALSE;
    }
    is_collided = mpProcessUpdateMain(&ip->coll_data, itRemixBasketballDetectCollision, item_gobj, ITREMIX_BASKETBALL_COLLISION_MASK);
    itMapCheckCollideAllRebound(item_gobj, ITREMIX_BASKETBALL_COLLISION_MASK, bounce, NULL);
    if (!is_collided)
    {
        return FALSE;
    }
    if ((ip->coll_data.mask_stat & ITREMIX_BASKETBALL_GROUND_MASK) == 0)
    {
        return FALSE;
    }
    if (ip->physics.vel_air.y >= ITREMIX_BASKETBALL_GROUND_LAND_SPEED)
    {
        return FALSE;
    }
    ip->physics.vel_air.y = 0.0F;
    itRemixBasketballSetGrounded(item_gobj);

    return FALSE;
}

/**
 * Basketball.collide_ (hit, shield and hop procs): slows the ball and sets the hitbox refresh.
 * item_gobj: item object.
 * Returns: FALSE (the ball is never destroyed here).
 */
static sb32 itRemixBasketballCollide(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    ip->physics.vel_air.x *= ITREMIX_BASKETBALL_HIT_MULTIPLIER;
    ip->physics.vel_air.y *= ITREMIX_BASKETBALL_HIT_MULTIPLIER;
    vars->hitbox_refresh_timer = ITREMIX_BASKETBALL_HITBOX_REFRESH;
    itRemixBasketballSetAerial(item_gobj);
    func_800269C0_275C0(vars->is_fire ? ITREMIX_BASKETBALL_FGM_FIRE_BOUNCE : ITREMIX_BASKETBALL_FGM_BOUNCE);

    return FALSE;
}

/**
 * Basketball.set_hit_owner_: the last attacker becomes the owner, with its combo owner port.
 * owner_gobj: attacker object, or NULL.
 * Returns: the port number for combo ownership, or ITREMIX_BASKETBALL_NO_COMBO_OWNER.
 */
static u8 itRemixBasketballGetComboOwner(GObj *owner_gobj)
{
    if (owner_gobj == NULL)
    {
        return ITREMIX_BASKETBALL_NO_COMBO_OWNER;
    }
    if (owner_gobj->id != ITREMIX_BASKETBALL_FIGHTER_GOBJ_ID)
    {
        return ITREMIX_BASKETBALL_NO_COMBO_OWNER;
    }
    return (u8) ftGetStruct(owner_gobj)->player;
}

/**
 * Basketball.collide_hitbox_ owner step: owner and combo owner from the hit data.
 * The hit data offsets in the asm are +4 from the decomp field names (see notes, "Hit data layout").
 * item_gobj: item object.
 */
static void itRemixBasketballSetHitOwner(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->owner_gobj = ip->damage_gobj;
    ip->player = itRemixBasketballGetComboOwner(ip->damage_gobj);
}

/**
 * Basketball.collide_hitbox_ percent step: adds the hit damage to the percent (capped at 300)
 * and restarts the percent timer.
 * ip: item struct, with the hit data already stored.
 */
static void itRemixBasketballAddHitPercent(ITStruct *ip)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(ip->item_gobj);
    s32 percent_total = vars->percent + ip->damage_queue;

    vars->percent = (percent_total < ITREMIX_BASKETBALL_PERCENT_MAX + 1) ? percent_total : ITREMIX_BASKETBALL_PERCENT_MAX;
    vars->percent_timer = ITREMIX_BASKETBALL_PERCENT_TIMER;
}

/**
 * Basketball.collide_hitbox_ knockback: the knockback value from the percent, damage and speed.
 * ip: item struct, with the hit data already stored.
 * Returns: knockback, capped at MAX_SPEED.
 */
static f32 itRemixBasketballGetHitKnockback(ITStruct *ip)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(ip->item_gobj);
    f32 damage = (f32) ip->damage_queue;
    f32 percent = (f32) vars->percent;
    f32 knockback = (ITREMIX_BASKETBALL_PERCENT_SCALE_A * percent) + (ITREMIX_BASKETBALL_PERCENT_SCALE_B * (percent * damage));
    f32 hit_mult = vars->is_fire ? ITREMIX_BASKETBALL_HIT_MULT_FIRE : ITREMIX_BASKETBALL_HIT_MULT_NORMAL;
    f32 speed_mult = vars->is_fire ? ITREMIX_BASKETBALL_HIT_SPEED_SCALE_FIRE : ITREMIX_BASKETBALL_HIT_SPEED_SCALE_NORMAL;

    knockback = (knockback * hit_mult) + ITREMIX_BASKETBALL_HIT_BASE_ADD;
    knockback *= 1.0F + (itRemixBasketballGetSpeed(ip) * speed_mult);
    if (knockback > ITREMIX_BASKETBALL_MAX_SPEED)
    {
        knockback = ITREMIX_BASKETBALL_MAX_SPEED;
    }
    return knockback;
}

/**
 * Basketball.collide_hitbox_ launch: sets the velocity from the knockback, the angle and the hit side.
 * item_gobj: item object.
 * knockback: launch speed.
 */
static void itRemixBasketballLaunch(GObj *item_gobj, f32 knockback)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    f32 angle = ftCommonDamageGetKnockbackAngle(ip->damage_angle, ip->ga, knockback);
    f32 vel_x = knockback * cosf(angle);
    f32 vel_y = knockback * sinf(angle);
    s32 direction = -ip->damage_lr;

    vars->current_max_speed = knockback;
    ip->lr = direction;
    ip->physics.vel_air.x = vel_x * (f32) direction;
    ip->physics.vel_air.y = vel_y;
    vars->hitbox_refresh_timer = ITREMIX_BASKETBALL_HITBOX_REFRESH;
}

/**
 * Basketball.collide_hitbox_ scoreable step: soccer is not scoreable, basketball is.
 * item_gobj: item object.
 */
static void itRemixBasketballSetScoreable(GObj *item_gobj)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    (void) item_gobj;
    if (itRemixBasketballIsSoccer())
    {
        return;
    }
    vars->is_scoreable = TRUE;
}

/**
 * Basketball.collide_hitbox_ (proc_damage): a hitbox launches the ball.
 * item_gobj: item object.
 * Returns: FALSE (the ball is never destroyed here).
 */
static sb32 itRemixBasketballCollideHitbox(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    f32 knockback;

    itRemixBasketballSetAerial(item_gobj);
    itRemixBasketballSetHitOwner(item_gobj);
    itRemixBasketballAddHitPercent(ip);
    knockback = itRemixBasketballGetHitKnockback(ip);
    itRemixBasketballLaunch(item_gobj, knockback);
    if (!vars->is_fire)
    {
        itMainCheckSetColAnimID(item_gobj, ITREMIX_BASKETBALL_HIT_COLANIM, 0);
    }
    itRemixBasketballSetScoreable(item_gobj);

    return FALSE;
}

/**
 * Basketball.barrel_shoot_: the ball leaves the barrel upward.
 * item_gobj: item object.
 */
static void itRemixBasketballBarrelShoot(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);

    grJungleTaruCannAddAnimShoot(vars->barrel_gobj);
    DObjGetStruct(item_gobj)->flags = DOBJ_FLAG_NONE;
    ip->attack_coll.attack_state = nGMAttackStateNew;
    ip->physics.vel_air.y = ITREMIX_BASKETBALL_BARREL_SPEED;
    vars->state_timer = ITREMIX_BASKETBALL_BARREL_REFRESH_TIMER;
    itMainSetStatus(item_gobj, sITRemixBasketballStatusDescs, nITRemixBasketballStatusMain);
}

/**
 * Basketball.barrel_setup_: freezes the ball in the barrel at the barrel's centre and hides it
 * (asm: DObj flags byte 0x54 = 2, DOBJ_FLAG_HIDDEN). The barrel shoot shows it again.
 * item_gobj: item object.
 */
static void itRemixBasketballBarrelSetup(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);
    DObj *barrel_joint = DObjGetStruct(vars->barrel_gobj);

    joint->flags = DOBJ_FLAG_HIDDEN;
    ip->physics.vel_air.x = 0.0F;
    ip->physics.vel_air.y = 0.0F;
    ip->physics.vel_air.z = 0.0F;
    joint->translate.vec.f.x = barrel_joint->translate.vec.f.x;
    joint->translate.vec.f.y = barrel_joint->translate.vec.f.y;
    joint->translate.vec.f.z = barrel_joint->translate.vec.f.z;
}

/**
 * Basketball.barrel_main_ (proc_update, status 1): the ball waits inside the barrel and shoots.
 * item_gobj: item object.
 * Returns: FALSE (the ball is never destroyed here).
 */
static sb32 itRemixBasketballBarrelProcUpdate(GObj *item_gobj)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    s32 timer = vars->state_timer;

    vars->state_timer += 1;
    if (timer == 0)
    {
        itRemixBasketballBarrelSetup(item_gobj);
    }
    if (vars->state_timer == ITREMIX_BASKETBALL_BARREL_SFX_TIMER)
    {
        func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_BARREL_SHOOT);
        return FALSE;
    }
    if (vars->state_timer < ITREMIX_BASKETBALL_BARREL_TIMER)
    {
        return FALSE;
    }
    itRemixBasketballBarrelShoot(item_gobj);
    func_800269C0_275C0(ITREMIX_BASKETBALL_FGM_BARREL_SHOOT);

    return FALSE;
}

/**
 * Basketball.respawn_main_ (proc_update, status 2): the ball waits, then it is destroyed so
 * Smashketball respawns it. The asm hides the ball on the first frame (DObj flags = DOBJ_FLAG_HIDDEN).
 * item_gobj: item object.
 * Returns: TRUE when the respawn time is over (the item is destroyed).
 */
static sb32 itRemixBasketballRespawnProcUpdate(GObj *item_gobj)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    ITStruct *ip = itGetStruct(item_gobj);
    s32 timer = vars->state_timer;

    vars->state_timer += 1;
    if (timer == 0)
    {
        DObjGetStruct(item_gobj)->flags = DOBJ_FLAG_HIDDEN;
        ip->physics.vel_air.x = 0.0F;
        ip->physics.vel_air.y = 0.0F;
        ip->physics.vel_air.z = 0.0F;
    }
    if (vars->state_timer < ITREMIX_BASKETBALL_RESPAWN_TIMER)
    {
        return FALSE;
    }

    return TRUE;
}

#define ITREMIX_BASKETBALL_HOOP_OFFSET 267.0F        /* Smashketball.asm basketball_main_ HOOP_OFFSET (0x43858000) */
#define ITREMIX_BASKETBALL_HOOP_MIN_X 3504.0F        /* HOOP_MIN (0x455B0000): the ball must be past this |x| */
#define ITREMIX_BASKETBALL_GOAL_X 4288.0F            /* soccer goal |x| (0x45860000) */

/**
 * Smashketball.asm basketball_main_: the first ball in the item list.
 * Returns: the ball object, or NULL when there is none.
 */
GObj* itRemixBasketballFind(void)
{
    GObj *item_gobj;

    for (item_gobj = gGCCommonLinks[nGCCommonLinkIDItem]; item_gobj != NULL; item_gobj = item_gobj->link_next)
    {
        if (itGetStruct(item_gobj)->kind == ITREMIX_BASKETBALL_KIND)
        {
            return item_gobj;
        }
    }
    return NULL;
}

/**
 * Smashketball.asm basketball_main_ _get_position: the ball position, taken from the item joint of its owner while held.
 * item_gobj: ball object.
 * pos: receives the world position.
 */
void itRemixBasketballGetPosition(GObj *item_gobj, Vec3f *pos)
{
    ITStruct *ip = itGetStruct(item_gobj);
    GObj *owner_gobj = ip->owner_gobj;

    if (owner_gobj != NULL)
    {
        FTStruct *fp = ftGetStruct(owner_gobj);

        if (fp->item_gobj == item_gobj)
        {
            s32 joint_id = (ip->weight == nITWeightHeavy) ? fp->attr->joint_itemheavy_id : fp->attr->joint_itemlight_id;

            pos->x = pos->y = pos->z = 0.0F;
            gmCollisionGetFighterPartsWorldPosition(fp->joints[joint_id], pos);
            return;
        }
    }
    *pos = DObjGetStruct(item_gobj)->translate.vec.f;
}

/* The x of the ball when it crossed the height of the hoop between the last frame and this one. */
static f32 itRemixBasketballGetHoopCrossX(ITRemixBasketballVars *vars, Vec3f *pos, f32 hoop_y)
{
    f32 dx = pos->x - vars->prev_x;
    f32 dy = pos->y - vars->prev_y;

    if ((dx == 0.0F) || (dy == 0.0F))
    {
        return pos->x;
    }
    return (((hoop_y - pos->y) * (dx / dy)) + pos->x);
}

/**
 * Smashketball.asm basketball_main_ (basketball branch): updates the above-rim state from the current position and tells
 * when the ball went from above the rim to below it through the hoop.
 * Returns: TRUE when it scored. `*x` receives the x the ball crossed the hoop at.
 */
static sb32 itRemixBasketballCheckRim(ITRemixBasketballVars *vars, Vec3f *pos, f32 baskets_y)
{
    f32 hoop_y = baskets_y - ITREMIX_BASKETBALL_HOOP_OFFSET;
    sb32 is_above = (pos->y >= hoop_y) ? TRUE : FALSE;
    f32 cross_x;
    sb32 is_passing;

    if (vars->is_above_rim == is_above)
    {
        return FALSE;
    }
    vars->is_above_rim = is_above;
    cross_x = itRemixBasketballGetHoopCrossX(vars, pos, hoop_y);
    is_passing = (ABSF(cross_x) >= ITREMIX_BASKETBALL_HOOP_MIN_X) ? TRUE : FALSE;

    if (is_above != FALSE)
    {
        vars->is_scoreable = (is_passing != FALSE) ? FALSE : TRUE;
        return FALSE;
    }
    return ((is_passing != FALSE) && (vars->is_scoreable != FALSE)) ? TRUE : FALSE;
}

/**
 * Smashketball.asm basketball_main_: one frame of basket or goal detection.
 * item_gobj: ball object.
 * baskets_y: y of the baskets joint (basketball only).
 * is_soccer: TRUE for the soccer goals.
 * side: receives -1 when the ball went into the left hoop, 1 for the right one.
 * Returns: TRUE when the ball scored this frame; the ball is then marked scored and goes to its respawn status.
 */
sb32 itRemixBasketballCheckHoop(GObj *item_gobj, f32 baskets_y, sb32 is_soccer, s32 *side)
{
    ITRemixBasketballVars *vars = itRemixBasketballGetVars(item_gobj);
    Vec3f pos;
    sb32 is_scored;

    if (vars->is_scored != FALSE)
    {
        return FALSE;
    }
    itRemixBasketballGetPosition(item_gobj, &pos);

    if (is_soccer != FALSE)
    {
        is_scored = (ABSF(pos.x) >= ITREMIX_BASKETBALL_GOAL_X) ? TRUE : FALSE;
    }
    else is_scored = itRemixBasketballCheckRim(vars, &pos, baskets_y);

    if (is_scored == FALSE)
    {
        return FALSE;
    }
    *side = (pos.x < 0.0F) ? -1 : 1;
    vars->is_scored = *side;

    return TRUE;
}

/** TRUE when the ball spawned as the golden soccer ball (itRemixBasketballIsGolden). */
sb32 itRemixBasketballIsGoldenBall(GObj *item_gobj)
{
    return itRemixBasketballGetVars(item_gobj)->is_golden;
}

/**
 * Basketball descriptor: the item dispatch reads this for id 0x40.
 * The asm has no pickup, drop, throw or touch routines.
 */
const ITRemixItemDesc dITRemixBasketballItemDesc =
{
    ITREMIX_BASKETBALL_KIND,            /* Item.Basketball.id */
    itRemixBasketballMakeItem,          /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT: none in the asm */
    NULL,                               /* PICKUP_ITEM_MAIN: none in the asm */
    NULL,                               /* DROP_ITEM: none in the asm */
    NULL,                               /* THROW_ITEM: none in the asm */
    NULL                                /* PLAYER_COLLISION: none in the asm */
};

#endif
