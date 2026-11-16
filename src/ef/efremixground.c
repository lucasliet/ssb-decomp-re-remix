/**
 * efremixground.c - ground actor data of the Remix stages (Stages.asm bg_animation_). See efremixground.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ef/efremixground.h"
#include "ef/efremixgroundrules.h"
#include <ef/efground.h>
#include <sys/objdisplay.h>
#include <port_log.h>

/* Same values in every stages/<stage>.bin sprite: flags 0x7, display link 4, transforms 0x1C / 0x1B, status -1. */
#define EFREMIXGROUND_SPRITE_FLAGS (0x4 | EFFECT_FLAG_USERDATA | 0x1)
#define EFREMIXGROUND_SPRITE_DL_LINK 4
#define EFREMIXGROUND_SPRITE_STATUS -1
/* Largest add_bg_animation stage: 7 sprite_info_arrays (Sector Z and Yoshi's Island variants), 6 sprites. */
#define EFREMIXGROUND_PARAMS_MAX 8
#define EFREMIXGROUND_SPRITES_MAX 8

extern EFGroundData dEFGroundDatas[];
extern EFGroundActor sEFGroundActor;

static EFGroundParam sEFRemixGroundParams[EFREMIXGROUND_PARAMS_MAX];
static EFGroundDesc sEFRemixGroundDescs[EFREMIXGROUND_SPRITES_MAX];
static EFGroundData sEFRemixGroundData;

/**
 * Fills the update and ground routines of a sprite desc from its .bin routine pair.
 *
 * @param desc Desc to fill.
 * @param proc Routine pair of the sprite.
 */
static void efRemixGroundSetSpriteProcs(EFGroundDesc *desc, EFRemixGroundSpriteProc proc)
{
    desc->proc_groundeffect = NULL;
    desc->effect_desc.proc_update = efGroundCommonProcUpdate;

    if (proc == nEFRemixGroundSpriteProcYaw)
    {
        desc->effect_desc.proc_update = efGroundUpdateEffectYaw;
    }
    else if (proc == nEFRemixGroundSpriteProcStep)
    {
        desc->proc_groundeffect = efGroundSetStepPositions;
        desc->effect_desc.proc_update = efGroundUpdateStepPositions;
    }
}

/**
 * Builds the EFGroundDesc of one sprite of a stage .bin.
 *
 * @param desc Desc to fill.
 * @param sprite Sprite row.
 */
static void efRemixGroundMakeDesc(EFGroundDesc *desc, const EFRemixGroundSprite *sprite)
{
    desc->alt_high = sprite->alt_high;
    desc->alt_low = sprite->alt_low;
    desc->pos_z = sprite->pos_z;
    desc->scale = sprite->scale;
    desc->effect_status = EFREMIXGROUND_SPRITE_STATUS;
    desc->effect_desc.flags = EFREMIXGROUND_SPRITE_FLAGS;
    desc->effect_desc.dl_link = EFREMIXGROUND_SPRITE_DL_LINK;
    desc->effect_desc.file_head = NULL;
    desc->effect_desc.transform_types1.tk1 = nGCMatrixKindTraRotRpyRSca;
    desc->effect_desc.transform_types1.tk2 = nGCMatrixKindNull;
    desc->effect_desc.transform_types1.tk3 = 0;
    desc->effect_desc.transform_types2.tk1 = nGCMatrixKindTraRotRpyR;
    desc->effect_desc.transform_types2.tk2 = nGCMatrixKindNull;
    desc->effect_desc.transform_types2.tk3 = 0;
    desc->effect_desc.proc_display = gcDrawDObjTreeForGObj;
    desc->effect_desc.o_dobjsetup = sprite->o_dobjsetup;
    desc->effect_desc.o_mobjsub = sprite->o_mobjsub;
    desc->effect_desc.o_anim_joint = sprite->o_anim_joint;
    desc->effect_desc.o_matanim_joint = sprite->o_matanim_joint;

    efRemixGroundSetSpriteProcs(desc, sprite->proc);
}

/**
 * Builds the EFGroundData of an add_bg_animation stage into the static Remix row.
 *
 * @param stage Stage row of efremixgroundrules.c.
 * @return The Remix row, or NULL when the stage has more params or sprites than the static arrays hold.
 */
static EFGroundData* efRemixGroundMakeData(const EFRemixGroundStage *stage)
{
    s32 i;

    if ((stage->params_num > EFREMIXGROUND_PARAMS_MAX) || (stage->sprites_num > EFREMIXGROUND_SPRITES_MAX))
    {
        port_log("[remix-ground] bg_info of stage 0x%X has %d params and %d sprites, at most %d and %d fit; no actor",
                 stage->gkind, stage->params_num, stage->sprites_num, EFREMIXGROUND_PARAMS_MAX, EFREMIXGROUND_SPRITES_MAX);
        return NULL;
    }
    for (i = 0; i < stage->params_num; i++)
    {
        sEFRemixGroundParams[i].effect_id = stage->params[i].effect_id;
        sEFRemixGroundParams[i].make_queue = stage->params[i].make_queue;
        sEFRemixGroundParams[i].lr = stage->params[i].lr;
        sEFRemixGroundParams[i].effect_weight = stage->params[i].effect_weight;
    }
    for (i = 0; i < stage->sprites_num; i++)
    {
        efRemixGroundMakeDesc(&sEFRemixGroundDescs[i], &stage->sprites[i]);
    }
    sEFRemixGroundData.params_num = stage->params_num;
    sEFRemixGroundData.effect_params = sEFRemixGroundParams;
    sEFRemixGroundData.o_data = stage->o_data;
    sEFRemixGroundData.effect_descs = sEFRemixGroundDescs;

    return &sEFRemixGroundData;
}

EFGroundData* efRemixGroundGetData(s32 gkind)
{
    const EFRemixGroundStage *stage;

    if (efRemixGroundRulesIsVanillaSlot(gkind))
    {
        return &dEFGroundDatas[gkind];
    }
    stage = efRemixGroundRulesFindStage(gkind);

    if (stage == NULL)
    {
        return NULL;
    }
    return efRemixGroundMakeData(stage);
}

s32 efRemixGroundGetActorMakeWait(void)
{
    return sEFGroundActor.make_wait;
}

#endif
