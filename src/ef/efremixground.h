#ifndef _EFREMIXGROUND_H_
#define _EFREMIXGROUND_H_

/*
 * Remix bg_animation_ (Stages.asm 2713-2770): the vanilla ground actor of efGroundMakeAppearActor reads its EFGroundData
 * from a per-stage table, the vanilla rows for ids 0..7 and the add_bg_animation rows (efremixgroundrules.c) for the
 * added stages.
 */

#include <ssb_types.h>
#include <ef/eftypes.h>

/**
 * Returns the ground actor data of a stage (asm bg_animation_ _id_check: the table entry, zero when none).
 *
 * @param gkind Stage id.
 * @return The vanilla dEFGroundDatas row, the Remix row built from efremixgroundrules.c, or NULL.
 */
EFGroundData* efRemixGroundGetData(s32 gkind);

/**
 * Countdown of the ground actor (sEFGroundActor make_wait, RAM 0x80131AD8), which Hazards.asm boo_sound_ reads.
 *
 * @return The frames left before the actor makes its next sprite.
 */
s32 efRemixGroundGetActorMakeWait(void);

#endif
