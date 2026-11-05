/**
 * ftremixinitialarmor.c - passive armor set by the Remix initial scripts of Metal Luigi and Giga Bowser, see
 * ftremixinitialarmor.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixinitialarmor.h"
#include "ft/ftremix/ftremixfields.h"
#include "remix/remix_roster_data.h"

#define FTREMIX_ARMOR_MLUIGI 30.0F  /* lui 0x41F0 */
#define FTREMIX_ARMOR_GBOWSER 55.0F /* lui 0x425C */
#define FTREMIX_ARMOR_PASSIVE_FIRST_WORD 0x0ADC

void ftRemixInitialScriptArmor(FTStruct *fp)
{
    switch (fp->fkind)
    {
    case REMIX_ID_MLUIGI:
        fp->knockback_resist_passive = FTREMIX_ARMOR_MLUIGI;
        break;

    case REMIX_ID_GBOWSER:
        fp->knockback_resist_passive = FTREMIX_ARMOR_GBOWSER;
        break;

    default:
        return;
    }
    *ftRemixFighterWord(fp, FTREMIX_ARMOR_PASSIVE_FIRST_WORD) = 0;
}

#endif
