#include "player.h"

Player::Player(const std::string& id, const std::string& player_name, double initial_balance) 
    : player_id(id), name(player_name), balance(initial_balance) {
}

std::string Player::get_id() const {
    return player_id;
}

std::string Player::get_name() const {
    return name;
}

double Player::get_balance() const {
    return balance;
}

bool Player::deduct_balance(double amount) {
    if (amount < 0) return false;  // Negatif miktarları kabul etme
    if (balance >= amount) {
        balance -= amount;
        return true;
    }
    return false;
}

void Player::add_balance(double amount) {
    balance += amount;
}