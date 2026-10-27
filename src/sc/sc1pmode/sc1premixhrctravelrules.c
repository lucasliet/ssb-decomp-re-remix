/**
 * sc1premixhrctravelrules.c - X of the objects past the Home-Run Contest threshold, see sc1premixhrctravelrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixhrctravelrules.h"

float sc1PRemixHrcTravelerStep(SC1PRemixHrcTraveler *traveler, float x, float camera_x, float warp_distance)
{
    float camera_distance;
    float new_x;

    if (x < SC1PREMIX_HRC_TRAVEL_LEFT_THRESHOLD)
    {
        traveler->previous_x = x;
        return x;
    }
    if (traveler->is_tracking == 0)
    {
        traveler->is_tracking = 1;
        traveler->distance = 0.0F;
        traveler->previous_x = SC1PREMIX_HRC_TRAVEL_LEFT_THRESHOLD;
    }
    traveler->distance += x - traveler->previous_x;
    camera_distance = (camera_x + warp_distance) - SC1PREMIX_HRC_TRAVEL_LEFT_THRESHOLD;

    if ((traveler->distance - camera_distance) < SC1PREMIX_HRC_TRAVEL_LEFT_PADDING)
    {
        new_x = SC1PREMIX_HRC_TRAVEL_LEFT_THRESHOLD;
    }
    else new_x = (traveler->distance - warp_distance) + SC1PREMIX_HRC_TRAVEL_LEFT_THRESHOLD;

    traveler->previous_x = new_x;
    return new_x;
}

#endif
