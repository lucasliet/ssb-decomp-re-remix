#ifndef _FTREMIXENVCOLORRULES_H_
#define _FTREMIXENVCOLORRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the Remix env colour opacity (CharEnvColor.asm override_env_color_ _override), so the headless unit tests
 * (tests/remix) can run it. Only int and unsigned int.
 */

/*
 * Render mode word the asm forces on the listed fighter parts when the env alpha is not 0xFF (CharEnvColor.asm
 * RENDER_MODE_ALPHA, set by the E200001C commands of custom_display_lists_*): G_RM_FOG_PRIM_A | G_RM_AA_ZB_XLU_SURF2. The
 * XLU blender keeps Z compare and drops Z update.
 */
#define FTREMIX_ENVCOLOR_FORCED_RENDER_MODE 0xC41049D8u

/**
 * TRUE when an env colour override makes the fighter parts use the alpha render mode: there is an override (0 = none) and
 * its alpha byte is not 0xFF (asm: andi t2, t2, 0xFF; bnel t2, 0xFF, _fix_parts with t6 = 0x20).
 */
extern int ftRemixEnvColorRulesForcesTranslucent(unsigned int rgba);

#endif

#endif
