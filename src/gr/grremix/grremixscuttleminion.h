#ifndef _GRREMIXSCUTTLEMINION_H_
#define _GRREMIXSCUTTLEMINION_H_

/*
 * Scuttle Town Scuttle Minion factory, ported from Hazards.asm minion_factory_. The factory rolls for a minion once the
 * match clock runs and spawns it through the Scuttle Minion item (itremixscuttleminion.c).
 */

#include <ssb_types.h>
#include <sys/objdef.h>

/**
 * Creates the minion factory process and clears its state. `stage_header` is the Scuttle Town stage header, which the
 * minion item reads its file from. The caller checks grRemixHazardsAreOn() first, as the asm checks Toggles.entry_hazard_mode.
 */
void grRemixScuttleMinionFactoryMake(void *stage_header);

/** Address of the stage header pointer. The minion descriptor uses it as p_file, as the asm passes 0x801313F4. */
void **grRemixScuttleMinionStageHeaderRef(void);

#endif
