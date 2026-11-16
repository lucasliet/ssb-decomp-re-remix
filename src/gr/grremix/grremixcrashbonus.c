/**
 * grremixcrashbonus.c - 1-up easter egg of Crash's Break the Targets and Board the Platforms. See grremixcrashbonus.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "grremixcrashbonus.h"
#include "grremixcrashbonusrules.h"
#include <ft/fighter.h>
#include <mp/map.h>
#include <sys/audio.h>
#include <sys/objman.h>
#include <port_log.h>

extern void* func_800269C0_275C0(u16 id);

static const GRRemixCrashBonusOneUp *sGRRemixCrashBonusOneUp;
static sb32 sGRRemixCrashBonusIsCollected;

/** Shrinks the 1-up object to almost nothing (asm: the three scale words become 0x38D10000). */
static void grRemixCrashBonusShrinkOneUp(void)
{
    DObj *one_up_dobj = gMPCollisionYakumonoDObjs->dobjs[sGRRemixCrashBonusOneUp->dobj_index];
    union
    {
        u32 word;
        f32 value;

    } scale;

    if (one_up_dobj == NULL)
    {
        port_log("[remix-hazard] crash bonus: no yakumono DObj %d for the 1-up", sGRRemixCrashBonusOneUp->dobj_index);
        return;
    }
    scale.word = GRREMIX_CRASH_BONUS_ONE_UP_SCALE_WORD;
    one_up_dobj->scale.vec.f.x = scale.value;
    one_up_dobj->scale.vec.f.y = scale.value;
    one_up_dobj->scale.vec.f.z = scale.value;
}

/** Collects the 1-up: shrink, sound and the Crash Bash Loading song as the stage song (asm _collect_one_up). */
static void grRemixCrashBonusCollectOneUp(void)
{
    sGRRemixCrashBonusIsCollected = TRUE;
    grRemixCrashBonusShrinkOneUp();
    func_800269C0_275C0(GRREMIX_CRASH_BONUS_FGM_ONE_UP);
    gMPCollisionBGMDefault = GRREMIX_CRASH_BONUS_BGM_ONE_UP;
    syAudioPlayBGM(0, GRREMIX_CRASH_BONUS_BGM_ONE_UP);
}

/**
 * Run function of the watcher (asm crash_btt_main / crash_btp_main): the first fighter on the 1-up's floor line
 * collects it, once per stage load.
 *
 * @param gobj Watcher object.
 */
static void grRemixCrashBonusFuncRun(GObj *gobj)
{
    GObj *fighter_gobj;

    if (sGRRemixCrashBonusIsCollected)
    {
        return;
    }
    for (fighter_gobj = gGCCommonLinks[nGCCommonLinkIDFighter]; fighter_gobj != NULL; fighter_gobj = fighter_gobj->link_next)
    {
        if (grRemixCrashBonusRulesIsCollected(sGRRemixCrashBonusOneUp, ftGetStruct(fighter_gobj)->coll_data.floor_line_id))
        {
            grRemixCrashBonusCollectOneUp();
            return;
        }
    }
}

void grRemixCrashBonusMake(s32 gkind)
{
    sGRRemixCrashBonusOneUp = grRemixCrashBonusRulesFind(gkind);
    if (sGRRemixCrashBonusOneUp == NULL)
    {
        return;
    }
    sGRRemixCrashBonusIsCollected = FALSE;

    /* The asm passes crash_bt*_main as the run function of object 0x3F2, group 1 (Render.CREATE_OBJECT_). */
    if (gcMakeGObjSPAfter(nGCCommonKindGround, grRemixCrashBonusFuncRun, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT) == NULL)
    {
        port_log("[remix-hazard] crash bonus: watcher object not created, the GObj pool is full");
    }
}

#endif
