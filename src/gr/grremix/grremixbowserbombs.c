/**
 * grremixbowserbombs.c - Bowser's Stadium bombs (Hazards.asm). See grremixbowserbombs.h.
 */

#ifdef PORT

#include "grremixbowserbombs.h"
#include "grremixids.h"
#include <ft/fighter.h>
#include <gr/ground.h>
#include <it/item.h>
#include <it/itground/ittarubomb.h>
#include <sc/scene.h>
#include <lb/lbreloc.h>
#include <sys/taskman.h>

extern void *func_800269C0_275C0(u16 id);

#define GRREMIX_BOWSER_BOMBS_RESPAWN_TIME (0x3C * 5)
#define GRREMIX_BOWSER_BOMBS_FLASH_TIME 120
#define GRREMIX_BOWSER_BOMBS_FLASH_BIT 0x0002
#define GRREMIX_BOWSER_BOMBS_EXPLODE_FGM 0x0001
#define GRREMIX_BOWSER_BOMBS_LAND_FGM 0x00C2
#define GRREMIX_BOWSER_BOMBS_SIDES 2
#define GRREMIX_BOWSER_BOMBS_RIGHT 0
#define GRREMIX_BOWSER_BOMBS_PLAYERS 4
#define GRREMIX_BOWSER_BOMBS_SPAWN_X 2988.0F
#define GRREMIX_BOWSER_BOMBS_SPAWN_Y 512.0F
/* Hazards.asm 1027-1041 (add_item_) writes the bottom ECB 0xC3D00000 (-416.0F) of the bomb item, the hover height */
#define GRREMIX_BOWSER_BOMBS_HOVER_ECB_BOTTOM -416.0F
#define GRREMIX_BOWSER_BOMBS_HEADER_FILE_ID 0x09B2 /* File.BOWSER_BOMB_HEADER: the RTTF bomb header with the Bowser hitbox */

typedef struct GRRemixBowserBombSlot
{
    GObj *bomb_gobj;
    s32 respawn_timer;
    s32 flash_timer;
    sb32 is_exploded;
} GRRemixBowserBombSlot;

typedef struct GRRemixBowserBombsState
{
    GRRemixBowserBombSlot slots[GRREMIX_BOWSER_BOMBS_SIDES];
    GObj *hit_by[GRREMIX_BOWSER_BOMBS_PLAYERS];
} GRRemixBowserBombsState;

static GRRemixBowserBombsState sBowserBombs;

static sb32 grRemixBowserBombsFlashProcUpdate(GObj *item_gobj);

/* Status 0 of the RTTF bomb (dITTaruBombStatusDescs) with the fall update replaced by the flash update, as the asm does. */
static ITStatusDesc sGRRemixBowserBombStatusDescs[] =
{
    {
        grRemixBowserBombsFlashProcUpdate,      // Proc Update
        itTaruBombFallProcMap,                  // Proc Map
        itTaruBombCommonProcHit,                // Proc Hit
        itTaruBombCommonProcHit,                // Proc Shield
        NULL,                                   // Proc Hop
        itTaruBombCommonProcHit,                // Proc Set-Off
        itTaruBombCommonProcHit,                // Proc Reflector
        itTaruBombCommonProcDamage              // Proc Damage
    },
};

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

/** Stage check of every bomb patch: the asm compares Global.match_info stage id with Stages.id.BOWSERB. */
sb32 grRemixBowserBombsIsStage(void)
{
    return gSCManagerBattleState->gkind == nGRRemixStage_BOWSERB;
}

/** Returns the slot whose bomb is item_gobj, or NULL. */
static GRRemixBowserBombSlot* grRemixBowserBombsFindSlot(GObj *item_gobj)
{
    s32 side;

    for (side = 0; side < GRREMIX_BOWSER_BOMBS_SIDES; side++)
    {
        if (sBowserBombs.slots[side].bomb_gobj == item_gobj)
        {
            return &sBowserBombs.slots[side];
        }
    }
    return NULL;
}

/** Position of the bomb on each side of the stadium (asm add_item_ arguments a1/a2). */
static void grRemixBowserBombsSetSpawnPos(s32 side, Vec3f *pos)
{
    pos->x = (side == GRREMIX_BOWSER_BOMBS_RIGHT) ? GRREMIX_BOWSER_BOMBS_SPAWN_X : -GRREMIX_BOWSER_BOMBS_SPAWN_X;
    pos->y = GRREMIX_BOWSER_BOMBS_SPAWN_Y;
    pos->z = 0.0F;
}

/** Creates the bomb of one side and starts its flash. A failed spawn retries after the respawn time. */
static void grRemixBowserBombsSpawn(s32 side)
{
    GRRemixBowserBombSlot *slot = &sBowserBombs.slots[side];
    Vec3f pos;
    Vec3f vel;
    GObj *bomb_gobj;

    grRemixBowserBombsSetSpawnPos(side, &pos);
    vel.x = 0.0F;
    vel.y = 0.0F;
    vel.z = 0.0F;

    bomb_gobj = itManagerMakeItemSetupCommon(NULL, nITKindTaruBomb, &pos, &vel, ITEM_FLAG_PARENT_GROUND);
    if (bomb_gobj == NULL)
    {
        slot->bomb_gobj = NULL;
        slot->respawn_timer = GRREMIX_BOWSER_BOMBS_RESPAWN_TIME;
        return;
    }
    itGetStruct(bomb_gobj)->coll_data.map_coll.bottom = GRREMIX_BOWSER_BOMBS_HOVER_ECB_BOTTOM;
    slot->bomb_gobj = bomb_gobj;
    slot->flash_timer = GRREMIX_BOWSER_BOMBS_FLASH_TIME;
    slot->is_exploded = FALSE;

    itMainSetStatus(bomb_gobj, sGRRemixBowserBombStatusDescs, 0);
    itGetStruct(bomb_gobj)->attack_coll.attack_state = nGMAttackStateOff;
}

/** bowser_bomb_flash_main: blinks the bomb, then enables its hitbox and plays the landing FGM. */
static sb32 grRemixBowserBombsFlashProcUpdate(GObj *item_gobj)
{
    GRRemixBowserBombSlot *slot = grRemixBowserBombsFindSlot(item_gobj);
    DObj *dobj = DObjGetStruct(item_gobj);

    if ((slot == NULL) || (slot->flash_timer == 0))
    {
        return FALSE;
    }
    slot->flash_timer--;
    dobj->flags = (slot->flash_timer & GRREMIX_BOWSER_BOMBS_FLASH_BIT) ? DOBJ_FLAG_HIDDEN : DOBJ_FLAG_NONE;

    if (slot->flash_timer != 0)
    {
        return FALSE;
    }
    dobj->flags = DOBJ_FLAG_NONE;
    itGetStruct(item_gobj)->attack_coll.attack_state = nGMAttackStateInterpolate;
    func_800269C0_275C0(GRREMIX_BOWSER_BOMBS_LAND_FGM);

    return FALSE;
}

/** Respawn countdown of one side: a live bomb keeps the timer full, an exploded one counts down and is replaced. */
static void grRemixBowserBombsUpdateRespawn(s32 side)
{
    GRRemixBowserBombSlot *slot = &sBowserBombs.slots[side];

    if (!slot->is_exploded && (slot->bomb_gobj != NULL))
    {
        slot->respawn_timer = GRREMIX_BOWSER_BOMBS_RESPAWN_TIME;
        return;
    }
    slot->respawn_timer--;

    if (slot->respawn_timer > 0)
    {
        return;
    }
    grRemixBowserBombsSpawn(side);
}

/** replenish_bowser_bombs_: per-frame respawn process of both sides. */
static void grRemixBowserBombsRespawnProcUpdate(GObj *respawn_gobj)
{
    s32 side;

    for (side = 0; side < GRREMIX_BOWSER_BOMBS_SIDES; side++)
    {
        grRemixBowserBombsUpdateRespawn(side);
    }
}

/** Loads BOWSER_BOMB_HEADER as the item file of the RTTF bomb (asm add_item_ stores it at 0x801313F4). */
static void grRemixBowserBombsLoadHeader(void)
{
    gGRCommonStruct.bonus3.item_head = lbRelocGetExternHeapFile
    (
        GRREMIX_BOWSER_BOMBS_HEADER_FILE_ID,
        syTaskmanMalloc(lbRelocGetFileSize(GRREMIX_BOWSER_BOMBS_HEADER_FILE_ID), 0x10)
    );
}

void grRemixBowserBombsStart(void)
{
    GObj *respawn_gobj;
    s32 side;
    s32 player;

    grRemixBowserBombsLoadHeader();

    for (side = 0; side < GRREMIX_BOWSER_BOMBS_SIDES; side++)
    {
        sBowserBombs.slots[side].bomb_gobj = NULL;
        sBowserBombs.slots[side].respawn_timer = 0;
        sBowserBombs.slots[side].flash_timer = 0;
        sBowserBombs.slots[side].is_exploded = FALSE;
        grRemixBowserBombsSpawn(side);
    }
    for (player = 0; player < GRREMIX_BOWSER_BOMBS_PLAYERS; player++)
    {
        sBowserBombs.hit_by[player] = NULL;
    }
    respawn_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);
    gcAddGObjProcess(respawn_gobj, grRemixBowserBombsRespawnProcUpdate, nGCProcessKindFunc, 4);
}

sb32 grRemixBowserBombsExplodeOnHit(GObj *item_gobj)
{
    GRRemixBowserBombSlot *slot;

    if (!grRemixBowserBombsIsStage())
    {
        return FALSE;
    }
    slot = grRemixBowserBombsFindSlot(item_gobj);
    if (slot != NULL)
    {
        slot->is_exploded = TRUE;
    }
    func_800269C0_275C0(GRREMIX_BOWSER_BOMBS_EXPLODE_FGM);

    return TRUE;
}

u16 grRemixBowserBombsMapFgm(u16 fgm_id)
{
    if (grRemixBowserBombsIsStage())
    {
        return GRREMIX_BOWSER_BOMBS_LAND_FGM;
    }
    return fgm_id;
}

void grRemixBowserBombsTrackHitObject(u8 player, GObj *item_gobj)
{
    if (player >= GRREMIX_BOWSER_BOMBS_PLAYERS)
    {
        return;
    }
    sBowserBombs.hit_by[player] = item_gobj;
}

/** True when the last object that hit this player is one of the bowser bombs on BOWSERB. */
static sb32 grRemixBowserBombsIsHitByBomb(u8 player)
{
    if (!grRemixBowserBombsIsStage() || (player >= GRREMIX_BOWSER_BOMBS_PLAYERS))
    {
        return FALSE;
    }
    return grRemixBowserBombsFindSlot(sBowserBombs.hit_by[player]) != NULL;
}

f32 grRemixBowserBombsHitstunTics(FTStruct *fp, f32 hitstun_tics)
{
    if (!grRemixBowserBombsIsHitByBomb(fp->player))
    {
        return hitstun_tics;
    }
    return hitstun_tics * (9.0F / (fp->attr->gravity + 4.0F));
}

#endif
