/**
 * ftremixinputdelay.c - Smash Remix InputDelay.asm: delays the human controller samples by the panel value of the port.
 *
 * ftRemixInputDelayController (ftMainProcUpdateInterrupt) reads the delayed sample and ftRemixInputDelayReset
 * (ftManagerInitFighter) clears the port buffer at the start of a match.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixhooks.h"
#include <ft/fighter.h>
#include <gm/gmdef.h>
#include <gm/gmremixpanel.h>
#include <sys/controller.h>

#define FTREMIX_INPUTDELAY_MAX_FRAMES 12
#define FTREMIX_INPUTDELAY_RING_SIZE (FTREMIX_INPUTDELAY_MAX_FRAMES + 1)

typedef struct FTRemixInputDelayRing
{
    SYController samples[FTREMIX_INPUTDELAY_RING_SIZE];
    s32 write_index;
} FTRemixInputDelayRing;

/* One ring per port (asm buffer). A ring of MAX_FRAMES + 1 slots holds the sample written `delay` calls ago. */
static FTRemixInputDelayRing sFTRemixInputDelayRings[GMCOMMON_PLAYERS_MAX];

/* Zeroed ring, copied over a port ring on match start (asm clears the buffer as it is refilled). */
static const FTRemixInputDelayRing sFTRemixInputDelayEmpty;

/**
 * Clears the input delay buffer of the fighter's port so the first frames of a match read neutral input.
 * Called from ftManagerInitFighter.
 */
void ftRemixInputDelayReset(FTStruct *fp)
{
    s32 player = fp->player;

    if (player >= GMCOMMON_PLAYERS_MAX)
    {
        return;
    }
    sFTRemixInputDelayRings[player] = sFTRemixInputDelayEmpty;
}

/**
 * Returns the controller sample the fighter reads this frame. The sample is stored in the port ring and the one
 * written `InputDelay` frames ago is returned. Returns controller unchanged when the delay is 0 or the port is invalid.
 * Called from ftMainProcUpdateInterrupt for human fighters only. The ring advances once per call, so the delay counts
 * frames where the fighter's control is enabled.
 */
SYController *ftRemixInputDelayController(FTStruct *fp, SYController *controller)
{
    s32 player = fp->player;
    s32 delay;
    s32 read_index;
    FTRemixInputDelayRing *ring;

    if (player >= GMCOMMON_PLAYERS_MAX)
    {
        return controller;
    }
    delay = gmRemixPanel(INPUT_DELAY, player);
    ring = &sFTRemixInputDelayRings[player];
    ring->samples[ring->write_index] = *controller;
    read_index = (ring->write_index - delay + FTREMIX_INPUTDELAY_RING_SIZE) % FTREMIX_INPUTDELAY_RING_SIZE;
    ring->write_index = (ring->write_index + 1) % FTREMIX_INPUTDELAY_RING_SIZE;

    if (delay == 0)
    {
        return controller;
    }
    return &ring->samples[read_index];
}

#endif
