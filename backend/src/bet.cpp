#include "bet.h"

Bet::Bet(const std::string& p_id, double bet_amount, int round) 
    : player_id(p_id), amount(bet_amount), cashout_multiplier(0.0), 
      status(BetStatus::ACTIVE), game_round(round) {
}

std::string Bet::get_player_id() const {
    return player_id;
}

double Bet::get_amount() const {
    return amount;
}

double Bet::get_cashout_multiplier() const {
    return cashout_multiplier;
}

BetStatus Bet::get_status() const {
    return status;
}

int Bet::get_game_round() const {
    return game_round;
}

bool Bet::cashout(double current_multiplier) {
    if (status == BetStatus::ACTIVE) {
        cashout_multiplier = current_multiplier;
        status = BetStatus::CASHED_OUT;
        return true;
    }
    return false;
}

void Bet::mark_as_crashed() {
    if (status == BetStatus::ACTIVE) {
        status = BetStatus::CRASHED;
    }
}

double Bet::calculate_winnings() const {
    if (status == BetStatus::CASHED_OUT) {
        return amount * cashout_multiplier;
    }
    return 0.0;
}