/**
 * grremixpiratecannon.c - Pirate Land cannon and its setup. See grremixpiratecannon.h.
 */

#ifdef PORT

#include "grremixpiratecannon.h"
#include "grremixpiratecannonball.h"
#include "grremixwater.h"
#include <gr/ground.h>
#include <gr/grdisplay.h>
#include <mp/map.h>
#include <wp/weapon.h>
#include <ef/efmanager.h>
#include <sys/objanim.h>
#include <sys/utils.h>

#define GRREMIX_PIRATE_CANNON_HEADER_CANNON_FILE 0x0080 /* map header pointer to the cannon file (asm: pirate_land_setup) */
#define GRREMIX_PIRATE_CANNON_HEADER_HITBOX_FILE 0x00AC /* map header pointer to the cannonball hitbox file */
#define GRREMIX_PIRATE_CANNON_HITBOX_TRACKS 0x0008 /* hitbox file word holding the animation track table pointer */
#define GRREMIX_PIRATE_CANNON_JOINT_DESC 0x07A8 /* cannon file offset of the joint descriptors (hard coded in the asm) */
#define GRREMIX_PIRATE_CANNON_MODEL 0x0970 /* cannon file offset of the model and textures (asm 0x8000F8F4, gcAddMObjAll) */
#define GRREMIX_PIRATE_CANNON_ANIM_JOINTS 0x0BB4 /* cannon file offset of the joint animation tracks */
#define GRREMIX_PIRATE_CANNON_ANIM_MATERIALS 0x0BF0 /* cannon file offset of the material animation tracks */
#define GRREMIX_PIRATE_CANNON_WATER_Y -600.0F
#define GRREMIX_PIRATE_CANNON_SPLASH_Y -360.0F
#define GRREMIX_PIRATE_CANNON_BARREL_Y 60.0F
#define GRREMIX_PIRATE_CANNON_TURN_SPEED 4.0F
#define GRREMIX_PIRATE_CANNON_FRENZY_TURN_SPEED 8.0F
#define GRREMIX_PIRATE_CANNON_SHOT_SCALING 1.125F
#define GRREMIX_PIRATE_CANNON_WAIT_BASE_TIME 90
#define GRREMIX_PIRATE_CANNON_WAIT_TIME_RANGE 60
#define GRREMIX_PIRATE_CANNON_FRENZY_WAIT_TIME 30
#define GRREMIX_PIRATE_CANNON_FRENZY_BASE_MOVES 10
#define GRREMIX_PIRATE_CANNON_FRENZY_MOVES_RANGE 10
#define GRREMIX_PIRATE_CANNON_CHANCE_FRENZY 5
#define GRREMIX_PIRATE_CANNON_FIRING_TIME 12
#define GRREMIX_PIRATE_CANNON_FGM_TURN 1012
#define GRREMIX_PIRATE_CANNON_FGM_SHOT 1013
#define GRREMIX_PIRATE_CANNON_SHAKE_LIGHT 0
#define GRREMIX_PIRATE_CANNON_POSITIONS_NUM 3
#define GRREMIX_PIRATE_CANNON_INITIAL_TIMER 360
#define GRREMIX_PIRATE_CANNON_CHANCE_RANGE 100

extern void *func_800269C0_275C0(u16 id);

typedef enum GRRemixPirateCannonState
{
    nGRRemixPirateCannonStateWaiting = 0,
    nGRRemixPirateCannonStateTurning = 1,
    nGRRemixPirateCannonStateFiring = 2

} GRRemixPirateCannonState;

typedef enum GRRemixPirateCannonPosition
{
    nGRRemixPirateCannonPositionLeft = 0,
    nGRRemixPirateCannonPositionMiddle = 1,
    nGRRemixPirateCannonPositionRight = 2

} GRRemixPirateCannonPosition;

/* Barrel x of each position (asm: Cannon.BARREL_X_LEFT, BARREL_X_MIDDLE = 0, BARREL_X_RIGHT). */
static const f32 sGRRemixPirateCannonBarrelX[GRREMIX_PIRATE_CANNON_POSITIONS_NUM] = { -128.0F, 0.0F, 128.0F };

/* Cannonball track offset of each position in the hitbox file (asm: Cannon.offset.LEFT, MIDDLE, RIGHT). */
static const u32 sGRRemixPirateCannonTrackOffset[GRREMIX_PIRATE_CANNON_POSITIONS_NUM] = { 0x0000, 0x0610, 0x0C50 };

static struct
{
    GRRemixPirateCannonState state;
    GRRemixPirateCannonState prev_state;
    GRRemixPirateCannonPosition position;
    GRRemixPirateCannonPosition prev_position;
    s32 state_timer;
    sb32 is_turning_left;
    s32 frenzy_moves;
    void *cannonball_hitbox_file;
} sGRRemixPirateCannon;

static DObjTransformTypes sGRRemixPirateCannonTransformKinds[DOBJ_ARRAY_MAX];

static void grRemixPirateCannonBeginWaiting(void);
static void grRemixPirateCannonBeginTurning(GObj *cannon_gobj);
static void grRemixPirateCannonBeginFiring(GObj *cannon_gobj);
static void grRemixPirateCannonRollFrenzy(void);
static void grRemixPirateCannonShoot(GObj *cannon_gobj);

/** Reads a stage file pointer from the map header; the value is a relocation token (asm: lw 0x0080 / 0x00AC). */
static void* grRemixPirateCannonHeaderFile(u32 header_offset)
{
    u32 token = *(u32*)((uintptr_t)gMPCollisionGroundData + header_offset);

    return (void*)PORT_RESOLVE(token);
}

/** The barrel is the second child of the cannon base joint: base -> child -> sib_next (asm: cannon_main_ s1). */
static DObj* grRemixPirateCannonBarrel(GObj *cannon_gobj)
{
    return DObjGetStruct(cannon_gobj)->child->child->sib_next;
}

/** Joint of the cannon base, the parent of the barrel (asm: cannonball_stage_setting_ t0). */
static DObj* grRemixPirateCannonBase(GObj *cannon_gobj)
{
    return DObjGetStruct(cannon_gobj)->child;
}

/** Turns the barrel towards its current destination; returns to waiting when the destination is reached. */
static void grRemixPirateCannonUpdateTurning(GObj *cannon_gobj)
{
    DObj *barrel = grRemixPirateCannonBarrel(cannon_gobj);
    f32 speed = (sGRRemixPirateCannon.frenzy_moves != 0) ? GRREMIX_PIRATE_CANNON_FRENZY_TURN_SPEED : GRREMIX_PIRATE_CANNON_TURN_SPEED;
    f32 target_x = sGRRemixPirateCannonBarrelX[sGRRemixPirateCannon.position];

    if (sGRRemixPirateCannon.is_turning_left)
    {
        speed = -speed;
    }
    barrel->translate.vec.f.x += speed;
    if (barrel->translate.vec.f.x == target_x)
    {
        grRemixPirateCannonBeginWaiting();
    }
}

/** Counts down the firing pose, then returns the scale to 1.0 and waits (asm: _while_firing). */
static void grRemixPirateCannonUpdateFiring(GObj *cannon_gobj)
{
    DObj *barrel = grRemixPirateCannonBarrel(cannon_gobj);

    sGRRemixPirateCannon.state_timer--;
    if (sGRRemixPirateCannon.state_timer != 0)
    {
        return;
    }
    barrel->scale.vec.f.x = 1.0F;
    barrel->scale.vec.f.y = 1.0F;
    barrel->scale.vec.f.z = 1.0F;
    grRemixPirateCannonBeginWaiting();
}

/** Counts down the wait; after a shot the cannon turns, after a turn it fires (asm: _while_waiting). */
static void grRemixPirateCannonUpdateWaiting(GObj *cannon_gobj)
{
    sGRRemixPirateCannon.state_timer--;
    if (sGRRemixPirateCannon.state_timer != 0)
    {
        return;
    }
    if (sGRRemixPirateCannon.prev_state == nGRRemixPirateCannonStateTurning)
    {
        grRemixPirateCannonBeginFiring(cannon_gobj);
        return;
    }
    grRemixPirateCannonBeginTurning(cannon_gobj);
}

/** Frenzy moves are consumed one per wait-turn-fire cycle (asm: _begin_waiting and _begin_turning). */
static void grRemixPirateCannonConsumeFrenzyMove(void)
{
    if (sGRRemixPirateCannon.frenzy_moves != 0)
    {
        sGRRemixPirateCannon.frenzy_moves--;
    }
}

/** Sets the state and records the previous one, as the asm does for every transition. */
static void grRemixPirateCannonSetState(GRRemixPirateCannonState state)
{
    sGRRemixPirateCannon.prev_state = sGRRemixPirateCannon.state;
    sGRRemixPirateCannon.state = state;
}

/** Waits for a random time, or for the short frenzy time while a frenzy is running (asm: _begin_waiting). */
static void grRemixPirateCannonBeginWaiting(void)
{
    if (sGRRemixPirateCannon.frenzy_moves != 0)
    {
        sGRRemixPirateCannon.state_timer = GRREMIX_PIRATE_CANNON_FRENZY_WAIT_TIME;
        grRemixPirateCannonConsumeFrenzyMove();
    }
    else
    {
        sGRRemixPirateCannon.state_timer = syUtilsRandIntRange(GRREMIX_PIRATE_CANNON_WAIT_TIME_RANGE + 1) +
                                           GRREMIX_PIRATE_CANNON_WAIT_BASE_TIME;
    }
    grRemixPirateCannonSetState(nGRRemixPirateCannonStateWaiting);
}

/** Picks a new random position and starts turning towards it; an unchanged position goes straight to waiting. */
static void grRemixPirateCannonBeginTurning(GObj *cannon_gobj)
{
    GRRemixPirateCannonPosition old_position = sGRRemixPirateCannon.position;
    GRRemixPirateCannonPosition new_position;

    grRemixPirateCannonConsumeFrenzyMove();
    grRemixPirateCannonSetState(nGRRemixPirateCannonStateTurning);
    new_position = (GRRemixPirateCannonPosition)syUtilsRandIntRange(GRREMIX_PIRATE_CANNON_POSITIONS_NUM);
    sGRRemixPirateCannon.position = new_position;
    sGRRemixPirateCannon.prev_position = old_position;
    if (new_position == old_position)
    {
        grRemixPirateCannonBeginWaiting();
        return;
    }
    sGRRemixPirateCannon.is_turning_left = (new_position < old_position);
    func_800269C0_275C0(GRREMIX_PIRATE_CANNON_FGM_TURN);
}

/** Fires the cannonball from the barrel, shakes the screen and rolls the chance of a frenzy (asm: _begin_firing). */
static void grRemixPirateCannonBeginFiring(GObj *cannon_gobj)
{
    DObj *barrel = grRemixPirateCannonBarrel(cannon_gobj);

    func_800269C0_275C0(GRREMIX_PIRATE_CANNON_FGM_SHOT);
    efManagerQuakeMakeEffect(GRREMIX_PIRATE_CANNON_SHAKE_LIGHT);
    grRemixPirateCannonShoot(cannon_gobj);
    grRemixPirateCannonSetState(nGRRemixPirateCannonStateFiring);
    sGRRemixPirateCannon.state_timer = GRREMIX_PIRATE_CANNON_FIRING_TIME;
    barrel->scale.vec.f.x = GRREMIX_PIRATE_CANNON_SHOT_SCALING;
    barrel->scale.vec.f.y = GRREMIX_PIRATE_CANNON_SHOT_SCALING;
    barrel->scale.vec.f.z = GRREMIX_PIRATE_CANNON_SHOT_SCALING;
    grRemixPirateCannonRollFrenzy();
}

/** Starts a frenzy with a random number of moves, with a small chance per shot when none is running (asm: _begin_firing). */
static void grRemixPirateCannonRollFrenzy(void)
{
    if (sGRRemixPirateCannon.frenzy_moves != 0)
    {
        grRemixPirateCannonConsumeFrenzyMove();
        return;
    }
    if (syUtilsRandIntRange(GRREMIX_PIRATE_CANNON_CHANCE_RANGE) >= GRREMIX_PIRATE_CANNON_CHANCE_FRENZY)
    {
        return;
    }
    sGRRemixPirateCannon.frenzy_moves = syUtilsRandIntRange(GRREMIX_PIRATE_CANNON_FRENZY_MOVES_RANGE + 1) +
                                        GRREMIX_PIRATE_CANNON_FRENZY_BASE_MOVES;
}

/** Spawns the cannonball at the barrel tip, on the track of the current position (asm: cannonball_stage_setting_). */
static void grRemixPirateCannonShoot(GObj *cannon_gobj)
{
    DObj *base = grRemixPirateCannonBase(cannon_gobj);
    DObj *barrel = grRemixPirateCannonBarrel(cannon_gobj);
    Vec3f spawn_pos;
    u8 *hitbox_file = sGRRemixPirateCannon.cannonball_hitbox_file;
    u8 *tracks = (u8*)PORT_RESOLVE(*(u32*)(hitbox_file + GRREMIX_PIRATE_CANNON_HITBOX_TRACKS));

    spawn_pos.x = base->translate.vec.f.x + barrel->translate.vec.f.x;
    spawn_pos.y = base->translate.vec.f.y + barrel->translate.vec.f.y;
    spawn_pos.z = base->translate.vec.f.z + barrel->translate.vec.f.z;

    grRemixPirateCannonballMake(&spawn_pos, hitbox_file,
        (AObjEvent32*)(tracks + sGRRemixPirateCannonTrackOffset[sGRRemixPirateCannon.position]));
}

/** Per-frame process of the cannon, one call per frame (asm: pirate_land_main_ and cannon_main_). */
static void grRemixPirateCannonProcUpdate(GObj *cannon_gobj)
{
    switch (sGRRemixPirateCannon.state)
    {
    case nGRRemixPirateCannonStateTurning:
        grRemixPirateCannonUpdateTurning(cannon_gobj);
        break;

    case nGRRemixPirateCannonStateFiring:
        grRemixPirateCannonUpdateFiring(cannon_gobj);
        break;

    default:
        grRemixPirateCannonUpdateWaiting(cannon_gobj);
        break;
    }
}

/** Resets the cannon state to the one of pirate_land_setup: waiting, middle position, 360 frames to the first shot. */
static void grRemixPirateCannonInitState(void)
{
    sGRRemixPirateCannon.state = nGRRemixPirateCannonStateWaiting;
    sGRRemixPirateCannon.prev_state = nGRRemixPirateCannonStateFiring;
    sGRRemixPirateCannon.position = nGRRemixPirateCannonPositionMiddle;
    sGRRemixPirateCannon.prev_position = nGRRemixPirateCannonPositionMiddle;
    sGRRemixPirateCannon.state_timer = GRREMIX_PIRATE_CANNON_INITIAL_TIMER;
    sGRRemixPirateCannon.is_turning_left = FALSE;
    sGRRemixPirateCannon.frenzy_moves = 0;
}

/** Builds the cannon GObj: model joints, joint animation, processes and the barrel start pose. */
static void grRemixPirateCannonMakeObject(void *cannon_file)
{
    GObj *cannon_gobj;
    DObj *barrel;

    cannon_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjDisplay(cannon_gobj, grDisplayLayer0PriProcDisplay, 4, GOBJ_PRIORITY_DEFAULT, ~0);
    grModelSetupGroundDObjs(cannon_gobj, (DObjDesc*)((uintptr_t)cannon_file + GRREMIX_PIRATE_CANNON_JOINT_DESC),
                            NULL, sGRRemixPirateCannonTransformKinds);
    gcAddMObjAll(cannon_gobj, (MObjSub***)((uintptr_t)cannon_file + GRREMIX_PIRATE_CANNON_MODEL));
    gcAddAnimAll(cannon_gobj, (AObjEvent32**)((uintptr_t)cannon_file + GRREMIX_PIRATE_CANNON_ANIM_JOINTS),
                 (AObjEvent32***)((uintptr_t)cannon_file + GRREMIX_PIRATE_CANNON_ANIM_MATERIALS), 0.0F);
    gcAddGObjProcess(cannon_gobj, gcPlayAnimAll, nGCProcessKindFunc, 5);
    gcAddGObjProcess(cannon_gobj, grRemixPirateCannonProcUpdate, nGCProcessKindFunc, 4);


    barrel = grRemixPirateCannonBarrel(cannon_gobj);
    barrel->translate.vec.f.x = 0.0F;
    barrel->translate.vec.f.y = GRREMIX_PIRATE_CANNON_BARREL_Y;
}

/**
 * Creates the Pirate Land cannon and the water. Runs only when hazards are on (asm: Toggles.entry_hazard_mode).
 */
void grRemixPirateMakeGround(void)
{
    void *cannon_file = grRemixPirateCannonHeaderFile(GRREMIX_PIRATE_CANNON_HEADER_CANNON_FILE);

    sGRRemixPirateCannon.cannonball_hitbox_file = grRemixPirateCannonHeaderFile(GRREMIX_PIRATE_CANNON_HEADER_HITBOX_FILE);
    grRemixPirateCannonInitState();
    grRemixPirateCannonMakeObject(cannon_file);
    grRemixWaterMakeGround(GRREMIX_PIRATE_CANNON_WATER_Y, GRREMIX_PIRATE_CANNON_SPLASH_Y);
}

#endif
