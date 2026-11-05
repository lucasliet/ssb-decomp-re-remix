#if defined(PORT) && defined(REGION_REMIX)
#include <ft/fighter.h>
#include <wp/weapon.h>
#include "wp/wpmario/wpmariofireball.h"
#include "ft/ftremix/ftremixfields.h"
#include "ft/ftremix/ftremixhooks.h"
#include "fighter_registry.h"
#include "remix/remix_roster_data.h"

extern void *func_800269C0_275C0(u16 id);
extern u8 dEFManagerDamageNormalLightIDs[];

#define WP_REMIX_CAPSULE_GFX_DR_MARIO 0x81 /* DR_MARIO_EFFECT_GFX_INST_ID, fireball.asm */
#define WP_REMIX_CAPSULE_GFX_DR_LUIGI 0x97 /* DR_LUIGI_EFFECT_GFX_INST_ID, fireball.asm */

#define WP_REMIX_CAPSULE_TYPE       4     /* Capsule.TYPE in fireball.asm (attack_coll.element) */
#define WP_REMIX_BOOK_TYPE          3     /* Book.TYPE in fireball.asm (attack_coll.element) */
#define WP_REMIX_CAPSULE_FGM        0x2C7 /* Capsule.FGM */
#define WP_REMIX_BOOK_FGM           0x1F  /* Book.FGM */
#define WP_REMIX_BANJO_EGG_KIND     0x21  /* Projectile.id.BANJO_EGG */
#define WP_REMIX_EGG_BOUNCE_FGM     0x506 /* egg bounce sfx, fireball.asm bounce_effect_ */
#define WP_REMIX_KIRBY_DRM_HAT      0x10  /* Kirby's Dr. Mario hat id, fireball.asm Capsule.capsule_subroutine_ */
#define WP_REMIX_PIANO_BONUS_AMMO   0x0ADC /* N64 passive_vars word holding Piano's bonus_ammo */
#define WP_REMIX_EGG_MIN_VEL_TOUCHING -1280.0F /* -HIGH value, fireball.asm bounce_destroy_check */
#define WP_REMIX_EGG_MIN_VEL_FREE     20.0F

/* Maps a fighter kind (or the Kirby copy id) to its fireball index; -1 for kinds that have no fireball.
 * NDRM and MLUIGI copy the fireball table entry of their parent (Character.asm:298 add_to_table(fireball, ...)). */
s32 wpRemixMarioFireballIndexForFkind(s32 fkind)
{
    switch (fkind)
    {
    case nFTKindMario:
    case nFTKindMMario:
    case nFTKindNMario:
    case REMIX_ID_NDRM:
        return WP_MARIO_FIREBALL_INDEX_MARIO;
    case nFTKindLuigi:
    case nFTKindNLuigi:
    case REMIX_ID_MLUIGI:
        return WP_MARIO_FIREBALL_INDEX_LUIGI;
    case REMIX_ID_DRM:
        return WP_MARIO_FIREBALL_INDEX_CAPSULE;
    case REMIX_ID_JMARIO:
        return WP_MARIO_FIREBALL_INDEX_JMARIO;
    case REMIX_ID_JLUIGI:
        return WP_MARIO_FIREBALL_INDEX_JLUIGI;
    case REMIX_ID_DRL:
        return WP_MARIO_FIREBALL_INDEX_DRL;
    default:
        return -1;
    }
}

/* The projectile file is the special1 file of the owner fighter (Character.<X>_file_6_ptr in the asm). */
void **wpRemixMarioFireballFile(s32 file_fkind)
{
    return port_fighter_data(file_fkind)->p_file_special1;
}

/* Capsule palette: random index in [palette_base, palette_base + palette_range). Kirby uses its hat instead of the entry. */
s32 wpRemixMarioFireballPaletteId(FTStruct *fp, s32 palette_base, s32 palette_range)
{
    s32 base = palette_base;

    if ((fp->fkind == REMIX_ID_KIRBY) || (fp->fkind == REMIX_ID_JKIRBY))
    {
        base = (*ftRemixFighterKirbyHat(fp) == WP_REMIX_KIRBY_DRM_HAT) ? 0 : 3;
    }
    return base + syUtilsRandIntRange(palette_range);
}

/* Banjo's egg does not die on bounce: 20.0 when it is free of map contact, -1280.0 (never dies) otherwise. */
f32 wpRemixMarioFireballMinVel(WPStruct *wp, f32 vanilla_min_vel)
{
    if (wp->kind != WP_REMIX_BANJO_EGG_KIND)
    {
        return vanilla_min_vel;
    }
    if ((wp->coll_data.mask_curr & 0x0021) == 0)
    {
        return WP_REMIX_EGG_MIN_VEL_FREE;
    }
    return WP_REMIX_EGG_MIN_VEL_TOUCHING;
}

/* Normal hit GFX of book hits: the vanilla instruction id, fireball.asm hit_effect_ _book. */
static void wpRemixMarioFireballNormalHitGfx(WPStruct *wp, Vec3f *pos)
{
    efManagerDamageNormalLightMakeEffect(pos, 0, wp->hit_normal_damage, FALSE);
}

/* Capsule hit GFX id: 0x97 (Dr. Luigi) when the effective character is DRL, else 0x81 (Dr. Mario). The asm flag
 * (projectile 0x2A0) is set from the copied power of Kirby and from the fighter otherwise (fireball.asm _drl_check). */
static u8 wpRemixMarioFireballCapsuleGfxId(WPStruct *wp)
{
    FTStruct *owner_fp;
    s32 char_id;

    if (wp->owner_gobj == NULL)
    {
        return WP_REMIX_CAPSULE_GFX_DR_MARIO;
    }
    owner_fp = ftGetStruct(wp->owner_gobj);
    char_id = owner_fp->fkind;
    if ((char_id == REMIX_ID_KIRBY) || (char_id == REMIX_ID_JKIRBY))
    {
        char_id = owner_fp->passive_vars.kirby.copy_id;
    }
    return (char_id == REMIX_ID_DRL) ? WP_REMIX_CAPSULE_GFX_DR_LUIGI : WP_REMIX_CAPSULE_GFX_DR_MARIO;
}

/* Capsule hit GFX: the asm writes the Dr. Mario or Dr. Luigi id into the normal hit table for this one call and
 * restores the vanilla id after it (fireball.asm hit_effect_ _capsule, 0x8012DF20 is entry 0). */
static void wpRemixMarioFireballCapsuleHitGfx(WPStruct *wp, Vec3f *pos)
{
    u8 vanilla_gfx_id = dEFManagerDamageNormalLightIDs[0];

    dEFManagerDamageNormalLightIDs[0] = wpRemixMarioFireballCapsuleGfxId(wp);
    efManagerDamageNormalLightMakeEffect(pos, 0, wp->hit_normal_damage, FALSE);
    dEFManagerDamageNormalLightIDs[0] = vanilla_gfx_id;
}

/* Capsule and book hits play their own FGM and GFX and skip the vanilla sparkle. Returns FALSE for fireball and egg. */
sb32 wpRemixMarioFireballHitEffect(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    Vec3f *pos = &DObjGetStruct(weapon_gobj)->translate.vec.f;

    if (wp->attack_coll.element == WP_REMIX_CAPSULE_TYPE)
    {
        func_800269C0_275C0(WP_REMIX_CAPSULE_FGM);
        wpRemixMarioFireballCapsuleHitGfx(wp, pos);
        return TRUE;
    }
    if (wp->attack_coll.element == WP_REMIX_BOOK_TYPE)
    {
        func_800269C0_275C0(WP_REMIX_BOOK_FGM);
        efManagerDamageSpawnMDustMakeEffect(pos, wp->lr);
        wpRemixMarioFireballNormalHitGfx(wp, pos);
        return TRUE;
    }
    return FALSE;
}

/* Capsule and book bounces play small smoke, the egg plays its bounce sfx; both skip the fire grind. Returns FALSE for fireball. */
sb32 wpRemixMarioFireballBounceEffect(GObj *weapon_gobj)
{
    WPStruct *wp = wpGetStruct(weapon_gobj);
    Vec3f *pos = &DObjGetStruct(weapon_gobj)->translate.vec.f;

    if ((wp->attack_coll.element == WP_REMIX_CAPSULE_TYPE) || (wp->attack_coll.element == WP_REMIX_BOOK_TYPE))
    {
        /* fireball.asm bounce_effect_ calls 0x800FF048 with a1/a2 left by the caller; lr and 1.0 are assumed. */
        efManagerDustLightMakeEffect(pos, wp->lr, 1.0F);
        return TRUE;
    }
    if (wp->kind == WP_REMIX_BANJO_EGG_KIND)
    {
        func_800269C0_275C0(WP_REMIX_EGG_BOUNCE_FGM);
        return TRUE;
    }
    return FALSE;
}

/* Piano's bonus_ammo is truncated to an int and spends the whole count on extra books; otherwise one book. */
static s32 wpRemixPianoBookCount(FTStruct *fp)
{
    s32 bonus_ammo;

    if (fp->fkind != REMIX_ID_PIANO)
    {
        return 1;
    }
    bonus_ammo = (s32)*ftRemixFighterFloat(fp, WP_REMIX_PIANO_BONUS_AMMO);
    if (bonus_ammo <= 0)
    {
        return 1;
    }
    *ftRemixFighterWord(fp, WP_REMIX_PIANO_BONUS_AMMO) = 0;
    return bonus_ammo;
}

/* Book angle for the given loop iteration: 60 degrees +/- 25 degrees, spread grows 25% per extra book. */
static f32 wpRemixPianoBookAngle(s32 iteration)
{
    f32 random_step = (f32)(syUtilsRandIntRange(101) - 50) * 0.01F;
    f32 intensity = 1.0F + (0.25F * (f32)iteration);

    return 1.0472F + (0.436332F * (random_step * intensity));
}

/* Book.create_: spawns one book per count, each at its own spread angle. Returns the first book, or NULL. */
GObj* wpRemixPianoBookMake(GObj *fighter_gobj, Vec3f *pos)
{
    FTStruct *fp = ftGetStruct(fighter_gobj);
    s32 count = wpRemixPianoBookCount(fp);
    GObj *first_book = NULL;
    GObj *book;
    s32 i;

    for (i = 0; i < count; i++)
    {
        book = wpMarioFireballMakeWeaponAtAngle(fighter_gobj, pos, WP_MARIO_FIREBALL_INDEX_BOOK, wpRemixPianoBookAngle(i));
        if (first_book == NULL)
        {
            first_book = book;
        }
    }
    return first_book;
}
#endif
