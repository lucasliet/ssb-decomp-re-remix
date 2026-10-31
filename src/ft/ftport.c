/**
 * ftport.c - decomp-side helpers for the port fighter registry.
 *
 * Lives in decomp/ because it needs access to the source-of-truth
 * vanilla arrays (dFTManagerDataFiles[] / dFTCommonSpecial*StatusList[] /
 * dFTCommonEntryAppearStatusIDs[] / ...) and to the per-character efManager
 * entry-effect functions called from the original ftCommonAppearSetStatus
 * switch. The port-side .cpp can't pull these headers in without dragging
 * the decomp shim headers across the C++ boundary.
 *
 * Two exports:
 *   port_fighter_seed_vanilla() - registers fkinds 0..26 from the vanilla
 *       decomp tables. Called once at PortInit.
 *   ftPortVanillaEntryMakeEffect() - the per-fkind appear-effect switch
 *       previously inlined in ftCommonAppearSetStatus. Registered as the
 *       entry_make_effect callback for every vanilla fkind during seeding.
 */

#ifdef PORT

#include <ft/fighter.h>
#include <ef/efmanager.h>
#include <sys/objman.h>          /* gGCCommonLinks */
#include <sys/objdef.h>          /* nGCCommonLinkIDFighter */
#include <sc/scsubsys/scsubsys.h>

#include <stdlib.h>
#include <string.h>
#include <port_log.h>
#include "fighter_registry.h"
#include "remix/remix_synth_ft.h"
#include "remix/remix_ram_image.h"
#ifdef REGION_REMIX
#include "ft/ftremix/ftremixhooks.h"
#endif
#include "bridge/lbreloc_figatree.h"
#include "ft/ftremix/ftremix.h"

/* Per-character status descs (used as the ft_data row's special_descs). */
extern FTStatusDesc *dFTMainSpecialStatusDescs[];

/* Special-enter dispatch tables (one function per fkind). */
extern void (*dFTCommonSpecialNStatusList[])(GObj *);
extern void (*dFTCommonSpecialHiStatusList[])(GObj *);
extern void (*dFTCommonSpecialLwStatusList[])(GObj *);
extern void (*dFTCommonSpecialAirNStatusList[])(GObj *);
extern void (*dFTCommonSpecialAirHiStatusList[])(GObj *);
extern void (*dFTCommonSpecialAirLwStatusList[])(GObj *);

/* Match-entry status IDs (one [R,L] pair per fkind). */
extern s32 dFTCommonEntryAppearStatusIDs[][2];

/* Per-fkind costume rows. */
extern FTCostume dFTParamCostumeIDs[];

/* Per-fkind float scale. */
extern f32 dSCSubsysFighterScales[];

/* Per-fkind base color-anim ID (damage_level offset added at read). */
extern s32 dFTParamSkeletonColAnimIDs[];

/* Per-fkind yoshi-egg damage collision descriptors. */
extern ftCommonYoshiEggDesc dFTCommonYoshiEggDamageCollDescs[];

/* Per-fkind down-bounce SFX IDs (u16). */
extern u16 dFTCommonDataDownBounceSFX[];

/* Per-fkind public-call FGM IDs (u16). */
extern u16 dFTCommonDataPublicFighterCallFGMs[];

/* Per-fkind CPU attack-list pointers. */
extern FTComputerAttack *dFTComputerAttackList[];

/* CSS attack1 motion descs - per-fkind row of 8 entries. */
extern void *dMNCharactersAttack1MotionDescs[][8];

#define PORT_VANILLA_FKIND_COUNT  27

/* Mirrors the per-fkind appear-effect switch that used to live inline in
 * ftCommonAppearSetStatus (decomp/src/ft/ftcommon/ftcommonentry.c). */
void ftPortVanillaEntryMakeEffect(FTStruct *fp)
{
    GObj *boss_target_gobj;

    switch (fp->fkind)
    {
    case nFTKindMario:
    case nFTKindLuigi:
    case nFTKindMMario:
        efManagerMarioEntryDokanMakeEffect(&fp->entry_pos, fp->fkind);
        break;

    case nFTKindFox:
        efManagerFoxEntryArwingMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        break;

    case nFTKindDonkey:
    case nFTKindGDonkey:
        efManagerDonkeyEntryTaruMakeEffect(&fp->entry_pos);
        break;

    case nFTKindSamus:
        efManagerSamusEntryPointMakeEffect(&fp->entry_pos);
        break;

    case nFTKindLink:
        efManagerLinkEntryWaveMakeEffect(&fp->entry_pos);
        efManagerLinkEntryBeamMakeEffect(&fp->entry_pos);
        break;

    case nFTKindYoshi:
#ifdef REGION_REMIX
        ftRemixSizeScaleEntryEgg(efManagerYoshiEntryEggMakeEffect(&fp->entry_pos), fp);
#else
        efManagerYoshiEntryEggMakeEffect(&fp->entry_pos);
#endif
        break;

    case nFTKindKirby:
        efManagerKirbyEntryStarMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        break;

    case nFTKindPikachu:
    case nFTKindPurin:
        efManagerMBallThrownMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        break;

    case nFTKindCaptain:
        if (fp->status_vars.common.entry.lr == -1)
        {
            fp->status_vars.common.entry.is_rotate = TRUE;
        }
        efManagerCaptainEntryCarMakeEffect(&fp->entry_pos, fp->status_vars.common.entry.lr);
        break;

    case nFTKindBoss:
        /* Master Hand picks its target from the live fighter link list
         * (first non-self entry) and stashes it on the boss passive
         * state. No visual effect. */
        boss_target_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

        while (boss_target_gobj != NULL)
        {
            if (boss_target_gobj != fp->fighter_gobj)
            {
                break;
            }
            else boss_target_gobj = boss_target_gobj->link_next;
        }
        fp->passive_vars.boss.p->target_gobj = boss_target_gobj;

        break;

    default:
        break;
    }
}

/* Seed slots 0..26 of the registry from the vanilla decomp arrays.
 * Mods may overwrite individual rows later; for safety, Mario (fkind 0)
 * is the fallback row consulted by accessors when a synth row left a
 * field NULL. */
void port_fighter_seed_vanilla(void)
{
    s32 fkind;
    FighterDescriptor desc;

    for (fkind = 0; fkind < PORT_VANILLA_FKIND_COUNT; fkind++)
    {
        /* Zero-init every field so untracked fields stay deterministic. */
        FTData *zero_data = NULL;
        s32 i;
        u8 *p = (u8 *)&desc;
        for (i = 0; i < (s32)sizeof(desc); i++) p[i] = 0;
        (void)zero_data;

        desc.parent_fkind                       = fkind;
        desc.ft_data                            = dFTManagerDataFiles[fkind];
        desc.special_descs                      = dFTMainSpecialStatusDescs[fkind];
        desc.special_descs_count                = 0;  /* vanilla never bounds-checks; synth fills this */

        desc.special_handler[PORT_FIGHTER_SPECIAL_N]      = dFTCommonSpecialNStatusList[fkind];
        desc.special_handler[PORT_FIGHTER_SPECIAL_HI]     = dFTCommonSpecialHiStatusList[fkind];
        desc.special_handler[PORT_FIGHTER_SPECIAL_LW]     = dFTCommonSpecialLwStatusList[fkind];
        desc.special_handler[PORT_FIGHTER_SPECIAL_AIR_N]  = dFTCommonSpecialAirNStatusList[fkind];
        desc.special_handler[PORT_FIGHTER_SPECIAL_AIR_HI] = dFTCommonSpecialAirHiStatusList[fkind];
        desc.special_handler[PORT_FIGHTER_SPECIAL_AIR_LW] = dFTCommonSpecialAirLwStatusList[fkind];

        desc.entry_appear_status[0] = dFTCommonEntryAppearStatusIDs[fkind][0];
        desc.entry_appear_status[1] = dFTCommonEntryAppearStatusIDs[fkind][1];
        desc.entry_make_effect      = ftPortVanillaEntryMakeEffect;

        /* D_ovl1_80390D20 is `FTOpeningDesc *[]`; vanilla code reads
         * D_ovl1_80390D20[fkind] and gets a FTOpeningDesc *. Store that
         * pointer directly so the accessor's return matches the original
         * read site one-for-one. */
        desc.opening_descs          = D_ovl1_80390D20[fkind];

        desc.costume_row            = &dFTParamCostumeIDs[fkind];
        /* dSCSubsysFighterScales has rows for the 12 playable fkinds only;
         * vanilla read sites never index it beyond nFTKindPlayableEnd. Give
         * boss/metal/polygon/giant rows a neutral 1.0 instead of reading
         * past the table. */
        desc.scale                  = (fkind <= nFTKindPlayableEnd) ? dSCSubsysFighterScales[fkind] : 1.0f;
        desc.skeleton_col_anim_base = dFTParamSkeletonColAnimIDs[fkind];
        desc.yoshi_egg_damage_coll  = &dFTCommonYoshiEggDamageCollDescs[fkind];
        desc.down_bounce_fgm        = (s32)dFTCommonDataDownBounceSFX[fkind];
        desc.public_call_fgm        = (s32)dFTCommonDataPublicFighterCallFGMs[fkind];
        desc.computer_attack_list   = dFTComputerAttackList[fkind];

        /* CSS fields seeded from mncharacters tables; specific motion id
         * lives elsewhere and is set per-character by the menu code at
         * CSS time, so the registry row just carries the per-fkind row
         * pointer for now. */
        desc.css_motion_special     = 0;
        desc.css_attack1_motion_descs = &dMNCharactersAttack1MotionDescs[fkind][0];

        /* SR engine extensions stay at NULL/0 for vanilla fkinds so the
         * accessors return the "no override" sentinel. team_costume[] is
         * a special case: 0x00 is a valid costume index, so we explicitly
         * set 0xFF (= "no team-costume override") for vanilla rows. */
        desc.team_costume[0] = 0xFFu;
        desc.team_costume[1] = 0xFFu;
        desc.team_costume[2] = 0xFFu;
        desc.team_costume[3] = 0xFFu;

        port_fighter_register(fkind, &desc);
    }
}

/* Storage behind one synth FTData. `data` must stay the first member: the
 * FTData pointer handed out is cast back to this block in
 * port_fighter_synth_ft_destroy. */
typedef struct PortSynthFtOwned
{
    FTData data;
    void *file_slots[PORT_SYNTH_FILE_COUNT];
    s32 particle_slot;
    s32 submotion_count_slot;
    FTMotionDesc *mainmotion;
    FTMotionDesc *submotion;
} PortSynthFtOwned;

/* Remix stores scripts it adds as raw RAM addresses into its image. Those
 * become host pointers, which ftMainSetStatus takes as-is (> 0x100000).
 * File offsets and the 0x80000000 "no script" sentinel pass through. */
static intptr_t ftPortMapMotionOffset(u32 command_offset)
{
    void *script = NULL;

#ifdef REGION_REMIX
    script = remix_ram_image_resolve(command_offset);

    if (script == NULL)
    {
        script = ftRemixFindDemoScript(command_offset);
    }
#endif
    if (script != NULL)
    {
        return (intptr_t)script;
    }
    /* Other N64 RAM addresses have no host mapping; read
     * as a host pointer they crash, so treat them as "no script". */
    if (command_offset > 0x80000000u)
    {
        port_log("SSB64: ftPortMapMotionOffset: unmapped script address 0x%08X, row gets no script\n",
                 command_offset);
        return (intptr_t)0x80000000u;
    }
    return (intptr_t)command_offset;
}

/* XRotN / TransN / YRotN: the hidden joints a motion row asks ftMainSetStatus
 * to build. */
#define FTPORT_HIDDEN_JOINT_FLAGS 0xE0000000u

/* Hidden-joint flags of the parent's row `i`, or 0 when it has none. Remix
 * rows for moves it reimplements omit them, but those moves still run the
 * parent's status routines, which read e.g. joints[nFTPartsJointTransN]. */
static u32 ftPortParentHiddenJointFlags(const FTData *parent, u32 i)
{
    if ((parent == NULL) || (parent->mainmotion == NULL) || ((s32)i >= parent->mainmotion_array_count))
    {
        return 0;
    }
    return parent->mainmotion->motion_desc[i].anim_desc.word & FTPORT_HIDDEN_JOINT_FLAGS;
}

/* Copies `count` rows into a new native FTMotionDesc array, merging in the
 * hidden-joint flags of `parent`'s matching rows (NULL skips the merge).
 * Returns NULL for count 0 or on allocation failure. */
static FTMotionDesc *ftPortCopyMotionArray(const PortSynthMotion *src, u32 count, const FTData *parent)
{
    FTMotionDesc *dst;
    u32 i;

    if (count == 0)
    {
        return NULL;
    }
    dst = malloc(count * sizeof(FTMotionDesc));
    if (dst == NULL)
    {
        return NULL;
    }
    for (i = 0; i < count; i++)
    {
        if (src[i].anim_file_id != 0)
        {
            portRelocMarkFighterFigatreeFile(src[i].anim_file_id);
        }
        dst[i].anim_file_id = src[i].anim_file_id;
        dst[i].offset = ftPortMapMotionOffset(src[i].command_offset);
        dst[i].anim_desc.word = src[i].flags | ftPortParentHiddenJointFlags(parent, i);
    }
    return dst;
}

/* Overwrites the Remix-supplied fields and points the runtime slots at storage
 * owned by the block, so the synth never writes into vanilla arrays. */
static void ftPortSetSynthFields(PortSynthFtOwned *owned, const PortSynthFtSpec *spec)
{
    FTData *data = &owned->data;

    data->file_main_id = spec->file_ids[0];
    data->file_mainmotion_id = spec->file_ids[1];
    data->file_submotion_id = spec->file_ids[2];
    data->file_model_id = spec->file_ids[3];
    data->file_shieldpose_id = spec->file_ids[4];
    data->file_special1_id = spec->file_ids[5];
    data->file_special2_id = spec->file_ids[6];
    data->file_special3_id = spec->file_ids[7];
    data->file_special4_id = spec->file_ids[8];
    data->file_main_size = spec->file_main_size;
    data->o_attributes = spec->o_attributes;
    data->file_anim_size = spec->file_anim_size;

    data->p_file_main = &owned->file_slots[0];
    data->p_file_mainmotion = &owned->file_slots[1];
    data->p_file_submotion = &owned->file_slots[2];
    data->p_file_model = &owned->file_slots[3];
    data->p_file_shieldpose = &owned->file_slots[4];
    data->p_file_special1 = &owned->file_slots[5];
    data->p_file_special2 = &owned->file_slots[6];
    data->p_file_special3 = &owned->file_slots[7];
    data->p_file_special4 = &owned->file_slots[8];
    data->p_particle = &owned->particle_slot;
    data->submotion_array_count = &owned->submotion_count_slot;

    data->mainmotion = (FTMotionDescArray *)owned->mainmotion;
    data->submotion = (FTMotionDescArray *)owned->submotion;
    data->mainmotion_array_count = (s32)spec->mainmotion_count;
    owned->submotion_count_slot = (s32)spec->submotion_count;
}

struct FTData *port_fighter_synth_ft_create(int parent_fkind, const PortSynthFtSpec *spec)
{
    const FTData *parent = port_fighter_data(parent_fkind);
    PortSynthFtOwned *owned;

    if (parent == NULL)
    {
        return NULL;
    }
    owned = malloc(sizeof(PortSynthFtOwned));
    if (owned == NULL)
    {
        return NULL;
    }
    owned->data = *parent;
    owned->mainmotion = ftPortCopyMotionArray(spec->mainmotion, spec->mainmotion_count, parent);
    owned->submotion = ftPortCopyMotionArray(spec->submotion, spec->submotion_count, NULL);
    if ((spec->mainmotion_count != 0 && owned->mainmotion == NULL)
        || (spec->submotion_count != 0 && owned->submotion == NULL))
    {
        port_fighter_synth_ft_destroy(&owned->data);
        return NULL;
    }
    ftPortSetSynthFields(owned, spec);
    return &owned->data;
}

void port_fighter_synth_ft_destroy(struct FTData *ft)
{
    PortSynthFtOwned *owned;

    if (ft == NULL)
    {
        return;
    }
    owned = (PortSynthFtOwned *)ft;
    free(owned->mainmotion);
    free(owned->submotion);
    free(owned);
}

int port_fighter_extend_mainmotion(int fkind, uint32_t vanilla_count, const PortSynthMotion *rows, uint32_t count)
{
    FTData *data = port_fighter_data(fkind);
    FTMotionDesc *grown;
    FTMotionDesc *added;

    if ((data == NULL) || (data->mainmotion == NULL) || (count <= vanilla_count))
    {
        return 0;
    }
    grown = malloc(count * sizeof(FTMotionDesc));

    if (grown == NULL)
    {
        return 0;
    }
    added = ftPortCopyMotionArray(&rows[vanilla_count], count - vanilla_count, NULL);

    if (added == NULL)
    {
        free(grown);
        return 0;
    }
    memcpy(grown, data->mainmotion, vanilla_count * sizeof(FTMotionDesc));
    memcpy(&grown[vanilla_count], added, (count - vanilla_count) * sizeof(FTMotionDesc));
    free(added);

    data->mainmotion = (FTMotionDescArray *)grown;
    data->mainmotion_array_count = (s32)count;
    return 1;
}

int port_fighter_set_attributes_offset(int fkind, uint32_t o_attributes)
{
    FTData *data = port_fighter_data(fkind);

    if (data == NULL)
    {
        return 0;
    }
    data->o_attributes = o_attributes;
    return 1;
}

#endif /* PORT */
