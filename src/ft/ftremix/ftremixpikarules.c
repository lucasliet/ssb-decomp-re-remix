/**
 * ftremixpikarules.c - pure rules of the Pikachu clone patches (pikashared.asm and JPika.asm), see ftremixpikarules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixpikarules.h"
#include "remix/remix_roster_data.h"

int ftRemixPikaRulesThunderUsesOwnFile(int fkind)
{
    return (fkind == REMIX_ID_EPIKA) || (fkind == REMIX_ID_JPIKA);
}

int ftRemixPikaRulesThunderJoltUsesOwnFile(int power_kind)
{
    return (power_kind == REMIX_ID_JPIKA) || (power_kind == REMIX_ID_EPIKA);
}

int ftRemixPikaRulesThunderJoltLifetime(int power_kind, int vanilla_lifetime)
{
    if (power_kind != REMIX_ID_JPIKA)
    {
        return vanilla_lifetime;
    }
    return FTREMIX_PIKA_THUNDERJOLT_LIFETIME_JPIKA;
}

#endif
