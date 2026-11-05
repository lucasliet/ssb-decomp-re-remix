/*
 * ftremixtestlog.c - env-gated logs for the headless smoke tests. See ftremixtestlog.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixtestlog.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_roster_data.h"
#include <port_log.h>
#include <sc/scmanager.h>
#include <sc/sctypes.h>
#include <stdlib.h>

#define FTREMIX_TESTLOG_PORTS 4
#define FTREMIX_TESTLOG_WEAPON_SLOTS 256
#define FTREMIX_TESTLOG_BOWSER_AMMO_N64_OFFSET 0x0ADC
#define FTREMIX_TESTLOG_KIRBY_AMMO_N64_OFFSET 0x0AE0
#define FTREMIX_TESTLOG_AMMO_HALFWORD_MASK 0xFFFF

typedef struct FTRemixTestLogAmmoState
{
    sb32 is_known;
    s32 value;

} FTRemixTestLogAmmoState;

static s32 sFTRemixTestLogFighterEnabled = -1;
static FTRemixTestLogAmmoState sFTRemixTestLogAmmoStates[FTREMIX_TESTLOG_PORTS];
static u32 sFTRemixTestLogWeaponSpawnTics[FTREMIX_TESTLOG_WEAPON_SLOTS];

/* Reads `name` once into `cache`: unset, empty or "0" is off. */
static sb32 ftRemixTestLogIsEnabled(const char *name, s32 *cache)
{
    const char *text;

    if (*cache < 0)
    {
        text = getenv(name);

        *cache = ((text != NULL) && (text[0] != '\0') && (text[0] != '0')) ? 1 : 0;
    }
    return (*cache != 0) ? TRUE : FALSE;
}

static sb32 ftRemixTestLogIsFighterEnabled(void)
{
    return ftRemixTestLogIsEnabled("SSB64_REMIX_FIGHTER_LOG", &sFTRemixTestLogFighterEnabled);
}

static u32 ftRemixTestLogGetTic(void)
{
    if (gSCManagerBattleState == NULL)
    {
        return 0;
    }
    return gSCManagerBattleState->time_passed;
}

void ftRemixTestLogStatus(FTStruct *fp, s32 status_id)
{
    if (ftRemixTestLogIsFighterEnabled() == FALSE)
    {
        return;
    }
    port_log("[remix-fighter] status t=%u port=%d fkind=%d status=%d\n", ftRemixTestLogGetTic(), (s32)fp->player, (s32)fp->fkind, status_id);
}

void ftRemixTestLogHit(FTStruct *fp)
{
    if (ftRemixTestLogIsFighterEnabled() == FALSE)
    {
        return;
    }
    port_log("[remix-fighter] hit t=%u port=%d fkind=%d damage=%d percent=%d knockback=%.2f\n", ftRemixTestLogGetTic(), (s32)fp->player,
             (s32)fp->fkind, (s32)fp->damage_queue, (s32)fp->percent_damage, (double)fp->damage_knockback);
}

/* Fire breath ammo halfword of Bowser (0x0ADE) or of Kirby with the Bowser hat (0x0AE2); -1 when the fighter has none. */
static s32 ftRemixTestLogReadAmmo(FTStruct *fp)
{
    u32 n64_offset;

    if (fp->fkind == REMIX_ID_BOWSER)
    {
        n64_offset = FTREMIX_TESTLOG_BOWSER_AMMO_N64_OFFSET;
    }
    else if (((fp->fkind == nFTKindKirby) || (fp->fkind == REMIX_ID_JKIRBY)) && (fp->passive_vars.kirby.copy_id == REMIX_ID_BOWSER))
    {
        n64_offset = FTREMIX_TESTLOG_KIRBY_AMMO_N64_OFFSET;
    }
    else
    {
        return -1;
    }
    return *ftRemixFighterWord(fp, n64_offset) & FTREMIX_TESTLOG_AMMO_HALFWORD_MASK;
}

void ftRemixTestLogAmmo(FTStruct *fp)
{
    FTRemixTestLogAmmoState *state;
    s32 ammo;

    if ((ftRemixTestLogIsFighterEnabled() == FALSE) || ((u32)fp->player >= FTREMIX_TESTLOG_PORTS))
    {
        return;
    }
    ammo = ftRemixTestLogReadAmmo(fp);

    if (ammo < 0)
    {
        return;
    }
    state = &sFTRemixTestLogAmmoStates[fp->player];

    if ((state->is_known != FALSE) && (state->value == ammo))
    {
        return;
    }
    state->is_known = TRUE;
    state->value = ammo;

    port_log("[remix-fighter] ammo t=%u port=%d value=%d\n", ftRemixTestLogGetTic(), (s32)fp->player, ammo);
}

void ftRemixTestLogWeaponSpawn(WPStruct *wp)
{
    if (ftRemixTestLogIsFighterEnabled() == FALSE)
    {
        return;
    }
    sFTRemixTestLogWeaponSpawnTics[wp->spawn_serial % FTREMIX_TESTLOG_WEAPON_SLOTS] = ftRemixTestLogGetTic();
}

void ftRemixTestLogWeaponEnd(WPStruct *wp)
{
    u32 tic;

    if (ftRemixTestLogIsFighterEnabled() == FALSE)
    {
        return;
    }
    tic = ftRemixTestLogGetTic();

    port_log("[remix-weapon] end t=%u kind=%d lifetime=%d\n", tic, (s32)wp->kind,
             (s32)(tic - sFTRemixTestLogWeaponSpawnTics[wp->spawn_serial % FTREMIX_TESTLOG_WEAPON_SLOTS]));
}

#endif
