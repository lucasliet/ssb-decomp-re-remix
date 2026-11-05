/**
 * ftremixpikashared.c - Smash Remix pikashared.asm and JPika.asm: E Pikachu and J Pikachu reuse the Pikachu code paths.
 * Remix swaps the Thunder and Thunder Jolt weapon structures for copies whose file pointer is the clone's own file; the
 * port keeps the vanilla WPDesc and swaps the file pointer of a copy, so the weapon attributes come from the clone's loaded
 * file instead of the (possibly not loaded) Pikachu globals. The values are in ftremixpikarules.h.
 *
 * Remix file n is the FTData slot of that number (docs/remix/fighters.md): file 1 p_file_main, file 6 p_file_special1.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixpikashared.h"
#include "ft/ftremix/ftremixpikarules.h"
#include "ft/ftremix/ftremixnessshared.h"
#include "fighter_registry.h"

/** Thunder Jolt weapon words beyond the vanilla WPStruct: `line_type` and `rotate` are weapon_vars.thunder_jolt. */
typedef struct WPRemixThunderJoltVars
{
    s32 line_type;
    Vec3f rotate;
    s32 power_kind;

} WPRemixThunderJoltVars;

_Static_assert(sizeof(WPRemixThunderJoltVars) <= sizeof(((WPStruct*)0)->weapon_vars), "WPRemixThunderJoltVars must fit in WPStruct.weapon_vars");

typedef enum FTRemixPikaFile
{
    nFTRemixPikaFileMain,
    nFTRemixPikaFileModel,
    nFTRemixPikaFileSpecial1,
    nFTRemixPikaFileSpecial3

} FTRemixPikaFile;

/** The loaded file `which` of `kind`, or `vanilla_file` if it is not loaded. */
static void **ftRemixPikaSharedLoadedFile(s32 kind, FTRemixPikaFile which, void **vanilla_file)
{
    FTData *data = port_fighter_data(kind);
    void **file;

    if (data == NULL)
    {
        return vanilla_file;
    }
    switch (which)
    {
    case nFTRemixPikaFileMain:
        file = data->p_file_main;
        break;

    case nFTRemixPikaFileModel:
        file = data->p_file_model;
        break;

    case nFTRemixPikaFileSpecial1:
        file = data->p_file_special1;
        break;

    default:
        file = data->p_file_special3;
        break;
    }

    if ((file == NULL) || (*file == NULL))
    {
        return vanilla_file;
    }
    return file;
}

void ftRemixPikaSharedThunderDesc(FTStruct *fp, WPDesc *desc)
{
    if (!ftRemixPikaRulesThunderUsesOwnFile(fp->fkind))
    {
        return;
    }
    desc->p_weapon = ftRemixPikaSharedLoadedFile(fp->fkind, nFTRemixPikaFileMain, desc->p_weapon);
}

/** Points `desc` at the file 6 of `power_kind` if that is J Pikachu. */
static void ftRemixPikaSharedSetJoltFile(s32 power_kind, WPDesc *desc)
{
    if (!ftRemixPikaRulesThunderJoltUsesOwnFile(power_kind))
    {
        return;
    }
    desc->p_weapon = ftRemixPikaSharedLoadedFile(power_kind, nFTRemixPikaFileSpecial1, desc->p_weapon);
}

void ftRemixPikaSharedThunderJoltDesc(FTStruct *fp, WPDesc *desc)
{
    ftRemixPikaSharedSetJoltFile(ftRemixNessSharedPowerKind(fp), desc);
}

void ftRemixPikaSharedThunderJoltSetKind(WPStruct *wp, FTStruct *fp)
{
    ((WPRemixThunderJoltVars*)&wp->weapon_vars)->power_kind = ftRemixNessSharedPowerKind(fp);
}

void ftRemixPikaSharedThunderJoltGroundDesc(WPStruct *prev_wp, WPDesc *desc)
{
    ftRemixPikaSharedSetJoltFile(((WPRemixThunderJoltVars*)&prev_wp->weapon_vars)->power_kind, desc);
}

void ftRemixPikaSharedThunderJoltInheritKind(WPStruct *new_wp, WPStruct *prev_wp)
{
    ((WPRemixThunderJoltVars*)&new_wp->weapon_vars)->power_kind = ((WPRemixThunderJoltVars*)&prev_wp->weapon_vars)->power_kind;
}

void **ftRemixPikaSharedModelFile(s32 fkind, void **vanilla_file)
{
    if (!ftRemixPikaRulesThunderUsesOwnFile(fkind))
    {
        return vanilla_file;
    }
    return ftRemixPikaSharedLoadedFile(fkind, nFTRemixPikaFileModel, vanilla_file);
}

void **ftRemixPikaSharedJoltEffectFile(s32 power_kind, void **vanilla_file)
{
    if (!ftRemixPikaRulesThunderJoltUsesOwnFile(power_kind))
    {
        return vanilla_file;
    }
    return ftRemixPikaSharedLoadedFile(power_kind, nFTRemixPikaFileSpecial3, vanilla_file);
}

void* ftRemixPikaSharedThunderJoltAnimFile(WPStruct *wp, void *vanilla_file_data)
{
    s32 power_kind = ((WPRemixThunderJoltVars*)&wp->weapon_vars)->power_kind;
    void **file = ftRemixPikaSharedJoltEffectFile(power_kind, NULL);

    if (file == NULL)
    {
        return vanilla_file_data;
    }
    return *file;
}

s32 ftRemixPikaSharedThunderJoltLifetime(FTStruct *fp, s32 vanilla_lifetime)
{
    return ftRemixPikaRulesThunderJoltLifetime(ftRemixNessSharedPowerKind(fp), vanilla_lifetime);
}

s32 ftRemixPikaSharedThunderJoltPowerKind(WPStruct *wp)
{
    return ((WPRemixThunderJoltVars*)&wp->weapon_vars)->power_kind;
}

#endif
