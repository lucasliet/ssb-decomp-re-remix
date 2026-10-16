#ifndef _FTREMIXENVCOLOR_H_
#define _FTREMIXENVCOLOR_H_

/*
 * Smash Remix CharEnvColor (src/CharEnvColor.asm): a per-port RGBA env colour that replaces the colour a fighter model
 * is drawn with. The alpha byte is the model opacity. override has priority over moveset; 0 means no override.
 */

#include <ssb_types.h>

#define FTREMIX_ENVCOLOR_PORTS 4

typedef enum FTRemixEnvColorState
{
    nFTRemixEnvColorStateNormal,
    nFTRemixEnvColorStateCloaked,
    nFTRemixEnvColorStateNone,
    nFTRemixEnvColorStateDark

} FTRemixEnvColorState;

/** Env colour override of a port: the override table, else the moveset table, else 0 (no override). */
u32 ftRemixEnvColorGetOverride(s32 port);

u32 ftRemixEnvColorGetOverrideValue(s32 port);
void ftRemixEnvColorSetOverride(s32 port, u32 rgba);
void ftRemixEnvColorSetMoveset(s32 port, u32 rgba);
void ftRemixEnvColorSetState(s32 port, FTRemixEnvColorState state);
FTRemixEnvColorState ftRemixEnvColorGetState(s32 port);

/** Override value that matches the state of a port (asm: the three-way setup of clear_active_cloaking_devices_). */
u32 ftRemixEnvColorGetStateValue(s32 port);

/** Resets the override of a port to the value of its state (decloak, fighter init). */
void ftRemixEnvColorRestoreState(s32 port);

/** Clears every table (new match). */
void ftRemixEnvColorClearAll(void);

#endif
