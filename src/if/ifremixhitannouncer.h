#ifndef _IFREMIXHITANNOUNCER_H_
#define _IFREMIXHITANNOUNCER_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>

/**
 * Plays the hit announcement of a victim that took a hit (asm hit_announcement_, Remix PokemonAnnouncer.asm). Reads the
 * damage, knockback and element the hit stored on the victim. Does nothing when the announcer is not active.
 */
void ifRemixHitAnnouncerPlayHit(FTStruct *victim_fp);

/**
 * Plays the throw hit announcement for a thrown fighter (asm throw_hit_announcement_). damage is the damage percent of the
 * throw and knockback is the final knockback. Does nothing when the announcer is not active.
 */
void ifRemixHitAnnouncerPlayThrow(s32 damage, f32 knockback);

#endif

#endif
