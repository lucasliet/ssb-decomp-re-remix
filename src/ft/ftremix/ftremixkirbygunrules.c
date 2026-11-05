/**
 * ftremixkirbygunrules.c - pure rule of the Kirby held-weapon parts, see ftremixkirbygunrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixkirbygunrules.h"

#define FTREMIX_KIRBY_HAT_CONKER 0x19
#define FTREMIX_KIRBY_HAT_WOLF 0x1A
#define FTREMIX_KIRBY_HAT_MARTH 0x1C
#define FTREMIX_KIRBY_HAT_GOEMON 0x22
#define FTREMIX_KIRBY_HAT_SLIPPY 0x23
#define FTREMIX_KIRBY_HAT_PEPPY 0x24
#define FTREMIX_KIRBY_HAT_EBI 0x28
#define FTREMIX_KIRBY_HAT_ROY 0x2C
#define FTREMIX_KIRBY_HAT_LANKY 0x2E

int ftRemixKirbyGunFileOffset(int hat_id)
{
    switch (hat_id)
    {
    case FTREMIX_KIRBY_HAT_MARTH:
        return 0x1D890;

    case FTREMIX_KIRBY_HAT_ROY:
        return 0x1D980;

    case FTREMIX_KIRBY_HAT_WOLF:
        return 0x1D830;

    case FTREMIX_KIRBY_HAT_SLIPPY:
        return 0x1D8C0;

    case FTREMIX_KIRBY_HAT_PEPPY:
        return 0x1D8F0;

    case FTREMIX_KIRBY_HAT_GOEMON:
        return 0x1D920;

    case FTREMIX_KIRBY_HAT_EBI:
        return 0x1D950;

    case FTREMIX_KIRBY_HAT_LANKY:
        return 0x1D9B0;

    case FTREMIX_KIRBY_HAT_CONKER:
        return 0x1D860;

    default:
        return FTREMIX_KIRBY_GUN_NO_OFFSET;
    }
}

#endif
