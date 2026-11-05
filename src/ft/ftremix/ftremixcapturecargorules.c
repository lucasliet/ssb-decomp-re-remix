/**
 * ftremixcapturecargorules.c - pure rule of the JDK cargo escape base, see ftremixcapturecargorules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixcapturecargorules.h"
#include "remix/remix_roster_data.h"

float ftRemixCaptureCargoEscapeBase(int grabber_fkind)
{
    switch (grabber_fkind)
    {
    case REMIX_ID_JDK:
    case REMIX_ID_MARINA:
    case REMIX_ID_NMARINA:
        return FTREMIX_CARGO_ESCAPE_BASE_HEAVY;

    default:
        return FTREMIX_CARGO_ESCAPE_BASE_DEFAULT;
    }
}

#endif
