/**
 * gmremixsmashketballcamera.c - the camera follows the basketball of Remix Smashketball (Smashketball.asm camera_follow).
 */

#if defined(PORT) && defined(REGION_REMIX)

#include <it/item.h>
#include <it/itremix/itremixbasketball.h>
#include <sys/objdef.h>
#include "gm/gmremixsmashketball.h"
#include "gm/gmremixsmashketballcamera.h"
#include "gm/gmremixsmashketballrules.h"

void gmRemixSmashketballCameraFollowBall(f32 *left, f32 *right, f32 *bottom, f32 *top)
{
    GObj *ball_gobj;
    Vec3f pos;

    if (gmRemixSmashketballIsActive() == FALSE)
    {
        return;
    }
    ball_gobj = itRemixBasketballFind();

    if ((ball_gobj == NULL) || (itRemixBasketballIsRespawning(ball_gobj) != FALSE))
    {
        return;
    }
    pos = DObjGetStruct(ball_gobj)->translate.vec.f;

    gmRemixSmashketballRulesFollowBall(pos.x, pos.y, left, right, bottom, top);
}

#endif
