/**
 * itremixclanbomb.c - Smash Remix Clan Bomb item (Item.ClanBomb, id 0x45).
 * Port of .toolchain/pkgs/smashremix-src/src/items/ClanBomb.asm. The item is
 * a Bob-omb variant: it reuses the vanilla Bob-omb hit, explode and attack
 * event routines and the Maxim Tomato fall and drop routines.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixclanbomb.h"
#include "it/itcommon/itbombhei.h"
#include "it/itcommon/ittomato.h"
#include <ef/efmanager.h>
#include "ft/ftremix/ftremix.h"

#define ITREMIX_CLANBOMB_ID 0x45
#define ITREMIX_CLANBOMB_FILE_OFFSET 0x130
#define ITREMIX_CLANBOMB_IMAGE_ID 2
#define ITREMIX_CLANBOMB_PALETTE_SPAWN 4.0F    /* item_table palette 0x4080 (high half of 4.0F) */
#define ITREMIX_CLANBOMB_DAMAGE 1
#define ITREMIX_CLANBOMB_KNOCKBACK_BASE 80
#define ITREMIX_CLANBOMB_KNOCKBACK_SCALE 70
#define ITREMIX_CLANBOMB_EXPLODE_LIFETIME 6
#define ITREMIX_CLANBOMB_EXPLODE_SCALE 1.4F    /* same value as ITBOMBHEI_EXPLODE_SCALE, see notes */
#define ITREMIX_CLANBOMB_FGM_HITBOX 1
#define ITREMIX_CLANBOMB_FGM_CLIPPING 1
#define ITREMIX_CLANBOMB_ANIM_TIMER_MASK 0x0F
#define ITREMIX_CLANBOMB_PALETTE_EXPLODE_FLAG 0x20

enum
{
    nITRemixClanBombStatusPrepickupGround,
    nITRemixClanBombStatusPrepickupAir,
    nITRemixClanBombStatusPickup,
    nITRemixClanBombStatusThrown,
    nITRemixClanBombStatusExplodeTransition,
    nITRemixClanBombStatusExplode
};

extern void *func_800269C0_275C0(u16 id);

static void itRemixClanBombPrepickupProc(GObj *item_gobj);
static sb32 itRemixClanBombHoldProcUpdate(GObj *item_gobj);
static void itRemixClanBombThrownProc(GObj *item_gobj);
static sb32 itRemixClanBombThrownDurationProcMap(GObj *item_gobj);
static sb32 itRemixClanBombCollidePlayerProcHit(GObj *item_gobj);
static void itRemixClanBombCollideTransition(GObj *item_gobj);
static sb32 itRemixClanBombExplodeTransitionPlayer(GObj *item_gobj);
static sb32 itRemixClanBombCollideClippingProcUpdate(GObj *item_gobj);
static void itRemixClanBombExplodeTransition(GObj *item_gobj);
static void itRemixClanBombCheckExplode(GObj *item_gobj);
static void itRemixClanBombExplodeInitial(GObj *item_gobj);
static sb32 itRemixClanBombExplodeProcUpdate(GObj *item_gobj);
static void itRemixClanBombAnimate(GObj *item_gobj);

/* ClanBomb.item_info_array. p_file and the owner-specific file are set per spawn. */
static ITDesc sITRemixClanBombDesc =
{
    ITREMIX_CLANBOMB_ID,                    /* Item ID (set by Item.add_item) */
    NULL,                                   /* Marina file 8 (p_file_special3), set per spawn */
    ITREMIX_CLANBOMB_FILE_OFFSET,           /* Offset of item footer in file */
    { nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 },
    nGMAttackStateOff,
    itTomatoFallProcUpdate,                 /* Maxim Tomato fall update */
    itTomatoFallProcMap,                    /* Maxim Tomato fall map */
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* ClanBomb.item_state_table. Each state has the 8 procs of ITStatusDesc, in order. */
static ITStatusDesc sITRemixClanBombStatusDescs[] =
{
    /* Prepickup, grounded */
    { NULL, itTomatoWaitProcMap, NULL, NULL, NULL, NULL, NULL, NULL },

    /* Prepickup, aerial */
    { itTomatoFallProcUpdate, itTomatoFallProcMap, NULL, NULL, NULL, NULL, NULL, itRemixClanBombCollidePlayerProcHit },

    /* Pickup (held) */
    { itRemixClanBombHoldProcUpdate, NULL, NULL, NULL, NULL, NULL, NULL, NULL },

    /* Thrown */
    {
        itBombHeiThrownProcUpdate,
        itRemixClanBombThrownDurationProcMap,
        itRemixClanBombCollidePlayerProcHit,
        itRemixClanBombCollidePlayerProcHit,
        itMainCommonProcHop,
        itRemixClanBombCollidePlayerProcHit,
        itMainCommonProcReflector,
        itRemixClanBombCollidePlayerProcHit
    },

    /* Explode transition */
    { itRemixClanBombCollideClippingProcUpdate, NULL, NULL, NULL, NULL, NULL, NULL, NULL },

    /* Exploding */
    { itRemixClanBombExplodeProcUpdate, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

/* ClanBomb.clanbomb_stage_setting_ (0x80177D9C pattern). Spawns the item and sets its state. */
GObj* itRemixClanBombMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITDesc desc = sITRemixClanBombDesc;
    GObj *item_gobj;
    DObj *dobj;
    ITStruct *ip;
    Vec3f translate;

    desc.p_file = ftRemixPowerData(fp)->p_file_special3;

    item_gobj = itManagerMakeItem(parent_gobj, &desc, pos, vel, flags);

    if (item_gobj == NULL)
    {
        return NULL;
    }

    dobj = DObjGetStruct(item_gobj);
    ip = itGetStruct(item_gobj);

    ip->multi = 0;

    itMainClearOwnerStats(item_gobj);

    translate = dobj->translate.vec.f;
    gcAddXObjForDObjFixed(dobj, 0x2E, 0);
    dobj->translate.vec.f = translate;

    dobj->mobj->palette_id = ITREMIX_CLANBOMB_PALETTE_SPAWN;
    dobj->mobj->texture_id_curr = ITREMIX_CLANBOMB_IMAGE_ID;

    ip->is_unused_item_bool = TRUE;

    ip->attack_coll.damage = ITREMIX_CLANBOMB_DAMAGE;
    ip->attack_coll.knockback_base = ITREMIX_CLANBOMB_KNOCKBACK_BASE;
    ip->attack_coll.knockback_scale = ITREMIX_CLANBOMB_KNOCKBACK_SCALE;

    dobj->rotate.vec.f.z = 0.0F;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/* ClanBomb.prepickup_ (based on the Bob-omb prepickup). */
static void itRemixClanBombPrepickupProc(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNone(item_gobj);
    itMainSetStatus(item_gobj, sITRemixClanBombStatusDescs, nITRemixClanBombStatusPickup);
}

/* ClanBomb.hold_clanbomb_. The held bomb never destroys itself. */
static sb32 itRemixClanBombHoldProcUpdate(GObj *item_gobj)
{
    itRemixClanBombAnimate(item_gobj);

    return FALSE;
}

/* ClanBomb.throw_initial_ (based on the Bob-omb throw). */
static void itRemixClanBombThrownProc(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixClanBombStatusDescs, nITRemixClanBombStatusThrown);
}

/* ClanBomb.throw_duration_ (based on the Bob-omb throw map routine). */
static sb32 itRemixClanBombThrownDurationProcMap(GObj *item_gobj)
{
    itMapCheckMapProcAll(item_gobj, itRemixClanBombCollideTransition);
    itRemixClanBombAnimate(item_gobj);

    return FALSE;
}

/* ClanBomb.collide_with_player_ (based on the red shell collision routine). */
static sb32 itRemixClanBombCollidePlayerProcHit(GObj *item_gobj)
{
    itRemixClanBombExplodeTransition(item_gobj);

    return FALSE;
}

/* ClanBomb.collide_transition (based on the Bob-omb map collision transition). */
static void itRemixClanBombCollideTransition(GObj *item_gobj)
{
    itBombHeiCommonSetHitStatusNormal(item_gobj);
    itMainSetStatus(item_gobj, sITRemixClanBombStatusDescs, nITRemixClanBombStatusExplodeTransition);
}

/* ClanBomb.explode_transition_player_. Not referenced by the item's state tables. */
static sb32 itRemixClanBombExplodeTransitionPlayer(GObj *item_gobj)
{
    itRemixClanBombExplodeTransition(item_gobj);

    return FALSE;
}

/* ClanBomb.collide_with_clipping_ (based on the Bob-omb map collision). */
static sb32 itRemixClanBombCollideClippingProcUpdate(GObj *item_gobj)
{
    itBombHeiCommonCheckMakeDustEffect(item_gobj, FALSE);
    func_800269C0_275C0(ITREMIX_CLANBOMB_FGM_CLIPPING);
    itRemixClanBombExplodeTransition(item_gobj);

    return FALSE;
}

/* ClanBomb.explode_transition_. Zeroes the air velocity, then starts the explosion. */
static void itRemixClanBombExplodeTransition(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    ip->physics.vel_air.x = ip->physics.vel_air.y = ip->physics.vel_air.z = 0.0F;

    itRemixClanBombCheckExplode(item_gobj);
}

/* ClanBomb.check_explode (based on the Bob-omb explode routine, 0x80177060). */
static void itRemixClanBombCheckExplode(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    ITStruct *ip = itGetStruct(item_gobj);
    LBParticle *particle;

    itBombHeiCommonSetHitStatusNone(item_gobj);

    particle = efManagerSparkleWhiteMultiExplodeMakeEffect(&dobj->translate.vec.f);

    if (particle != NULL)
    {
        particle->xf->scale.x = ITREMIX_CLANBOMB_EXPLODE_SCALE;
        particle->xf->scale.y = ITREMIX_CLANBOMB_EXPLODE_SCALE;
        particle->xf->scale.z = ITREMIX_CLANBOMB_EXPLODE_SCALE;
    }

    efManagerQuakeMakeEffect(1);

    dobj->flags = DOBJ_FLAG_HIDDEN;

    ip->attack_coll.fgm_id = ITREMIX_CLANBOMB_FGM_HITBOX;

    itMainRefreshAttackColl(item_gobj);
    itMainClearOwnerStats(item_gobj);
    itRemixClanBombExplodeInitial(item_gobj);
}

/* ClanBomb.explode_initial_ (based on the Bob-omb explode status set). */
static void itRemixClanBombExplodeInitial(GObj *item_gobj)
{
    itBombHeiExplodeInitVars(item_gobj);
    itMainSetStatus(item_gobj, sITRemixClanBombStatusDescs, nITRemixClanBombStatusExplode);
}

/* ClanBomb.explode_ (based on the Bob-omb explode routine, 0x80177BE8). Returns TRUE to destroy. */
static sb32 itRemixClanBombExplodeProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);

    itBombHeiCommonUpdateAttackEvent(item_gobj);

    ip->multi++;

    return ip->multi == ITREMIX_CLANBOMB_EXPLODE_LIFETIME;
}

/* Returns the MObj to tint: the DObj's own, or its first child's when it has none. */
static MObj* itRemixClanBombGetMObj(DObj *dobj)
{
    if (dobj->mobj != NULL)
    {
        return dobj->mobj;
    }

    if (dobj->child == NULL)
    {
        return NULL;
    }

    return dobj->child->mobj;
}

/* Palette cycle step: returns to 3.0F first, then alternates 4.0F and 2.0F by the timer bit. */
static f32 itRemixClanBombNextPalette(f32 palette, u8 timer)
{
    if (palette != 3.0F)
    {
        return 3.0F;
    }

    if ((timer & ITREMIX_CLANBOMB_PALETTE_EXPLODE_FLAG) != 0)
    {
        return 4.0F;
    }

    return 2.0F;
}

/* ClanBomb.animate_clanbomb_. Cycles the palette every 16 frames, using the GObj frame byte as the timer. */
static void itRemixClanBombAnimate(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    MObj *mobj = itRemixClanBombGetMObj(dobj);
    u8 timer = item_gobj->frame_draw_last;

    if (mobj == NULL)
    {
        return;
    }

    if ((timer & ITREMIX_CLANBOMB_ANIM_TIMER_MASK) != 0)
    {
        return;
    }

    mobj->palette_id = itRemixClanBombNextPalette(mobj->palette_id, timer);
}

/* ClanBomb descriptor. Pickup is NULL: pickup_ only jumped to the end of ftCommonLightGetProcDamage. */
const ITRemixItemDesc dITRemixClanBombItemDesc =
{
    ITREMIX_CLANBOMB_ID,                    /* Item.ClanBomb.id */
    itRemixClanBombMakeItem,                /* SPAWN_ITEM */
    TRUE,                                   /* SHOW_GFX_WHEN_SPAWNED */
    itRemixClanBombPrepickupProc,           /* PICKUP_ITEM_INIT */
    itRemixConsumeTomatoTail,               /* PICKUP_ITEM_MAIN: pickup_ jumps to 0x80145C4C */
    itTomatoDroppedSetStatus,               /* DROP_ITEM (same as Maxim Tomato) */
    itRemixClanBombThrownProc,              /* THROW_ITEM */
    NULL                                    /* PLAYER_COLLISION */
};

#endif
