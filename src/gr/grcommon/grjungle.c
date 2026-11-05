#include <gr/ground.h>
#include <ft/fighter.h>
#include <reloc_data.h>
#ifdef REGION_REMIX
#include <sc/scene.h>
#include <gr/grremix/grremixids.h>
#include "gr/grremix/grremixjunglebarrels.h"
#include "it/itremix/itremixbasketball.h"
#endif

// // // // // // // // // // // //
//                               //
//       INITIALIZED DATA        //
//                               //
// // // // // // // // // // // //

// 0x8012EB50
DObjTransformTypes dGRJungleTaruCannTransformKinds[/* */] =
{
    { 0x28, nGCMatrixKindRotRpyR, 0x00 },
    { nGCMatrixKindTraRotRpyRSca, nGCMatrixKindNull, 0x00 }
};

// // // // // // // // // // // //
//                               //
//          ENUMERATORS          //
//                               //
// // // // // // // // // // // //

enum grJungleTaruCannStatus
{
    nGRJungleTaruCannStatusMove,
    nGRJungleTaruCannStatusRotate
};

// // // // // // // // // // // //
//                               //
//           FUNCTIONS           //
//                               //
// // // // // // // // // // // //

// 0x80109CB0
void grJungleTaruCannAddAnimOffset(GObj *ground_gobj, intptr_t offset)
{
    DObj *dobj = DObjGetStruct(ground_gobj)->child;

    gcAddDObjAnimJoint(dobj, (AObjEvent32*) ((uintptr_t)gGRCommonStruct.jungle.map_head + (intptr_t)offset), 0.0F);
    gcParseDObjAnimJoint(dobj);
    gcPlayDObjAnimJoint(dobj);
}

// 0x80109CFC
void grJungleTaruCannAddAnimFill(GObj *ground_gobj)
{
#ifdef PORT
    grJungleTaruCannAddAnimOffset(ground_gobj, llGRJungleMapTaruCannFillAnimJoint);
#else
    grJungleTaruCannAddAnimOffset(ground_gobj, &llGRJungleMapTaruCannFillAnimJoint);
#endif
}

// 0x80109D20
void grJungleTaruCannAddAnimShoot(GObj *ground_gobj)
{
#ifdef PORT
    grJungleTaruCannAddAnimOffset(ground_gobj, llGRJungleMapTaruCannShootAnimJoint);
#else
    grJungleTaruCannAddAnimOffset(ground_gobj, &llGRJungleMapTaruCannShootAnimJoint);
#endif
}

// 0x80109D44
void grJungleTaruCannUpdateMove(GObj *ground_gobj)
{
    gGRCommonStruct.jungle.tarucann_wait--;

    if (gGRCommonStruct.jungle.tarucann_wait == 0)
    {
        gGRCommonStruct.jungle.tarucann_status = nGRJungleTaruCannStatusRotate;

        gGRCommonStruct.jungle.tarucann_rotate_step = ((syUtilsRandUShort() % 2) != 0) ? 0.07F : -0.07F;

        gGRCommonStruct.jungle.tarucann_wait = 90;
    }
}

// 0x80109DBC
void grJungleTaruCannUpdateRotate(GObj *ground_gobj)
{
    DObj *dobj = DObjGetStruct(ground_gobj);

    gGRCommonStruct.jungle.tarucann_wait--;

    if (gGRCommonStruct.jungle.tarucann_wait == 0)
    {
        gGRCommonStruct.jungle.tarucann_status = nGRJungleTaruCannStatusMove;

        gGRCommonStruct.jungle.tarucann_wait = syUtilsRandIntRange(180) + 180;

        dobj->rotate.vec.f.z = F_CST_DTOR32(0.0F);
    }
    else dobj->rotate.vec.f.z += gGRCommonStruct.jungle.tarucann_rotate_step;
}

// 0x80109E34
void grJungleTaruCannProcUpdate(GObj *ground_gobj)
{
    switch (gGRCommonStruct.jungle.tarucann_status)
    {
    case nGRJungleTaruCannStatusMove:
        grJungleTaruCannUpdateMove(ground_gobj);
        break;

    case nGRJungleTaruCannStatusRotate:
        grJungleTaruCannUpdateRotate(ground_gobj);
        break;
    }
}

// 0x80109E84
void grJungleMakeTaruCann(void)
{
    void *map_head;
    GObj *tarucann_gobj;

#ifdef PORT
    map_head = (void*) ((uintptr_t)PORT_RESOLVE(gMPCollisionGroundData->map_nodes) - (intptr_t)llGRJungleMapMapHead);
#else
    map_head = (void*) ((uintptr_t)gMPCollisionGroundData->map_nodes - (intptr_t)&llGRJungleMapMapHead);
#endif
    gGRCommonStruct.jungle.map_head = map_head;

    gGRCommonStruct.jungle.tarucann_gobj = tarucann_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(tarucann_gobj, gcDrawDObjTreeForGObj, 6, GOBJ_PRIORITY_DEFAULT, ~0);

#ifdef PORT
    grModelSetupGroundDObjs(tarucann_gobj, (DObjDesc*) ((intptr_t)llGRJungleMapMapHead + (uintptr_t)map_head), NULL, dGRJungleTaruCannTransformKinds);
#else
    grModelSetupGroundDObjs(tarucann_gobj, (DObjDesc*) ((intptr_t)&llGRJungleMapMapHead + (uintptr_t)map_head), NULL, dGRJungleTaruCannTransformKinds);
#endif
    gcAddGObjProcess(tarucann_gobj, gcPlayAnimAll, nGCProcessKindFunc, 5);

#ifdef PORT
    gcAddAnimJointAll(tarucann_gobj, (AObjEvent32 **)((uintptr_t)map_head + (intptr_t)llGRJungleMapTaruCannDefaultAnimJoint), 0.0F);
#else
    gcAddAnimJointAll(tarucann_gobj, ((uintptr_t)map_head + (intptr_t)&llGRJungleMapTaruCannDefaultAnimJoint), 0.0F);
#endif
    gcPlayAnimAll(tarucann_gobj);

    gcAddGObjProcess(tarucann_gobj, grJungleTaruCannProcUpdate, nGCProcessKindFunc, 4);
    ftMainCheckAddGroundObstacle(tarucann_gobj, grJungleTaruCannCheckGetDamageKind);

    gGRCommonStruct.jungle.tarucann_status = nGRJungleTaruCannStatusMove;
    gGRCommonStruct.jungle.tarucann_wait = syUtilsRandIntRange(180) + 180;
    gGRCommonStruct.jungle.tarucann_rotate_step = F_CST_DTOR32(0.0F);
}

#ifdef REGION_REMIX
/**
 * Hazards.asm add_barrel_ (0x80109E84 body): makes one static barrel at a position, with the enter and exit animations and
 * an optional animation track. The barrel is registered as a ground obstacle, so the fighters enter it.
 *
 * @param map_head Base of the jungle map file the barrel model is read from (Banjo's Board the Platforms has its own).
 * @param x X of the barrel.
 * @param y Y of the barrel.
 * @param rotate_z Rotation z of the barrel in radians (the exit direction).
 * @param anim_joints Animation track of the barrel, NULL for none.
 * @return The barrel object.
 */
GObj* grJungleRemixMakeTaruCannAt(void *map_head, f32 x, f32 y, f32 rotate_z, AObjEvent32 **anim_joints)
{
    GObj *tarucann_gobj;
    DObj *dobj;

    gGRCommonStruct.jungle.map_head = map_head;

    tarucann_gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjDisplay(tarucann_gobj, gcDrawDObjTreeForGObj, 6, GOBJ_PRIORITY_DEFAULT, ~0);
    grModelSetupGroundDObjs(tarucann_gobj, (DObjDesc*) ((intptr_t)llGRJungleMapMapHead + (uintptr_t)map_head), NULL, dGRJungleTaruCannTransformKinds);

    dobj = DObjGetStruct(tarucann_gobj);
    dobj->translate.vec.f.x = x;
    dobj->translate.vec.f.y = y;
    dobj->rotate.vec.f.z = rotate_z;

    if (anim_joints != NULL)
    {
        gcAddAnimJointAll(tarucann_gobj, anim_joints, 0.0F);
    }
    gcAddGObjProcess(tarucann_gobj, gcPlayAnimAll, nGCProcessKindFunc, 5);
    ftMainCheckAddGroundObstacle(tarucann_gobj, grJungleTaruCannCheckGetDamageKind);

    return tarucann_gobj;
}
#endif

// 0x80109FB4
GObj* grJungleMakeGround(void)
{
#ifdef REGION_REMIX
    grRemixJungleBarrelsMakeGround();
#else
    grJungleMakeTaruCann();
#endif

    return NULL;
}

/**
 * Overlap test shared by the fighters and the Smashketball ball (the tail of 0x80109FD8, entered at 0x8010A018 by the ball):
 * within 280 units of the barrel on both axes and no other fighter inside it.
 *
 * @param ground_gobj Barrel object.
 * @param subject_gobj Fighter or ball object that tries to enter.
 * @param kind Receives the hit environment kind of the barrel.
 * @return TRUE when the subject enters the barrel; the fill animation is started then.
 */
static sb32 grJungleTaruCannCheckEnter(GObj *ground_gobj, GObj *subject_gobj, s32 *kind)
{
    DObj *gr_dobj = DObjGetStruct(ground_gobj);
    DObj *subject_dobj = DObjGetStruct(subject_gobj);
    f32 dist_x;
    f32 dist_y;

    if (gr_dobj->translate.vec.f.x < subject_dobj->translate.vec.f.x)
    {
        dist_x = -(gr_dobj->translate.vec.f.x - subject_dobj->translate.vec.f.x);
    }
    else dist_x = gr_dobj->translate.vec.f.x - subject_dobj->translate.vec.f.x;

    if (gr_dobj->translate.vec.f.y < subject_dobj->translate.vec.f.y)
    {
        dist_y = -(gr_dobj->translate.vec.f.y - subject_dobj->translate.vec.f.y);
    }
    else dist_y = gr_dobj->translate.vec.f.y - subject_dobj->translate.vec.f.y;

    if ((dist_x < 280.0F) && (dist_y < 280.0F))
    {
        GObj *other_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

        while (other_gobj != NULL)
        {
            if (other_gobj != subject_gobj)
            {
                FTStruct *other_fp = ftGetStruct(other_gobj);

                if ((other_fp->status_id == nFTCommonStatusTaruCann) && (ground_gobj == other_fp->status_vars.common.tarucann.tarucann_gobj))
                {
                    return FALSE;
                }
            }
            other_gobj = other_gobj->link_next;
        }
#ifdef REGION_REMIX
        if (itRemixBasketballIsBarrelOccupied()) /* Basketball.asm prevent_fighter_enter_barrel_ (0x8010A0E0) */
        {
            return FALSE;
        }
#endif
        *kind = nGMHitEnvironmentTaruCann;

        grJungleTaruCannAddAnimFill(ground_gobj);

        return TRUE;
    }
    return FALSE;
}

// 0x80109FD8
sb32 grJungleTaruCannCheckGetDamageKind(GObj *ground_gobj, GObj *fighter_gobj, s32 *kind)
{
    FTStruct *this_fp = ftGetStruct(fighter_gobj);

    if ((this_fp->tarucann_wait == 0) && (this_fp->status_id != nFTCommonStatusTaruCann) && !(this_fp->capture_immune_mask & FTCATCHKIND_MASK_TARUCANN))
    {
        return grJungleTaruCannCheckEnter(ground_gobj, fighter_gobj, kind);
    }
    return FALSE;
}

#ifdef REGION_REMIX
/**
 * Basketball.asm check_basketball_barrel_collision_ (the jump into 0x8010A018): the overlap test of a barrel with the ball.
 *
 * @param ground_gobj Barrel object.
 * @param item_gobj Basketball object.
 * @return TRUE when the ball enters the barrel.
 */
sb32 grJungleTaruCannCheckBasketball(GObj *ground_gobj, GObj *item_gobj)
{
    s32 kind;

    return grJungleTaruCannCheckEnter(ground_gobj, item_gobj, &kind);
}

/**
 * barrel_angle_fix_ (0x8010A12C): the rotation of the barrel a fighter is in, read from the fighter instead of the single
 * barrel object of the stage, so extra barrels shoot along their own rotation.
 *
 * @param tarucann_gobj Barrel object (status_vars.common.tarucann.tarucann_gobj of the fighter).
 * @return Rotation z of the barrel in radians.
 */
f32 grJungleTaruCannGetRotateOf(GObj *tarucann_gobj)
{
    return DObjGetStruct(tarucann_gobj)->rotate.vec.f.z;
}
#endif

// 0x8010A104
void grJungleTaruCannGetPosition(Vec3f *pos)
{
    *pos = DObjGetStruct(gGRCommonStruct.jungle.tarucann_gobj)->translate.vec.f;
}

// 0x8010A12C
f32 grJungleTaruCannGetRotate(void)
{
    return DObjGetStruct(gGRCommonStruct.jungle.tarucann_gobj)->rotate.vec.f.z;
}
