/**
 * mnremixcssrandomtoken.c - Random token of the 1P and Training character selects, see mnremixcssrandomtoken.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/ftdef.h>
#include "mn/mnplayers/mnremixcssrandom.h"
#include "mn/mnplayers/mnremixcssrandomtoken.h"
#include "mn/mnplayers/mnremixvariants.h"

#define MNREMIX_CSS_RANDOM_TOKEN_SCREENS 2

/* The flags live as long as the session, like the asm tables in Remix RAM: the screen that rolled the token is gone when it is read. */
static sb32 sMNRemixCssRandomTokenIsResolved[MNREMIX_CSS_RANDOM_TOKEN_SCREENS][MNREMIX_CSS_RANDOM_TOKEN_PORTS];

s32 mnRemixCssRandomTokenRoll(s32 port, MNRemixCssFighterFilter is_unavailable)
{
    s32 fkind;

    do
    {
        fkind = mnRemixRandomPickFighterKind();
    }
    while ((fkind == nFTKindNull) || (is_unavailable(fkind) != FALSE) || (mnRemixCssRandomKeepRoll(port, fkind) == FALSE));

    return fkind;
}

static sb32 mnRemixCssRandomTokenIsValid(s32 screen, s32 port)
{
    return ((screen >= 0) && (screen < MNREMIX_CSS_RANDOM_TOKEN_SCREENS) && (port >= 0) && (port < MNREMIX_CSS_RANDOM_TOKEN_PORTS)) ? TRUE : FALSE;
}

void mnRemixCssRandomTokenMarkResolved(s32 screen, s32 port)
{
    if (mnRemixCssRandomTokenIsValid(screen, port) != FALSE)
    {
        sMNRemixCssRandomTokenIsResolved[screen][port] = TRUE;
    }
}

sb32 mnRemixCssRandomTokenTakeResolved(s32 screen, s32 port)
{
    sb32 is_resolved;

    if (mnRemixCssRandomTokenIsValid(screen, port) == FALSE)
    {
        return FALSE;
    }
    is_resolved = sMNRemixCssRandomTokenIsResolved[screen][port];
    sMNRemixCssRandomTokenIsResolved[screen][port] = FALSE;

    return is_resolved;
}

#endif
