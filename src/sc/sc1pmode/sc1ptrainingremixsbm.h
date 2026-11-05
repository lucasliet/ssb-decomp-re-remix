#ifndef _SC1PTRAININGREMIXSBM_H_
#define _SC1PTRAININGREMIXSBM_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>

/**
 * Gate of the Single Button Mode filter for the scene being played (FTREMIX_SBM_GATE_*): the Training menu path while a Training menu
 * is up (SingleButtonMode.asm _check_pause_training), open everywhere else.
 */
extern s32 sc1PTrainingRemixGetSingleButtonGate(void);

#endif

#endif
