/**
 * ftremixentry.c - Smash Remix entry_script table (Character.entry_script, get_entry_script_ at 0x8013DC78).
 *
 * Every Remix character has an entry script: a pointer to one of the vanilla appear routines
 * (0x8013DC8C Mario pipe ... 0x8013DD14 Captain car, 0x8013DD68 = none) or to a Remix routine.
 * A clone copies its parent's script unless its own file patches it, and all polygon variants
 * skip it. The vanilla routines read their model from the vanilla fighter's file globals, so the
 * port points the effect at the entering fighter instead: the Remix patches that swap in a
 * custom effect struct are rows of dFTRemixEntryOverrides, and every other clone draws from the
 * file slot that holds the same file id as the vanilla effect's file.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixentry.h"
#include "ftremixhooks.h"
#include <ef/efmanager.h>
#include <port_log.h>
#include "fighter_registry.h"

typedef enum FTRemixEntryScript
{
    nFTRemixEntryScriptNone,
    nFTRemixEntryScriptMarioPipe,
    nFTRemixEntryScriptFoxArwing,
    nFTRemixEntryScriptDonkeyBarrel,
    nFTRemixEntryScriptSamusPoint,
    nFTRemixEntryScriptLink,
    nFTRemixEntryScriptLinkRotate,
    nFTRemixEntryScriptYoshiEgg,
    nFTRemixEntryScriptKirbyStar,
    nFTRemixEntryScriptMBall,
    nFTRemixEntryScriptCaptainCar

} FTRemixEntryScript;

extern EFDesc dEFManagerMarioEntryDokanEffectDesc;
extern EFDesc dEFManagerFoxEntryArwingEffectDesc;
extern EFDesc dEFManagerDonkeyEntryTaruEffectDesc;
extern EFDesc dEFManagerSamusEntryPointEffectDesc;
extern EFDesc dEFManagerLinkEntryWaveEffectDesc;
extern EFDesc dEFManagerLinkEntryBeamEffectDesc;
extern EFDesc dEFManagerYoshiEntryEggEffectDesc;
extern EFDesc dEFManagerKirbyEntryStarEffectDesc;
extern EFDesc dEFManagerCaptainEntryCarEffectDesc;

/* Effective entry_script of each synth fighter; ids not listed skip the entry script. */
static const u8 dFTRemixEntryScripts[REMIX_ID_NCRASH + 1] =
{
    [REMIX_ID_FALCO]      = nFTRemixEntryScriptFoxArwing,
    [REMIX_ID_YLINK]      = nFTRemixEntryScriptLink,
    [REMIX_ID_DRM]        = nFTRemixEntryScriptMarioPipe,
    [REMIX_ID_DSAMUS]     = nFTRemixEntryScriptSamusPoint,
    [REMIX_ID_ELINK]      = nFTRemixEntryScriptLink,
    [REMIX_ID_JSAMUS]     = nFTRemixEntryScriptSamusPoint,
    [REMIX_ID_JLINK]      = nFTRemixEntryScriptLink,
    [REMIX_ID_JFALCON]    = nFTRemixEntryScriptCaptainCar,
    [REMIX_ID_JFOX]       = nFTRemixEntryScriptFoxArwing,
    [REMIX_ID_JMARIO]     = nFTRemixEntryScriptMarioPipe,
    [REMIX_ID_JLUIGI]     = nFTRemixEntryScriptMarioPipe,
    [REMIX_ID_JDK]        = nFTRemixEntryScriptDonkeyBarrel,
    [REMIX_ID_EPIKA]      = nFTRemixEntryScriptMBall,
    [REMIX_ID_JPUFF]      = nFTRemixEntryScriptMBall,
    [REMIX_ID_EPUFF]      = nFTRemixEntryScriptMBall,
    [REMIX_ID_JKIRBY]     = nFTRemixEntryScriptKirbyStar,
    [REMIX_ID_JYOSHI]     = nFTRemixEntryScriptYoshiEgg,
    [REMIX_ID_JPIKA]      = nFTRemixEntryScriptMBall,
    [REMIX_ID_ESAMUS]     = nFTRemixEntryScriptSamusPoint,
    [REMIX_ID_BOWSER]     = nFTRemixEntryScriptCaptainCar,
    [REMIX_ID_WOLF]       = nFTRemixEntryScriptFoxArwing,
    [REMIX_ID_CONKER]     = nFTRemixEntryScriptCaptainCar,
    [REMIX_ID_MARTH]      = nFTRemixEntryScriptLinkRotate,
    [REMIX_ID_SONIC]      = nFTRemixEntryScriptDonkeyBarrel,
    [REMIX_ID_SANDBAG]    = nFTRemixEntryScriptCaptainCar,
    [REMIX_ID_MARINA]     = nFTRemixEntryScriptLinkRotate,
    [REMIX_ID_DEDEDE]     = nFTRemixEntryScriptKirbyStar,
    [REMIX_ID_GOEMON]     = nFTRemixEntryScriptLinkRotate,
    [REMIX_ID_PEPPY]      = nFTRemixEntryScriptFoxArwing,
    [REMIX_ID_SLIPPY]     = nFTRemixEntryScriptFoxArwing,
    [REMIX_ID_BANJO]      = nFTRemixEntryScriptLink,
    [REMIX_ID_MLUIGI]     = nFTRemixEntryScriptMarioPipe,
    [REMIX_ID_EBI]        = nFTRemixEntryScriptLinkRotate,
    [REMIX_ID_CRASH]      = nFTRemixEntryScriptSamusPoint,
    [REMIX_ID_ROY]        = nFTRemixEntryScriptLinkRotate,
    [REMIX_ID_DRL]        = nFTRemixEntryScriptMarioPipe,
    [REMIX_ID_LANKY]      = nFTRemixEntryScriptDonkeyBarrel
};

/* Remix struct replacements of the vanilla effect descs, one row per fighter and effect. */
extern const FTRemixEntryOverride dFTRemixEntryOverrides[];
extern const s32 dFTRemixEntryOverridesNum;

extern const FTRemixEntryCarAnim dFTRemixEntryCarAnims[];
extern const s32 dFTRemixEntryCarAnimsNum;

extern const FTRemixEntryArwingAnim dFTRemixEntryArwingAnims[];
extern const s32 dFTRemixEntryArwingAnimsNum;

/* Fighter whose car animation the running efManagerCaptainEntryCarMakeEffect call reads, NULL outside it. */
static const FTStruct *sFTRemixEntryCarFighter;

/* Fighter whose Arwing animation the running efManagerFoxEntryArwingMakeEffect call reads, NULL outside it. */
static const FTStruct *sFTRemixEntryArwingFighter;

static void **ftRemixEntryGetFile(const FTData *data, s32 file_n)
{
    switch (file_n)
    {
    case 1: return data->p_file_main;
    case 2: return data->p_file_mainmotion;
    case 3: return data->p_file_submotion;
    case 4: return data->p_file_model;
    case 5: return data->p_file_shieldpose;
    case 6: return data->p_file_special1;
    case 7: return data->p_file_special2;
    case 8: return data->p_file_special3;
    case 9: return data->p_file_special4;
    default: return NULL;
    }
}

static u32 ftRemixEntryGetFileID(const FTData *data, s32 file_n)
{
    switch (file_n)
    {
    case 6: return data->file_special1_id;
    case 7: return data->file_special2_id;
    case 8: return data->file_special3_id;
    case 9: return data->file_special4_id;
    default: return 0;
    }
}

/* File slot (6..9) of `vanilla_fkind` that the vanilla effect desc points at, 0 when none does. */
static s32 ftRemixEntryFindVanillaSlot(s32 vanilla_fkind, void **vanilla_file)
{
    const FTData *vanilla = port_fighter_data(vanilla_fkind);
    s32 file_n;

    for (file_n = 6; file_n <= 9; file_n++)
    {
        if (ftRemixEntryGetFile(vanilla, file_n) == vanilla_file)
        {
            return file_n;
        }
    }
    return 0;
}

/* Clone file slot that holds the vanilla effect's file id; Remix gives a clone the parent's file ids
 * unless the effect is customised. */
static void **ftRemixEntryFindOwnerFile(const FTStruct *fp, s32 vanilla_fkind, void **vanilla_file)
{
    s32 vanilla_n = ftRemixEntryFindVanillaSlot(vanilla_fkind, vanilla_file);
    u32 file_id;
    s32 file_n;

    if (vanilla_n == 0)
    {
        return NULL;
    }
    file_id = ftRemixEntryGetFileID(port_fighter_data(vanilla_fkind), vanilla_n);

    for (file_n = 6; file_n <= 9; file_n++)
    {
        if (ftRemixEntryGetFileID(fp->data, file_n) == file_id)
        {
            return ftRemixEntryGetFile(fp->data, file_n);
        }
    }
    return NULL;
}

static EFDesc *ftRemixEntryGetVanillaDesc(FTRemixEntryEffect effect)
{
    switch (effect)
    {
    case nFTRemixEntryEffectMarioPipe: return &dEFManagerMarioEntryDokanEffectDesc;
    case nFTRemixEntryEffectFoxArwing: return &dEFManagerFoxEntryArwingEffectDesc;
    case nFTRemixEntryEffectDonkeyBarrel:
    case nFTRemixEntryEffectDonkeyBarrelRight: return &dEFManagerDonkeyEntryTaruEffectDesc;
    case nFTRemixEntryEffectSamusPoint: return &dEFManagerSamusEntryPointEffectDesc;
    case nFTRemixEntryEffectLinkWave: return &dEFManagerLinkEntryWaveEffectDesc;
    case nFTRemixEntryEffectLinkBeam: return &dEFManagerLinkEntryBeamEffectDesc;
    case nFTRemixEntryEffectYoshiEgg: return &dEFManagerYoshiEntryEggEffectDesc;
    case nFTRemixEntryEffectKirbyStar: return &dEFManagerKirbyEntryStarEffectDesc;
    case nFTRemixEntryEffectCaptainCar: return &dEFManagerCaptainEntryCarEffectDesc;
    default: return NULL;
    }
}

static const FTRemixEntryOverride *ftRemixEntryFindOverride(s32 fkind, FTRemixEntryEffect effect)
{
    s32 i;

    for (i = 0; i < dFTRemixEntryOverridesNum; i++)
    {
        if ((dFTRemixEntryOverrides[i].fkind == fkind) && (dFTRemixEntryOverrides[i].effect == effect))
        {
            return &dFTRemixEntryOverrides[i];
        }
    }
    return NULL;
}

/*
 * Points `desc` at the entering fighter's own file (and effect struct offsets) and returns the
 * vanilla values in `saved`. Returns FALSE when the Remix effect is absent or not ported, in which
 * case no effect must be made.
 */
static sb32 ftRemixEntryBeginEffect(FTStruct *fp, EFDesc *desc, s32 vanilla_fkind, FTRemixEntryEffect effect,
                                    EFDesc *saved)
{
    const FTRemixEntryOverride *row = ftRemixEntryFindOverride(fp->fkind, effect);
    void **file;

    *saved = *desc;

    if (row != NULL)
    {
        if (row->file_n == FTREMIX_ENTRY_SKIP)
        {
            return FALSE;
        }
        file = ftRemixEntryGetFile(fp->data, row->file_n);

        if (file == NULL)
        {
            port_log("SSB64: ftRemixEntry: id 0x%02X effect %d: file_%d is not loaded, no effect\n", fp->fkind, effect, row->file_n);
            return FALSE;
        }
        if (row->base_effect != FTREMIX_ENTRY_SAME_BASE)
        {
            *desc = *ftRemixEntryGetVanillaDesc(row->base_effect);
        }
        desc->file_head = file;

        if (row->o_dobjsetup != FTREMIX_ENTRY_KEEP)
        {
            desc->o_dobjsetup = row->o_dobjsetup;
        }
        if (row->o_mobjsub != FTREMIX_ENTRY_KEEP)
        {
            desc->o_mobjsub = row->o_mobjsub;
        }
        if (row->o_anim_joint != FTREMIX_ENTRY_KEEP)
        {
            desc->o_anim_joint = row->o_anim_joint;
        }
        if (row->o_matanim_joint != FTREMIX_ENTRY_KEEP)
        {
            desc->o_matanim_joint = row->o_matanim_joint;
        }
        return TRUE;
    }
    file = ftRemixEntryFindOwnerFile(fp, vanilla_fkind, desc->file_head);

    if (file == NULL)
    {
        port_log("SSB64: ftRemixEntry: id 0x%02X effect %d: no file with the vanilla effect's id and no Remix struct ported, no effect\n",
                 fp->fkind, effect);
        return FALSE;
    }
    desc->file_head = file;

    return TRUE;
}

static void ftRemixEntryEndEffect(EFDesc *desc, const EFDesc *saved)
{
    *desc = *saved;
}

void ftRemixEntryGetCarAnim(void **car_file, intptr_t *o_car_anim, intptr_t *o_wheel_anim_l, intptr_t *o_wheel_anim_r)
{
    s32 i;

    if (sFTRemixEntryCarFighter == NULL)
    {
        return;
    }
    for (i = 0; i < dFTRemixEntryCarAnimsNum; i++)
    {
        const FTRemixEntryCarAnim *row = &dFTRemixEntryCarAnims[i];
        void **file;

        if (row->fkind != sFTRemixEntryCarFighter->fkind)
        {
            continue;
        }
        file = ftRemixEntryGetFile(sFTRemixEntryCarFighter->data, row->file_n);

        if (file != NULL)
        {
            *car_file = *file;
        }
        if (row->o_car_anim != FTREMIX_ENTRY_KEEP)
        {
            *o_car_anim = row->o_car_anim;
            *o_wheel_anim_l = row->o_wheel_anim_l;
            *o_wheel_anim_r = row->o_wheel_anim_r;
        }
        return;
    }
}

void ftRemixEntryGetArwingAnim(void **wing_file, intptr_t *o_wing_anim, void **body_file, intptr_t *o_body_anim_r, intptr_t *o_body_anim_l)
{
    s32 i;

    if (sFTRemixEntryArwingFighter == NULL)
    {
        return;
    }
    for (i = 0; i < dFTRemixEntryArwingAnimsNum; i++)
    {
        const FTRemixEntryArwingAnim *row = &dFTRemixEntryArwingAnims[i];
        void **wing;
        void **body;

        if (row->fkind != sFTRemixEntryArwingFighter->fkind)
        {
            continue;
        }
        wing = ftRemixEntryGetFile(sFTRemixEntryArwingFighter->data, row->wing_file_n);
        body = ftRemixEntryGetFile(sFTRemixEntryArwingFighter->data, row->body_file_n);

        if ((wing == NULL) || (body == NULL))
        {
            port_log("SSB64: ftRemixEntry: id 0x%02X Arwing animation files are not loaded, keeping the vanilla ones\n", row->fkind);
            return;
        }
        *wing_file = *wing;
        *o_wing_anim = row->o_wing_anim;
        *body_file = *body;
        *o_body_anim_r = row->o_body_anim_r;
        *o_body_anim_l = row->o_body_anim_l;
        return;
    }
}

static void ftRemixEntryMakeLinkEffects(FTStruct *fp)
{
    EFDesc saved_wave;
    EFDesc saved_beam;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerLinkEntryWaveEffectDesc, nFTKindLink, nFTRemixEntryEffectLinkWave, &saved_wave) != FALSE)
    {
        efManagerLinkEntryWaveMakeEffect(&fp->entry_pos);
    }
    ftRemixEntryEndEffect(&dEFManagerLinkEntryWaveEffectDesc, &saved_wave);

    if (ftRemixEntryBeginEffect(fp, &dEFManagerLinkEntryBeamEffectDesc, nFTKindLink, nFTRemixEntryEffectLinkBeam, &saved_beam) != FALSE)
    {
        efManagerLinkEntryBeamMakeEffect(&fp->entry_pos);
    }
    ftRemixEntryEndEffect(&dEFManagerLinkEntryBeamEffectDesc, &saved_beam);
}

static void ftRemixEntryMakeMarioPipe(FTStruct *fp)
{
    EFDesc saved;
    s32 vanilla_fkind = (port_fighter_parent_fkind(fp->fkind) == nFTKindLuigi) ? nFTKindLuigi : nFTKindMario;

    dEFManagerMarioEntryDokanEffectDesc.file_head = (vanilla_fkind == nFTKindLuigi) ? &gFTDataLuigiSpecial2 : &gFTMarioFileSpecial2;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerMarioEntryDokanEffectDesc, vanilla_fkind, nFTRemixEntryEffectMarioPipe, &saved) != FALSE)
    {
        efManagerMarioEntryDokanMakeEffect(&fp->entry_pos, fp->fkind);
    }
    ftRemixEntryEndEffect(&dEFManagerMarioEntryDokanEffectDesc, &saved);
}

static void ftRemixEntryMakeFoxArwing(FTStruct *fp)
{
    EFDesc saved;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerFoxEntryArwingEffectDesc, nFTKindFox, nFTRemixEntryEffectFoxArwing, &saved) != FALSE)
    {
        GObj *arwing_gobj;

        sFTRemixEntryArwingFighter = fp;
        arwing_gobj = efManagerFoxEntryArwingMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        sFTRemixEntryArwingFighter = NULL;
        ftRemixSizeEntryArwingBind(arwing_gobj, fp->fighter_gobj);
    }
    ftRemixEntryEndEffect(&dEFManagerFoxEntryArwingEffectDesc, &saved);
}

static void ftRemixEntryMakeDonkeyBarrel(FTStruct *fp)
{
    EFDesc saved;
    FTRemixEntryEffect effect = ((fp->fkind == REMIX_ID_SONIC) && (fp->status_vars.common.entry.lr == +1)) ?
                                 nFTRemixEntryEffectDonkeyBarrelRight : nFTRemixEntryEffectDonkeyBarrel;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerDonkeyEntryTaruEffectDesc, nFTKindDonkey, effect, &saved) != FALSE)
    {
        efManagerDonkeyEntryTaruMakeEffect(&fp->entry_pos);
    }
    ftRemixEntryEndEffect(&dEFManagerDonkeyEntryTaruEffectDesc, &saved);
}

static void ftRemixEntryMakeSamusPoint(FTStruct *fp)
{
    EFDesc saved;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerSamusEntryPointEffectDesc, nFTKindSamus, nFTRemixEntryEffectSamusPoint, &saved) != FALSE)
    {
        efManagerSamusEntryPointMakeEffect(&fp->entry_pos);
    }
    ftRemixEntryEndEffect(&dEFManagerSamusEntryPointEffectDesc, &saved);
}

static void ftRemixEntryMakeYoshiEgg(FTStruct *fp)
{
    EFDesc saved;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerYoshiEntryEggEffectDesc, nFTKindYoshi, nFTRemixEntryEffectYoshiEgg, &saved) != FALSE)
    {
        ftRemixSizeScaleEntryEgg(efManagerYoshiEntryEggMakeEffect(&fp->entry_pos), fp);
    }
    ftRemixEntryEndEffect(&dEFManagerYoshiEntryEggEffectDesc, &saved);
}

static void ftRemixEntryMakeKirbyStar(FTStruct *fp)
{
    EFDesc saved;

    if (ftRemixEntryBeginEffect(fp, &dEFManagerKirbyEntryStarEffectDesc, nFTKindKirby, nFTRemixEntryEffectKirbyStar, &saved) != FALSE)
    {
        efManagerKirbyEntryStarMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
    }
    ftRemixEntryEndEffect(&dEFManagerKirbyEntryStarEffectDesc, &saved);
}

static void ftRemixEntryMakeCaptainCar(FTStruct *fp)
{
    EFDesc saved;

    if (fp->status_vars.common.entry.lr == -1)
    {
        fp->status_vars.common.entry.is_rotate = TRUE;
    }
    if (ftRemixEntryBeginEffect(fp, &dEFManagerCaptainEntryCarEffectDesc, nFTKindCaptain, nFTRemixEntryEffectCaptainCar, &saved) != FALSE)
    {
        sFTRemixEntryCarFighter = fp;
        efManagerCaptainEntryCarMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        sFTRemixEntryCarFighter = NULL;
    }
    ftRemixEntryEndEffect(&dEFManagerCaptainEntryCarEffectDesc, &saved);
}

/* Entry hook of every synth fighter: runs the entry script the Remix tables give its id. */
/* REMIX-TODO(Size): Size.asm entry gfx_ scope not ported. The top joint z offset and scale of the effects built by
 * efManagerMarioEntryDokan, DonkeyEntryTaru, SamusEntryPoint, LinkEntryWave, LinkEntryBeam, KirbyEntryStar and CaptainEntryCar
 * (0x80103768, 0x8010345C, 0x801034B8, 0x80102B28, 0x80102B84, 0x80102C10, 0x80103670) keep their vanilla scale. */
void ftRemixEntryMakeEffect(FTStruct *fp)
{
    if (fp->fkind >= ARRAY_COUNT(dFTRemixEntryScripts))
    {
        return;
    }
    switch (dFTRemixEntryScripts[fp->fkind])
    {
    case nFTRemixEntryScriptMarioPipe:
        ftRemixEntryMakeMarioPipe(fp);
        break;

    case nFTRemixEntryScriptFoxArwing:
        ftRemixEntryMakeFoxArwing(fp);
        break;

    case nFTRemixEntryScriptDonkeyBarrel:
        ftRemixEntryMakeDonkeyBarrel(fp);
        break;

    case nFTRemixEntryScriptSamusPoint:
        ftRemixEntryMakeSamusPoint(fp);
        break;

    case nFTRemixEntryScriptLinkRotate:
        if (fp->status_vars.common.entry.lr == -1)
        {
            fp->status_vars.common.entry.is_rotate = TRUE;
        }
        ftRemixEntryMakeLinkEffects(fp);
        break;

    case nFTRemixEntryScriptLink:
        ftRemixEntryMakeLinkEffects(fp);
        break;

    case nFTRemixEntryScriptYoshiEgg:
        ftRemixEntryMakeYoshiEgg(fp);
        break;

    case nFTRemixEntryScriptKirbyStar:
        ftRemixEntryMakeKirbyStar(fp);
        break;

    case nFTRemixEntryScriptMBall:
        efManagerMBallThrownMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        break;

    case nFTRemixEntryScriptCaptainCar:
        ftRemixEntryMakeCaptainCar(fp);
        break;

    default:
        break;
    }
}

#endif
