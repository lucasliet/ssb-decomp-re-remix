#ifndef _FTREMIXMAIN_H_
#define _FTREMIXMAIN_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Remix hooks of ftMainProcPhysicsMap and ftMainSetStatus (ft/ftmain.c) that need the fighter struct. The rules behind them are
 * in ftremixmainrules.h.
 */

#include <ft/fighter.h>

/**
 * thrown_marina_patch_ (Marina/Marina.asm:734, RAM 0x800E7568): animation file ftMainSetStatus loads for the new status. A
 * Donkey Kong or Captain Falcon family fighter grabbed by Marina in ThrownDK loads the Marina throw animation instead.
 *
 * @param fp Fighter struct, with status_id already set to the new status.
 * @param anim_file_id Animation file of the motion descriptor.
 * @return The file to load.
 */
extern u32 ftRemixMainGetAnimFileID(FTStruct *fp, u32 anim_file_id);

#endif

#endif
