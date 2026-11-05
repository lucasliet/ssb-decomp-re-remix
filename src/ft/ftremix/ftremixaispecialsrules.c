#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaispecialsrules.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_AI_SPECIALS_CHARGE_DF 0x0DF /* Samus ChargeShotCharging and Mewtwo ShadowBallCharge */
#define FTREMIX_AI_SPECIALS_KIRBY_DSAMUS_CHARGE_AIR 0x1AD
#define FTREMIX_AI_SPECIALS_KIRBY_DSCHARGING 0x133
#define FTREMIX_AI_SPECIALS_SHEIK_NSPG_CHARGE 0x0EB
#define FTREMIX_AI_SPECIALS_SHEIK_NSPA_CHARGE 0x0EE
#define FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_CHARGE_GROUND 0x167
#define FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_CHARGE_AIR 0x16A
#define FTREMIX_AI_SPECIALS_MTWO_SHADOW_CHARGE_AIR 0x0E2
#define FTREMIX_AI_SPECIALS_KIRBY_SHADOW_CHARGE 0x14B
#define FTREMIX_AI_SPECIALS_KIRBY_SHADOW_CHARGE_AIR 0x14E
#define FTREMIX_AI_SPECIALS_BOWSER_USP_GROUND 0x0DE
#define FTREMIX_AI_SPECIALS_JUMPSQUAT 0x014 /* Action.JumpSquat */
#define FTREMIX_AI_SPECIALS_NEEDLE_AMMO_FULL 6
#define FTREMIX_AI_SPECIALS_SHEIK_NSPG_BEGIN 0x0EA
#define FTREMIX_AI_SPECIALS_SHEIK_NSPG_SHOOT 0x0EC
#define FTREMIX_AI_SPECIALS_SHEIK_NSPA_BEGIN 0x0ED
#define FTREMIX_AI_SPECIALS_SHEIK_NSPA_SHOOT 0x0EF
#define FTREMIX_AI_SPECIALS_PEPPY_NSPG_BEGIN 0x0E1
#define FTREMIX_AI_SPECIALS_PEPPY_NSPG_CHARGE 0x0E2
#define FTREMIX_AI_SPECIALS_PEPPY_NSPG_SHOOT 0x0ED
#define FTREMIX_AI_SPECIALS_PEPPY_NSPA_BEGIN 0x0EE
#define FTREMIX_AI_SPECIALS_PEPPY_NSPA_CHARGE 0x0F2
#define FTREMIX_AI_SPECIALS_PEPPY_NSPA_SHOOT 0x0F3
#define FTREMIX_AI_SPECIALS_MTWO_NSPG_SHOOT 0x0E0
#define FTREMIX_AI_SPECIALS_MTWO_NSPA_SHOOT 0x0E4
#define FTREMIX_AI_SPECIALS_KIRBY_SHADOW_FIRST 0x14A /* ShadowBallStart to ShadowBallShootAir */
#define FTREMIX_AI_SPECIALS_KIRBY_SHADOW_LAST 0x14F
#define FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_FIRST 0x166 /* NeedleStormStartGround to NeedleStormShootAir */
#define FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_LAST 0x16B

/** The asm checks DSamusNSPChargeAir twice; the second check is a copy of the first and changes nothing. */
int ftRemixAiSpecialsDSamusCancels(int action)
{
    return (action == FTREMIX_AI_SPECIALS_CHARGE_DF) || (action == FTREMIX_AI_SPECIALS_KIRBY_DSAMUS_CHARGE_AIR) ||
           (action == FTREMIX_AI_SPECIALS_KIRBY_DSCHARGING);
}

/** Sheik cancels only while charging, on the ground or in the air, including the Kirby needle copy. */
int ftRemixAiSpecialsSheikCancels(int action)
{
    return (action == FTREMIX_AI_SPECIALS_SHEIK_NSPG_CHARGE) || (action == FTREMIX_AI_SPECIALS_SHEIK_NSPA_CHARGE) ||
           (action == FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_CHARGE_GROUND) || (action == FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_CHARGE_AIR);
}

/** Mewtwo cancels only while charging the shadow ball, on the ground or in the air, including the Kirby copy. */
int ftRemixAiSpecialsMewtwoCancels(int action)
{
    return (action == FTREMIX_AI_SPECIALS_CHARGE_DF) || (action == FTREMIX_AI_SPECIALS_MTWO_SHADOW_CHARGE_AIR) ||
           (action == FTREMIX_AI_SPECIALS_KIRBY_SHADOW_CHARGE) || (action == FTREMIX_AI_SPECIALS_KIRBY_SHADOW_CHARGE_AIR);
}

/** Bowser and GBowser grounded up special (Bowser.Action.USPGround 0xDE): the stick X is zeroed (AI.asm improve_remix_specials). */
int ftRemixAiSpecialsIsBowserUspGround(int action)
{
    return action == FTREMIX_AI_SPECIALS_BOWSER_USP_GROUND;
}

/** Sheik "keep charging" actions (begin, ground charge) and "stop" actions (shoots, air charge, jump squat), AI.asm _check_needle. */
static int ftRemixAiSpecialsSheikNeedleOutcome(int action)
{
    if ((action == FTREMIX_AI_SPECIALS_SHEIK_NSPG_BEGIN) || (action == FTREMIX_AI_SPECIALS_SHEIK_NSPA_BEGIN) ||
        (action == FTREMIX_AI_SPECIALS_SHEIK_NSPG_CHARGE))
    {
        return FTREMIX_AI_SPECIALS_NEEDLE_KEEP;
    }
    if ((action == FTREMIX_AI_SPECIALS_SHEIK_NSPG_SHOOT) || (action == FTREMIX_AI_SPECIALS_SHEIK_NSPA_SHOOT) ||
        (action == FTREMIX_AI_SPECIALS_SHEIK_NSPA_CHARGE) || (action == FTREMIX_AI_SPECIALS_JUMPSQUAT))
    {
        return FTREMIX_AI_SPECIALS_NEEDLE_STOP;
    }
    return -1;
}

/** Peppy checks of AI.asm _check_revolver: same shape as Sheik's with Peppy's action ids; -1 when none matches. */
static int ftRemixAiSpecialsPeppyRevolverOutcome(int action)
{
    if ((action == FTREMIX_AI_SPECIALS_PEPPY_NSPG_BEGIN) || (action == FTREMIX_AI_SPECIALS_PEPPY_NSPA_BEGIN) ||
        (action == FTREMIX_AI_SPECIALS_PEPPY_NSPG_CHARGE))
    {
        return FTREMIX_AI_SPECIALS_NEEDLE_KEEP;
    }
    if ((action == FTREMIX_AI_SPECIALS_PEPPY_NSPG_SHOOT) || (action == FTREMIX_AI_SPECIALS_PEPPY_NSPA_SHOOT) ||
        (action == FTREMIX_AI_SPECIALS_PEPPY_NSPA_CHARGE) || (action == FTREMIX_AI_SPECIALS_JUMPSQUAT))
    {
        return FTREMIX_AI_SPECIALS_NEEDLE_STOP;
    }
    return -1;
}

int ftRemixAiSpecialsNeedleChargeOutcome(int is_sheik, int action, int ammo)
{
    int outcome = -1;

    if (is_sheik != 0)
    {
        outcome = ftRemixAiSpecialsSheikNeedleOutcome(action);
    }
    if (outcome < 0)
    {
        outcome = ftRemixAiSpecialsPeppyRevolverOutcome(action);
    }
    if (outcome >= 0)
    {
        return outcome;
    }
    return (ammo < FTREMIX_AI_SPECIALS_NEEDLE_AMMO_FULL) ? FTREMIX_AI_SPECIALS_NEEDLE_PRESS : FTREMIX_AI_SPECIALS_NEEDLE_STOP;
}

/** Charge shot actions of Dark Samus (Kirby copy), Mewtwo (shoots and the Kirby shadow ball copy) and Sheik (own and Kirby needles). */
int ftRemixAiSpecialsChargeShotActive(int fkind, int action)
{
    switch (fkind)
    {
    case REMIX_ID_DSAMUS:
        return action == FTREMIX_AI_SPECIALS_KIRBY_DSCHARGING;

    case REMIX_ID_MTWO:
        return (action == FTREMIX_AI_SPECIALS_MTWO_NSPG_SHOOT) || (action == FTREMIX_AI_SPECIALS_MTWO_NSPA_SHOOT) ||
               ((action >= FTREMIX_AI_SPECIALS_KIRBY_SHADOW_FIRST) && (action <= FTREMIX_AI_SPECIALS_KIRBY_SHADOW_LAST));

    case REMIX_ID_SHEIK:
        return ((action >= FTREMIX_AI_SPECIALS_SHEIK_NSPG_BEGIN) && (action <= FTREMIX_AI_SPECIALS_SHEIK_NSPA_SHOOT)) ||
               ((action >= FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_FIRST) && (action <= FTREMIX_AI_SPECIALS_KIRBY_NEEDLE_LAST));
    }
    return 0;
}

int ftRemixAiSpecialsUsesSamusActionList(int fkind)
{
    return (fkind == REMIX_ID_SAMUS) || (fkind == REMIX_ID_JSAMUS) || (fkind == REMIX_ID_ESAMUS) || (fkind == REMIX_ID_DSAMUS) ||
           (fkind == REMIX_ID_MTWO);
}

/** Opponent blocks: the always group, or the Ness group unless the own fighter is Sheik. */
int ftRemixAiSamusNspOpponentBlocks(int always_blocks, int sheik_exempt_blocks, int own_is_sheik)
{
    return (always_blocks != 0) || ((sheik_exempt_blocks != 0) && (own_is_sheik == 0));
}

#endif
