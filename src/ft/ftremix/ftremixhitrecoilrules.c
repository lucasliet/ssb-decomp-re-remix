/**
 * ftremixhitrecoilrules.c - pure rules of the hit recoil patches, see ftremixhitrecoilrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhitrecoilrules.h"
#include "remix/remix_roster_data.h"

#define FTREMIXHITRECOIL_STATUS_WARIO_NSP_GROUND 0xDF
#define FTREMIXHITRECOIL_STATUS_WARIO_NSP_AIR 0xE0
#define FTREMIXHITRECOIL_STATUS_KIRBY_WARIO_NSP_GROUND 0x138
#define FTREMIXHITRECOIL_STATUS_KIRBY_WARIO_NSP_AIR 0x139
#define FTREMIXHITRECOIL_STATUS_SHEIK_DSP_ATTACK 0xF1
#define FTREMIXHITRECOIL_STATUS_BANJO_USP_ATTACK 0xF0
#define FTREMIXHITRECOIL_STATUS_EBI_NSP_END 0xEB
#define FTREMIXHITRECOIL_STATUS_EBI_NSP_END_AIR 0xEE
#define FTREMIXHITRECOIL_STATUS_KIRBY_EBI_NSP_END 0x1A7
#define FTREMIXHITRECOIL_STATUS_KIRBY_EBI_NSP_END_AIR 0x1AA

static int ftRemixHitRecoilIsKirby(int fkind)
{
    return (fkind == REMIX_ID_KIRBY) || (fkind == REMIX_ID_JKIRBY);
}

/* _check_type: a hurtbox or shield contact always recoils, a clang only when the shoulder is not invincible. */
static int ftRemixHitRecoilGetBodySlamKind(int hit_type, int is_shoulder_invincible)
{
    if ((hit_type == FTREMIXHITRECOIL_HIT_HURTBOX) || (hit_type == FTREMIXHITRECOIL_HIT_SHIELD))
    {
        return nFTRemixHitRecoilRecoil;
    }
    if ((hit_type != FTREMIXHITRECOIL_HIT_CLANG) || is_shoulder_invincible)
    {
        return nFTRemixHitRecoilNone;
    }
    return nFTRemixHitRecoilRecoil;
}

/* _food_check_type: only a hurtbox contact makes food. */
static int ftRemixHitRecoilGetFoodKind(int hit_type)
{
    return (hit_type == FTREMIXHITRECOIL_HIT_HURTBOX) ? nFTRemixHitRecoilFood : nFTRemixHitRecoilNone;
}

static int ftRemixHitRecoilGetKirbyKind(int status_id, int hit_type, int is_shoulder_invincible)
{
    if ((status_id == FTREMIXHITRECOIL_STATUS_KIRBY_EBI_NSP_END) || (status_id == FTREMIXHITRECOIL_STATUS_KIRBY_EBI_NSP_END_AIR))
    {
        return ftRemixHitRecoilGetFoodKind(hit_type);
    }
    if ((status_id == FTREMIXHITRECOIL_STATUS_KIRBY_WARIO_NSP_GROUND) || (status_id == FTREMIXHITRECOIL_STATUS_KIRBY_WARIO_NSP_AIR))
    {
        return ftRemixHitRecoilGetBodySlamKind(hit_type, is_shoulder_invincible);
    }
    return nFTRemixHitRecoilNone;
}

int ftRemixHitRecoilGetKind(int fkind, int status_id, int hit_type, int is_shoulder_invincible)
{
    if (ftRemixHitRecoilIsKirby(fkind))
    {
        return ftRemixHitRecoilGetKirbyKind(status_id, hit_type, is_shoulder_invincible);
    }
    switch (fkind)
    {
    case REMIX_ID_WARIO:
        if ((status_id == FTREMIXHITRECOIL_STATUS_WARIO_NSP_GROUND) || (status_id == FTREMIXHITRECOIL_STATUS_WARIO_NSP_AIR))
        {
            return ftRemixHitRecoilGetBodySlamKind(hit_type, is_shoulder_invincible);
        }
        return nFTRemixHitRecoilNone;

    case REMIX_ID_SHEIK:
        return (status_id == FTREMIXHITRECOIL_STATUS_SHEIK_DSP_ATTACK) ? nFTRemixHitRecoilRecoil : nFTRemixHitRecoilNone;

    case REMIX_ID_BANJO:
        return (status_id == FTREMIXHITRECOIL_STATUS_BANJO_USP_ATTACK) ? nFTRemixHitRecoilRecoil : nFTRemixHitRecoilNone;

    case REMIX_ID_EBI:
        if ((status_id == FTREMIXHITRECOIL_STATUS_EBI_NSP_END) || (status_id == FTREMIXHITRECOIL_STATUS_EBI_NSP_END_AIR))
        {
            return ftRemixHitRecoilGetFoodKind(hit_type);
        }
        return nFTRemixHitRecoilNone;

    default:
        return nFTRemixHitRecoilNone;
    }
}

int ftRemixHitRecoilIsBodySlam(int fkind, int status_id)
{
    if (fkind == REMIX_ID_WARIO)
    {
        return (status_id == FTREMIXHITRECOIL_STATUS_WARIO_NSP_GROUND) || (status_id == FTREMIXHITRECOIL_STATUS_WARIO_NSP_AIR);
    }
    if (ftRemixHitRecoilIsKirby(fkind))
    {
        return (status_id == FTREMIXHITRECOIL_STATUS_KIRBY_WARIO_NSP_GROUND) || (status_id == FTREMIXHITRECOIL_STATUS_KIRBY_WARIO_NSP_AIR);
    }
    return 0;
}

#endif
