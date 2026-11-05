/**
 * sc1premixallstarrestrules.c - layout rules of the All-Star Rest Area icons, see sc1premixallstarrestrules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixallstarrestrules.h"

#define SC1PREMIX_ALLSTAR_REST_ICON_X 30.0F
#define SC1PREMIX_ALLSTAR_REST_ICON_X_STEP 9.0F
#define SC1PREMIX_ALLSTAR_REST_ICON_Y 30.0F
#define SC1PREMIX_ALLSTAR_REST_ICON_Y_DRAGON_KING 180.0F
#define SC1PREMIX_ALLSTAR_REST_ICON_Y_STEP 11.0F
#define SC1PREMIX_ALLSTAR_REST_DRAGON_KING_HUD_ALWAYS 1

int sc1PRemixAllstarRestIconCount(int fighter_count, int progress)
{
    if ((progress < 0) || (progress >= fighter_count))
    {
        return 0;
    }
    return fighter_count - progress;
}

void sc1PRemixAllstarRestIconPosition(int index, int dragon_king_hud, float *x, float *y)
{
    int row = index / SC1PREMIX_ALLSTAR_REST_ICONS_PER_ROW;
    int column = index % SC1PREMIX_ALLSTAR_REST_ICONS_PER_ROW;
    float first_row_y = (dragon_king_hud == SC1PREMIX_ALLSTAR_REST_DRAGON_KING_HUD_ALWAYS) ? SC1PREMIX_ALLSTAR_REST_ICON_Y_DRAGON_KING : SC1PREMIX_ALLSTAR_REST_ICON_Y;

    *x = SC1PREMIX_ALLSTAR_REST_ICON_X + (SC1PREMIX_ALLSTAR_REST_ICON_X_STEP * (float)column);
    *y = first_row_y + (SC1PREMIX_ALLSTAR_REST_ICON_Y_STEP * (float)row);
}

#endif
