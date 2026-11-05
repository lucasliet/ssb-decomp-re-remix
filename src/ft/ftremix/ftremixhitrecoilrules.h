#ifndef _FTREMIXHITRECOILRULES_H_
#define _FTREMIXHITRECOILRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rules of the hit recoil patches (Wario/Wario.asm body_slam_recoil_ and check_body_slam_invincible_): which attacks
 * make the attacker recoil or spawn food when they connect, and when Wario's body slam wins a clang. Only int, so the
 * headless unit tests (tests/remix) can run them.
 */

/** Hit types of GMHitType as body_slam_recoil_ reads them in a3. */
#define FTREMIXHITRECOIL_HIT_HURTBOX 0
#define FTREMIXHITRECOIL_HIT_SHIELD 1
#define FTREMIXHITRECOIL_HIT_SHIELD_REHIT 2
#define FTREMIXHITRECOIL_HIT_CLANG 3

typedef enum FTRemixHitRecoilKind
{
    nFTRemixHitRecoilNone,       /* the hit changes nothing */
    nFTRemixHitRecoilRecoil,     /* temp variable 1 = 1: the moveset script starts the recoil */
    nFTRemixHitRecoilFood        /* a Dango (99%) or Tomato (1%) pops out of the victim */

} FTRemixHitRecoilKind;

/**
 * body_slam_recoil_ (0x800E26D0): what the attacker's current status does when its hitbox makes contact.
 *
 * @param fkind Remix fighter id of the attacker (also its port fkind).
 * @param status_id Current status of the attacker.
 * @param hit_type FTREMIXHITRECOIL_HIT_* contact type.
 * @param is_shoulder_invincible 1 when the body slam shoulder hurtbox is invincible.
 * @return FTRemixHitRecoilKind.
 */
extern int ftRemixHitRecoilGetKind(int fkind, int status_id, int hit_type, int is_shoulder_invincible);

/**
 * check_body_slam_invincible_ (Wario.asm:557) status test: Wario and Kirby with the Wario hat in their neutral special.
 *
 * @param fkind Remix fighter id (also the port fkind).
 * @param status_id Current status.
 * @return 1 while the fighter performs the body slam.
 */
extern int ftRemixHitRecoilIsBodySlam(int fkind, int status_id);

#endif

#endif
