#include <gr/ground.h>
#include <it/item.h>
#include <sc/scene.h>
#include <reloc_data.h>
#ifdef REGION_REMIX
#include "gr/grremix/grremixgating.h"
#endif

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x8010B340
void grCastleBumperProcUpdate(GObj *ground_gobj)
{
    Vec3f *ground_pos = &DObjGetStruct(ground_gobj)->translate.vec.f;

    if (gGRCommonStruct.castle.bumper_gobj != NULL)
    {
        Vec3f *bumper_pos = &DObjGetStruct(gGRCommonStruct.castle.bumper_gobj)->translate.vec.f;

        bumper_pos->x = ground_pos->x + gGRCommonStruct.castle.bumper_pos.x;
    }
}

// 0x8010B378
void grCastleInitAll(void)
{
    void *map_head;
    GObj *ground_gobj;
    Vec3f yakumono_pos;
    Vec3f vel;
    s32 pos_id;
    DObj *dobj;

#ifdef PORT
    gGRCommonStruct.castle.map_head = map_head = (void*)((uintptr_t)PORT_RESOLVE(gMPCollisionGroundData->map_nodes) - (intptr_t)llGRCastleMapMapHead);
#else
    gGRCommonStruct.castle.map_head = map_head = (void*)((uintptr_t)gMPCollisionGroundData->map_nodes - (intptr_t)&llGRCastleMapMapHead);
#endif

    ground_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjProcess(ground_gobj, grCastleBumperProcUpdate, nGCProcessKindFunc, 4);

    dobj = gcAddDObjForGObj(ground_gobj, NULL);
    dobj->translate.vec.f.x = dobj->translate.vec.f.y = dobj->translate.vec.f.z = 0.0F;

    gcAddGObjProcess(ground_gobj, gcPlayAnimAll, nGCProcessKindFunc, 5);

#ifdef PORT
    gcAddAnimJointAll(ground_gobj, (AObjEvent32**)PORT_RESOLVE(gMPCollisionGroundData->map_nodes), 0.0F);
#else
    gcAddAnimJointAll(ground_gobj, gMPCollisionGroundData->map_nodes, 0.0F);
#endif
    gcPlayAnimAll(ground_gobj);

    mpCollisionGetMapObjIDsKind(nMPMapObjKindBumper, &pos_id);
    mpCollisionGetMapObjPositionID(pos_id, &yakumono_pos);

    gGRCommonStruct.castle.bumper_pos = yakumono_pos;

    vel.x = 0.0F;
    vel.y = 0.0F;
    vel.z = 0.0F;

    gGRCommonStruct.castle.bumper_gobj = itManagerMakeItemSetupCommon(NULL, nITKindGBumper, &yakumono_pos, &vel, ITEM_FLAG_PARENT_GROUND);
}

// 0x8010B4AC
GObj* grCastleMakeGround(void)
{
#ifdef REGION_REMIX
    /* REMIX-TODO(items): Bowser's Stadium RTTF bombs (Hazards.asm peach_bumper_ _bowser_stadium) are not spawned. */
    if (grRemixStageHasPeachBumper(gSCManagerBattleState->gkind) && grRemixHazardToggleIsOn())
#endif
    grCastleInitAll();

    return NULL;
}
