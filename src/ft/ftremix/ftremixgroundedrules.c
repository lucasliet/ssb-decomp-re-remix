/**
 * ftremixgroundedrules.c - pure rule of the Remix grounded_script table, see ftremixgroundedrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixgroundedrules.h"
#include "remix/remix_roster_data.h"

int ftRemixGroundedScriptKind(int fkind)
{
    switch (fkind)
    {
    case REMIX_ID_DRM:      /* inherited from MARIO */
    case REMIX_ID_JMARIO:
    case REMIX_ID_GOEMON:
    case REMIX_ID_CRASH:
    case REMIX_ID_LANKY:
    case REMIX_ID_NDRM:
    case REMIX_ID_NCRASH:
    case REMIX_ID_JLUIGI:   /* inherited from LUIGI */
    case REMIX_ID_MLUIGI:
    case REMIX_ID_DRL:
    case REMIX_ID_GND:      /* inherited from CAPTAIN */
    case REMIX_ID_JFALCON:
    case REMIX_ID_SANDBAG:
    case REMIX_ID_DRAGONKING:
    case REMIX_ID_SHEIK:
    case REMIX_ID_NSHEIK:
    case REMIX_ID_NGND:
    case REMIX_ID_NMARTH:
    case REMIX_ID_MARTH:    /* Marth.asm:490, Roy.asm:506: 0x800DE428 */
    case REMIX_ID_ROY:
    case REMIX_ID_JPUFF:    /* inherited from JIGGLY */
    case REMIX_ID_EPUFF:
    case REMIX_ID_SONIC:    /* Sonic.asm:399, SSonic.asm:330, NSonic.asm:252: 0x800DE428 */
    case REMIX_ID_SSONIC:
    case REMIX_ID_NSONIC:
        return nFTRemixGroundedScriptClearWord;

    case REMIX_ID_DSAMUS:   /* inherited from SAMUS */
    case REMIX_ID_JSAMUS:
    case REMIX_ID_ESAMUS:
    case REMIX_ID_NDSAMUS:
        return nFTRemixGroundedScriptClearRecoil;

    case REMIX_ID_EBI:
        return nFTRemixGroundedScriptEbi;

    case REMIX_ID_KIRBY:    /* Kirby.asm:1114, JKirby.asm:384 */
    case REMIX_ID_JKIRBY:
        return nFTRemixGroundedScriptKirbyHat;

    case REMIX_ID_PEACH:    /* Peach.asm:507, NPeach.asm:271 */
    case REMIX_ID_NPEACH:
        return nFTRemixGroundedScriptPeachFloat;

    default:
        return nFTRemixGroundedScriptNone;
    }
}

#endif
