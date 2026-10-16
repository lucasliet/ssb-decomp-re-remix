/**
 * ftremixcommand.c - Smash Remix custom moveset commands 0xD0-0xFF
 * (.toolchain/pkgs/smashremix-src/src/command.asm). See ftremixcommand.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixenvcolor.h"
#include "ftremixcommand.h"

#include <sys/utils.h>
#include <lb/lbcommon.h>
#include "remix/remix_ram_image.h"

extern void *func_800269C0_275C0(u16 id);

#define FTREMIX_COMMAND_FIRST 0xD0
#define FTREMIX_COMMAND_ATTACKS_NUM 4
#define FTREMIX_COMMAND_FGM_NONE 0xFFFF
#define FTREMIX_COMMAND_FGM_PLAY_BOTH 0x8000
#define FTREMIX_COMMAND_DIRECTION_FORWARD 1
#define FTREMIX_COMMAND_DIRECTION_BACKWARD 2
#define FTREMIX_COMMAND_SFX_TYPE_VOICE 1
#define FTREMIX_COMMAND_ONE_ULP_ABOVE_ONE 0x3F800001u

typedef enum FTRemixCommandKind
{
    nFTRemixCommandFrameSpeed = 0xD0,
    nFTRemixCommandArmour,
    nFTRemixCommandHitboxDirection,
    nFTRemixCommandTranslationMultiplier,
    nFTRemixCommandVelAirY,
    nFTRemixCommandFastFall,
    nFTRemixCommandRandomSFX,
    nFTRemixCommandSetKinetics,
    nFTRemixCommandHitboxFGM,
    nFTRemixCommandEnvColor,
    nFTRemixCommandSwitchDirection,
    nFTRemixCommandGotoMovesetFile,
    nFTRemixCommandTauntVoice

} FTRemixCommandKind;

typedef union FTRemixCommandFloat
{
    u32 word;
    f32 f;

} FTRemixCommandFloat;

/* apply_direction_.hitbox_dir_table, apply_fgm_.hitbox_fgm_table,
 * translation_multiplier_.multiplier_table and CharEnvColor.moveset_table. */
static s32 sFTRemixCommandHitboxDirections[MAXCONTROLLERS][FTREMIX_COMMAND_ATTACKS_NUM];
static u16 sFTRemixCommandHitboxFGMs[MAXCONTROLLERS][FTREMIX_COMMAND_ATTACKS_NUM] =
{
    { FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE },
    { FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE },
    { FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE },
    { FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE, FTREMIX_COMMAND_FGM_NONE }
};
static f32 sFTRemixCommandTranslationMultipliers[MAXCONTROLLERS] = { 1.0F, 1.0F, 1.0F, 1.0F };
static u32 sFTRemixCommandEnvColors[MAXCONTROLLERS];

static sb32 ftRemixCommandCheckPlayer(s32 player)
{
    return (player >= 0) && (player < MAXCONTROLLERS);
}

/* Upper halfword of a command word as a float (XXXX0000). */
static f32 ftRemixCommandGetFloat(u32 word)
{
    FTRemixCommandFloat value;

    value.word = word << 16;

    return value.f;
}

/* Halfword at Remix RAM address `addr`; the image holds native u32 words. */
static u16 ftRemixCommandReadHalf(u32 addr)
{
    u32 *word = remix_ram_image_resolve(addr & ~3u);

    if (word == NULL)
    {
        return FTREMIX_COMMAND_FGM_NONE;
    }
    return (addr & 2) ? (*word & 0xFFFF) : (*word >> 16);
}

/* fsm_ (0xD0): with flag YY set, every joint but TopN takes the speed, so
 * the script execution speed (TopN's) is left alone. */
static void ftRemixCommandSetFrameSpeed(GObj *fighter_gobj, u32 word)
{
    DObj *topn = DObjGetStruct(fighter_gobj);
    FTRemixCommandFloat speed;
    DObj *dobj;

    speed.f = ftRemixCommandGetFloat(word);

    if (((word >> 16) & 0xFF) == 0)
    {
        gcSetAnimSpeed(fighter_gobj, speed.f);
        return;
    }
    /* ftMainSetStatus resets the speed only when TopN's differs from 1.0. */
    if (*(u32*)&topn->anim_speed == 0x3F800000u)
    {
        *(u32*)&topn->anim_speed = FTREMIX_COMMAND_ONE_ULP_ABOVE_ONE;
    }
    for (dobj = topn->child; dobj != NULL; dobj = gcGetTreeDObjNext(dobj))
    {
        dobj->anim_speed = speed.f;
    }
}

/* fast_fall_ (0xD5): clearing rewrites the whole byte with `andi 0x07`. */
static void ftRemixCommandSetFastFall(FTStruct *fp, u32 word)
{
    if ((word & 0xFF) != 0)
    {
        fp->is_fastfall = TRUE;
        return;
    }
    fp->is_absorb = FALSE;
    fp->absorb_lr = 0;
    fp->is_goto_attack100 = FALSE;
    fp->is_fastfall = FALSE;
}

/* play_sfx_random_ (0xD6): CCXXYYZZ AAAAAAAA. */
static void ftRemixCommandPlayRandomSFX(FTStruct *fp, const u32 *script)
{
    u32 word = script[0];
    u16 fgm_id;

    if (fp->is_muted)
    {
        return;
    }
    if ((u32)syUtilsRandIntRange(100) >= ((word >> 16) & 0xFF))
    {
        return;
    }
    fgm_id = ftRemixCommandReadHalf(script[1] + (syUtilsRandIntRange(word & 0xFF) * 2));

    /* 0xFFFF marks "no sound" in Remix's arrays. */
    if (fgm_id == FTREMIX_COMMAND_FGM_NONE)
    {
        return;
    }
    if (((word >> 8) & 0xFF) == FTREMIX_COMMAND_SFX_TYPE_VOICE)
    {
        ftParamPlayVoice(fp, fgm_id);
    }
    else func_800269C0_275C0(fgm_id);
}

/* L_voice_sfx (0xDC): CC00AAAA 0000BBBB, BBBB when the taunt button is held. */
static void ftRemixCommandPlayTauntVoice(FTStruct *fp, const u32 *script)
{
    u16 fgm_id = script[0] & 0xFFFF;

    if (fp->input.pl.button_hold & fp->input.button_mask_l)
    {
        fgm_id = script[1] & 0xFFFF;
    }
    ftParamPlayVoice(fp, fgm_id);
}

/* set_kinetic_ (0xD7) */
static void ftRemixCommandSetKinetics(FTStruct *fp, u32 word)
{
    if ((word & 0xFF) != 0)
    {
        fp->jumps_used = 1;
        fp->ga = nMPKineticsAir;
    }
    else
    {
        fp->jumps_used = 0;
        fp->ga = nMPKineticsGround;
    }
}

/* hitbox_direction_ (0xD2): CC00XXYY, XX = hitbox id, YY = direction. */
static void ftRemixCommandSetHitboxDirection(FTStruct *fp, u32 word)
{
    u32 attack_id = (word >> 8) & 0xFF;

    if (attack_id < FTREMIX_COMMAND_ATTACKS_NUM)
    {
        sFTRemixCommandHitboxDirections[fp->player][attack_id] = word & 0xFF;
    }
}

/* set_hitbox_fgm_ (0xD8): CCXYZZZZ, X = all hitboxes, Y = hitbox id. */
static void ftRemixCommandSetHitboxFGM(FTStruct *fp, u32 word)
{
    u16 *fgms = sFTRemixCommandHitboxFGMs[fp->player];
    u32 flags = (word >> 16) & 0xFF;
    u16 fgm_id = word & 0xFFFF;
    s32 i;

    if ((flags >> 4) != 0)
    {
        for (i = 0; i < FTREMIX_COMMAND_ATTACKS_NUM; i++)
        {
            fgms[i] = fgm_id;
        }
    }
    else if (flags < FTREMIX_COMMAND_ATTACKS_NUM)
    {
        fgms[flags] = fgm_id;
    }
}

/* jump_to_moveset_file_ (0xDB): CC00XXXX, offset into the moveset file. */
static void ftRemixCommandGotoMovesetFile(FTStruct *fp, FTMotionScript *ms)
{
    u32 offset = ms->p_script[0] & 0xFFFF;

    if ((fp->data->p_file_mainmotion == NULL) || (*fp->data->p_file_mainmotion == NULL))
    {
        ms->p_script = NULL;
        return;
    }
    ms->p_script = (u32*)((uintptr_t)*fp->data->p_file_mainmotion + offset);
}

/* Commands 0xD4, 0xD5 and 0xDA are skipped by load_command_2_; 0xD6, 0xD9
 * and 0xDC are skipped there as 8-byte commands. */
static sb32 ftRemixCommandCheckForwardSkip(u32 kind, s32 *length)
{
    switch (kind)
    {
    case nFTRemixCommandVelAirY:
    case nFTRemixCommandFastFall:
    case nFTRemixCommandSwitchDirection:
        *length = 4;
        return TRUE;

    case nFTRemixCommandRandomSFX:
    case nFTRemixCommandEnvColor:
    case nFTRemixCommandTauntVoice:
        *length = 8;
        return TRUE;

    default:
        return FALSE;
    }
}

/* Runs command `kind` and returns its length in bytes (0 = script moved). */
static s32 ftRemixCommandRun(GObj *fighter_gobj, FTStruct *fp, FTMotionScript *ms, u32 kind)
{
    const u32 *script = ms->p_script;

    switch (kind)
    {
    case nFTRemixCommandFrameSpeed:
        ftRemixCommandSetFrameSpeed(fighter_gobj, script[0]);
        return 4;

    case nFTRemixCommandArmour:
        fp->knockback_resist_status = ftRemixCommandGetFloat(script[0]);
        return 4;

    case nFTRemixCommandHitboxDirection:
        ftRemixCommandSetHitboxDirection(fp, script[0]);
        return 4;

    case nFTRemixCommandTranslationMultiplier:
        sFTRemixCommandTranslationMultipliers[fp->player] = ftRemixCommandGetFloat(script[0]);
        return 4;

    case nFTRemixCommandVelAirY:
        fp->physics.vel_air.y = ftRemixCommandGetFloat(script[0]);
        return 4;

    case nFTRemixCommandFastFall:
        ftRemixCommandSetFastFall(fp, script[0]);
        return 4;

    case nFTRemixCommandRandomSFX:
        ftRemixCommandPlayRandomSFX(fp, script);
        return 8;

    case nFTRemixCommandSetKinetics:
        ftRemixCommandSetKinetics(fp, script[0]);
        return 4;

    case nFTRemixCommandHitboxFGM:
        ftRemixCommandSetHitboxFGM(fp, script[0]);
        return 4;

    case nFTRemixCommandEnvColor:
        sFTRemixCommandEnvColors[fp->player] = script[1];
        ftRemixEnvColorSetMoveset(fp->player, script[1]);
        return 8;

    case nFTRemixCommandSwitchDirection:
        fp->lr = -fp->lr;
        return 4;

    case nFTRemixCommandGotoMovesetFile:
        ftRemixCommandGotoMovesetFile(fp, ms);
        return 0;

    case nFTRemixCommandTauntVoice:
        ftRemixCommandPlayTauntVoice(fp, script);
        return 8;

    default:
        /* Unassigned table slots (OS.NULL) still advance one word. */
        return 4;
    }
}

sb32 ftRemixCommandCheckRun(GObj *fighter_gobj, FTStruct *fp, FTMotionScript *ms, sb32 is_forward)
{
    u32 kind = ms->p_script[0] >> 24;
    s32 length;

    if ((kind < FTREMIX_COMMAND_FIRST) || !ftRemixCommandCheckPlayer(fp->player))
    {
        return FALSE;
    }
    if (!is_forward || !ftRemixCommandCheckForwardSkip(kind, &length))
    {
        length = ftRemixCommandRun(fighter_gobj, fp, ms, kind);
    }
    if (ms->p_script != NULL)
    {
        ms->p_script = (u32*)((uintptr_t)ms->p_script + length);
    }
    return TRUE;
}

void ftRemixCommandResetStatus(FTStruct *fp)
{
    if (!ftRemixCommandCheckPlayer(fp->player))
    {
        return;
    }
    sFTRemixCommandTranslationMultipliers[fp->player] = 1.0F;
    sFTRemixCommandEnvColors[fp->player] = 0;
    ftRemixEnvColorSetMoveset(fp->player, 0);
}

void ftRemixCommandResetAttack(s32 player, s32 attack_id)
{
    if (!ftRemixCommandCheckPlayer(player) || (attack_id < 0) || (attack_id >= FTREMIX_COMMAND_ATTACKS_NUM))
    {
        return;
    }
    sFTRemixCommandHitboxDirections[player][attack_id] = 0;
    sFTRemixCommandHitboxFGMs[player][attack_id] = FTREMIX_COMMAND_FGM_NONE;
}

void ftRemixCommandResetAttackAll(FTStruct *fp)
{
    s32 i;

    for (i = 0; i < FTREMIX_COMMAND_ATTACKS_NUM; i++)
    {
        ftRemixCommandResetAttack(fp->player, i);
    }
}

static s32 ftRemixCommandGetAttackID(FTStruct *fp, FTAttackColl *attack_coll)
{
    s32 attack_id = attack_coll - fp->attack_colls;

    if (!ftRemixCommandCheckPlayer(fp->player) || (attack_id < 0) || (attack_id >= FTREMIX_COMMAND_ATTACKS_NUM))
    {
        return -1;
    }
    return attack_id;
}

void ftRemixCommandApplyDamageDirection(FTStruct *this_fp, FTStruct *attacker_fp, FTAttackColl *attack_coll)
{
    s32 attack_id = ftRemixCommandGetAttackID(attacker_fp, attack_coll);
    s32 direction;

    if (attack_id < 0)
    {
        return;
    }
    direction = sFTRemixCommandHitboxDirections[attacker_fp->player][attack_id];

    if (direction == FTREMIX_COMMAND_DIRECTION_BACKWARD)
    {
        this_fp->damage_lr = attacker_fp->lr;
    }
    else if (direction == FTREMIX_COMMAND_DIRECTION_FORWARD)
    {
        this_fp->damage_lr = -attacker_fp->lr;
    }
}

void ftRemixCommandPlayHitFGM(FTStruct *fp, FTAttackColl *attack_coll, u16 fgm_id, f32 pos_x)
{
    s32 attack_id = ftRemixCommandGetAttackID(fp, attack_coll);
    u16 override_id;

    if (attack_id < 0)
    {
        lbCommonMakePositionFGM(fgm_id, pos_x);
        return;
    }
    override_id = sFTRemixCommandHitboxFGMs[fp->player][attack_id];

    if (override_id == FTREMIX_COMMAND_FGM_NONE)
    {
        lbCommonMakePositionFGM(fgm_id, pos_x);
    }
    else if (override_id & FTREMIX_COMMAND_FGM_PLAY_BOTH)
    {
        lbCommonMakePositionFGM(fgm_id, pos_x);
        lbCommonMakePositionFGM(override_id & ~FTREMIX_COMMAND_FGM_PLAY_BOTH, pos_x);
    }
    else lbCommonMakePositionFGM(override_id, pos_x);
}

f32 ftRemixCommandGetTranslationMultiplier(FTStruct *fp)
{
    if (!ftRemixCommandCheckPlayer(fp->player))
    {
        return 1.0F;
    }
    return sFTRemixCommandTranslationMultipliers[fp->player];
}

u32 ftRemixCommandGetEnvColor(s32 player)
{
    if (!ftRemixCommandCheckPlayer(player))
    {
        return 0;
    }
    return sFTRemixCommandEnvColors[player];
}

#endif
