/**
 * sc1premixbonus3rules.c - state-free rules of the Remix Bonus 3 mode, see sc1premixbonus3rules.h.
 */

#if defined(PORT) && defined(REGION_REMIX)

#include "sc/sc1pmode/sc1premixbonus3rules.h"

#define SC1PREMIX_BONUS3_TIMER_DIGIT_Y 30.0F
#define SC1PREMIX_BONUS3_TIMER_SYMBOL_Y 20.0F
#define SC1PREMIX_BONUS3_TIMER_DIGIT_Y_DRAGON_KING 210.0F
#define SC1PREMIX_BONUS3_TIMER_SYMBOL_Y_DRAGON_KING 200.0F
#define SC1PREMIX_BONUS3_DRAGON_KING_HUD_ALWAYS 1

int sc1PRemixBonus3RecordVerdict(unsigned int previous, unsigned int finish_time)
{
    if ((previous == 0) || (previous == SC1PREMIX_BONUS3_RECORD_FILLER))
    {
        return SC1PREMIX_BONUS3_RECORD_FIRST;
    }
    return (finish_time < previous) ? SC1PREMIX_BONUS3_RECORD_BEATEN : SC1PREMIX_BONUS3_RECORD_KEEP;
}

int sc1PRemixBonus3CompleteFgm(int verdict, int complete_fgm, int record_fgm)
{
    return (verdict == SC1PREMIX_BONUS3_RECORD_BEATEN) ? record_fgm : complete_fgm;
}

void sc1PRemixBonus3TimerY(int dragon_king_hud, float *digit_y, float *symbol_y)
{
    int is_low = (dragon_king_hud == SC1PREMIX_BONUS3_DRAGON_KING_HUD_ALWAYS);

    *digit_y = is_low ? SC1PREMIX_BONUS3_TIMER_DIGIT_Y_DRAGON_KING : SC1PREMIX_BONUS3_TIMER_DIGIT_Y;
    *symbol_y = is_low ? SC1PREMIX_BONUS3_TIMER_SYMBOL_Y_DRAGON_KING : SC1PREMIX_BONUS3_TIMER_SYMBOL_Y;
}

#endif
