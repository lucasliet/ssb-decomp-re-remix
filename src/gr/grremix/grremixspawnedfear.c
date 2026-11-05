/**
 * grremixspawnedfear.c - setup and acid routine of the Spawned Fear hazards
 * (.toolchain/pkgs/smashremix-src/src/Hazards.asm spawned_fear_acid_setup_, spawned_fear_acid_main_).
 * The Cacodemon spawner is in grremixcacodemonfactory.c and the Thunderball in grremixthunderball.c.
 */

#ifdef PORT

#include <stdint.h>
#include <ft/fighter.h>
#include <ft/ftcommon/ftcommonfunctions.h>
#include <ft/ftparam.h>
#include <gm/gmremixcolscripts.h>
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
#define GRREMIX_SPAWNED_FEAR_DOOM_ACID_COLANIM 0x79 /* asm: GFXRoutine.DOOM_ACID, the same colanim id as ftremixpoison.c */

/* asm: Hazards.asm ACID_DURATION, ACID_DAMAGE and the poison type 0 (doom acid) */
#define GRREMIX_SPAWNED_FEAR_POISON_DURATION 60
#define GRREMIX_SPAWNED_FEAR_POISON_DAMAGE 3
#define GRREMIX_SPAWNED_FEAR_POISON_TYPE 0

/**
 * Starts the doom acid colanim when no colour animation override is active (asm 0x0A28 is zero), then refreshes the
 * doom acid poison. The damage and the rest of the GFX run in ftremixpoison.c.
 */
static void grRemixSpawnedFearPoison(GObj *fighter_gobj, FTStruct *fp)
{
    if (gmRemixColScriptsGetOverride(fp->player) == 0)
    {
        gmRemixColScriptsSetOverride(fp->player, GRREMIX_SPAWNED_FEAR_DOOM_ACID_COLANIM);
        ftParamCheckSetFighterColAnimID(fighter_gobj, GRREMIX_SPAWNED_FEAR_DOOM_ACID_COLANIM, 0);
    }
    ftRemixPoisonSet(fp->player, GRREMIX_SPAWNED_FEAR_POISON_DURATION, GRREMIX_SPAWNED_FEAR_POISON_DAMAGE, GRREMIX_SPAWNED_FEAR_POISON_TYPE);
}

/**
 * Clears the words the asm calls the Practice overlay routine, flag and current overlay (Hazards.asm 8554..8558). They
 * are vanilla FTStruct fields: 0x0A20 p_loop_sfx, 0x0A24 loop_sfx_id, 0x0A30 colanim.cs[0].p_subroutine[0] and the word at
 * 0x0A88 with the colanim use-flags and skeleton id (struct_offsets.txt). 0x0A28, the gfx routine, is cleared separately
 * through gmRemixColScriptsSetOverride. The overlay colour of ftremixoverlay.c is the colanim colour 1 use-flag.
 */
static void grRemixSpawnedFearClearOverlay(FTStruct *fp)
{
    fp->p_loop_sfx = NULL;
    fp->loop_sfx_id = 0;
    fp->colanim.cs[0].p_subroutine[0] = NULL;
    fp->colanim.is_use_color1 = FALSE;
    fp->colanim.is_use_light = FALSE;
    fp->colanim.is_use_color2 = FALSE;
    fp->colanim.skeleton_id = 0;
}

/**
 * KO part of spawned_fear_acid_main_: the overlay words and the gfx routine (colanim override) are cleared, then the
 * fighter is KO'd.
 */
static void grRemixSpawnedFearKnockOut(GObj *fighter_gobj, FTStruct *fp)
{
    grRemixSpawnedFearClearOverlay(fp);
    gmRemixColScriptsSetOverride(fp->player, 0);
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
        grRemixSpawnedFearPoison(fighter_gobj, fp);
        return;
    }
    if (fp->star_invincible_tics != 0)
    {
        return;
    }
    grRemixSpawnedFearKnockOut(fighter_gobj, fp);
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
