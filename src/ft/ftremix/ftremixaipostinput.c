#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>
#include <mp/mpdef.h>
#include <sys/taskman.h>
#include "ft/ftcomputer.h"
#include "ft/ftremix/ftremixailv10rules.h"
#include "ft/ftremix/ftremixaipostcommon.h"
#include "ft/ftremix/ftremixaipostinput.h"
#include "ft/ftremix/ftremixaipostrules.h"
#include "remix/remix_roster_data.h"

/**
 * Character.get_hitbox_collision_flags_ masked with 0x00F0 (Character.asm:3322): any interact bit of an active attack
 * collision of the fighter.
 */
static sb32 ftRemixAiPostAttackCollHasInteraction(FTAttackColl *attack_coll)
{
    s32 i;

    if (attack_coll->attack_state == 0)
    {
        return FALSE;
    }
    for (i = 0; i < ARRAY_COUNT(attack_coll->attack_records); i++)
    {
        GMAttackRecord *record = &attack_coll->attack_records[i];

        if (record->victim_flags.is_interact_hurt || record->victim_flags.is_interact_shield ||
            record->victim_flags.is_interact_reflect || record->victim_flags.is_interact_absorb)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/** Tells if any attack collision of the fighter has hit, shielded, reflected or been absorbed this frame. */
static sb32 ftRemixAiPostHitboxCollided(FTStruct *fp)
{
    s32 i;

    for (i = 0; i < ARRAY_COUNT(fp->attack_colls); i++)
    {
        if (ftRemixAiPostAttackCollHasInteraction(&fp->attack_colls[i]))
        {
            return TRUE;
        }
    }
    return FALSE;
}

void ftRemixAiPostProcessMewtwo(FTStruct *fp)
{
    FTStruct *target_fp;

    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_INPUT_DSP) == FALSE)
    {
        return;
    }
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (ftRemixAiPostMewtwoSkipsDsp(fp->lr, target_fp->lr, target_fp->ga, target_fp->status_id))
    {
        ftRemixAiPostSetNullCommand(fp);
    }
}

void ftRemixAiPostProcessPuff(FTStruct *fp)
{
    FTStruct *target_fp;

    if (ftRemixAiIsLevelTen(fp->level) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_INPUT_DSP) == FALSE)
    {
        return;
    }
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (ftRemixAiPostPuffSkipsDsp(target_fp->status_id, target_fp->percent_damage))
    {
        ftRemixAiPostSetNullCommand(fp);
    }
}

void ftRemixAiPostProcessKirby(FTStruct *fp)
{
    sb32 is_grounded;

    if (ftRemixAiIsLevelTen(fp->level) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostIsKirbyStone(fp->status_id) == FALSE)
    {
        return;
    }
    is_grounded = (fp->ga == nMPKineticsGround) ? TRUE : FALSE;

    if (ftRemixAiPostKirbyCancelsStone(ftRemixAiPostHasFloorBelow(fp), is_grounded, ftRemixAiPostHitboxCollided(fp)) == FALSE)
    {
        return;
    }
    fp->input.cp.button_inputs = (u16)ftRemixAiPostKirbyMashButtons(fp->input.cp.button_inputs, (s32)dSYTaskmanUpdateCount);
    ftRemixAiPostSetNullCommand(fp);
}

void ftRemixAiPostProcessYoshi(FTStruct *fp)
{
    FTStruct *target_fp;

    if (ftRemixAiIsLevelTen(fp->level) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_INPUT_NSP) == FALSE)
    {
        return;
    }
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (ftRemixAiPostYoshiSkipsNsp(target_fp->status_id))
    {
        ftRemixAiPostSetNullCommand(fp);
    }
}

void ftRemixAiPostProcessLink(FTStruct *fp)
{
    if ((fp->fkind != REMIX_ID_YLINK) && (ftRemixAiIsLevelTen(fp->level) == FALSE))
    {
        return;
    }
    if (ftRemixAiPostCommandIs(fp, FTREMIX_AI_POST_INPUT_NSP) == FALSE)
    {
        return;
    }
    if (fp->passive_vars.link.boomerang_gobj != NULL)
    {
        ftRemixAiPostSetNullCommand(fp);
    }
}

void ftRemixAiPostProcessDedede(FTStruct *fp)
{
    FTStruct *target_fp;

    if (ftRemixAiPostIsDededeCharge(fp->status_id) == FALSE)
    {
        return;
    }
    if (ftRemixAiPostHasFloorBelow(fp) == FALSE)
    {
        fp->input.cp.button_inputs = 0;
        return;
    }
    target_fp = ftRemixAiPostGetTarget(fp);

    if (target_fp == NULL)
    {
        return;
    }
    if (ftRemixAiPostDededeHoldsB(ftRemixAiPostDistanceToTarget(fp, target_fp)))
    {
        fp->input.cp.button_inputs |= FTREMIX_AI_POST_BUTTON_B;
        return;
    }
    fp->input.cp.button_inputs = 0;
}

#endif
