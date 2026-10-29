#ifndef _GMREMIXTIMEDSTOCKRULES_H_
#define _GMREMIXTIMEDSTOCKRULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Score arithmetic of Timed Stock (TimedStock.asm), pure so the headless unit tests (tests/remix) can run it: the stocks left weigh
 * more than any damage, the damage taken lowers the score.
 */

#define GMREMIX_TIMEDSTOCK_STOCK_SHIFT 12
#define GMREMIX_TIMEDSTOCK_TUG_STOCK_SHIFT 24

/**
 * Score of a fighter when the match is decided by stocks.
 *
 * @param stocks_left Stocks the fighter still has, counting the one in play.
 * @param stock_damage Damage taken in the stock in play.
 * @return 0 when no stock is left, else the stocks left in the high bits minus the damage.
 */
extern int gmRemixTimedStockRulesStockScore(int stocks_left, int stock_damage);

/**
 * Score of a fighter in Tug of War, where the stocks are the score.
 *
 * @param stock_count Stock count of the fighter (-1 when out).
 * @param kos KOs of the fighter.
 * @param falls Times the fighter fell.
 * @param stock_damage Damage taken in the stock in play.
 */
extern int gmRemixTimedStockRulesTugScore(int stock_count, int kos, int falls, int stock_damage);

/**
 * TRUE when the timer ran out; a match with no time limit never does.
 *
 * @param time_limit Time limit of the match (infinite is 100).
 * @param time_remain Tics left.
 * @param infinite The value of an infinite limit.
 */
extern int gmRemixTimedStockRulesIsTimeUp(int time_limit, int time_remain, int infinite);

#endif

#endif
