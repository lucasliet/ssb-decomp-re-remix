#ifndef _FTREMIXGROUNDEDRULES_H_
#define _FTREMIXGROUNDEDRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of the Character.grounded_script table (Character.asm:2164, read by get_grounded_script_ at 0x800DE408 in
 * mpCommonSetFighterLandingParams): which landing reset each Remix fighter runs. A fighter inherits the entry of its
 * parent (Character.asm:266) unless its own file patches it. Only int, so the headless unit tests (tests/remix) can run it.
 */

typedef enum FTRemixGroundedScriptKind
{
    nFTRemixGroundedScriptNone,           /* DISABLED (0x800DE44C) or a parent entry that is a no-op */
    nFTRemixGroundedScriptClearWord,      /* Mario/Luigi/Captain/Purin case (0x800DE428): `sw r0, 0x0ADC` */
    nFTRemixGroundedScriptClearRecoil,    /* Samus case: charge_recoil (0x0AE0) = 0 */
    nFTRemixGroundedScriptEbi,            /* Ebi.asm grounded_script_: clears 0x0ADC and 0x0AE0 */
    nFTRemixGroundedScriptKirbyHat,       /* Kirby.asm clear_marth_flag_ */
    nFTRemixGroundedScriptPeachFloat      /* Peach.asm grounded_script_ */

} FTRemixGroundedScriptKind;

/**
 * Landing reset of a Remix fighter id.
 *
 * @param fkind Remix fighter id (also the port fkind).
 * @return FTRemixGroundedScriptKind of its effective grounded_script entry. Vanilla ids return None: the vanilla switch in
 * mpCommonSetFighterLandingParams handles them.
 */
extern int ftRemixGroundedScriptKind(int fkind);

#endif

#endif
