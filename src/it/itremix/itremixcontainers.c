/**
 * itremixcontainers.c - Smash Remix Item Containers setting (Item.asm item_containers_spawn_toggle and
 * item_containers_explosions, Toggles.asm entry_item_containers).
 *
 * Values: 0 DEFAULT, 1 OFF, 2 NO_EXPLODE, 3 ALL_EXPLODE. OFF removes the crates, barrels, capsules and eggs from the item
 * mask; the other values keep them. NO_EXPLODE and ALL_EXPLODE override whether a container explodes after it is hit.
 * Every entry point is a no-op for the vanilla values (DEFAULT and OFF) where the asm leaves vanilla behaviour alone.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include "gm/gmremixtoggle.h"
#include "gm/gmsound.h"
#include "gr/grremix/grremixids.h"
#include <sc/scene.h>
#include <sc/scmanager.h>

#define ITREMIX_CONTAINERS_OFF 1
#define ITREMIX_CONTAINERS_NO_EXPLODE 2
#define ITREMIX_CONTAINERS_ALL_EXPLODE 3

/* Item mask bits of the containers in gSCManagerBattleState->item_toggles: crate 1, barrel 2, capsule 4, egg 8. */
#define ITREMIX_CONTAINERS_MASK_ALL 0x000F
#define ITREMIX_CONTAINERS_MASK_EGG 0x0008

/* Yoshi Island variants use eggs instead of capsules, so OFF keeps the egg bit only on the other stages. */
static sb32 itRemixContainersIsYoshiStage(s32 gkind)
{
    switch (gkind)
    {
        case nGRRemixStage_YOSHIS_ISLAND:
        case nGRRemixStage_YOSHI_ISLAND_DL:
        case nGRRemixStage_YOSHI_ISLAND_O:
        case nGRRemixStage_MINI_YOSHIS_ISLAND:
        case nGRRemixStage_YOSHI_STORY_2:
        case nGRRemixStage_YOSHIS_ISLAND_II:
        case nGRRemixStage_YOSHIS_ISLAND_MELEE:
            return TRUE;
        default:
            return FALSE;
    }
}

/* The VS screen only: the asm checks Global.current_screen against VS_BATTLE. */
static sb32 itRemixContainersIsVSScreen(void)
{
    return gSCManagerSceneData.scene_curr == nSCKindVSBattle;
}

/* Mask of the OFF value: the containers are cleared and the egg is kept on the stages without Yoshi Island variants. */
static u16 itRemixContainersOffMask(u16 mask)
{
    mask &= ~ITREMIX_CONTAINERS_MASK_ALL;

    if (itRemixContainersIsYoshiStage(gSCManagerBattleState->gkind) == FALSE)
    {
        mask |= ITREMIX_CONTAINERS_MASK_EGG;
    }
    return mask;
}

/* Item.asm item_containers_spawn_toggle (0x8016EF64, itManagerSetupContainerDrops): rewrites the low halfword of the
 * item mask (sh t3, 0x000E(t2)); the high halfword stays. Called before the item weights are set up. */
void itRemixContainersApplyToggle(void)
{
    u32 toggles;
    u16 mask;

    if (gSCManagerBattleState->item_appearance_rate == nSCBattleItemSwitchNone)
    {
        return;
    }
    toggles = gSCManagerBattleState->item_toggles;

    if ((toggles == 0) || (itRemixContainersIsVSScreen() == FALSE))
    {
        return;
    }
    mask = (u16) toggles;

    if (gmRemixToggle(item_containers) == ITREMIX_CONTAINERS_OFF)
    {
        mask = itRemixContainersOffMask(mask);
    }
    else mask |= ITREMIX_CONTAINERS_MASK_ALL;

    gSCManagerBattleState->item_toggles = (toggles & 0xFFFF0000) | mask;
}

/* Item.asm item_containers_explosions (_check_container_toggle): TRUE when a hit container is kept (no explosion), given
 * the result of its spawn check. The vanilla result stands for DEFAULT and OFF. */
sb32 itRemixContainerKeeps(sb32 is_spawned)
{
    switch (gmRemixToggle(item_containers))
    {
        case ITREMIX_CONTAINERS_NO_EXPLODE:
            return TRUE;
        case ITREMIX_CONTAINERS_ALL_EXPLODE:
            return FALSE;
        default:
            return is_spawned;
    }
}

/* Item.asm item_containers_explosions crate_throw_sfx (0x801795F8, itBoxCommonCheckSpawnItems): the sound of a crate
 * that is hit. ALL_EXPLODE plays the explosion (fgm 1, nSYAudioFGMExplodeL), otherwise the vanilla crate open. */
s32 itRemixContainerSmashFgm(void)
{
    if (gmRemixToggle(item_containers) == ITREMIX_CONTAINERS_ALL_EXPLODE)
    {
        return nSYAudioFGMExplodeL;
    }
    return nSYAudioFGMContainerSmash;
}

#endif
