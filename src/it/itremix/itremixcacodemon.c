/**
 * itremixcacodemon.c - C port of the Smash Remix Cacodemon item
 * (.toolchain/pkgs/smashremix-src/src/items/Cacodemon.asm, Hazards.asm cacodemon_*).
 *
 * The Cacodemon hovers over the stage, fires a Thunderball on the loop frames and is
 * destroyed by the blast zones or by a hit. Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "sys/objanim.h"
#include "it/itremix/itremixcacodemon.h"
#include "it/itmain.h"
#include "it/itmanager.h"
#include "gr/grremix/grremixcacodemonfactory.h"
#include "gr/grremix/grremixthunderball.h"
#include "sys/obj.h"

#define ITREMIX_CACODEMON_FILE_OFFSET 0xE0 /* asm FILE_OFFSET: item footer in the Cacodemon file */
#define ITREMIX_CACODEMON_HIT_FGM 0x26 /* asm 0x156 hitbox sound (medium kick) */
#define ITREMIX_CACODEMON_SHOOT_FGM 0x55D
#define ITREMIX_CACODEMON_DEATH_FGM 0x55E
#define ITREMIX_CACODEMON_DAMAGE 5
#define ITREMIX_CACODEMON_ANGLE 0x361 /* asm sakurai angle */
#define ITREMIX_CACODEMON_HITBOX_SIZE 125.0F
#define ITREMIX_CACODEMON_KB_SCALE 0x25
#define ITREMIX_CACODEMON_KB_BASE 0x50
#define ITREMIX_CACODEMON_ANIM_JOINT_OFFSET 0x0918 /* asm cacodemon_factory_: joint track, the same for both directions */
#define ITREMIX_CACODEMON_ANIM_MATERIAL_OFFSET 0x10C4 /* asm cacodemon_factory_: material track */
#define ITREMIX_CACODEMON_DL_LEFT 0x6C8 /* Cacodemon file offset of the facing-left display list */
#define ITREMIX_CACODEMON_DL_RIGHT 0x3058 /* Cacodemon file offset of the facing-right display list */
#define ITREMIX_CACODEMON_LOOP_START 0x3C /* spawn_timer value that starts the attack loop */
#define ITREMIX_CACODEMON_TURN_RIGHT 0x106
#define ITREMIX_CACODEMON_LOOP_RESTART 0x20C
#define ITREMIX_CACODEMON_MULTI 0xB4 /* asm 0x033E halfword at stage setting (Hazards.asm 8719), the pirhana flag */
#define ITREMIX_CACODEMON_HIT_RECORD_GROUP 7 /* asm 0x0E written to the first byte of each hit record (group_id) */

enum itRemixCacodemonStatus
{
    nITRemixCacodemonStatusMain,
    nITRemixCacodemonStatusRest,
    nITRemixCacodemonStatusExplosion,
    nITRemixCacodemonStatusEnumCount
};

/* Loop frames on which the Cacodemon fires (asm _shoot labels). */
static const u16 sITRemixCacodemonShotFrames[] = { 0x3E, 0x70, 0x12E, 0x160 };

extern void* func_800269C0_275C0(u16 id);

/* Base of the Cacodemon file. The item attributes and display lists are read from here. */
static void *sITRemixCacodemonFileTop;

static sb32 itRemixCacodemonProcMain(GObj *item_gobj);
static sb32 itRemixCacodemonProcDead(GObj *item_gobj);
static sb32 itRemixCacodemonDestroy(GObj *item_gobj);

/* asm item_info_array, p_file is the Cacodemon file top. */
static ITDesc sITRemixCacodemonDesc =
{
    ITREMIX_CACODEMON_KIND,
    &sITRemixCacodemonFileTop,
    ITREMIX_CACODEMON_FILE_OFFSET,
    {
        nGCMatrixKindTraRotRpyR,
        nGCMatrixKindNull,
        0
    },
    nGMAttackStateOff,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/* asm cacodemon_item_states. Every state uses the main routine and destroys the item on a hurtbox hit. */
static ITStatusDesc sITRemixCacodemonStatusDescs[nITRemixCacodemonStatusEnumCount] =
{
    {
        itRemixCacodemonProcMain,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixCacodemonDestroy
    },
    {
        itRemixCacodemonProcMain,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixCacodemonDestroy
    },
    {
        itRemixCacodemonProcMain,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        itRemixCacodemonDestroy
    }
};

/** Cacodemon.destroy_: plays the death sound and lets the factory spawn the next Cacodemon. */
static sb32 itRemixCacodemonDestroy(GObj *item_gobj)
{
    func_800269C0_275C0(ITREMIX_CACODEMON_DEATH_FGM);
    grRemixCacodemonFactoryAllowSpawn();

    return TRUE;
}

/** cacodemon_blast_zone_ (proc_dead): destroys the Cacodemon when it crosses a blast zone. */
static sb32 itRemixCacodemonProcDead(GObj *item_gobj)
{
    grRemixCacodemonFactoryAllowSpawn();

    return TRUE;
}

/** Sets the Cacodemon file top used by the item attributes and display lists. */
void itRemixCacodemonSetFileTop(void *file_top)
{
    sITRemixCacodemonFileTop = file_top;
}

void itRemixCacodemonStartAnimation(GObj *item_gobj)
{
    DObj *dobj = DObjGetStruct(item_gobj);
    u8 *file = (u8*) sITRemixCacodemonFileTop;

    gcAddDObjAnimJoint(dobj, (AObjEvent32*) (file + ITREMIX_CACODEMON_ANIM_JOINT_OFFSET), 0.0F);
    if (dobj->mobj != NULL)
    {
        gcAddMObjMatAnimJoint(dobj->mobj, (AObjEvent32*) (file + ITREMIX_CACODEMON_ANIM_MATERIAL_OFFSET), 0.0F);
    }
}

/** Hitbox part of cacodemon_stage_setting_: damage, angle, size, sound and knockback. */
static void itRemixCacodemonApplyHitbox(ITStruct *ip)
{
    ip->attack_coll.attack_state = 1; /* asm 0x10C = 1 enables the hitbox */
    ip->attack_coll.size = ITREMIX_CACODEMON_HITBOX_SIZE;
    ip->attack_coll.damage = ITREMIX_CACODEMON_DAMAGE;
    ip->attack_coll.angle = ITREMIX_CACODEMON_ANGLE;
    ip->attack_coll.fgm_id = ITREMIX_CACODEMON_HIT_FGM;
    ip->attack_coll.knockback_scale = ITREMIX_CACODEMON_KB_SCALE;
    ip->attack_coll.knockback_base = ITREMIX_CACODEMON_KB_BASE;
    /* asm 0x11C is attack_coll.element (Hazards.asm 8736, 0 = punch); asm 0x1D4 is attack_pos[1].pos_prev.y. */
    ip->attack_coll.element = 0;
    ip->attack_coll.attack_pos[1].pos_prev.y = 0.0F;
}

/** Cacodemon.stage_setting_ (SPAWN_ITEM): creates the item with its hitbox and blast zone routine. */
GObj* itRemixCacodemonMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixCacodemonDesc, pos, vel, flags);
    ITStruct *ip;
    ITRemixCacodemonVars *vars;

    if (item_gobj == NULL)
    {
        return NULL;
    }
    ip = itGetStruct(item_gobj);
    vars = ITREMIX_CACODEMON_VARS(ip);
    ((s32 *) vars)[3] = 0; /* asm 0x35C = item_vars+0xC; cleared before target_gobj, which overlaps it on LP64 */
    vars->spawn_timer = 0;
    vars->loop_timer = 0;
    vars->target_gobj = NULL;
    vars->target_x_diff = 0.0F;

    itRemixCacodemonApplyHitbox(ip);
    ip->multi = ITREMIX_CACODEMON_MULTI;
    ip->attack_coll.can_setoff = FALSE;
    ip->attack_coll.can_rehit_item = FALSE;
    ip->attack_coll.can_rehit_fighter = FALSE;
    ip->attack_coll.can_rehit_shield = FALSE;
    ip->attack_coll.can_hop = FALSE;
    ip->attack_coll.can_reflect = FALSE;
    ip->attack_coll.can_shield = FALSE;
    ip->attack_coll.motion_attack_id &= 0x1F; /* asm sb r0 to 0x158 also clears the top bit of motion_attack_id */
    ip->is_allow_knockback = TRUE;
    ip->proc_dead = itRemixCacodemonProcDead;
    itMainSetStatus(item_gobj, sITRemixCacodemonStatusDescs, nITRemixCacodemonStatusMain);

    return item_gobj;
}

/** Points the display list of the Cacodemon at its facing: asm writes the position struct display list. */
static void itRemixCacodemonSetFacing(GObj *item_gobj, u32 dl_offset)
{
    DObjGetStruct(item_gobj)->dl = lbRelocGetFileData(Gfx*, sITRemixCacodemonFileTop, dl_offset);
}

/** Clears the four attack-record slots that the asm resets on each loop turn. */
static void itRemixCacodemonResetHitRecords(ITStruct *ip)
{
    GMAttackRecord *record;
    s32 i;

    for (i = 0; i < GMATTACKREC_NUM_MAX; i++)
    {
        record = &ip->attack_coll.attack_records[i];
        record->victim_gobj = NULL;
        record->victim_flags.is_interact_hurt = FALSE;
        record->victim_flags.is_interact_shield = FALSE;
        record->victim_flags.is_interact_reflect = FALSE;
        record->victim_flags.is_interact_absorb = FALSE;
        record->victim_flags.group_id = ITREMIX_CACODEMON_HIT_RECORD_GROUP;
        record->victim_flags.timer_rehit &= 0x1F; /* asm sb 0x0E at 0x0228 + 8*i: top bit of the byte cleared */
    }
}

/** Shot frame test over sITRemixCacodemonShotFrames. */
static sb32 itRemixCacodemonIsShotFrame(u16 frame)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sITRemixCacodemonShotFrames); i++)
    {
        if (sITRemixCacodemonShotFrames[i] == frame)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** Fires one Thunderball and plays the shoot sound (asm _shoot label). */
static void itRemixCacodemonShoot(GObj *item_gobj)
{
    grRemixThunderballShoot(item_gobj);
    func_800269C0_275C0(ITREMIX_CACODEMON_SHOOT_FGM);
}

/** cacodemon_main_ loop part: fires on the shot frames, turns on the loop turn and restarts the loop. */
static void itRemixCacodemonProcLoop(GObj *item_gobj, ITRemixCacodemonVars *vars)
{
    u16 frame = vars->loop_timer;

    vars->loop_timer++;
    if (itRemixCacodemonIsShotFrame(frame))
    {
        itRemixCacodemonShoot(item_gobj);
        return;
    }
    if (frame == ITREMIX_CACODEMON_TURN_RIGHT)
    {
        itRemixCacodemonResetHitRecords(itGetStruct(item_gobj));
        itRemixCacodemonSetFacing(item_gobj, ITREMIX_CACODEMON_DL_RIGHT);
        return;
    }
    if (frame == ITREMIX_CACODEMON_LOOP_RESTART)
    {
        itRemixCacodemonResetHitRecords(itGetStruct(item_gobj));
        itRemixCacodemonSetFacing(item_gobj, ITREMIX_CACODEMON_DL_LEFT);
        vars->loop_timer = 0;
    }
}

/** cacodemon_main_ (proc_update): waits for the loop start frame, then runs the attack loop. */
static sb32 itRemixCacodemonProcMain(GObj *item_gobj)
{
    ITRemixCacodemonVars *vars = ITREMIX_CACODEMON_VARS(itGetStruct(item_gobj));

    if (vars->spawn_timer != ITREMIX_CACODEMON_LOOP_START)
    {
        vars->spawn_timer++;
        return FALSE;
    }
    itRemixCacodemonProcLoop(item_gobj, vars);

    return FALSE;
}

/** Descriptor of the Cacodemon (item id 0x3F) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixCacodemonItemDesc =
{
    ITREMIX_CACODEMON_KIND,             /* Item.Cacodemon.id */
    itRemixCacodemonMakeItem,           /* SPAWN_ITEM */
    FALSE,                              /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT */
    NULL,                               /* PICKUP_ITEM_MAIN */
    NULL,                               /* DROP_ITEM */
    NULL,                               /* THROW_ITEM */
    NULL                                /* PLAYER_COLLISION */
};

#endif
