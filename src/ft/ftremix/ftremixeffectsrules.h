#ifndef _FTREMIXEFFECTSRULES_H_
#define _FTREMIXEFFECTSRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <stdint.h>

/*
 * Pure rules of the fighter effect patches that vanilla effect and capture code calls (REGION_REMIX only): the Dedede inhale
 * wind and captured timer (Dedede.asm), the Falcon punch gfx of the Falcon clones, Wolf and the Kirby that copied them
 * (captainshared.asm get_punch_anim_struct_, get_punch_bone_), the Wolf and Slippy reflector (Wolf.asm wolf_reflect_graphic)
 * and the Lucas PK Thunder and PSI Magnet gfx (nessshared.asm, Lucas.asm). Only int and float, so the headless unit tests
 * (tests/remix) can run them without game state. The engine side is ftremixeffects.c and ftremixdkcaptainshared.c.
 */

/* dedede_gfx_ (Dedede.asm): x offset of the inhale wind, 0x447A0000; the Kirby routine at 0x80104240 uses 800 */
#define FTREMIX_EFFECTS_INHALE_WIND_X_KIRBY 800.0F
#define FTREMIX_EFFECTS_INHALE_WIND_X_DEDEDE 1000.0F

/* custom_initial_absorbed_timer_ (Dedede.asm): INITIAL_ABSORB_TIME, plus the damage of the inhaled fighter */
#define FTREMIX_EFFECTS_INHALE_BREAKOUT_BASE_DEDEDE 250

/* slash_anim_struct_WOLF (captainshared.asm): the offsets of the Wolf slash in WOLF file 9 (the animation was added to an existing file) */
#define FTREMIX_EFFECTS_WOLF_SLASH_O_DOBJSETUP 0x8F0
#define FTREMIX_EFFECTS_WOLF_SLASH_O_MOBJSUB 0xA90
#define FTREMIX_EFFECTS_WOLF_SLASH_O_ANIM_JOINT 0x0
#define FTREMIX_EFFECTS_WOLF_SLASH_O_MATANIM_JOINT 0xABC

/* pkthunder_anim_struct_lucas (nessshared.asm): offsets of the Lucas PK Thunder head gfx in LUCAS file 4 */
#define FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_DOBJSETUP 0x121B8
#define FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_MOBJSUB 0x122F8
#define FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_ANIM_JOINT 0x123E8
#define FTREMIX_EFFECTS_LUCAS_PKTHUNDER_WAVE_O_MATANIM_JOINT 0x124A0

/* pkthunder_anim_struct2_lucas and pkthunder_anim_struct3_lucas (nessshared.asm): offset of the Lucas PK Thunder trail gfx in LUCAS file 4 */
#define FTREMIX_EFFECTS_LUCAS_PKTHUNDER_TRAIL_O_DOBJSETUP 0x125A8

/* dsp_graphic_position_fix_ (Lucas.asm): X_TRANSLATION 0x4370, x translation of the PSI Magnet gfx times the facing */
#define FTREMIX_EFFECTS_LUCAS_PSIMAGNET_GFX_X 240.0F

/* Where the Falcon punch gfx of a power reads its model from. */
typedef enum FTRemixEffectsPunchGfx
{
    nFTRemixEffectsPunchGfxFalcon,
    nFTRemixEffectsPunchGfxOwnFile8,
    nFTRemixEffectsPunchGfxWolfSlash

} FTRemixEffectsPunchGfx;

/* dedede_gfx_: x offset (before the size multiplier) of the inhale wind of `fkind`. Dedede only, not Kirby with his hat. */
float ftRemixEffectsRulesInhaleWindOffsetX(int fkind);

/* custom_initial_absorbed_timer_: breakout wait of a fighter just inhaled by `absorber_fkind`. Only Dedede changes it. */
int ftRemixEffectsRulesInhaleBreakoutWait(int absorber_fkind, int captured_percent, int vanilla_wait);

/* get_punch_anim_struct_: the punch gfx of `power_kind`, the fighter or the power a Kirby copied. Mewtwo's Up Smash does not call the Falcon punch maker. */
FTRemixEffectsPunchGfx ftRemixEffectsRulesPunchGfx(int power_kind);

/* get_punch_anim_struct_: the Falcon punch announcement plays for every power except GND, WOLF and MTWO. */
int ftRemixEffectsRulesPunchAnnounces(int power_kind);

/* wolf_reflect_graphic: Wolf and Slippy draw the reflector from their own file 7. */
int ftRemixEffectsRulesReflectorUsesOwnFile(int fkind);

/* wolf_reflect_graphic: o_dobjsetup and first o_anim_joint of the Wolf and Slippy reflector desc, `vanilla_offset` for any other fighter. */
intptr_t ftRemixEffectsRulesReflectorDObjSetup(int fkind, intptr_t vanilla_offset);
intptr_t ftRemixEffectsRulesReflectorFirstAnimJoint(int fkind, intptr_t vanilla_offset);

/* wolf_reflector_struct and slippy_reflector_struct: animation joint offset of reflector animation 0 to 3 (start, loop, hit, end). */
intptr_t ftRemixEffectsRulesReflectorAnimJoint(int fkind, int anim_id, intptr_t vanilla_offset);

/* The PSI Magnet gfx file 6 of J Ness, Lucas and N Lucas is the Ness file 0x160, the one the Ness desc reads. */
int ftRemixEffectsRulesPsiMagnetUsesFile6(int fkind);

#endif

#endif
