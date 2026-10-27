#ifndef _SC1PREMIXHRCTRAVELRULES_H_
#define _SC1PREMIXHRCTRAVELRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * X of the projectiles and of the player in the Home-Run Contest (HRC.asm update_projectile_and_hmn_positions_). The camera warps back as
 * the Sandbag flies, so whatever is far down the field has to be moved back with it to stay where the camera looks. Kept apart so the
 * headless unit tests (tests/remix) run them.
 */

#define SC1PREMIX_HRC_TRAVEL_LEFT_THRESHOLD (-1152.0F)
#define SC1PREMIX_HRC_TRAVEL_LEFT_PADDING (-4560.0F)

/** What is remembered about one object that went past the threshold: the distance it travelled from there and its previous X. */
typedef struct SC1PRemixHrcTraveler
{
    int is_tracking;
    float distance;
    float previous_x;

} SC1PRemixHrcTraveler;

/**
 * Where an object goes this frame. Below the threshold it stays; past it the distance travelled is followed and the object is placed
 * relative to the warped camera, or held at the threshold while it has not travelled far enough to be seen.
 *
 * @param traveler Memory of the object, updated.
 * @param x X of the object.
 * @param camera_x X the camera looks at, after this frame's warp.
 * @param warp_distance Total the camera has warped back.
 * @return The X to give the object.
 */
extern float sc1PRemixHrcTravelerStep(SC1PRemixHrcTraveler *traveler, float x, float camera_x, float warp_distance);

#endif

#endif
