#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixcloud.h"
#include <it/itmain.h>
#include <it/itmanager.h>
#include <gm/gmcollision.h>
#include <gm/gmdef.h>
#include <sys/objman.h>
#include "ft/ftremix/ftremix.h"

#define ITREMIX_CLOUD_TOTAL_TIMER 30
#define ITREMIX_CLOUD_TIME_UNTIL_FADE 8
#define ITREMIX_CLOUD_FLASH_FRAME_BEGIN 70
#define ITREMIX_CLOUD_FADE_FGM 0x113 /* FGM.CLOUD_FADE in Remix FGM.asm (the asm comment "was 0x501" is stale) */

enum ITRemixCloudGoemonStatus
{
    nITRemixCloudGoemonStatusUSP = 0xDF,        /* Goemon.asm scope Action: USP */
    nITRemixCloudGoemonStatusUSPAttack = 0xE0,  /* USPAttack */
    nITRemixCloudGoemonStatusUSPJump = 0xE1,    /* USPJump */
    nITRemixCloudGoemonStatusUSPEscape = 0xE2,  /* USPEscape */
};

enum ITRemixCloudStatus
{
    nITRemixCloudStatusNull,
    nITRemixCloudStatusAerial,
};

/* Item state kept in ITStruct.item_vars. The N64 offsets are the Remix item-special-struct slots. */
typedef struct ITRemixCloudVars
{
    s32 life_timer;                             /* N64 0x02C0 */
    s16 death_timer;                            /* N64 0x033E */
    GObj *player_gobj;                          /* N64 0x01C4, owning Goemon (no reader in this item) */
    s32 rotation_direction;                     /* N64 0x01CC (no reader in this item) */
    s32 hitbox_refresh_timer;                   /* N64 0x01D0 (no reader in this item) */
    sb32 hitbox_collision_flag;                 /* N64 0x01D4 (no reader in this item) */
    void (*blast_zone_proc)(GObj *item_gobj);   /* N64 0x0398 */
} ITRemixCloudVars;

_Static_assert(sizeof(ITRemixCloudVars) <= sizeof(((ITStruct*)0)->item_vars), "ITRemixCloudVars must fit in ITStruct.item_vars");

extern alSoundEffect* func_800269C0_275C0(u16 sfx_id);

/* Item kinds whose GObj flash flags itProcessProcItemMain must not clear.
 * Cloud.asm enable_flash (Cloud.asm 287-315) patches the vanilla item main
 * loop so the per-frame `item_gobj->flags = GOBJ_FLAG_NONE` is skipped for
 * Item.Cloud (0x49) and the Bowser bomb (0x11A); both write the flags word
 * themselves (the cloud's flash while Goemon stands on it). The itprocess.c
 * hook is a shared-file patch, see the notes file. */
sb32 itRemixKeepsFlashFlags(s32 kind)
{
    return (kind == 0x49) || (kind == 0x11A);
}

static sb32 itRemixCloudNullProcUpdate(GObj *item_gobj);
static sb32 itRemixCloudAerialProcUpdate(GObj *item_gobj);
static void itRemixCloudBlastZoneProc(GObj *item_gobj);
static void itRemixCloudSetupItem(GObj *item_gobj, GObj *parent_gobj);
static sb32 itRemixCloudIsPinnedAction(s32 status_id);
static void itRemixCloudPinToFeet(GObj *item_gobj);
static void itRemixCloudFlashWhileOwnerFrameTimer(GObj *item_gobj);
static void itRemixCloudDetach(GObj *item_gobj);
static void itRemixCloudCountLife(ITStruct *ip);

/* Item info array (Remix item_info_array). p_file is set per owner fighter at spawn. */
static ITDesc sITRemixCloudInfoDesc =
{
    0x49,                                       /* Item.Cloud.id */
    NULL,                                       /* p_file: Goemon file 7 (p_file_special2), set at spawn */
    0x40,                                       /* o_attributes: item footer offset */
    {
        nGCMatrixKindTraRotRpyR,                /* REMIX-TODO: item_info_array 0x0C is unknown (0x1B000000 or 0x1C000000) */
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    itRemixCloudNullProcUpdate,                 /* state 0 main, the initial state */
    NULL,                                       /* Proc Map */
    NULL,                                       /* Proc Hit */
    NULL,                                       /* Proc Shield */
    NULL,                                       /* Proc Hop */
    NULL,                                       /* Proc Set-Off */
    NULL,                                       /* Proc Reflector */
    NULL                                        /* Proc Damage */
};

/* Item state table (Remix item_state_table). */
static ITStatusDesc sITRemixCloudStatusDescs[] =
{
    {
        itRemixCloudNullProcUpdate,             /* Proc Update */
        NULL,                                   /* Proc Map */
        NULL,                                   /* Proc Hit */
        NULL,                                   /* Proc Shield */
        NULL,                                   /* Proc Hop */
        NULL,                                   /* Proc Set-Off */
        NULL,                                   /* Proc Reflector */
        NULL                                    /* Proc Damage */
    },
    {
        itRemixCloudAerialProcUpdate,           /* Proc Update */
        NULL,                                   /* Proc Map */
        NULL,                                   /* Proc Hit */
        NULL,                                   /* Proc Shield */
        NULL,                                   /* Proc Hop */
        NULL,                                   /* Proc Set-Off */
        NULL,                                   /* Proc Reflector */
        NULL                                    /* Proc Damage */
    }
};

/* Overlays the Remix cloud state on the item's shared item_vars union. */
static ITRemixCloudVars* itRemixCloudGetVars(ITStruct *ip)
{
    return (ITRemixCloudVars*) &ip->item_vars;
}

/* Remix sets the flash visibility by writing the GObj flags word directly. */
static void itRemixCloudSetFlashFlag(GObj *item_gobj, s32 timer)
{
    item_gobj->flags = (timer & 2) >> 1;
}

/* REMIX-TODO(Size): Size.asm item scope not ported. apply_size_to_animation_ (0x8003DE5C jump table, cloud joint scale) needs
 * the animation command interpreter, and render_routine_ (ECB scale of the cloud from its holder, wraps 0x80171F4C) needs the
 * item render wiring. The cloud keeps its vanilla scale and ECB at every size until both are ported. */
/* Cloud.asm stage_setting_: the spawn routine, called with Item.Cloud's desc from the Remix dispatcher. */
GObj* itRemixCloudMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    ITDesc item_desc = sITRemixCloudInfoDesc;
    FTStruct *fp = ftGetStruct(parent_gobj);
    GObj *item_gobj;

    item_desc.p_file = ftRemixPowerData(fp)->p_file_special2;
    /* The asm passes 0 as the fifth argument, so the `flags` argument is not forwarded. */
    item_gobj = itManagerMakeItem(parent_gobj, &item_desc, pos, vel, 0);
    if (item_gobj == NULL)
    {
        return NULL;
    }
    itRemixCloudSetupItem(item_gobj, parent_gobj);

    return item_gobj;
}

/* Initialises the item's owner, team, hitboxes and state variables after it has spawned. */
static void itRemixCloudSetupItem(GObj *item_gobj, GObj *parent_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    DObj *joint = DObjGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(parent_gobj);
    ITRemixCloudVars *cp = itRemixCloudGetVars(ip);

    joint->translate.vec.f.z = 0;
    gcAddXObjForDObjFixed(joint, 0x2E, 0);

    ip->owner_gobj = parent_gobj;
    ip->player = fp->player;
    ip->team = fp->team;
    cp->player_gobj = parent_gobj;

    ip->damage_coll.hitstatus = 0;              /* disable hurtbox */
    ip->attack_coll.attack_state = nGMAttackStateOff; /* disable hitbox */

    cp->life_timer = 0;
    cp->death_timer = 0;
    cp->rotation_direction = 0;
    cp->hitbox_refresh_timer = 0;
    cp->hitbox_collision_flag = FALSE;
    cp->blast_zone_proc = itRemixCloudBlastZoneProc;
}

/* cloud_null_main_: runs while Goemon stands on the cloud. */
static sb32 itRemixCloudNullProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(ip->owner_gobj);

    if (itRemixCloudIsPinnedAction(fp->status_id))
    {
        itRemixCloudPinToFeet(item_gobj);
        itRemixCloudFlashWhileOwnerFrameTimer(item_gobj);
    }
    else
    {
        itRemixCloudDetach(item_gobj);
    }
    itRemixCloudCountLife(ip);

    return FALSE;
}

/* USP and USPAttack keep the cloud under Goemon's feet. */
static sb32 itRemixCloudIsPinnedAction(s32 status_id)
{
    return (status_id == nITRemixCloudGoemonStatusUSP) || (status_id == nITRemixCloudGoemonStatusUSPAttack);
}

/* Moves the cloud to Goemon's top joint, then zeroes its z rotation. */
static void itRemixCloudPinToFeet(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(ip->owner_gobj);
    DObj *joint = DObjGetStruct(item_gobj);
    ITRemixCloudVars *cp = itRemixCloudGetVars(ip);

    joint->translate.vec.f.x = 0;
    joint->translate.vec.f.y = 0;
    joint->translate.vec.f.z = 0;
    gmCollisionGetFighterPartsWorldPosition(fp->joints[0], &joint->translate.vec.f);

    /* REMIX-TODO(uncertain): the asm reads the item DObj from s0, which this routine never sets. Using the item's own DObj here. */
    joint->rotate.vec.f.z = 0;
    cp->rotation_direction = 0;
}

/* Remix flashes the cloud while the owner's USP frame timer is at or above ITREMIX_CLOUD_FLASH_FRAME_BEGIN. */
static void itRemixCloudFlashWhileOwnerFrameTimer(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(ip->owner_gobj);
    s32 frame_timer = fp->status_vars.common.dead.wait; /* N64 0x0B18, GOEMON_USP_FRAME_TIMER in ftremixgoemonusp.c */

    if (frame_timer < ITREMIX_CLOUD_FLASH_FRAME_BEGIN)
    {
        return;
    }
    itRemixCloudSetFlashFlag(item_gobj, frame_timer);
}

/* Leaves Goemon's hand after a jump or escape. The cloud falls with half of Goemon's aerial speed. */
static void itRemixCloudDetach(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    FTStruct *fp = ftGetStruct(ip->owner_gobj);
    ITRemixCloudVars *cp = itRemixCloudGetVars(ip);
    s32 owner_status = fp->status_id;

    cp->death_timer = 1;
    if ((owner_status == nITRemixCloudGoemonStatusUSPJump) || (owner_status == nITRemixCloudGoemonStatusUSPEscape))
    {
        cp->death_timer = ITREMIX_CLOUD_TOTAL_TIMER;
    }
    ip->physics.vel_air.x = fp->coll_data.pos_diff.x * 0.5F;
    ip->physics.vel_air.y = fp->coll_data.pos_diff.y * 0.5F;

    item_gobj->flags = 0;
    itMainSetStatus(item_gobj, sITRemixCloudStatusDescs, nITRemixCloudStatusAerial);
}

/* cloud_aerial_main_: counts down the death timer, flashes, then destroys the cloud. */
static sb32 itRemixCloudAerialProcUpdate(GObj *item_gobj)
{
    ITStruct *ip = itGetStruct(item_gobj);
    ITRemixCloudVars *cp = itRemixCloudGetVars(ip);

    cp->death_timer -= 1;
    if (cp->death_timer == 0)
    {
        func_800269C0_275C0(ITREMIX_CLOUD_FADE_FGM);
        return TRUE;
    }
    if (cp->death_timer < (ITREMIX_CLOUD_TOTAL_TIMER - ITREMIX_CLOUD_TIME_UNTIL_FADE))
    {
        itRemixCloudSetFlashFlag(item_gobj, cp->death_timer);
    }
    itRemixCloudCountLife(ip);

    return FALSE;
}

/* Increments the life counter at the end of each update, as the asm `_end` label does. */
static void itRemixCloudCountLife(ITStruct *ip)
{
    ITRemixCloudVars *cp = itRemixCloudGetVars(ip);

    cp->life_timer += 1;
}

/* cloud_blast_zone_: a bare return in Remix. Blast-wall code calls it through blast_zone_proc (REMIX-TODO(BlastWall)). */
static void itRemixCloudBlastZoneProc(GObj *item_gobj)
{
}

const ITRemixItemDesc dITRemixCloudItemDesc =
{
    0x49,                       /* Item.Cloud.id */
    itRemixCloudMakeItem,       /* SPAWN_ITEM (vanilla ABI: parent, pos, vel, flags) */
    FALSE,                      /* SHOW_GFX_WHEN_SPAWNED (OS.FALSE) */
    NULL,                       /* PICKUP_ITEM_INIT */
    NULL,                       /* PICKUP_ITEM_MAIN */
    NULL,                       /* DROP_ITEM */
    NULL,                       /* THROW_ITEM */
    NULL                        /* PLAYER_COLLISION */
};

#endif
