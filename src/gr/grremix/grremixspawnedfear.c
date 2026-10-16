/**
 * grremixspawnedfear.c - setup and acid routine of the Spawned Fear hazards
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm spawned_fear_acid_setup_, spawned_fear_acid_main_).
 * The Cacodemon spawner is in grremixcacodemonfactory.c and the Thunderball in grremixthunderball.c.
 */

#ifdef PORT

#include <stdint.h>
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <mp/mpcollision.h>
#include <sys/obj.h>
#include "gr/grremix/grremixcacodemonfactory.h"
#include "gr/grremix/grremixhazards.h"
#include "gr/grremix/grremixspawnedfear.h"
#include "gr/grremix/grremixthunderball.h"
#include "ft/ftremix/ftremixhooks.h"
#include "it/itremix/itremixcacodemon.h"
#include "sys/objdef.h"
#include "sys/objman.h"
#include "sys/objtypes.h"

#define GRREMIX_SPAWNED_FEAR_HEADER_BIAS 0x14 /* asm: the ground data pointer is the stage header + 0x14 */
#define GRREMIX_SPAWNED_FEAR_CACODEMON_TOKEN 0xE0 /* stage header: pointer to the Cacodemon file */
#define GRREMIX_SPAWNED_FEAR_THUNDERBALL_TOKEN 0x130 /* stage header: pointer to the Thunderball hitbox file */
#define GRREMIX_SPAWNED_FEAR_CACODEMON_TOP_BIAS 0x7F8 /* asm: the Cacodemon file top is the pointer - 0x7F8 */
#define GRREMIX_SPAWNED_FEAR_DOOM_ACID_CLIP 0x23 /* asm: clipping id of the doom acid surface, the byte at FTStruct 0xF7 */
#define GRREMIX_SPAWNED_FEAR_KILL_PERCENT 666 /* asm: percent at which a grounded fighter on the acid is KO'd */

/* asm: Hazards.asm ACID_DURATION, ACID_DAMAGE and the poison type 0 (doom acid) */
#define GRREMIX_SPAWNED_FEAR_POISON_DURATION 60
#define GRREMIX_SPAWNED_FEAR_POISON_DAMAGE 3
#define GRREMIX_SPAWNED_FEAR_POISON_TYPE 0

/** Poison part of spawned_fear_acid_main_: refreshes the doom acid poison; the damage and GFX run in ftremixpoison.c. */
static void grRemixSpawnedFearPoison(FTStruct *fp)
{
    /* REMIX-TODO(GFX): asm sets the player's colanim pointer to DOOM_ACID right away when it is 0 (Hazards.asm 0x0A28); the port applies it on the next poison tick. */
    ftRemixPoisonSet(fp->player, GRREMIX_SPAWNED_FEAR_POISON_DURATION, GRREMIX_SPAWNED_FEAR_POISON_DAMAGE, GRREMIX_SPAWNED_FEAR_POISON_TYPE);
}

/** KO part of spawned_fear_acid_main_: the Remix overlay state is cleared and the fighter is KO'd. */
static void grRemixSpawnedFearKnockOut(GObj *fighter_gobj)
{
    /* REMIX-TODO(overlay): asm clears the player's overlay routine and flag fields (0x0A20..0x0A30, 0x0A88) first. */
    ftCommonDeadDownSetStatus(fighter_gobj);
}

/** One fighter of the acid check: poison below the kill percent, KO at or above it, unless invincible. */
static void grRemixSpawnedFearCheckFighter(GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);

    if (fp->ga != nMPKineticsGround)
    {
        return;
    }
    if ((fp->coll_data.floor_flags & 0xFF) != GRREMIX_SPAWNED_FEAR_DOOM_ACID_CLIP)
    {
        return;
    }
    if (fp->percent_damage < GRREMIX_SPAWNED_FEAR_KILL_PERCENT)
    {
        grRemixSpawnedFearPoison(fp);
        return;
    }
    if (fp->star_invincible_tics != 0)
    {
        return;
    }
    grRemixSpawnedFearKnockOut(fighter_gobj);
}

/** spawned_fear_acid_main_ (per-frame routine): checks every fighter standing on the doom acid. */
static void grRemixSpawnedFearAcidProcUpdate(GObj *routine_gobj)
{
    GObj *fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter];

    ftRemixPoisonAdvanceTimer();
    while (fighter_gobj != NULL)
    {
        grRemixSpawnedFearCheckFighter(fighter_gobj);
        fighter_gobj = fighter_gobj->link_next;
    }
}

/** Creates the per-frame acid routine object. */
static void grRemixSpawnedFearMakeAcidRoutine(void)
{
    GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);

    gcAddGObjProcess(gobj, grRemixSpawnedFearAcidProcUpdate, nGCProcessKindFunc, 3);
}

/**
 * Hands the Cacodemon and Thunderball file pointers of the stage header to their ports.
 * The pointers are relocation tokens; PORT_RESOLVE turns them into real addresses.
 */
static void grRemixSpawnedFearSetupFiles(void)
{
    void *header = (void *) ((uintptr_t) gMPCollisionGroundData - GRREMIX_SPAWNED_FEAR_HEADER_BIAS);
    u32 cacodemon_token = *(u32 *) ((uintptr_t) header + GRREMIX_SPAWNED_FEAR_CACODEMON_TOKEN);
    u32 thunderball_token = *(u32 *) ((uintptr_t) header + GRREMIX_SPAWNED_FEAR_THUNDERBALL_TOKEN);

    itRemixCacodemonSetFileTop((void *) ((uintptr_t) PORT_RESOLVE(cacodemon_token) - GRREMIX_SPAWNED_FEAR_CACODEMON_TOP_BIAS));
    grRemixThunderballSetFile(PORT_RESOLVE(thunderball_token));
}

void grRemixSpawnedFearSetup(void)
{
    if (!grRemixHazardsAreOn())
    {
        return;
    }
    grRemixSpawnedFearMakeAcidRoutine();
    grRemixSpawnedFearSetupFiles();
    grRemixCacodemonFactoryMake();
}

#endif
