/**
 * ftremixtwelvecbdefeated.c - defeated pose of the 12-Character Battle select, see ftremixtwelvecbdefeated.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixtwelvecbdefeated.h"
#include "ft/ftremix/ftremixtwelvecbdefeatedrules.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <sc/scdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>

#define FTREMIX_TWELVECB_DEFEATED_ROW_SCRIPT_OFFSET 4
#define FTREMIX_TWELVECB_DEFEATED_ROW_FLAGS_OFFSET 8
#define FTREMIX_TWELVECB_NO_SCRIPT ((intptr_t)0x80000000u)

static sb32 sFTRemixTwelveCBDefeatedPose[GMCOMMON_PLAYERS_MAX];
static FTMotionDesc sFTRemixTwelveCBDefeatedMotion[GMCOMMON_PLAYERS_MAX];

void ftRemixTwelveCBDefeatedSetPose(s32 player, sb32 is_defeated)
{
    if ((player >= 0) && (player < GMCOMMON_PLAYERS_MAX))
    {
        sFTRemixTwelveCBDefeatedPose[player] = is_defeated;
    }
}

/** Host address of the motion script a defeated_array row points to, or the "no script" marker. */
static intptr_t ftRemixTwelveCBDefeatedResolveScript(u32 script_address)
{
    void *script = remix_ram_image_resolve(script_address);

    return (script != NULL) ? (intptr_t)script : FTREMIX_TWELVECB_NO_SCRIPT;
}

FTMotionDesc* ftRemixTwelveCBDefeatedSwapMotion(const FTStruct *fp, FTMotionDesc *motion_desc)
{
    u32 row_address;
    FTMotionDesc *defeated_motion;

    if ((fp->player < 0) || (fp->player >= GMCOMMON_PLAYERS_MAX))
    {
        return motion_desc;
    }
    row_address = ftRemixTwelveCBDefeatedRulesRowAddress(REMIX_SYM_TwelveCharBattle__defeated_array, fp->fkind);

    if ((row_address == 0) ||
        (ftRemixTwelveCBDefeatedRulesReplacesMotion((gSCManagerSceneData.scene_curr == nSCKindPlayersVS) ? 1 : 0, sFTRemixTwelveCBDefeatedPose[fp->player],
                                                    remix_ram_image_read_word(row_address)) == 0))
    {
        return motion_desc;
    }
    defeated_motion = &sFTRemixTwelveCBDefeatedMotion[fp->player];
    defeated_motion->anim_file_id = remix_ram_image_read_word(row_address);
    defeated_motion->offset = ftRemixTwelveCBDefeatedResolveScript(remix_ram_image_read_word(row_address + FTREMIX_TWELVECB_DEFEATED_ROW_SCRIPT_OFFSET));
    defeated_motion->anim_desc.word = remix_ram_image_read_word(row_address + FTREMIX_TWELVECB_DEFEATED_ROW_FLAGS_OFFSET);

    return defeated_motion;
}

#endif
