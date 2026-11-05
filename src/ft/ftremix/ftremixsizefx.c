/**
 * ftremixsizefx.c - Smash Remix body size, phase (c) of Size.asm: effect GFX of the owner.
 *
 * Scales the Yoshi egg (shield, entry and lay) and shield, the ground impact wave (save_player_struct_ and update_routine_),
 * the Kirby inhale wind, the Thunder connect gfx and Falcon's punch by the owner's size multiplier. A multiplier of 1.0F
 * leaves the scale unchanged except for the Yoshi shield, which Size.asm sets to its Remix size, and the Yoshi egg lay, whose
 * z scale Size.asm sets to the egg size. The default egg size of each character (Character.yoshi_egg.table) is the
 * effect_size of the port egg descriptor. The Samus gun blast, Kirby Final Cutter and Falcon kick are in ftremixsizemovefx.c,
 * the entry gfx in ftremixsizeentrygfx.c.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ef/efmanager.h>
#include <ft/fighter.h>
#include <lb/lbcommon.h>
#include <lb/lbtypes.h>
#include <ft/ftcommon.h>
#include "fighter_registry.h"

/** Scales the three axes of a DObj by the same factor. */
static void ftRemixSizeScaleDObjUniform(DObj *dobj, f32 scale)
{
    dobj->scale.vec.f.x = scale;
    dobj->scale.vec.f.y = scale;
    dobj->scale.vec.f.z = scale;
}

/**
 * Size.asm adjust_size_egg_shield_ (RAM 0x801011A4): the egg shield is drawn at 1.5F times the owner multiplier.
 * Called from efManagerYoshiShieldProcDisplay before the draw.
 */
void ftRemixSizeScaleYoshiShield(GObj *effect_gobj, FTStruct *fp)
{
    f32 scale = FTREMIX_SIZE_YOSHI_SHIELD_SCALE * ftRemixSizeGetMultiplier(fp->player);

    ftRemixSizeScaleDObjUniform(DObjGetStruct(effect_gobj), scale);
}

/**
 * Size.asm adjust_size_egg_entry_ (RAM 0x80102F64): multiplies the uniform scale of a new entry egg by the owner multiplier.
 * Accepts a NULL egg, which efManagerYoshiEntryEggMakeEffect returns when no effect slot is free. Every entry egg goes through
 * here, so it also gets its Shield.asm alter_yoshi_egg_color_ palette (ftremixyoshiegg.c).
 */
void ftRemixSizeScaleEntryEgg(GObj *egg_gobj, FTStruct *fp)
{
    DObj *dobj;
    f32 scale;

    if (egg_gobj == NULL)
    {
        return;
    }
    dobj = DObjGetStruct(egg_gobj);
    scale = dobj->scale.vec.f.x * ftRemixSizeGetMultiplier(fp->player);

    ftRemixSizeScaleDObjUniform(dobj, scale);
    ftRemixYoshiEggColorEntryEgg(egg_gobj, fp);
}

/**
 * Size.asm adjust_size_egg_lay_ (RAM 0x8014C994, runs every frame the captured fighter is in the egg): the three scale axes of
 * the egg are the default egg size of the captured character (Character.yoshi_egg.table, the effect_size of the port egg
 * descriptor) times the multiplier of the captured fighter. The vanilla z scale of 1.0F is replaced as well. Does nothing
 * when the effect has no captured fighter.
 */
void ftRemixSizeScaleYoshiEggLay(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    FTStruct *fp;
    ftCommonYoshiEggDesc *egg_desc;

    if (ep->fighter_gobj == NULL)
    {
        return;
    }
    fp = ftGetStruct(ep->fighter_gobj);
    egg_desc = ftRemixYoshiEggGetDesc(fp->fkind);

    ftRemixSizeScaleDObjUniform(DObjGetStruct(effect_gobj), egg_desc->effect_size * ftRemixSizeGetMultiplier(fp->player));
}

/**
 * Size.asm save_player_struct_ (RAM 0x800EB0E8, 0x800EB0F8, 0x80141B40, 0x801551BC, 0x8014C40C): stores the owner in
 * the impact wave gfx, so its update can read the owner size. The owner is stored in the effect, not in the vanilla struct.
 */
void ftRemixSizeAttachGroundGfx(GObj *effect_gobj, GObj *fighter_gobj)
{
    EFStruct *ep;

    if (effect_gobj == NULL)
    {
        return;
    }
    ep = efGetStruct(effect_gobj);
    ep->fighter_gobj = fighter_gobj;
}

/**
 * Size.asm update_routine_ (RAM 0x8012DFC4 + 0x10): runs the impact wave update, then scales its DObj by the owner multiplier.
 * Impact waves without an owner (fighter_gobj NULL) are left as they are.
 */
void ftRemixSizeGroundGfxProcUpdate(GObj *effect_gobj)
{
    EFStruct *ep;
    DObj *dobj;
    f32 multiplier;

    efManagerImpactWaveProcUpdate(effect_gobj);

    ep = efGetStruct(effect_gobj);
    if (ep->fighter_gobj == NULL)
    {
        return;
    }
    multiplier = ftRemixSizeGetMultiplier(ftGetStruct(ep->fighter_gobj)->player);
    dobj = DObjGetStruct(effect_gobj);

    dobj->scale.vec.f.x *= multiplier;
    dobj->scale.vec.f.y *= multiplier;
    dobj->scale.vec.f.z *= multiplier;
}

/**
 * Size.asm adjust_suck_gfx_ (RAM 0x8010429C): the inhale wind offsets and its transform scale follow the owner multiplier.
 * Replaces the vanilla offsets of efManagerKirbyInhaleWindProcUpdate (lr * 800 on x, 230 on y).
 */
void ftRemixSizeKirbySuckTransform(LBTransform *xf, GObj *fighter_gobj)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    f32 multiplier = ftRemixSizeGetMultiplier(fp->player);

    xf->translate.x += fp->lr * 800.0F * multiplier;
    xf->translate.y += 230.0F * multiplier;

    xf->scale.x = multiplier;
    xf->scale.y = multiplier;
    xf->scale.z = multiplier;
}

/**
 * Size.asm pikachu.adjust_gfx_size_ (RAM 0x8010146C): the x and y scale of the Thunder connect gfx is the multiplier of the
 * fighter it hits (Pikachu DSP) or zaps (Lightning item). The asm reads that fighter from the caller register s1.
 * A NULL fighter keeps the gfx scale.
 */
void ftRemixSizeScaleThunderAmp(LBTransform *xf, GObj *fighter_gobj)
{
    f32 multiplier;

    if (fighter_gobj == NULL)
    {
        return;
    }
    multiplier = ftRemixSizeGetMultiplier(ftGetStruct(fighter_gobj)->player);

    xf->scale.x = multiplier;
    xf->scale.y = multiplier;
}

/**
 * Size.asm falcon punch render_routine_ (vanilla render 0x800CB4B0, installed by captainshared.asm): sets the punch
 * DObj scale to the owner multiplier, then runs lbCommonDObjScaleXProcDisplay.
 */
void ftRemixSizeFalconPunchProcDisplay(GObj *effect_gobj)
{
    EFStruct *ep = efGetStruct(effect_gobj);
    f32 multiplier = ftRemixSizeGetMultiplier(ftGetStruct(ep->fighter_gobj)->player);

    ftRemixSizeScaleDObjUniform(DObjGetStruct(effect_gobj), multiplier);
    lbCommonDObjScaleXProcDisplay(effect_gobj);
}

#endif
