/**
 * itremixfranklinbadge.c - C port of the Smash Remix Franklin Badge item
 * (.toolchain/pkgs/smashremix-src/src/items/FranklinBadge.asm).
 *
 * The badge is a consume item. Its pickup starts a per-player reflect countdown,
 * which runs as a process on the fighter GObj. The item itself spawns and falls
 * like the Maxim Tomato. Each routine names its Remix label in its doc comment.
 */

#ifdef PORT

#include "it/itremix/itremix.h"
#include "it/itremix/itremixfranklinbadge.h"
#include "it/itremix/itremixsupermushroom.h"
#include "it/itmanager.h"
#include "it/itcommon/ittomato.h"
#include "if/ifcommon.h"
#include "sys/objman.h"
#include "PR/os.h"

#define ITREMIX_FRANKLIN_KIND 0x33
#define ITREMIX_FRANKLIN_FILE_OFFSET 0xF20 /* asm FILE_OFFSET: item footer in file 0xFB */
#define ITREMIX_FRANKLIN_TRANSFORM_KIND 0x1C /* asm item_info_array word 0x0C; the matrix kind has no port name */
#define ITREMIX_FRANKLIN_DURATION (24 * 60) /* asm BADGE_DURATION: 24 seconds at 60 frames per second */
#define ITREMIX_FRANKLIN_IDLE_ACTION 0x0A /* Action.Idle (Action.asm); a lower status id means the player is dead */
#define ITREMIX_FRANKLIN_PROCESS_PRIORITY 1 /* fighter process priority, not stated in the asm: chosen, not verified */
#define ITREMIX_FRANKLIN_REFLECT_FGM 0x041B /* asm reflect_initial_: FGM id for franklin badge reflect */

typedef struct ITRemixFranklinBadgeState
{
    sb32 is_active;
    s32 timer;
    GObj *process_fighter_gobj;
} ITRemixFranklinBadgeState;

extern void* func_800269C0_275C0(u16 id);

/* asm players_with_franklin_badge: one state per player port. */
static ITRemixFranklinBadgeState sITRemixFranklinBadgeStates[MAXCONTROLLERS];

/* asm reflect_hitbox_struct. The first word packs index 0 and custom reflect type 4. */
static FTSpecialColl sITRemixFranklinBadgeReflectColl =
{
    (0x0000 << 16) | 0x0004,
    4, /* asm word "?" after the reflect type */
    { 0.0F, 60.0F, 0.0F },
    { 350.0F, 350.0F, 350.0F }, /* asm comments say 512, but 0x43AF0000 is 350.0F */
    0x18000000
};

/* asm item_info_array. The state table is the Maxim Tomato one, see notes. */
static ITDesc sITRemixFranklinBadgeItemInfo =
{
    ITREMIX_FRANKLIN_KIND,
    &gITManagerCommonData,                  /* item file 0xFB, see notes REMIX-TODO(file) */
    ITREMIX_FRANKLIN_FILE_OFFSET,

    {
        ITREMIX_FRANKLIN_TRANSFORM_KIND,
        nGCMatrixKindNull,
        0
    },

    nGMAttackStateOff,
    itTomatoFallProcUpdate,
    itTomatoFallProcMap,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

/** Returns the badge state slot of the player that owns `fp`. */
static ITRemixFranklinBadgeState* itRemixFranklinBadgeGetState(FTStruct *fp)
{
    return &sITRemixFranklinBadgeStates[fp->player];
}

/** TRUE when the player is alive, using the asm Action.Idle threshold. */
static sb32 itRemixFranklinBadgeIsAlive(FTStruct *fp)
{
    return fp->status_id >= ITREMIX_FRANKLIN_IDLE_ACTION;
}

/**
 * handle_active_franklin_badge_ (destroy path). Frees the player's slot and clears
 * the reflect flag, unless the player is dead. REMIX-TODO(GFX): the asm also
 * restores the player's default gfx routine here (vanilla 0x800E98D4); not ported.
 */
static void itRemixFranklinBadgeEnd(FTStruct *fp, ITRemixFranklinBadgeState *state)
{
    state->is_active = FALSE;
    if (itRemixFranklinBadgeIsAlive(fp))
    {
        fp->is_reflect = FALSE;
    }
}

/**
 * handle_active_franklin_badge_ (per-frame countdown). Runs as a process on the
 * fighter GObj. A player who is dead ends the badge at once.
 */
static void itRemixFranklinBadgeProcUpdate(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    ITRemixFranklinBadgeState *state = itRemixFranklinBadgeGetState(fp);

    if (state->is_active == FALSE)
    {
        return;
    }
    if (itRemixFranklinBadgeIsAlive(fp) == FALSE)
    {
        itRemixFranklinBadgeEnd(fp, state);
        return;
    }
    if (fp->special_coll == &sITRemixFranklinBadgeReflectColl)
    {
        fp->is_reflect = TRUE;
    }
    state->timer -= 1;
    if (state->timer == 0)
    {
        itRemixFranklinBadgeEnd(fp, state);
    }
}

/**
 * handle_active_franklin_badge_ registration. The asm creates one render routine per
 * badge. The port adds one fighter process per fighter GObj, once.
 */
static void itRemixFranklinBadgeRegisterProcess(FTStruct *fp, ITRemixFranklinBadgeState *state)
{
    if (state->process_fighter_gobj == fp->fighter_gobj)
    {
        return;
    }
    gcAddGObjProcess(fp->fighter_gobj, itRemixFranklinBadgeProcUpdate, nGCProcessKindFunc, ITREMIX_FRANKLIN_PROCESS_PRIORITY);
    state->process_fighter_gobj = fp->fighter_gobj;
}

/**
 * pickup_franklin_badge, first badge for this player. Starts the countdown and writes
 * the reflect hitbox. REMIX-TODO(GFX): the badge gfx routine override is not ported.
 */
static void itRemixFranklinBadgeActivate(FTStruct *fp, ITRemixFranklinBadgeState *state)
{
    state->is_active = TRUE;
    state->timer = ITREMIX_FRANKLIN_DURATION;
    fp->special_coll = &sITRemixFranklinBadgeReflectColl;
    itRemixFranklinBadgeRegisterProcess(fp, state);
}

/**
 * pickup_franklin_badge (PICKUP_ITEM_MAIN). Refreshes the timer when the player already
 * has a badge, then jumps to the vanilla tomato tail, which destroys the item.
 */
static void itRemixFranklinBadgePickupConsume(FTStruct *fp, GObj *item_gobj)
{
    ITRemixFranklinBadgeState *state = itRemixFranklinBadgeGetState(fp);

    if (state->is_active)
    {
        state->timer = ITREMIX_FRANKLIN_DURATION;
    }
    else
    {
        itRemixFranklinBadgeActivate(fp, state);
    }
    itRemixConsumeTomatoTail(fp, item_gobj);
}

/**
 * spawn_custom_item_based_on_tomato_ (SPAWN_ITEM). Same as the vanilla Maxim Tomato
 * spawn, with the Franklin Badge item info. Vanilla ABI: (parent, pos, vel, flags).
 */
GObj* itRemixFranklinBadgeMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)
{
    GObj *item_gobj = itManagerMakeItem(parent_gobj, &sITRemixFranklinBadgeItemInfo, pos, vel, flags);
    DObj *joint;
    Vec3f translate;
    ITStruct *ip;

    if (item_gobj == NULL)
    {
        return NULL;
    }
    ip = itGetStruct(item_gobj);
    joint = DObjGetStruct(item_gobj);
    translate = joint->translate.vec.f;

    gcAddXObjForDObjFixed(joint, 0x2E, 0);
    joint->translate.vec.f = translate;
    ip->is_unused_item_bool = TRUE;
    ip->arrow_gobj = ifCommonItemArrowMakeInterface(ip);

    return item_gobj;
}

/**
 * reflect_initial_. Sets the reflect flag and plays the reflect FGM. The routine is
 * the custom reflect action of the badge hitbox, which the reflect system must call.
 * REMIX-TODO(Reflect): that dispatch is not ported. REMIX-TODO(GFX): reflect_gfx_ is skipped.
 */
void itRemixFranklinBadgeReflectInitial(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    fp->is_reflect = TRUE;
    func_800269C0_275C0(ITREMIX_FRANKLIN_REFLECT_FGM);
}

/** TRUE while the player's badge countdown runs. Used by the vanilla immunity hooks. */
sb32 itRemixFranklinBadgeIsActive(FTStruct *fp)
{
    return itRemixFranklinBadgeGetState(fp)->is_active;
}

/**
 * keep_on_action_change_ condition. TRUE when a status change must keep the reflect
 * flag: the badge is active and its reflect hitbox is still the player's hitbox.
 */
sb32 itRemixFranklinBadgeKeepsReflect(FTStruct *fp)
{
    return itRemixFranklinBadgeIsActive(fp) && fp->special_coll == &sITRemixFranklinBadgeReflectColl;
}

/**
 * grant_item_immunity_ condition. TRUE when the badge absorbs this item's
 * hitbox. Mushrooms and the Star are never absorbed; without a badge nothing
 * is; with a badge everything except the Piranha Plant and the Pokémon
 * range (Electrode..Mew).
 */
sb32 itRemixFranklinBadgeIsItemImmune(FTStruct *fp, ITStruct *ip)
{
    s32 kind = ip->kind;

    if ((kind == ITREMIX_SUPERMUSHROOM_KIND) || (kind == ITREMIX_SUPERMUSHROOM_POISON_KIND) || (kind == nITKindStar))
    {
        return FALSE;
    }
    if (itRemixFranklinBadgeIsActive(fp) == FALSE)
    {
        return FALSE;
    }
    if (kind == nITKindPakkun) /* Hazards.stage.PIRANHA_PLANT */
    {
        return FALSE;
    }
    if (kind < nITKindMarumine) /* Hazards.stage.ELECTRODE */
    {
        return TRUE;
    }
    if (kind <= nITKindMew) /* Hazards.pokemon.MEW */
    {
        return FALSE;
    }
    return TRUE;
}

/** clear_active_franklin_badges_. Drops every active badge at scene end. */
void itRemixFranklinBadgeClearAll(void)
{
    s32 port;

    for (port = 0; port < MAXCONTROLLERS; port++)
    {
        sITRemixFranklinBadgeStates[port].is_active = FALSE;
    }
}

/** Descriptor of Franklin Badge (item id 0x33) for the Remix item dispatch. */
const ITRemixItemDesc dITRemixFranklinBadgeItemDesc =
{
    ITREMIX_FRANKLIN_KIND,              /* Item.FranklinBadge.id */
    itRemixFranklinBadgeMakeItem,       /* SPAWN_ITEM */
    TRUE,                               /* SHOW_GFX_WHEN_SPAWNED */
    NULL,                               /* PICKUP_ITEM_INIT (asm 0) */
    itRemixFranklinBadgePickupConsume,  /* PICKUP_ITEM_MAIN: pickup_franklin_badge */
    itTomatoDroppedSetStatus,           /* DROP_ITEM (asm 0x801745FC, same as Maxim Tomato) */
    NULL,                               /* THROW_ITEM (asm 0) */
    NULL                                /* PLAYER_COLLISION (asm 0) */
};

#endif
