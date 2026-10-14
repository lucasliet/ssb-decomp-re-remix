/**
 * itremixcar.c - C port of the Smash Remix Car (Item.Car.id 0x3C) and its Onett hazard
 * (.toolchain/pkgs/smashremix-src/src/items/Car.asm, Hazards.asm onett_setup and car_*).
 *
 * The car is parked off stage. After the match has started it honks, then drives across
 * the stage. The hazard timer lives in sITRemixCarState because the Remix asm keeps it in
 * the shared hazard storage (0x801313F0). Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixcar.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "mp/map.h"
#include "sc/scmanager.h"
#include "sc/sctypes.h"
#include "sys/objman.h"
#include "sys/utils.h"

#define ITREMIX_CAR_KIND 0x3C /* Item.Car.id */
#define ITREMIX_CAR_FILE_OFFSET 0xE0 /* asm FILE_OFFSET: item footer in the Onett header */
#define ITREMIX_CAR_DAMAGE 20
#define ITREMIX_CAR_ANGLE 16 /* asm 0x13C: horizontal hit */
#define ITREMIX_CAR_SIZE 700.0F /* asm 0x442F0000 */
#define ITREMIX_CAR_OFFSET_Y (-700.0F) /* asm 0xC42F0000 */
#define ITREMIX_CAR_OFFSET_Z (-1200.0F) /* asm 0xC4960000 */
#define ITREMIX_CAR_KNOCKBACK_SCALE 32
#define ITREMIX_CAR_KNOCKBACK_BASE 128
#define ITREMIX_CAR_HIT_FGM 0x011F
#define ITREMIX_CAR_HONK_FGM 0x041C
#define ITREMIX_CAR_MOVE_FGM 0x0481
#define ITREMIX_CAR_SCALE 0.75F /* asm 0x3F400000 */
#define ITREMIX_CAR_SPEED (-150.0F) /* asm 0xC3160000, written to physics.vel_air.x */
#define ITREMIX_CAR_ITEM_FLAG ITEM_FLAG_PARENT_GROUND /* asm argument 4 of 0x8016E174 is 1 */
#define ITREMIX_CAR_RANDOM_RANGE 200
#define ITREMIX_CAR_HONK_ROLL 80 /* a roll of 80 out of 200 honks at once */
#define ITREMIX_CAR_WAIT_MIN_TIMER 2700 /* frames before the honk roll starts (asm 0x0A8C) */
#define ITREMIX_CAR_WAIT_MAX_TIMER 3600 /* frames after which the car honks without a roll (asm 0x0E10) */
#define ITREMIX_CAR_HONK_TO_SPAWN_FRAMES 35 /* asm 0x23 */

/* asm car_coordinates: the car waits off stage at this point. */
static Vec3f sITRemixCarSpawnPos = { 7000.0F, 1400.0F, 1500.0F };
static Vec3f sITRemixCarSpawnVel = { 0.0F, 0.0F, 0.0F };

/* The hazard state of the Onett stage. stage_header is the p_file slot of the car item. */
typedef struct ITRemixCarState
{
    void *stage_header;
    Gfx *display_list;
    s32 honk_timer;
    sb32 is_honked;
} ITRemixCarState;

static ITRemixCarState sITRemixCarState;

static sb32 itRemixCarMainProcUpdate(GObj *item_gobj);
static sb32 itRemixCarBlastZone(GObj *item_gobj);
static void itRemixCarApplyHitbox(ITStruct *ip);
static sb32 itRemixCarCanTick(void);
static void itRemixCarWaitTick(s32 timer);
static void itRemixCarHonkedTick(GObj *item_gobj, s32 timer);
static void itRemixCarHonk(void);
static void itRemixCarDrive(GObj *item_gobj);

extern void* func_800269C0_275C0(u16 id);

/* asm item_info_array. p_file is the Onett header slot, so the car footer is read from the stage file. */
static ITDesc sITRemixCarDesc =
{
    ITREMIX_CAR_KIND,
    &sITRemixCarState.stage_header,
    ITREMIX_CAR_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyRSca,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixCarMainProcUpdate,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/**
 * Car.main_ (proc_update, asm states 0 and 1). The honk timer counts up only once the match has
 * started, outside training mode.
 */
static sb32 itRemixCarMainProcUpdate(GObj *item_gobj)
{
    s32 timer;

    if (!itRemixCarCanTick())
    {
        return FALSE;
    }
    timer = sITRemixCarState.honk_timer;
    sITRemixCarState.honk_timer = timer + 1;
    if (sITRemixCarState.is_honked)
    {
        itRemixCarHonkedTick(item_gobj, timer);
    }
    else
    {
        itRemixCarWaitTick(timer);
    }

    return FALSE;
}

/**
 * Car.main_ start check: training mode skips the match timer test, any other screen waits until
 * time_passed is nonzero.
 *
 * Uncertainty: the asm compares Global.current_screen with 0x36 (the Remix screen id of training
 * mode). The port's scene ids are the decomp SCKind values, so this uses nSCKind1PTrainingMode.
 */
static sb32 itRemixCarCanTick(void)
{
    if (gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode)
    {
        return TRUE;
    }

    return gSCManagerBattleState->time_passed != 0;
}

/**
 * Car.main_ before the first honk: after 2700 frames, a roll of 80 out of 200 honks. The honk is
 * forced once the timer passes 3600 frames without a roll.
 */
static void itRemixCarWaitTick(s32 timer)
{
    s32 roll;

    if (timer < ITREMIX_CAR_WAIT_MIN_TIMER)
    {
        return;
    }
    roll = syUtilsRandIntRange(ITREMIX_CAR_RANDOM_RANGE);
    if (roll == ITREMIX_CAR_HONK_ROLL)
    {
        itRemixCarHonk();
        return;
    }
    if (timer > ITREMIX_CAR_WAIT_MAX_TIMER)
    {
        return;
    }
    itRemixCarHonk();
}

/**
 * Car.main_ after the honk: waits 35 frames, then drives the car out.
 * The asm compares the timer before it is incremented, so timer is the value before this frame.
 */
static void itRemixCarHonkedTick(GObj *item_gobj, s32 timer)
{
    if (timer < ITREMIX_CAR_HONK_TO_SPAWN_FRAMES)
    {
        return;
    }
    sITRemixCarState.is_honked = FALSE;
    itRemixCarDrive(item_gobj);
}

/** Car.main_ honk: plays the honk and restarts the timer. */
static void itRemixCarHonk(void)
{
    func_800269C0_275C0(ITREMIX_CAR_HONK_FGM);
    sITRemixCarState.is_honked = TRUE;
    sITRemixCarState.honk_timer = 0;
}

/**
 * Car.main_ spawn part: enables the hitbox, turns off the clang (can_setoff), shows the model,
 * sets the drive speed and plays the moving sound.
 */
static void itRemixCarDrive(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    sITRemixCarState.honk_timer = 0;
    ip->attack_coll.attack_state = 1;
    ip->attack_coll.can_setoff = FALSE;
    joint->dl = sITRemixCarState.display_list;
    ip->physics.vel_air.x = ITREMIX_CAR_SPEED;
    func_800269C0_275C0(ITREMIX_CAR_MOVE_FGM);
}

/**
 * Car.blast_zone_ (ITStruct proc_dead): disables the hitbox, stops the car, moves the model back
 * to the spawn x and hides it.
 *
 * REMIX-TODO(vanilla 0x8016F8C0): the asm ends with a jump to the bomb/grenade routine at
 * 0x8016F8C0 (inside itProcessProcItemMain, not identified in the port). The car must survive
 * the blast zone to drive again, so the routine returns FALSE and the jump is not made.
 * REMIX-TODO(unnamed ITStruct 0x354): the asm stores the spawn y in 0x354 of the item struct.
 */
static sb32 itRemixCarBlastZone(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);

    ip->attack_coll.attack_state = 0;
    ip->physics.vel_air.x = 0.0F;
    joint->translate.vec.f.x = sITRemixCarSpawnPos.x;
    joint->dl = NULL;

    return FALSE;
}

/**
 * Car.stage_setting_ hitbox part: damage, size, angle, offsets, knockback and sound of the car.
 * The asm writes the same fields in the item struct without a named type.
 *
 * REMIX-TODO(unknown-halfword): the asm writes 0xB4 to the halfword at 0x33E (no named field).
 * REMIX-TODO(bitfield): the asm sets bit 0x10 of the byte at 0x158 (unnamed attack bitfield).
 * REMIX-TODO(bitfield): the asm sets bit 0x08 of the byte at 0x2D3 (unnamed ITStruct field).
 * REMIX-TODO(hitbox refresh): the asm zeroes the words at 0x1D0 (refresh timer), 0x1D4 (collision
 * flag) and 0x35C (no named field).
 */
static void itRemixCarApplyHitbox(ITStruct *ip)
{
    ip->attack_coll.damage = ITREMIX_CAR_DAMAGE;
    ip->attack_coll.size = ITREMIX_CAR_SIZE;
    ip->attack_coll.angle = ITREMIX_CAR_ANGLE;
    ip->attack_coll.element = 0;
    ip->attack_coll.offsets[0].y = ITREMIX_CAR_OFFSET_Y;
    ip->attack_coll.offsets[0].z = ITREMIX_CAR_OFFSET_Z;
    ip->attack_coll.fgm_id = ITREMIX_CAR_HIT_FGM;
    ip->attack_coll.knockback_scale = ITREMIX_CAR_KNOCKBACK_SCALE;
    ip->attack_coll.knockback_base = ITREMIX_CAR_KNOCKBACK_BASE;
}

/**
 * Hazards.onett_setup (asm, after the hazard toggle check): clears the car timer and honk flag,
 * saves the Onett header and spawns the car through the stage item path.
 */
void itRemixCarHazardSetup(void *stage_header)
{
    sITRemixCarState.stage_header = stage_header;
    sITRemixCarState.honk_timer = 0;
    sITRemixCarState.is_honked = FALSE;
    itManagerMakeItemSetupCommon(NULL, ITREMIX_CAR_KIND, &sITRemixCarSpawnPos, &sITRemixCarSpawnVel, ITREMIX_CAR_ITEM_FLAG);
}

/**
 * Car.stage_setting_ (SPAWN_ITEM). Builds the car, sets its scale and hitbox, and stores the
 * display list to restore when the car drives out. The car is not visible until it honks out.
 */
GObj* itRemixCarMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj;
    DObj *joint;
    ITStruct *ip;

    item_gobj = itManagerMakeItem(parent_gobj, &sITRemixCarDesc, pos, vel, flags);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    joint = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);

    joint->translate.vec.f = *pos;
    joint->scale.vec.f.x = ITREMIX_CAR_SCALE;
    joint->scale.vec.f.y = ITREMIX_CAR_SCALE;
    joint->scale.vec.f.z = ITREMIX_CAR_SCALE;
    sITRemixCarState.display_list = joint->dl;

    itRemixCarApplyHitbox(ip);
    ip->proc_dead = itRemixCarBlastZone;

    return item_gobj;
}

/**
 * Descriptor of the Car (item id 0x3C) for the Remix item dispatch. The asm sets
 * SHOW_GFX_WHEN_SPAWNED to FALSE and has no pickup, drop, throw or touch routine.
 */
const ITRemixItemDesc dITRemixCarItemDesc =
{
    ITREMIX_CAR_KIND,                   /* Item.Car.id */
    itRemixCarMakeItem,                 /* SPAWN_ITEM */
    FALSE,                              /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT */
    NULL,                               /* PICKUP_ITEM_MAIN */
    NULL,                               /* DROP_ITEM */
    NULL,                               /* THROW_ITEM */
    NULL                                /* PLAYER_COLLISION */
};

#endif
