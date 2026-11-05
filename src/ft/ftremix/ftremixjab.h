#ifndef _FTREMIXJAB_H_
#define _FTREMIXJAB_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Remix jab 3 and rapid jab tables read by the vanilla jab code (ftcommonattack1.c, ftcommonattack100.c).
 *
 * Character.asm replaces the fkind whitelists of those routines with per-id tables (jab_3, jab_3_timer, jab_3_action,
 * rapid_jab, rapid_jab_begin/loop/ending_action, rapid_jab_unknown). The port reads the same tables from the RAM image
 * for the fighters whose fkind is a Remix id (fkind >= nFTKindEnumCount); the vanilla fighters keep the vanilla switches.
 * Rules: ftremixjabrules.h.
 */

#include <ft/fighter.h>

#include "ft/ftremix/ftremixjabrules.h"

/** Returned by ftRemixJabRapidStatus for a vanilla fighter: the caller keeps the vanilla switch. */
#define FTREMIX_JAB_RAPID_VANILLA (-1)

/** Returned by ftRemixJabRapidStatus for a Remix fighter whose table word has no status: the caller does nothing. */
#define FTREMIX_JAB_RAPID_UNRESOLVED (-2)

/** TRUE when fkind is a Remix id that the jab tables describe (the vanilla fighters use the vanilla whitelists). */
sb32 ftRemixJabIsRemixKind(s32 fkind);

/** check_jab_3_ (0x8014EFE0): TRUE when the Remix fighter has a third jab (jab_3 gate enabled and a jab 3 status). */
sb32 ftRemixJab3IsEnabled(s32 fkind);

/** set_jab_3_timer_ (0x8014EB2C): sets the followup window of jab 2 from the jab_3_timer word of a Remix fighter. */
void ftRemixJabSetFollowupFrames(FTStruct *fp);

/**
 * get_jab_3_action_ (0x8014EBD8): status id of the third jab of a Remix fighter, or FTREMIX_JAB_STATUS_NONE for a vanilla
 * fighter.
 */
s32 ftRemixJab3GetStatus(s32 fkind);

/** Mewtwo.asm rapid_jab_patch_ (0x8014EAD8): TRUE when jab 2 of the fighter is the start of the rapid jab. */
sb32 ftRemixJabStartsRapidJabAtJab2(s32 fkind);

/** Slippy.asm unique_jab_loop_ (0x8014F0F4): TRUE when the rapid jab of the fighter goes back to jab 1. */
sb32 ftRemixJabRapidJabRestartsJab1(s32 fkind);

/** check_rapid_jab_ (0x8014F4EC): TRUE when the rapid_jab gate of the Remix fighter is enabled. */
sb32 ftRemixJabRapidGateIsOpen(s32 fkind);

/**
 * get_rapid_jab_begin/loop/ending_action_ (0x8014F118, 0x8014F3D0, 0x8014F46C): status id of one rapid jab phase, or
 * FTREMIX_JAB_RAPID_VANILLA / FTREMIX_JAB_RAPID_UNRESOLVED.
 *
 * @param fkind Fighter id.
 * @param phase FTRemixJabRapidPhase.
 */
s32 ftRemixJabRapidStatus(s32 fkind, s32 phase);

/**
 * fix_rapid_jab_unknown_ (0x8014F584): A presses that start the rapid jab and the status the fighter must be in, for a Remix
 * fighter. Returns FALSE when the rapid_jab_unknown word has no entry (the rapid jab cannot start).
 *
 * @param fkind Fighter id.
 * @param inputs_min Receives the number of A presses.
 * @param status_id Receives the status the fighter is in when the rapid jab starts.
 */
sb32 ftRemixJabRapidInputQuery(s32 fkind, s32 *inputs_min, s32 *status_id);

#endif

#endif
