#ifndef _FTREMIXAILONGRANGERULES_H_
#define _FTREMIXAILONGRANGERULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix LONG_RANGE routine (AI.asm LONG_RANGE scope, 2530-2620), so the headless unit tests
 * (tests/remix) can run them without game state. Numbers in, numbers out.
 */

/* Vanilla LONG_RANGE.ROUTINE addresses (AI.asm, the entries of Character.ai_long_range.table). */
#define FTREMIX_AI_LONG_RANGE_ROUTINE_NSP_SHOOT 0x80138D24u
#define FTREMIX_AI_LONG_RANGE_ROUTINE_PHYSICAL_SHOOT 0x80138CD4u
#define FTREMIX_AI_LONG_RANGE_ROUTINE_NONE 0x80138ECCu

#define FTREMIX_AI_LONG_RANGE_KIND_UNKNOWN 0
#define FTREMIX_AI_LONG_RANGE_KIND_NONE 1
#define FTREMIX_AI_LONG_RANGE_KIND_NSP_SHOOT 2
#define FTREMIX_AI_LONG_RANGE_KIND_PHYSICAL_SHOOT 3

/* Remix polygon fighters: the NMario..NNess variants (ids 0x0E-0x19) and the Remix polygons (ids 0x4D-0x60). */
#define FTREMIX_AI_LONG_RANGE_NMARIO 0x0E
#define FTREMIX_AI_LONG_RANGE_NNESS 0x19
#define FTREMIX_AI_LONG_RANGE_NWARIO 0x4D
#define FTREMIX_AI_LONG_RANGE_NCRASH 0x60

/** TRUE when the fighter kind reads its routine from Character.ai_long_range.table: every id except the vanilla ones. */
int ftRemixAiLongRangeRulesUsesTable(int fkind);

/** TRUE when the fighter kind is a polygon variant, which never uses the far-range move (NONE). */
int ftRemixAiLongRangeRulesIsPolygon(int fkind);

/** Maps a routine address read from the table to FTREMIX_AI_LONG_RANGE_KIND_*, UNKNOWN when it is not a vanilla routine. */
int ftRemixAiLongRangeRulesRoutineKind(unsigned int routine_addr);

/** The table command replaces the default command when it is not 0 (asm: `beqzl t1` keeps the default on 0). */
int ftRemixAiLongRangeRulesCustomCommand(int table_command, int default_command);

#endif

#endif
