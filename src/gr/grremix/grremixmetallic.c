/**
 * grremixmetallic.c - Metallic Madness hazards: stage setup, Robot Bee factory and shrink timer.
 * Ported from Hazards.asm metallic_madness_setup, bee_factory_ and shrink_timer_.
 */

#ifdef PORT

#include "grremixmetallic.h"
#include "grremixhazards.h"
#include "it/itremix/itremixrobotbee.h"
#include "ft/fighter.h"
#include "ft/ftremix/ftremixhooks.h"
#include "it/itmanager.h"
#include "mp/map.h"
#include "sc/scene.h"
#include "sys/objman.h"
#include "sys/utils.h"

#define GRREMIX_METALLIC_HEADER_TO_GROUND_DATA 0x14   /* asm metallic_madness_setup: header = 0x80131300 pointer - 0x14 */
#define GRREMIX_METALLIC_LASER_FILE_FIELD 0x130       /* asm: stage header + 0x130, pointer to the laser hitbox file */
#define GRREMIX_METALLIC_SPAWN_WAIT_TIME 0x5DC        /* asm bee_factory_: 1500 tics before the first roll */
#define GRREMIX_METALLIC_SPAWN_ROLL_RANGE 0x5DC       /* asm: Global.get_random_int_(1500) */
#define GRREMIX_METALLIC_SPAWN_ROLL_HIT 0x50          /* asm: roll result that spawns the bee (80) */
#define GRREMIX_METALLIC_SPAWN_TIME_FORCED 0xBB8      /* asm: the bee spawns at timer 3000 when no roll hit */
#define GRREMIX_METALLIC_SHRINK_DURATION 0x384        /* asm robot_bee_laser_hit_: 900 tics */
#define GRREMIX_METALLIC_SHRUNK_MULTIPLIER 0.5F       /* asm robot_bee_laser_hit_: 0x3F00 */
#define GRREMIX_METALLIC_NORMAL_MULTIPLIER 1.0F       /* asm shrink_timer_ NORMAL state; GIANT and TINY are REMIX-TODO below */
#define GRREMIX_METALLIC_PORT_COUNT 4
#define GRREMIX_METALLIC_TRAINING_SCENE nSCKind1PTrainingMode /* asm bee_factory_: Global.current_screen == 0x36 */
#define GRREMIX_METALLIC_BEE_FACING_RANGE 2

static void *sGRRemixMetallicStageHeader;
static void *sGRRemixMetallicLaserFile;
static u32 sGRRemixMetallicBeeTimer;
static s32 sGRRemixMetallicBeeFacing;
static u32 sGRRemixMetallicShrinkTimer[GRREMIX_METALLIC_PORT_COUNT];

static void grRemixMetallicClearState(void);
static void grRemixMetallicLoadStageFiles(void);
static void grRemixMetallicMakeProcessGObj(void (*proc)(GObj*));
static void grRemixMetallicBeeFactoryProcUpdate(GObj *factory_gobj);
static void grRemixMetallicShrinkTimerProcUpdate(GObj *timer_gobj);
static sb32 grRemixMetallicBeeFactoryIsActive(void);
static sb32 grRemixMetallicBeeRollSpawn(u32 timer);
static void grRemixMetallicSpawnBee(void);
static void grRemixMetallicShrinkTimerTick(s32 port);
static void grRemixMetallicEndShrink(s32 port);

/**
 * Hazards.asm metallic_madness_setup. The caller checks grRemixHazardsAreOn() first, as the asm checks
 * Toggles.entry_hazard_mode.
 */
void grRemixMetallicSetup(void)
{
    grRemixMetallicClearState();
    grRemixMetallicLoadStageFiles();
    grRemixMetallicMakeProcessGObj(grRemixMetallicBeeFactoryProcUpdate);
    grRemixMetallicMakeProcessGObj(grRemixMetallicShrinkTimerProcUpdate);
}

/** Returns the address of the stage header pointer for item descriptors. */
void **grRemixMetallicStageHeaderRef(void)
{
    return &sGRRemixMetallicStageHeader;
}

/** Returns the address of the laser file pointer for projectile descriptors. */
void **grRemixMetallicLaserFileRef(void)
{
    return &sGRRemixMetallicLaserFile;
}

/** Returns the Robot Bee facing set by the last spawn. */
s32 grRemixMetallicBeeFacing(void)
{
    return sGRRemixMetallicBeeFacing;
}

/**
 * Shrinks the fighter on `port` to half size for the shrink duration (asm robot_bee_laser_hit_).
 * The asm also destroys an active Super Mushroom effect of that port; that is not ported yet.
 */
void grRemixMetallicShrinkPlayer(s32 port)
{
    /* REMIX-TODO(SuperMushroom): asm robot_bee_laser_hit_ destroys the active shroom routine of the port (Item.SuperMushroom.player_shrooms); no accessor exists. */
    sGRRemixMetallicShrinkTimer[port] = GRREMIX_METALLIC_SHRINK_DURATION;
    ftRemixSizeSetMultiplier(port, GRREMIX_METALLIC_SHRUNK_MULTIPLIER);
}

/** Clears the bee timer, the facing and every shrink timer. */
static void grRemixMetallicClearState(void)
{
    s32 port;

    sGRRemixMetallicBeeTimer = 0;
    sGRRemixMetallicBeeFacing = 0;
    for (port = 0; port < GRREMIX_METALLIC_PORT_COUNT; port++)
    {
        sGRRemixMetallicShrinkTimer[port] = 0;
    }
}

/**
 * Stores the stage header and the laser file. The header is 0x14 bytes before the ground data
 * (asm metallic_madness_setup). The fields are relocation tokens of the stage file.
 */
static void grRemixMetallicLoadStageFiles(void)
{
    u8 *header = (u8 *)gMPCollisionGroundData - GRREMIX_METALLIC_HEADER_TO_GROUND_DATA;

    sGRRemixMetallicStageHeader = header;
    sGRRemixMetallicLaserFile = PORT_RESOLVE(*(u32 *)(header + GRREMIX_METALLIC_LASER_FILE_FIELD));
}

/** Creates a ground GObj that runs `proc` every frame with the default process priority of the stage routines. */
static void grRemixMetallicMakeProcessGObj(void (*proc)(GObj*))
{
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjProcess(gobj, proc, nGCProcessKindFunc, 4);
}

/**
 * bee_factory_ (asm). Counts the match tics and rolls for a Robot Bee once 1500 tics have passed.
 */
static void grRemixMetallicBeeFactoryProcUpdate(GObj *factory_gobj)
{
    u32 timer;

    if (!grRemixMetallicBeeFactoryIsActive())
    {
        return;
    }
    timer = sGRRemixMetallicBeeTimer;
    sGRRemixMetallicBeeTimer = timer + 1;
    if (timer < GRREMIX_METALLIC_SPAWN_WAIT_TIME)
    {
        return;
    }
    if (!grRemixMetallicBeeRollSpawn(timer))
    {
        return;
    }
    sGRRemixMetallicBeeTimer = 0;
    grRemixMetallicSpawnBee();
}

/** Training mode always runs the factory; other modes wait for the match clock to start. */
static sb32 grRemixMetallicBeeFactoryIsActive(void)
{
    if (gSCManagerSceneData.scene_curr == GRREMIX_METALLIC_TRAINING_SCENE)
    {
        return TRUE;
    }
    return gSCManagerBattleState->time_passed != 0;
}

/**
 * The bee spawns when the random roll hits 80, or when the timer reaches 3000 without a roll hit.
 * Returns TRUE when the bee spawns.
 */
static sb32 grRemixMetallicBeeRollSpawn(u32 timer)
{
    s32 roll = syUtilsRandIntRange(GRREMIX_METALLIC_SPAWN_ROLL_RANGE);

    if (roll == GRREMIX_METALLIC_SPAWN_ROLL_HIT)
    {
        return TRUE;
    }
    return timer == GRREMIX_METALLIC_SPAWN_TIME_FORCED;
}

/**
 * Spawns the Robot Bee at the stage origin and draws its facing. The asm draws the facing twice and keeps
 * the second draw; the first draw is kept here too so the random sequence matches.
 * REMIX-TODO(animation): the asm sets Robot Bee animation tracks 1 and 2 (0x8000BD1C, 0x8000BD54) from the facing; not ported.
 */
static void grRemixMetallicSpawnBee(void)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, 0.0F, 0.0F };

    itManagerMakeItemSetupCommon(NULL, ITREMIX_ROBOTBEE_KIND, &pos, &vel, 0);
    syUtilsRandIntRange(GRREMIX_METALLIC_BEE_FACING_RANGE);
    sGRRemixMetallicBeeFacing = syUtilsRandIntRange(GRREMIX_METALLIC_BEE_FACING_RANGE);
}

/**
 * shrink_timer_ (asm). Counts down every port's shrink timer and restores its size when it ends.
 */
static void grRemixMetallicShrinkTimerProcUpdate(GObj *timer_gobj)
{
    s32 port;

    for (port = 0; port < GRREMIX_METALLIC_PORT_COUNT; port++)
    {
        grRemixMetallicShrinkTimerTick(port);
    }
}

/** Advances the shrink timer of one port by one tic; a timer that reaches 1 ends the shrink. */
static void grRemixMetallicShrinkTimerTick(s32 port)
{
    u32 timer = sGRRemixMetallicShrinkTimer[port];

    if (timer == 0)
    {
        return;
    }
    if (timer == 1)
    {
        grRemixMetallicEndShrink(port);
        return;
    }
    sGRRemixMetallicShrinkTimer[port] = timer - 1;
}

/**
 * Restores the normal size of `port`. The asm chooses NORMAL, GIANT or TINY from Size.match_state_table; the
 * Remix size state is not readable from C yet, so the port always restores NORMAL.
 */
static void grRemixMetallicEndShrink(s32 port)
{
    /* REMIX-TODO(Size): asm restores 2.25F for GIANT and 0.5F for TINY from Size.match_state_table, not ported. */
    ftRemixSizeSetMultiplier(port, GRREMIX_METALLIC_NORMAL_MULTIPLIER);
    sGRRemixMetallicShrinkTimer[port] = 0;
}

#endif
