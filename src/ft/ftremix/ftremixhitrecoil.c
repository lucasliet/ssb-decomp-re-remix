/**
 * ftremixhitrecoil.c - hit recoil patches of Wario/Wario.asm, see ftremixhitrecoil.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhitrecoil.h"
#include "ft/ftremix/ftremixhitrecoilrules.h"
#include "gm/gmcollision.h"
#include "it/itdef.h"
#include "it/itmanager.h"
#include "mp/mpcommon.h"
#include "remix/remix_roster_data.h"
#include <sys/audio.h>
#include <sys/utils.h>

/* wario shoulder hurtbox: FTStruct 0x05BC is damage_colls[0], and 0x0640 of Kirby is damage_colls[3]. */
#define FTREMIXHITRECOIL_SHOULDER_DAMAGE_COLL 0
#define FTREMIXHITRECOIL_KIRBY_SHOULDER_DAMAGE_COLL 3

/* Joint at FTStruct 0x095C, the food spawn point (Ebisumaru's hand). */
#define FTREMIXHITRECOIL_FOOD_JOINT 29
#define FTREMIXHITRECOIL_FOOD_VEL_Y 30.0F           /* 0x41F00000 */
#define FTREMIXHITRECOIL_TOMATO_ODDS 100            /* get_random_int_safe_(100) == 0 */
#define FTREMIXHITRECOIL_DANGO_KIND 0x36            /* Item.Dango.id, itremixdango.c */
#define FTREMIXHITRECOIL_FOOD_FGM 0x524             /* FGM.play(0x524), audience laugh */

static sb32 ftRemixHitRecoilIsKirby(FTStruct *fp)
{
    return (fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY);
}

sb32 ftRemixHitRecoilIsBodySlamInvincible(FTStruct *fp)
{
    s32 damage_coll_id = ftRemixHitRecoilIsKirby(fp) ? FTREMIXHITRECOIL_KIRBY_SHOULDER_DAMAGE_COLL : FTREMIXHITRECOIL_SHOULDER_DAMAGE_COLL;

    if (!ftRemixHitRecoilIsBodySlam(fp->fkind, fp->status_id))
    {
        return FALSE;
    }
    return fp->damage_colls[damage_coll_id].hitstatus == nGMHitStatusInvincible;
}

/* _create_food: Dango 99 times in 100, Tomato otherwise. */
static s32 ftRemixHitRecoilGetFoodKind(void)
{
    return (syUtilsRandIntRange(FTREMIXHITRECOIL_TOMATO_ODDS) == 0) ? nITKindTomato : FTREMIXHITRECOIL_DANGO_KIND;
}

/* _create_food: the food pops up from the joint, is pushed out of the walls and the audience laughs. */
static void ftRemixHitRecoilMakeFood(FTStruct *fp)
{
    Vec3f pos = { 0.0F, 0.0F, 0.0F };
    Vec3f vel = { 0.0F, FTREMIXHITRECOIL_FOOD_VEL_Y, 0.0F };
    GObj *item_gobj;

    if (fp->joints[FTREMIXHITRECOIL_FOOD_JOINT] == NULL)
    {
        return;
    }
    gmCollisionGetFighterPartsWorldPosition(fp->joints[FTREMIXHITRECOIL_FOOD_JOINT], &pos);
    pos.z = 0.0F;

    item_gobj = itManagerMakeItemSetupCommon(NULL, ftRemixHitRecoilGetFoodKind(), &pos, &vel, ITEM_FLAG_PARENT_GROUND);

    if (item_gobj == NULL)
    {
        return;
    }
    mpCommonRunItemCollisionDefault(item_gobj, fp->coll_data.p_translate, &fp->coll_data);
    syAudioPlayFGM(FTREMIXHITRECOIL_FOOD_FGM);
}

void ftRemixHitRecoilOnHit(FTStruct *fp, s32 hit_type)
{
    s32 kind = ftRemixHitRecoilGetKind(fp->fkind, fp->status_id, hit_type, ftRemixHitRecoilIsBodySlamInvincible(fp));

    if (kind == nFTRemixHitRecoilRecoil)
    {
        fp->motion_vars.flags.flag0 = 1;
        return;
    }
    if (kind == nFTRemixHitRecoilFood)
    {
        ftRemixHitRecoilMakeFood(fp);
    }
}

#endif
