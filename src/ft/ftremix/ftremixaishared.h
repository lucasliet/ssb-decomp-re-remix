#ifndef _FTREMIXAISHARED_H_
#define _FTREMIXAISHARED_H_

#if defined(PORT) && defined(REGION_REMIX)

#include <ft/fighter.h>

/* CPU hooks of the Remix <family>shared.asm files (dkshared, jigglypuffkirbyshared, nessshared, yoshishared). */

/** kirby_cpu_inhale / _2 selector: the copied power of a Kirby or J Kirby, the own fkind for everyone else (Dedede included). */
s32 ftRemixAiCopiedPowerKind(FTStruct *fp);

/** cpu_fix_2: TRUE when a Marina or N Marina CPU carries a victim in her Cargo, so it must not evade. */
sb32 ftRemixAiEvadeIsMarinaCargo(FTStruct *fp);

/** kirby_cpu_inhale_3: presses B when a Dedede, or a Kirby wearing the Dedede hat, holds an inhaled victim. TRUE when it did. */
sb32 ftRemixAiInhaleSpitsHeldVictim(FTStruct *fp);

/** kirby_cpu_inhale_3: TRUE when the vanilla Kirby inhale block runs (Kirby and J Kirby without the Dedede hat). */
sb32 ftRemixAiInhaleUsesKirbyBlock(FTStruct *fp);

/** multi_jump_recovery_fix_: TRUE for the fighters that take the Kirby and Jigglypuff walk branch (Kirby, Puff, JPuff, EPuff, Marina, Dedede). */
sb32 ftRemixAiWalkUsesMultiJumpBranch(FTStruct *fp);

/** multi_jump_recovery_fix_: TRUE when a Dedede in USP_MOVE reaches the multi jump branch and the walk objective must return. */
sb32 ftRemixAiWalkExitsInDededeUsp(FTStruct *fp);

/** remix_skip_offensive_usp_air_: TRUE for Jigglypuff, JPuff and EPuff, whose up special is skipped (90 percent) as an attack. */
sb32 ftRemixAiUspIsPuffAttackSkipKind(s32 fkind);

/** nessshared.asm USP hooks: TRUE for Ness, J Ness and Lucas, who share the PK Thunder logic. */
sb32 ftRemixAiIsPkThunderKind(s32 fkind);

/** yoshi_cpu_fix_1: FALSE when the fighter does not recover with its up special (Yoshi, J Yoshi, Purin, Marina with jumps left). */
sb32 ftRemixAiRecoveryMayUseUsp(FTStruct *fp);

/** save_cpu_recovery_location_: remembers the ledge nearest to a PK Thunder owner (left of the stage center, else right). */
void ftRemixAiPkThunderSaveLedge(GObj *fighter_gobj);

/** improved_pk_thunder_: the Y the CPU aims its PK Thunder at; the vanilla 100 unit offset unless the advanced AI aims at the ledge. */
f32 ftRemixAiPkThunderTargetY(FTStruct *fp);

#endif

#endif
