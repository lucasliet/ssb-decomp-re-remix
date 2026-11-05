/**
 * grremixjunglebarrels.c - barrels of the Remix stages, see grremixjunglebarrels.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "gr/grremix/grremixjunglebarrels.h"
#include "gr/grremix/grremixbarrelrules.h"
#include "gr/grremix/grremixgating.h"
#include "gr/grremix/grremixids.h"
#include "gm/gmremixsmashketball.h"
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"
#include <stdlib.h>
#include <gr/ground.h>
#include <gr/grcommon/grjungle.h>
#include <sc/scene.h>
#include <reloc_data.h>
#include <port_log.h>

#define GRREMIX_JUNGLE_BARRELS_MAX 12
#define GRREMIX_JUNGLE_BARRELS_NODES_FIELD_BANJO 0xE0 /* stage header: file of the barrel model, instead of map_nodes (0x80) */
#define GRREMIX_JUNGLE_BARRELS_ANIM_FIELD_BANJO 0xE4  /* stage header: animation tracks of the barrels, 8 bytes each */
#define GRREMIX_JUNGLE_BARRELS_ANIM_STRIDE 8
#define GRREMIX_JUNGLE_BARRELS_STRUCT_SIZE 0x10 /* _banjo_btp_barrel_structs row: x, y, rotation, animation address */

static GObj *sGRRemixJungleBarrels[GRREMIX_JUNGLE_BARRELS_MAX];
static s32 sGRRemixJungleBarrelsNum;

static void grRemixJungleBarrelsRegister(GObj *barrel_gobj)
{
    if (sGRRemixJungleBarrelsNum >= GRREMIX_JUNGLE_BARRELS_MAX)
    {
        return;
    }
    sGRRemixJungleBarrels[sGRRemixJungleBarrelsNum++] = barrel_gobj;
}

/** Reads a float of the Remix image (big-endian word of the N64 bits). */
static f32 grRemixJungleBarrelsReadImageFloat(u32 ram_addr)
{
    union
    {
        u32 bits;
        f32 value;

    } word;

    word.bits = remix_ram_image_read_word(ram_addr);

    return word.value;
}

/** Base of the jungle map file for a token of the stage header: the token resolved minus the map head offset. */
static void* grRemixJungleBarrelsGetMapHead(u32 nodes_token)
{
    return (void*) ((uintptr_t)PORT_RESOLVE(nodes_token) - (intptr_t)llGRJungleMapMapHead);
}

/** congo_jungle_barrel_ _nba_jam: two static barrels pointing down on each side of the main platform. */
static void grRemixJungleBarrelsMakeSmashketball(void)
{
    sb32 is_mode = gmRemixSmashketballIsActive();
    void *map_head;
    f32 y;
    GObj *right_gobj;

    if (grRemixBarrelRulesSmashketballIsMade(is_mode, grRemixHazardToggleIsOn()) == FALSE)
    {
        return;
    }
    map_head = grRemixJungleBarrelsGetMapHead(gMPCollisionGroundData->map_nodes);
    y = grRemixBarrelRulesGetSmashketballY(is_mode);

    grRemixJungleBarrelsRegister(grJungleRemixMakeTaruCannAt(map_head, -GRREMIX_BARREL_SMASHKETBALL_X, y, GRREMIX_BARREL_SMASHKETBALL_ROTATE, NULL));
    right_gobj = grJungleRemixMakeTaruCannAt(map_head, GRREMIX_BARREL_SMASHKETBALL_X, y, GRREMIX_BARREL_SMASHKETBALL_ROTATE, NULL);
    grRemixJungleBarrelsRegister(right_gobj);

    gGRCommonStruct.jungle.tarucann_gobj = right_gobj;
}

void grRemixJungleBarrelsMakeGround(void)
{
    sGRRemixJungleBarrelsNum = 0;

    switch (grRemixBarrelRulesGetStageKind(gSCManagerBattleState->gkind))
    {
    case GRREMIX_BARREL_STAGE_HAZARD:
        if (grRemixHazardToggleIsOn() == FALSE)
        {
            return;
        }
        grJungleMakeTaruCann();
        grRemixJungleBarrelsRegister(gGRCommonStruct.jungle.tarucann_gobj);
        return;

    case GRREMIX_BARREL_STAGE_SMASHKETBALL:
        grRemixJungleBarrelsMakeSmashketball();
        return;

    default:
        return;
    }
}

/** TRUE when SSB64_REMIX_BARREL_LOG is set to something other than empty or "0". */
static sb32 grRemixJungleBarrelsLogIsEnabled(void)
{
    const char *text = getenv("SSB64_REMIX_BARREL_LOG");

    return ((text != NULL) && (text[0] != '\0') && (text[0] != '0')) ? TRUE : FALSE;
}

/** Test aid: logs the two joint tokens of the animation track of a Banjo barrel and the barrel's start position. */
static void grRemixJungleBarrelsLogBanjoBarrel(s32 index, AObjEvent32 **anim_joints, GObj *barrel_gobj)
{
    u32 *joint_tokens = (u32*) anim_joints;
    DObj *dobj = DObjGetStruct(barrel_gobj);

    port_log("[remix-barrel] banjo index=%d joint0=0x%08X joint1=0x%08X script=%p base=(%.0f,%.0f)\n", index, joint_tokens[0], joint_tokens[1],
             (void*) PORT_RESOLVE(joint_tokens[0]), dobj->translate.vec.f.x, dobj->translate.vec.f.y);
}

/** One barrel of Banjo's Board the Platforms: position and rotation from the Remix table, the animation track of the header. */
static void grRemixJungleBarrelsMakeBanjoBarrel(void *map_head, u8 *anim_file, s32 index)
{
    u32 row = REMIX_SYM_Hazards___banjo_btp_barrel_structs + (index * GRREMIX_JUNGLE_BARRELS_STRUCT_SIZE);
    f32 x = grRemixJungleBarrelsReadImageFloat(row);
    f32 y = grRemixJungleBarrelsReadImageFloat(row + 4);
    f32 rotate_z = grRemixJungleBarrelsReadImageFloat(row + 8);
    AObjEvent32 **anim_joints = (AObjEvent32**) (anim_file + (index * GRREMIX_JUNGLE_BARRELS_ANIM_STRIDE));

    GObj *barrel_gobj = grJungleRemixMakeTaruCannAt(map_head, x, y, rotate_z, anim_joints);

    grRemixJungleBarrelsRegister(barrel_gobj);

    if (grRemixJungleBarrelsLogIsEnabled() != FALSE)
    {
        grRemixJungleBarrelsLogBanjoBarrel(index, anim_joints, barrel_gobj);
    }
}

void grRemixJungleBarrelsMakeBanjo(void)
{
    u32 nodes_token;
    u32 anim_token;
    u8 *anim_file;
    void *map_head;
    s32 i;

    if (gSCManagerBattleState->gkind != nGRRemixStage_BTP_BANJO)
    {
        return;
    }
    sGRRemixJungleBarrelsNum = 0;
    nodes_token = *(u32*) ((uintptr_t)gMPCollisionGroundData + GRREMIX_JUNGLE_BARRELS_NODES_FIELD_BANJO);
    anim_token = *(u32*) ((uintptr_t)gMPCollisionGroundData + GRREMIX_JUNGLE_BARRELS_ANIM_FIELD_BANJO);
    anim_file = (u8*) PORT_RESOLVE(anim_token);

    if ((nodes_token == 0) || (anim_file == NULL))
    {
        port_log("grremixjunglebarrels: BTP_BANJO header words 0xE0=0x%08X 0xE4=0x%08X unresolved, no barrels\n", nodes_token, anim_token);
        return;
    }
    map_head = grRemixJungleBarrelsGetMapHead(nodes_token);

    for (i = 0; i < GRREMIX_JUNGLE_BARRELS_MAX; i++)
    {
        grRemixJungleBarrelsMakeBanjoBarrel(map_head, anim_file, i);
    }
}

GObj* grRemixJungleBarrelsFindForBasketball(GObj *item_gobj)
{
    s32 i;

    if (grRemixBarrelRulesGetStageKind(gSCManagerBattleState->gkind) == GRREMIX_BARREL_STAGE_NONE)
    {
        return NULL;
    }
    for (i = 0; i < sGRRemixJungleBarrelsNum; i++)
    {
        if (grJungleTaruCannCheckBasketball(sGRRemixJungleBarrels[i], item_gobj) != FALSE)
        {
            return sGRRemixJungleBarrels[i];
        }
    }
    return NULL;
}

#endif
