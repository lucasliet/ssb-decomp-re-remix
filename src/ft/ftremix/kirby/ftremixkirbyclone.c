/**
 * ftremixkirbyclone.c - Kirby.asm clone_power_action_swap_ (Remix RAM 0x8058E4F0), the patch at the top of
 * ftMainSetStatus (vanilla RAM 0x800E6F34, ftmain.c). Kirby or J Kirby copying a Remix clone power swaps the
 * parent's action id for the clone's own action id, and keeps every other action unchanged.
 *
 * The action ids come from the assembled Remix ROM (build/remixasm/remix.z64): each clone_action(original, clone)
 * pair compiled to `ori t7, zero, original` followed by `ori a1, zero, clone`, which the disassembler shows at
 * 0x8058E574..0x8058E6BC.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremix.h"
#include "ft/ftremix/ftremixhooks.h"

typedef struct FTRemixKirbyCloneAction
{
    s32 copy_id;
    s32 original_action_id;
    s32 clone_action_id;
} FTRemixKirbyCloneAction;

static const FTRemixKirbyCloneAction dFTRemixKirbyCloneActions[] =
{
    { REMIX_ID_GND,     0x127, 0x12F }, /* Kirby.asm _gnd_actions */
    { REMIX_ID_GND,     0x128, 0x130 },
    { REMIX_ID_DRM,     0x0E9, 0x131 }, /* _drm_actions */
    { REMIX_ID_DRM,     0x0EA, 0x132 },
    { REMIX_ID_DRL,     0x0E9, 0x1C3 }, /* _drl_actions */
    { REMIX_ID_DRL,     0x0EA, 0x1C4 },
    { REMIX_ID_JMARIO,  0x0E9, 0x0E7 }, /* _jmario_actions */
    { REMIX_ID_JMARIO,  0x0EA, 0x0E8 },
    { REMIX_ID_DSAMUS,  0x0EE, 0x133 }, /* _dsamus_actions */
    { REMIX_ID_YLINK,   0x11F, 0x134 }, /* _ylink_actions */
    { REMIX_ID_YLINK,   0x122, 0x135 },
    { REMIX_ID_LUCAS,   0x0FE, 0x136 }, /* _lucas_actions */
    { REMIX_ID_LUCAS,   0x0FF, 0x137 },
    { REMIX_ID_FALCO,   0x0EB, 0x13C }, /* _falco_actions */
    { REMIX_ID_FALCO,   0x0EC, 0x13D },
    { REMIX_ID_PIANO,   0x0E9, 0x140 }, /* _piano_actions */
    { REMIX_ID_PIANO,   0x0EA, 0x141 },
    { REMIX_ID_WOLF,    0x0EB, 0x142 }, /* _wolf_actions */
    { REMIX_ID_WOLF,    0x0EC, 0x143 },
    { REMIX_ID_LANKY,   0x0E9, 0x1C5 }, /* _lanky_actions */
    { REMIX_ID_LANKY,   0x0EA, 0x1C6 }
};

/** Kirby.asm clone_power_action_swap_: TRUE for the fighter kinds whose copy power is read from passive_vars.kirby. */
static sb32 ftRemixKirbyCloneIsKirbyFkind(s32 fkind)
{
    return (fkind == REMIX_ID_KIRBY) || (fkind == REMIX_ID_JKIRBY);
}

/**
 * Kirby.asm clone_power_action_swap_ (0x800E6F34): returns the clone's action id when a Kirby or J Kirby copying
 * a Remix clone is changing to that clone's parent action, otherwise action_id unchanged.
 */
s32 ftRemixKirbyCloneActionSwap(GObj *fighter_gobj, s32 action_id)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 i;

    if (!ftRemixKirbyCloneIsKirbyFkind(fp->fkind))
    {
        return action_id;
    }
    for (i = 0; i < ARRAY_COUNT(dFTRemixKirbyCloneActions); i++)
    {
        const FTRemixKirbyCloneAction *entry = &dFTRemixKirbyCloneActions[i];

        if ((entry->copy_id == fp->passive_vars.kirby.copy_id) && (entry->original_action_id == action_id))
        {
            return entry->clone_action_id;
        }
    }
    return action_id;
}

#endif
