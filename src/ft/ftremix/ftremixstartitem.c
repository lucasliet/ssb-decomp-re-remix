/**
 * ftremixstartitem.c - Smash Remix Item.asm start_with_item and taunt_spawn_item: the item a fighter begins the match
 * with (match_begin_with_item_, respawn_with_item_) and the item a taunt spawns (taunt_spawn_item_).
 *
 * The 20-entry tables of the asm are kept as one array per use, indexed by the CSS panel value.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <ft/fthammer.h>
#include <gm/gmremixpanel.h>
#include <it/item.h>
#include <it/itdef.h>
#include <it/itmain.h>
#include <sys/utils.h>
#include "it/itremix/itremix.h"

#define FTREMIX_START_ITEM_RANDOM_INDEX 19
#define FTREMIX_START_ITEM_HAMMER_TICS 0x2D0
#define FTREMIX_START_ITEM_BLUESHELL_KIND 0x30 /* Item.BlueShell.id */
#define FTREMIX_START_ITEM_DEKUNUT_KIND 0x32 /* Item.DekuNut.id */
/* Item.Pitfall.id: Item.asm add_item list (its start table comment says 0x33) */
#define FTREMIX_START_ITEM_PITFALL_KIND 0x34
#define FTREMIX_START_ITEM_GOLDENGUN_KIND 0x35 /* Item.GoldenGun.id */
#define FTREMIX_START_ITEM_MRSATURN_KIND 0x38 /* Item.MrSaturn.id */

extern GObj* (*dITManagerProcMakeList[])(GObj*, Vec3f*, Vec3f*, u32);

/* Item.asm start_with_table_ (index 0 = none, 19 = random); index 1..18 are the item kinds, in asm order */
static const s32 sFTRemixStartItemKinds[FTREMIX_START_ITEM_RANDOM_INDEX] = {
    0,
    nITKindSword,
    nITKindBat,
    nITKindHarisen,
    nITKindStarRod,
    nITKindLGun,
    nITKindFFlower,
    nITKindHammer,
    nITKindMSBomb,
    nITKindBombHei,
    nITKindNBumper,
    nITKindGShell,
    nITKindRShell,
    nITKindMBall,
    FTREMIX_START_ITEM_BLUESHELL_KIND,
    FTREMIX_START_ITEM_DEKUNUT_KIND,
    FTREMIX_START_ITEM_PITFALL_KIND,
    FTREMIX_START_ITEM_GOLDENGUN_KIND,
    FTREMIX_START_ITEM_MRSATURN_KIND,
};

/**
 * Picks the kind of a panel index. Random (19) rolls 0..18 and maps a roll of 0 to 1, as Item.asm does.
 */
static s32 ftRemixStartItemKindOfIndex(s32 index)
{
    if (index != FTREMIX_START_ITEM_RANDOM_INDEX)
    {
        return sFTRemixStartItemKinds[index];
    }
    index = syUtilsRandIntRange(FTREMIX_START_ITEM_RANDOM_INDEX);
    if (index == 0)
    {
        index = 1;
    }
    return sFTRemixStartItemKinds[index];
}

/**
 * Spawns item `kind` at the fighter top joint with the asm setup velocity (0, 30, 0), without the spawn swirl and
 * appear spin (Item.asm create_and_assign_item_ sets the skip spawn gfx flag). Returns NULL when the kind has no port.
 */
static GObj* ftRemixStartItemMake(GObj *fighter_gobj, s32 kind)
{
    Vec3f *pos = &DObjGetStruct(fighter_gobj)->translate.vec.f;
    Vec3f vel = { 0.0F, 30.0F, 0.0F };

    if (itRemixIsCustomKind(kind))
    {
        return itRemixMakeItem(NULL, kind, pos, &vel, ITEM_FLAG_PARENT_GROUND);
    }
    return dITManagerProcMakeList[kind](NULL, pos, &vel, ITEM_FLAG_PARENT_GROUND);
}

/**
 * Gives the fighter one item of the panel index (create_and_assign_item_): nothing when the fighter already holds an item.
 * A hammer is swung at once, as Item.asm does for the hammer id.
 */
static void ftRemixStartItemGive(GObj *fighter_gobj, s32 index)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 kind;
    GObj *item_gobj;

    if ((index == 0) || (fp->item_gobj != NULL))
    {
        return;
    }
    kind = ftRemixStartItemKindOfIndex(index);
    item_gobj = ftRemixStartItemMake(fighter_gobj, kind);
    if (item_gobj == NULL)
    {
        return;
    }
    itMainSetFighterHold(item_gobj, fighter_gobj);
    if (kind == nITKindHammer)
    {
        fp->hammer_tics = FTREMIX_START_ITEM_HAMMER_TICS;
        /* REMIX-TODO(audio): Item.asm also plays the hammer midi 0x2D (0x800E7AFC) */
        ftHammerSetStatusHammerWait(fighter_gobj);
    }
}

/**
 * Item at the start of the match (Item.asm match_begin_with_item_, the GO hook): the panel Start With value of the port.
 */
void ftRemixStartItemMatchBegin(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixStartItemGive(fighter_gobj, gmRemixPanel(START_WITH, fp->player));
}

/**
 * Item on respawn (Item.asm respawn_with_item_, action Revive1): the panel Start With value of the port.
 */
void ftRemixStartItemRespawn(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixStartItemGive(fighter_gobj, gmRemixPanel(START_WITH, fp->player));
}

/**
 * Item of a taunt (Item.asm taunt_spawn_item_): the panel Taunt Item value of the port. Called after the taunt status is set.
 */
void ftRemixTauntItemSpawn(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    ftRemixStartItemGive(fighter_gobj, gmRemixPanel(TAUNT_ITEM, fp->player));
}

#endif
