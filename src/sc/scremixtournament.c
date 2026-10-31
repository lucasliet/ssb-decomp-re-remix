#include <sc/scremixtournament.h>

/* Settings.asm constants: Global.vs.game_mode 3 (time and stock), time 8 minutes,
 * 3 stocks, team attack on, item frequency 0 (none). */
#define SC_REMIX_TOURNAMENT_TIME_LIMIT 8
#define SC_REMIX_TOURNAMENT_STOCKS 3

/**
 * Applies the Smash Remix tournament VS rules to a match settings block.
 * Settings.asm hooks scManagerInit at 0x800A1B48, right after the default VS state is copied.
 *
 * @param battle_state Match settings block, the one Global.vs maps to (gSCManagerTransferBattleState).
 */
void scRemixTournamentApplyVSDefaults(SCBattleState *battle_state)
{
    battle_state->game_rules = SCBATTLE_GAMERULE_TIME | SCBATTLE_GAMERULE_STOCK;
    battle_state->time_limit = SC_REMIX_TOURNAMENT_TIME_LIMIT;
    battle_state->stocks = SC_REMIX_TOURNAMENT_STOCKS;
    battle_state->is_team_attack = TRUE;
    battle_state->item_appearance_rate = nSCBattleItemSwitchNone;
}
