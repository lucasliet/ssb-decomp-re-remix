/**
 * ftremixsonicclassic.c - Classic Sonic on the match side: model file, FGM swaps, muted voices, shield colour offset, select
 * animation frame and menu animations (.toolchain/pkgs/smashremix-src/src/Sonic/Sonic.asm).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/sonic/ftremixsonicclassic.h"
#include "ft/ftremix/sonic/ftremixsonicclassicrules.h"
#include "remix/remix_debug.h"
#include "remix/remix_roster_data.h"
#include "sc/scmanager.h"
#include "sc/sctypes.h"
#include "sc/scdef.h"

#define FTREMIX_SONIC_NO_SCRIPT ((intptr_t)0x80000000u)

static f32 sFTRemixSonicClassicSelectFrame[GMCOMMON_PLAYERS_MAX];
static FTMotionDesc sFTRemixSonicClassicMenuMotion;

static sb32 ftRemixSonicClassicIsPlayerValid(s32 player)
{
    return ((player >= 0) && (player < GMCOMMON_PLAYERS_MAX)) ? TRUE : FALSE;
}

/**
 * Smoke test override: SSB64_REMIX_CLASSIC_SONIC_P1..P4=1 makes the Sonic of that port classic, because the VS shortcut
 * (SSB64_VS_FKINDS) never goes through the CSS toggle. Unset or 0 leaves the flag as it is.
 */
static sb32 ftRemixSonicClassicIsDebugForced(s32 player)
{
    static const char *const names[] = { "SSB64_REMIX_CLASSIC_SONIC_P1", "SSB64_REMIX_CLASSIC_SONIC_P2", "SSB64_REMIX_CLASSIC_SONIC_P3", "SSB64_REMIX_CLASSIC_SONIC_P4" };

    return (remix_debug_env_int(names[player], 0) != 0) ? TRUE : FALSE;
}

/** TRUE for a Sonic (not Super Sonic or Polygon Sonic) of a port with the classic flag set. */
static sb32 ftRemixSonicClassicIsClassicSonic(const FTStruct *fp)
{
    if (fp->fkind != REMIX_ID_SONIC)
    {
        return FALSE;
    }
    return ftRemixSonicIsClassic(fp->player);
}

void* ftRemixSonicClassicGetMainFile(const FTData *data, s32 fkind, s32 player)
{
    if (ftRemixSonicClassicIsPlayerValid(player) == FALSE)
    {
        return *data->p_file_main;
    }
    sFTRemixSonicClassicSelectFrame[player] = 0.0F;

    if (fkind != REMIX_ID_SONIC)
    {
        ftRemixSonicSetClassic(player, FALSE);

        return *data->p_file_main;
    }
    if (ftRemixSonicClassicIsDebugForced(player) != FALSE)
    {
        ftRemixSonicSetClassic(player, TRUE);
    }
    if ((ftRemixSonicIsClassic(player) != FALSE) && (data->p_file_special2 != NULL) && (*data->p_file_special2 != NULL))
    {
        return *data->p_file_special2;
    }
    return *data->p_file_main;
}

s32 ftRemixSonicClassicPlayFgm(const FTStruct *fp, s32 fgm_id)
{
    if (ftRemixSonicClassicIsClassicSonic(fp) == FALSE)
    {
        return fgm_id;
    }
    return ftRemixSonicClassicRulesPlayFgm(fgm_id);
}

s32 ftRemixSonicClassicPlayLoopFgm(const FTStruct *fp, s32 fgm_id)
{
    if (ftRemixSonicClassicIsClassicSonic(fp) == FALSE)
    {
        return fgm_id;
    }
    return ftRemixSonicClassicRulesPlayLoopFgm(fgm_id);
}

sb32 ftRemixSonicClassicIsVoiceMuted(const FTStruct *fp)
{
    return ftRemixSonicClassicIsClassicSonic(fp);
}

s32 ftRemixSonicClassicShieldCostumeOffset(const FTStruct *fp)
{
    return (ftRemixSonicIsClassic(fp->player) != FALSE) ? FTREMIX_SONIC_CLASSIC_COSTUME_OFFSET : 0;
}

f32 ftRemixSonicClassicGetSelectFrame(s32 player)
{
    if (ftRemixSonicClassicIsPlayerValid(player) == FALSE)
    {
        return 0.0F;
    }
    return sFTRemixSonicClassicSelectFrame[player];
}

void ftRemixSonicClassicSetSelectFrame(s32 player, f32 frame)
{
    if (ftRemixSonicClassicIsPlayerValid(player) != FALSE)
    {
        sFTRemixSonicClassicSelectFrame[player] = frame;
    }
}

f32 ftRemixSonicClassicGetStatusFrame(const FTStruct *fp, s32 status_id)
{
    if ((fp->fkind != REMIX_ID_SONIC) || (status_id != nFTDemoStatusWin4))
    {
        return 0.0F;
    }
    return ftRemixSonicClassicGetSelectFrame(fp->player);
}

FTMotionDesc* ftRemixSonicClassicSwapMenuMotion(const FTStruct *fp, FTMotionDesc *motion_desc)
{
    s32 classic_file;

    if ((ftRemixSonicClassicIsClassicSonic(fp) == FALSE) || (ftRemixSonicClassicRulesIsMenuAnimScene(gSCManagerSceneData.scene_curr) == 0))
    {
        return motion_desc;
    }
    classic_file = ftRemixSonicClassicRulesMenuAnimFile(motion_desc->anim_file_id);

    if (classic_file == 0)
    {
        return motion_desc;
    }
    sFTRemixSonicClassicMenuMotion.anim_file_id = classic_file;
    sFTRemixSonicClassicMenuMotion.offset = FTREMIX_SONIC_NO_SCRIPT;
    sFTRemixSonicClassicMenuMotion.anim_desc.word = 0;

    return &sFTRemixSonicClassicMenuMotion;
}

#endif
