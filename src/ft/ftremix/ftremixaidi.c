/**
 * ftremixaidi.c - Remix CPU DI, see ftremixaidi.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <ft/ftdef.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <sys/utils.h>
#include <port_log.h>
#include "ft/ftremix/ftremixaidi.h"
#include "ft/ftremix/ftremixaidirules.h"
#include "gm/gmremixtoggle.h"
#include "remix/remix_training.h"
#include "sc/sc1pmode/sc1premix1p.h"

#define FTREMIXAIDI_PORTS 4

static int8_t sFTRemixAiDiParams[FTREMIXAIDI_PORTS][FTREMIXAIDI_PARAM_COUNT] =
{
    { -1, -1, -1, -1 }, { -1, -1, -1, -1 }, { -1, -1, -1, -1 }, { -1, -1, -1, -1 }
};
static uint8_t sFTRemixAiDiScripts[FTREMIXAIDI_PORTS][FTREMIXAIDI_SCRIPT_SIZE];

static s32 ftRemixAiDiRandom(s32 range)
{
    return syUtilsRandIntRange(range);
}

static sb32 ftRemixAiDiIsTraining(void)
{
    return (gSCManagerBattleState->game_type == nSCBattleGameTypeTraining) ? TRUE : FALSE;
}

/** Remix 1P always uses the advanced roll, other modes need Improved AI outside the classic 1P game (AI.asm _check_advanced_ai). */
static sb32 ftRemixAiDiUsesAdvancedAi(void)
{
    if (sc1PRemix1PIsActive() != FALSE)
    {
        return TRUE;
    }
    if (gmRemixToggle(improved_ai) == 0)
    {
        return FALSE;
    }
    return (gSCManagerBattleState->game_type != nSCBattleGameType1PGame) ? TRUE : FALSE;
}

static void ftRemixAiDiBuildInput(FTStruct *fp, FTRemixAiDiIn *in)
{
    in->is_training = ftRemixAiDiIsTraining();
    in->menu_type = remix_training_get(REMIX_TRAINING_DI_TYPE);
    in->menu_strength = remix_training_get(REMIX_TRAINING_DI_STRENGTH);
    in->menu_direction = remix_training_get(REMIX_TRAINING_DI_DIRECTION);
    in->menu_first_hit = remix_training_get(REMIX_TRAINING_DI_FIRST_HIT);
    in->hit_count = gSCManagerBattleState->players[fp->player].combo_count_foe;
    in->is_in_hitlag = (fp->hitlag_tics != 0) ? TRUE : FALSE;
    in->status_frame = (s32)fp->status_total_tics;
    in->is_damage_status = ((fp->status_id >= nFTCommonStatusDamageHi1) && (fp->status_id <= nFTCommonStatusDamageFlyRoll)) ? TRUE : FALSE;
    in->cpu_level = fp->level;
    in->is_facing_right = (fp->lr > 0) ? TRUE : FALSE;
    in->uses_advanced_ai = ftRemixAiDiUsesAdvancedAi();
}

sb32 ftRemixAiCpuDiSetObjective(FTStruct *fp)
{
    FTRemixAiDiIn in;
    FTRemixAiDiOut out;
    s32 i;

    if (fp->player >= FTREMIXAIDI_PORTS)
    {
        return FALSE;
    }
    ftRemixAiDiBuildInput(fp, &in);
    ftRemixAiDiDecide(&in, sFTRemixAiDiParams[fp->player], ftRemixAiDiRandom, &out);

    for (i = 0; i < FTREMIXAIDI_PARAM_COUNT; i++)
    {
        sFTRemixAiDiParams[fp->player][i] = out.params[i];
    }
    if (out.has_routine == FALSE)
    {
        return FALSE;
    }
    ftRemixAiDiBuildScript(out.is_slide, out.strength, out.direction, sFTRemixAiDiScripts[fp->player]);
    fp->computer.p_command = sFTRemixAiDiScripts[fp->player];
    fp->computer.input_wait = 1;
    port_log("SSB64: cpu di: port %d slide %d strength %d direction %d\n", (int)fp->player, (int)out.is_slide, (int)out.strength,
             (int)out.direction);

    return TRUE;
}

#endif
