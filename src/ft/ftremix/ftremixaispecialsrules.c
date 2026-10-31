#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaispecialsrules.h"

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

/** Opponent blocks: the always group, or the Ness group unless the own fighter is Sheik. */
int ftRemixAiSamusNspOpponentBlocks(int always_blocks, int sheik_exempt_blocks, int own_is_sheik)
{
    return (always_blocks != 0) || ((sheik_exempt_blocks != 0) && (own_is_sheik == 0));
}

#endif
