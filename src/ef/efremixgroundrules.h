#ifndef _EFREMIXGROUNDRULES_H_
#define _EFREMIXGROUNDRULES_H_

#include <stdint.h>

/*
 * Background sprite data of the Remix stages (Stages.asm bg_animation_, add_bg_animation and the stages/<stage>.asm bg_info
 * files): which stages run the vanilla ground actor (efGroundMakeAppearActor) and with which sprite paths and sprites.
 * Plain ints and floats, so the headless tests can read the tables; efremixground.c turns a row into EFGroundData.
 */

/* Stages.asm bg_animation_ table: the first 8 slots point at the vanilla dEFGroundDatas (0x8012F840 + n * 0x10). */
#define EFREMIXGROUND_VANILLA_SLOTS 8

/* Update routine pair of one sprite (EFGroundDesc proc_groundeffect and EFDesc proc_update in the stage .bin). */
typedef enum EFRemixGroundSpriteProc
{
    nEFRemixGroundSpriteProcCommon, /* efGroundCommonProcUpdate (0x80115F5C) */
    nEFRemixGroundSpriteProcYaw,    /* efGroundUpdateEffectYaw (0x80116090) */
    nEFRemixGroundSpriteProcStep    /* efGroundUpdateStepPositions (0x801160E8) + efGroundSetStepPositions (0x801161A0) */

} EFRemixGroundSpriteProc;

/* One 0x40-byte sprite of the stage .bin: the EFGroundDesc fields that differ between sprites. */
typedef struct EFRemixGroundSprite
{
    float alt_high;
    float alt_low;
    float pos_z;
    float scale;
    EFRemixGroundSpriteProc proc;
    int32_t o_dobjsetup;
    int32_t o_mobjsub;
    int32_t o_anim_joint;
    int32_t o_matanim_joint;

} EFRemixGroundSprite;

/* One Stages.add_sprite_info_array row (EFGroundParam). */
typedef struct EFRemixGroundSpriteParam
{
    uint16_t effect_id;
    uint16_t make_queue;
    int32_t lr;
    uint8_t effect_weight;

} EFRemixGroundSpriteParam;

/* One add_bg_animation stage: its bg_info struct (count, sprite_info_arrays, STAGE_FILE_OFFSET, sprite_data). */
typedef struct EFRemixGroundStage
{
    int32_t gkind;
    int32_t o_data;
    const EFRemixGroundSpriteParam *params;
    int32_t params_num;
    const EFRemixGroundSprite *sprites;
    int32_t sprites_num;

} EFRemixGroundStage;

/**
 * Tells whether a stage id reads one of the vanilla slots of the bg_animation_ table.
 *
 * @param gkind Stage id.
 * @return 1 for ids 0..7 (Peach's Castle to Saffron City), else 0.
 */
int32_t efRemixGroundRulesIsVanillaSlot(int32_t gkind);

/**
 * Finds the bg_info of an added stage (the add_bg_animation calls of Stages.asm).
 *
 * @param gkind Stage id.
 * @return The stage row, or NULL when the bg_animation_ table holds zero for that id (no background sprites).
 */
const EFRemixGroundStage* efRemixGroundRulesFindStage(int32_t gkind);

/**
 * Number of add_bg_animation stages, for the tests.
 *
 * @return The row count of the stage table.
 */
int32_t efRemixGroundRulesGetStageCount(void);

/**
 * Stage row by index, for the tests.
 *
 * @param index Row index, 0 to efRemixGroundRulesGetStageCount() - 1.
 * @return The row, or NULL when the index is out of range.
 */
const EFRemixGroundStage* efRemixGroundRulesGetStage(int32_t index);

#endif
