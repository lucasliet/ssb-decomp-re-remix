/**
 * grremixbbh.c - Big Boo's Haunt Remix hazards (Hazards.asm bbh_setup_, boo_sound_). See grremixbbh.h.
 */

#ifdef PORT

#include "grremixbbh.h"
#include <sys/objdef.h>
#include <sys/objman.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <ef/efremixground.h>
#include <port_log.h>

/* Hazards.asm bbh_setup_ creates object 0x03F2, which is nGCCommonKindGround (1010). */
#define GRREMIX_BBH_OBJECT_KIND nGCCommonKindGround
/* Hazards.asm bbh_setup_ uses group 1, which is nGCCommonLinkIDGround. */
#define GRREMIX_BBH_LINK_ID nGCCommonLinkIDGround
/* Hazards.asm bbh_setup_ registers boo_sound_ with priority 4. */
#define GRREMIX_BBH_PROC_PRIORITY 4
/* Hazards.asm boo_sound_: FGM 0x0527 is the Boo laugh. */
#define GRREMIX_BBH_FGM_BOO_LAUGH 0x0527

/* Same vanilla FGM player declaration as grpupupu.c and gryamabuki.c. */
extern void *func_800269C0_275C0(u16 id);

/**
 * Returns TRUE when the current scene is the training mode, where Hazards.asm bbh_setup_ creates nothing.
 *
 * @return TRUE in training mode.
 */
static sb32 grRemixBbhIsTrainingScene(void)
{
    return gSCManagerSceneData.scene_curr == nSCKind1PTrainingMode;
}

/**
 * Per-frame routine of the Boo laugh object (Hazards.asm boo_sound_): laughs while the countdown of the ground actor
 * (sEFGroundActor make_wait, RAM 0x80131AD8), which makes the Boos of stages/bbh.asm, reads zero.
 *
 * @param gobj Boo laugh object created by grRemixBbhSetup.
 */
static void grRemixBbhBooSoundProcUpdate(GObj *gobj)
{
    if (efRemixGroundGetActorMakeWait() == 0)
    {
        func_800269C0_275C0(GRREMIX_BBH_FGM_BOO_LAUGH);
    }
}

/**
 * Creates the Boo laugh object of Big Boo's Haunt (Hazards.asm bbh_setup_), skipped in training mode.
 */
void grRemixBbhSetup(void)
{
    GObj *gobj;

    if (grRemixBbhIsTrainingScene())
    {
        return;
    }
    gobj = gcMakeGObjSPAfter(GRREMIX_BBH_OBJECT_KIND, NULL, GRREMIX_BBH_LINK_ID, GOBJ_PRIORITY_DEFAULT);
    if (gobj == NULL)
    {
        port_log("[remix-hazard] bbh: Boo laugh object not created, the GObj pool is full");
        return;
    }
    gcAddGObjProcess(gobj, grRemixBbhBooSoundProcUpdate, nGCProcessKindFunc, GRREMIX_BBH_PROC_PRIORITY);
}

#endif
