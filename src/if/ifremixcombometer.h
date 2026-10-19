#ifndef _IFREMIXCOMBOMETER_H_
#define _IFREMIXCOMBOMETER_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ssb_types.h>
#include <ft/fighter.h>
#include <sys/objtypes.h>
#include <sc/scene.h>

/*
 * Smash Remix combo meter (src/ComboMeter.asm), one per port. The hit count and damage of the victim are read from
 * gSCManagerBattleState->players[port] (combo_count_foe / combo_damage_foe) on every frame.
 */
typedef struct IFRemixComboMeter
{
    FTStruct *fp;                                   /* player struct of the port, NULL when the port has no fighter */
    GObj *meter_gobj;                               /* display GObj, set by ifRemixComboMeterSetup */
    f32 x_coord;                                    /* left edge of the meter, from the HUD player position */
    f32 y_coord;                                    /* top edge of the meter, 167.0F or 50.0F (Dragon King) */
    s32 frame_buffer;                               /* frames left before the meter hides after the combo ends */
    s32 max_combo_hits;
    s32 max_combo_damage;
    s32 combo_count;                                /* hit count drawn on the previous frame */
    s32 color_current;                              /* attacker port (0-3) of the last hit, 4 = silver */
    s32 color_display;                              /* color index used by the numbers and text of this frame */
    s32 attribution_start_hit;                      /* hit count where the current attacker started its part of the combo */
    s32 highest_combo_vs[GMCOMMON_PLAYERS_MAX];     /* highest combo this port dealt to each port, 0-based */
} IFRemixComboMeter;

#define IFREMIX_COMBO_COLOR_SILVER GMCOMMON_PLAYERS_MAX

/**
 * Advances the hit state of the victim's meter for one frame (asm draw_hit_count_ logic).
 * Returns the combo count to draw, or -1 when the meter stays hidden this frame.
 */
s32 ifRemixComboHitsUpdate(IFRemixComboMeter *meters, s32 victim, s32 hit_count, s32 damage);

#endif

#endif
