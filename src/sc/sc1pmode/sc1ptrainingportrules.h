#ifndef _SC1PTRAININGPORTRULES_H_
#define _SC1PTRAININGPORTRULES_H_

#include <stdint.h>

/*
 * Pure rules of the per-port chain of the Remix Training menu (Training.asm tail_px: Type, Character, Costume, Spawn). Ints only,
 * compiled standalone by tests/remix. Every entry has an Auto value that keeps what the character select chose, so the chain only
 * overrides what the player edited. The scene code reads the options and writes the players; these functions only decide.
 */

#define SC1PTRAININGPORTRULES_PORT_COUNT 4

/** Type entry values (Training.asm string_table_type is Human, CPU, Disabled; Auto is the port addition). */
#define SC1PTRAININGPORTRULES_TYPE_AUTO 0
#define SC1PTRAININGPORTRULES_TYPE_HUMAN 1
#define SC1PTRAININGPORTRULES_TYPE_CPU 2
#define SC1PTRAININGPORTRULES_TYPE_DISABLED 3

/** Spawn entry values: Auto keeps the port's own spawn, 1 to 4 are Port 1 to Port 4 (Training.asm string_table_spawn). */
#define SC1PTRAININGPORTRULES_SPAWN_AUTO 0

/**
 * Type a port ends up with. Auto means the menu owner is Human, the character select CPU is CPU and the rest are Disabled. The
 * menu owner is always Human (Training.asm handle_reset_menu_selections_ resets a Disabled human port to Human).
 *
 * @param option Type entry value of the port, SC1PTRAININGPORTRULES_TYPE_*.
 * @param port Port 0 to 3.
 * @param menu_port Port of the human that owns the Training menu.
 * @param select_cpu_port Port the character select gave to the CPU.
 * @return SC1PTRAININGPORTRULES_TYPE_HUMAN, _CPU or _DISABLED.
 */
extern int32_t sc1PTrainingPortRulesResolveType(int32_t option, int32_t port, int32_t menu_port, int32_t select_cpu_port);

/**
 * Port the dummy statistics and behavior follow: the first CPU, or the menu owner when no port is a CPU so the scene never reads a
 * missing fighter.
 *
 * @param resolved_types SC1PTRAININGPORTRULES_PORT_COUNT resolved types.
 * @param menu_port Port of the human that owns the Training menu.
 */
extern int32_t sc1PTrainingPortRulesPickDummy(const int32_t *resolved_types, int32_t menu_port);

/** Number of ports in `resolved_types` with the resolved type `type`. */
extern int32_t sc1PTrainingPortRulesCountType(const int32_t *resolved_types, int32_t type);

/**
 * Value of an entry with an Auto choice (Character, Costume): entry 0 keeps `auto_value`, entry n picks index n - 1.
 */
extern int32_t sc1PTrainingPortRulesPick(int32_t option, int32_t auto_value);

/** Port whose spawn point a port uses: its own for Auto, Port n for the entry n. */
extern int32_t sc1PTrainingPortRulesSpawnPort(int32_t option, int32_t port);

#endif
