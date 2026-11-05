#ifndef _FTREMIXKIRBYGUNRULES_H_
#define _FTREMIXKIRBYGUNRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the Kirby held-weapon parts (KirbyHats.asm:112 use_extended_special_parts_pause_, gun branch at 0x800E8D04 in
 * ftParamSetModelPartID): a Kirby or J Kirby wearing the hat of a fighter with a held weapon replaces the model part
 * descriptor of joint 0x11 with a special part struct that sits in Kirby's model file. Only int, so the headless unit tests
 * (tests/remix) can run it.
 */

#define FTREMIX_KIRBY_GUN_JOINT 0x11
#define FTREMIX_KIRBY_GUN_NO_OFFSET (-1)

/**
 * File offset of the special part struct of a Kirby hat.
 *
 * @param hat_id Kirby hat id (N64 FTStruct 0x0980).
 * @return Offset in Kirby's model file, or FTREMIX_KIRBY_GUN_NO_OFFSET when the hat holds no weapon.
 */
extern int ftRemixKirbyGunFileOffset(int hat_id);

#endif

#endif
