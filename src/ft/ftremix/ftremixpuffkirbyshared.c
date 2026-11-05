/**
 * ftremixpuffkirbyshared.c - Smash Remix jigglypuffkirbyshared.asm (scope
 * JigglypuffKirbyShared) helpers, plus the pipe_turn table of marioshared.asm.
 *
 * The vanilla routines these patches touch get their clone ids as extra
 * `case REMIX_ID_*` labels or `||` terms inline under REGION_REMIX. Only the
 * data and the logic that is too long for that live here.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ftremix.h"
#include "ftremixhooks.h"
#include "ftremixfields.h"
#include "ftremixpipeturnrules.h"
#include <wp/weapon.h>
#include <ft/ftchar/ftkirby/ftkirby.h>

/* N64 halfword at 0x0AE2 (low half of the copysamus_charge_level word at 0x0AE0) that
 * Remix uses as the Kirby copy's flame ammo: `ori t3, r0, 0x0014` in the kirby_ammo_ patch. */
#define FTREMIX_KIRBY_BOWSER_COPY_AMMO 0x0014
#define FTREMIX_KIRBY_AMMO_N64_OFFSET 0x0AE0
#define FTREMIX_KIRBY_AMMO_HALFWORD_MASK 0x0000FFFF

/* Dedede.asm jump_multiplier_table, indexed by jumps_used (entries 0 and 1 are unused). */
static const f32 dFTRemixDededeJumpAerialMultipliers[] =
{
    0.0F,
    0.0F,
    68.0F,
    58.0F,
    52.0F,
    0.0F,
};

static WPDesc sFTRemixJKirbyCutterWeaponDesc;

/** Jump multiplier for Dedede and N Dedede clones on their 3rd or later jump. Returns 0 for an out-of-table index. */
f32 ftRemixDededeJumpAerialMultiplier(s32 jumps_used)
{
    if ((jumps_used < 0) || (jumps_used >= ARRAY_COUNT(dFTRemixDededeJumpAerialMultipliers)))
    {
        return 0.0F;
    }
    return dFTRemixDededeJumpAerialMultipliers[jumps_used];
}

/** TRUE for the fighters whose Character.pipe_turn table entry is set (12 ids, see ftremixpipeturnrules.h). They turn like Mario in pipes. */
sb32 ftRemixIsPipeTurnFkind(s32 fkind)
{
    return ftRemixPipeTurnRulesHasEntry(fkind);
}

/** Kirby copying Bowser fills the flame ammo (kirby_ammo_ patch). Keeps the upper half of the copy word. */
void ftRemixKirbyCopyAmmo(FTStruct *fp)
{
    s32 *copy_word;

    if (fp->passive_vars.kirby.copy_id != REMIX_ID_BOWSER)
    {
        return;
    }
    copy_word = ftRemixFighterWord(fp, FTREMIX_KIRBY_AMMO_N64_OFFSET);
    *copy_word = (*copy_word & ~FTREMIX_KIRBY_AMMO_HALFWORD_MASK) | FTREMIX_KIRBY_BOWSER_COPY_AMMO;
}

/**
 * Weapon descriptor for Kirby's cutter (kirby_special_struct_fix patch). J Kirby
 * spawns the cutter from his own main file; the vanilla descriptor is copied, not changed.
 */
WPDesc *ftRemixKirbyCutterWeaponDesc(FTStruct *fp, WPDesc *vanilla_desc)
{
    if ((fp->fkind != REMIX_ID_JKIRBY) || (fp->data->p_file_main == NULL))
    {
        return vanilla_desc;
    }
    sFTRemixJKirbyCutterWeaponDesc = *vanilla_desc;
    sFTRemixJKirbyCutterWeaponDesc.p_weapon = fp->data->p_file_main;

    return &sFTRemixJKirbyCutterWeaponDesc;
}

/* kirby_rock: J Kirby's Stone takes up to 50% damage and uses the J thresholds; other ids keep the vanilla value. */
#define FTREMIX_JKIRBY_STONE_HEALTH_MAX 50
#define FTREMIX_JKIRBY_STONE_HEALTH_MID 25
#define FTREMIX_JKIRBY_STONE_RELEASE_DURATION_MIN 14

/** Maximum Stone damage (kirby_rock). */
s32 ftRemixKirbyStoneHealthMax(FTStruct *fp, s32 vanilla_max)
{
    return (fp->fkind == REMIX_ID_JKIRBY) ? FTREMIX_JKIRBY_STONE_HEALTH_MAX : vanilla_max;
}

/** Stone HP below which the colour overlay flashes (kirby_rock_2). */
s32 ftRemixKirbyStoneHealthMid(FTStruct *fp, s32 vanilla_mid)
{
    return (fp->fkind == REMIX_ID_JKIRBY) ? FTREMIX_JKIRBY_STONE_HEALTH_MID : vanilla_mid;
}

/** Stone duration that must pass before the release check (kirby_rock_3). */
s32 ftRemixKirbyStoneReleaseDuration(FTStruct *fp, s32 vanilla_duration)
{
    if (fp->fkind != REMIX_ID_JKIRBY)
    {
        return vanilla_duration;
    }
    return FTKIRBY_STONE_DURATION_MAX - FTREMIX_JKIRBY_STONE_RELEASE_DURATION_MIN;
}

#endif
