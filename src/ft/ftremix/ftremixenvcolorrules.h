#ifndef _FTREMIXENVCOLORRULES_H_
#define _FTREMIXENVCOLORRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the Remix env colour override (CharEnvColor.asm override_env_color_), so the headless unit tests
 * (tests/remix) can run them. Only int and unsigned int.
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

/**
 * TRUE when the env colour override is the colour the fighter draws with. The asm patches only the stage light colour
 * routines (override_env_color_ at 0x800FC9DC and 0x80390548), which the fighter draw reaches when neither the colanim
 * colour 2 nor the fog colour is in use.
 *
 * rgba: override of the port (0 = none).
 * is_use_color2: the colour animation set its colour 2 (GMColAnim.is_use_color2).
 * is_use_fogcolor: the fighter draws with its fog colour (FTStruct.is_use_fogcolor).
 */
extern int ftRemixEnvColorRulesReplacesLight(unsigned int rgba, int is_use_color2, int is_use_fogcolor);

#endif

#endif
