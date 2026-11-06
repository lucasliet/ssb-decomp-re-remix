#if defined(PORT) && defined(REGION_REMIX)

#include <sc/sccommon/scremixvsdemo.h>
#include <sc/sccommon/scremixvsdemorules.h>
#include <sc/sctypes.h>
#include <sc/scmanager.h>
#include <sys/utils.h>
#include "remix/remix_ram_image.h"
#include "remix/remix_roster_data.h"
#include "remix/remix_symbols.h"

#define SC_REMIX_VS_DEMO_NO_FKIND -1

/* VsDemo.asm draws from Global.get_random_int_(Stages.id.MAX_STAGE_ID), an exclusive range, so HTP_FALL (0xD7) is never picked. */
#define SC_REMIX_VS_DEMO_STAGE_ID_RANGE 0xD7
#define SC_REMIX_VS_DEMO_STAGE_ID_REST 0x8B
#define SC_REMIX_VS_DEMO_STAGE_CLASS_BATTLE 0x00

/* VsDemo.asm: "fix the offsets (original table adds 0x10)". */
#define SC_REMIX_VS_DEMO_NAME_TABLE_FIX 0x10

/**
 * Tells whether a Remix fighter may be an auto-demo fighter: NA variant only, and not one of the excluded kinds.
 *
 * @param fkind Remix fighter kind to test, an index into kRemixRoster.
 * @param excluded_a First fighter kind that must not be picked, or SC_REMIX_VS_DEMO_NO_FKIND.
 * @param excluded_b Second fighter kind that must not be picked, or SC_REMIX_VS_DEMO_NO_FKIND.
 * @return true when fkind can be picked.
 */
static bool scRemixVsDemoIsEligibleFighter(s32 fkind, s32 excluded_a, s32 excluded_b)
{
    if ((fkind == excluded_a) || (fkind == excluded_b))
    {
        return false;
    }
    return kRemixRoster[fkind].variant_type == REMIX_VARIANT_NA;
}

/**
 * Draws random Remix fighter kinds until one is eligible, the same loop as VsDemo.asm.
 *
 * @param excluded_a First fighter kind to skip, or SC_REMIX_VS_DEMO_NO_FKIND.
 * @param excluded_b Second fighter kind to skip, or SC_REMIX_VS_DEMO_NO_FKIND.
 * @return A fighter kind that passes scRemixVsDemoIsEligibleFighter.
 */
static s32 scRemixVsDemoPickEligibleFighter(s32 excluded_a, s32 excluded_b)
{
    s32 fkind;

    do
    {
        fkind = syUtilsRandIntRange(ARRAY_COUNT(kRemixRoster));
    } while (!scRemixVsDemoIsEligibleFighter(fkind, excluded_a, excluded_b));

    return fkind;
}

/**
 * Tells whether a stage can be the auto-demo battle: class BATTLE in the Remix class table, and not Rest Area.
 *
 * @param stage_id Stage id to test.
 * @return true when stage_id can be picked.
 */
static bool scRemixVsDemoIsBattleStage(s32 stage_id)
{
    if (stage_id == SC_REMIX_VS_DEMO_STAGE_ID_REST)
    {
        return false;
    }
    return remix_ram_image_read_byte(REMIX_SYM_Stages__class_table + stage_id) == SC_REMIX_VS_DEMO_STAGE_CLASS_BATTLE;
}

/**
 * Sets the two focused auto-demo fighters to distinct NA Remix fighters. Replaces the vanilla pick in
 * mnTitleSetDemoFighterKinds at its epilogue (VsDemo.asm enable_focused_remix_chars_, RAM 0x80131CDC).
 */
void scRemixVsDemoPickFocusFighters(void)
{
    s32 first_fkind;
    s32 second_fkind;

    first_fkind = scRemixVsDemoPickEligibleFighter(SC_REMIX_VS_DEMO_NO_FKIND, SC_REMIX_VS_DEMO_NO_FKIND);
    second_fkind = scRemixVsDemoPickEligibleFighter(first_fkind, SC_REMIX_VS_DEMO_NO_FKIND);

    gSCManagerSceneData.demo_fkind[0] = (u8)first_fkind;
    gSCManagerSceneData.demo_fkind[1] = (u8)second_fkind;
}

/**
 * Picks a fighter for a non-focused auto-demo slot: NA Remix fighter that differs from both focused fighters, and for
 * port 4 also from port 3's fighter. Replaces the body of scAutoDemoGetFighterKind for players 2 and up (VsDemo.asm
 * enable_nonfocused_remix_chars_, RAM 0x8018D928). The vanilla fighter mask is not updated, as in the asm.
 *
 * @param player Demo player index being filled, 2 or 3; players[2] must already hold its fighter for player 3.
 * @return Remix fighter kind for the slot.
 */
s32 scRemixVsDemoPickNonFocusedFighter(s32 player)
{
    s32 fkind;

    do
    {
        fkind = scRemixVsDemoPickEligibleFighter(gSCManagerSceneData.demo_fkind[0], gSCManagerSceneData.demo_fkind[1]);
    } while (scRemixVsDemoRepeatsPort3Fighter(player, fkind, gSCManagerBattleState->players[2].fkind));

    return fkind;
}

/**
 * Picks a random battle stage for the auto demo (VsDemo.asm enable_remix_stages_, RAM 0x8018DA20). Returns the
 * fallback when the Remix RAM image is not loaded, so the class table is never read from an unloaded image.
 *
 * @param fallback_stage_id Stage id to return when the Remix class table is unavailable.
 * @return Stage id for the demo battle.
 */
s32 scRemixVsDemoPickStage(s32 fallback_stage_id)
{
    s32 stage_id;

    if (remix_ram_image_resolve(REMIX_SYM_Stages__class_table) == NULL)
    {
        return fallback_stage_id;
    }
    do
    {
        stage_id = syUtilsRandIntRange(SC_REMIX_VS_DEMO_STAGE_ID_RANGE);
    } while (!scRemixVsDemoIsBattleStage(stage_id));

    return stage_id;
}

/**
 * Reads the 1P name texture offset of a Remix fighter from the Remix name table, plus the asm fix of 0x10
 * (VsDemo.asm, RAM 0x8018DBD4 and 0x8018DC08). The offset points into the names file llCharacterNamesFileID, the
 * file the 1P title card (sc1pintro.c) already reads with the same table.
 *
 * @param fkind Remix fighter kind, an index into the name table.
 * @return Texture offset, or SC_REMIX_VS_DEMO_NO_FKIND when fkind is out of range or the Remix RAM image is not loaded.
 */
s32 scRemixVsDemoGetNameTextureOffset(s32 fkind)
{
    uint32_t texture_offset;

    if ((fkind < 0) || (fkind >= ARRAY_COUNT(kRemixRoster)))
    {
        return SC_REMIX_VS_DEMO_NO_FKIND;
    }
    if (remix_ram_image_resolve(REMIX_SYM_SinglePlayer__name_texture_table) == NULL)
    {
        return SC_REMIX_VS_DEMO_NO_FKIND;
    }
    texture_offset = remix_ram_image_read_word(REMIX_SYM_SinglePlayer__name_texture_table + ((uint32_t)fkind * 4u));

    return (s32)(texture_offset + SC_REMIX_VS_DEMO_NAME_TABLE_FIX);
}

#endif
