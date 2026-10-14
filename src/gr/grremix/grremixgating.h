#ifndef _GRREMIXGATING_H_
#define _GRREMIXGATING_H_

/*
 * Gates of the vanilla ground makers that the Remix build applies per stage or per hazard toggle
 * (Hazards.asm hazard_toggle, mk_clone_hazard_removal_, peach_bumper_). The vanilla files call these
 * under #ifdef REGION_REMIX.
 */

#include <ssb_types.h>

/** Hazard toggle of Hazards.asm hazard_toggle: TRUE unless the screen is VS or Training and hazards are disabled. */
sb32 grRemixHazardToggleIsOn(void);

/** Movement toggle (Toggles.entry_hazard_mode bit 1), with the same VS/Training rule as grRemixHazardToggleIsOn. */
sb32 grRemixMovementToggleIsOn(void);

/** TRUE for the Mushroom Kingdom clones without scale platforms and piranha plants (SMBBF, SMBO, MK_REMIX). */
sb32 grRemixIsMushroomKingdomClone(s32 gkind);

/** TRUE for the stages that get the Peach's Castle bumper: Peach's Castle and its Remix clones (Hazards.asm peach_bumper_). */
sb32 grRemixStageHasPeachBumper(s32 gkind);

#endif
