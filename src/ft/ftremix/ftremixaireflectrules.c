/**
 * ftremixaireflectrules.c - pure rules of the Remix CPU reflect and absorb hooks, see ftremixaireflectrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "ft/ftremix/ftremixaireflectrules.h"
#include "remix/remix_roster_data.h"

/** Vanilla reflectors that reach the flag code before the Remix hook: Fox, N Fox and Ness (they never skip a projectile). */
static int ftRemixAiReflectRulesIsVanillaFlagger(int fkind)
{
    return (fkind == REMIX_ID_FOX) || (fkind == REMIX_ID_NFOX) || (fkind == REMIX_ID_NESS);
}

/** Fox-like fighters that reflect (Remix chain of extend_projectile_reflect_initial_, _fox_reflect). */
static int ftRemixAiReflectRulesIsFoxLike(int fkind)
{
    return (fkind == REMIX_ID_JFOX) || (fkind == REMIX_ID_FALCO) || (fkind == REMIX_ID_SLIPPY) || (fkind == REMIX_ID_WOLF);
}

/** Ness-like fighters that absorb projectiles (_ness_absorb), which skip Sheik needles and Conker nuts. */
static int ftRemixAiReflectRulesIsNessLikeAbsorber(int fkind)
{
    return (fkind == REMIX_ID_NNESS) || (fkind == REMIX_ID_JNESS) || (fkind == REMIX_ID_LUCAS) || (fkind == REMIX_ID_NLUCAS) ||
           (fkind == REMIX_ID_PIANO);
}

float ftRemixAiReflectRulesDetectionFrames(int fkind)
{
    if (fkind == REMIX_ID_MARINA)
    {
        return FTREMIX_AI_REFLECT_FRAMES_MARINA;
    }
    return FTREMIX_AI_REFLECT_FRAMES_DEFAULT;
}

int ftRemixAiReflectRulesProjectileRaisesFlag(int fkind, int projectile_kind, int marina_cannot_absorb)
{
    if (ftRemixAiReflectRulesIsVanillaFlagger(fkind) || ftRemixAiReflectRulesIsFoxLike(fkind))
    {
        return 1;
    }
    if (ftRemixAiReflectRulesIsNessLikeAbsorber(fkind))
    {
        return (projectile_kind != FTREMIX_AI_REFLECT_PROJECTILE_SHEIK_NEEDLE) &&
               (projectile_kind != FTREMIX_AI_REFLECT_PROJECTILE_CONKER_NUT);
    }
    if (fkind == REMIX_ID_MARINA)
    {
        return (marina_cannot_absorb == 0) ? 1 : 0;
    }
    return 0;
}

int ftRemixAiReflectRulesItemRaisesFlag(int fkind, int is_table_reflector, int marina_cannot_absorb)
{
    if (ftRemixAiReflectRulesIsVanillaFlagger(fkind))
    {
        return 1;
    }
    if (is_table_reflector == 0)
    {
        return 0;
    }
    if (fkind == REMIX_ID_MARINA)
    {
        return (marina_cannot_absorb == 0) ? 1 : 0;
    }
    return 1;
}

int ftRemixAiReflectRulesMaintainKind(int fkind)
{
    if (ftRemixAiReflectRulesIsFoxLike(fkind))
    {
        return FTREMIX_AI_REFLECT_KIND_FOX;
    }
    if ((fkind == REMIX_ID_JNESS) || (fkind == REMIX_ID_LUCAS) || (fkind == REMIX_ID_NLUCAS))
    {
        return FTREMIX_AI_REFLECT_KIND_NESS;
    }
    if (fkind == REMIX_ID_PIANO)
    {
        return FTREMIX_AI_REFLECT_KIND_PIANO;
    }
    if (fkind == REMIX_ID_MARINA)
    {
        return FTREMIX_AI_REFLECT_KIND_MARINA;
    }
    return FTREMIX_AI_REFLECT_KIND_NONE;
}

/* The asm compares LUCAS twice where the second is labelled NLUCAS, so NLucas never matches (Reflect.asm apply_reflect_input_). */
int ftRemixAiReflectRulesApplyKind(int fkind)
{
    if (fkind == REMIX_ID_NLUCAS)
    {
        return FTREMIX_AI_REFLECT_KIND_NONE;
    }
    return ftRemixAiReflectRulesMaintainKind(fkind);
}

int ftRemixAiReflectRulesIsAbsorbAction(int kind, int action)
{
    if (kind == FTREMIX_AI_REFLECT_KIND_PIANO)
    {
        return (action >= FTREMIX_AI_REFLECT_PIANO_ABSORB_FIRST) && (action < FTREMIX_AI_REFLECT_PIANO_ABSORB_END);
    }
    if (kind == FTREMIX_AI_REFLECT_KIND_MARINA)
    {
        return (action >= FTREMIX_AI_REFLECT_MARINA_ABSORB_FIRST) && (action < FTREMIX_AI_REFLECT_MARINA_ABSORB_END);
    }
    return 0;
}

int ftRemixAiReflectRulesSkipsNspCandidate(int is_target_reflector, int is_own_chase)
{
    return (is_target_reflector != 0) && (is_own_chase != 0);
}

#endif
