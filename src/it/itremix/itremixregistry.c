/**
 * itremixregistry.c - lookup of ported Remix custom items. See itremix.h.
 */

#ifdef PORT

#include "itremix.h"
#include <port_log.h>
#include <it/itmain.h>
#include <sc/scene.h>

#define ITREMIX_KIND_MAX 0x100

static u8 sITRemixMissingLogged[ITREMIX_KIND_MAX];

sb32 itRemixIsCustomKind(s32 kind)
{
    return kind >= ITREMIX_KIND_START;
}

static void itRemixLogMissing(s32 kind)
{
    if ((kind < 0) || (kind >= ITREMIX_KIND_MAX) || (sITRemixMissingLogged[kind] != FALSE))
    {
        return;
    }
    sITRemixMissingLogged[kind] = TRUE;
    port_log("itRemix: item kind 0x%X has no port; add decomp/src/it/itremix/<item>.c and rerun tools/gen_remix_items.py\n", kind);
}

const ITRemixItemDesc* itRemixGetItemDesc(s32 kind)
{
    s32 i;

    for (i = 0; i < dITRemixItemDescsNum; i++)
    {
        if (dITRemixItemDescs[i]->kind == kind)
        {
            return dITRemixItemDescs[i];
        }
    }
    itRemixLogMissing(kind);

    return NULL;
}

void itRemixConsumeTomatoTail(FTStruct *fp, GObj *item_gobj)
{
    itMainDestroyItem(item_gobj);

    if (gSCManagerBattleState->game_type != nSCBattleGameType1PGame)
    {
        return;
    }
    if (fp->player != gSCManagerSceneData.player)
    {
        return;
    }
    if (gSC1PGameBonusTomatoCount < U8_MAX)
    {
        gSC1PGameBonusTomatoCount++;
    }
}

GObj* itRemixMakeItem(GObj *parent_gobj, s32 kind, Vec3f *pos, Vec3f *vel, u32 flags)
{
    const ITRemixItemDesc *desc = itRemixGetItemDesc(kind);

    if ((desc == NULL) || (desc->proc_make == NULL))
    {
        return NULL;
    }
    return desc->proc_make(parent_gobj, pos, vel, flags);
}

#endif
