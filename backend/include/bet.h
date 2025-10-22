#pragma once

#include <string>

enum class BetStatus {
    ACTIVE,      // Bahis aktif, henüz cashout yapılmamış
    CASHED_OUT,  // Oyuncu cashout yaptı
    CRASHED      // Oyun crash oldu, bahis kaybedildi
};

class Bet {
private:
    std::string player_id;
    double amount;
    double cashout_multiplier;
    BetStatus status;
    int game_round;
    std::string player_name;
    
public:
    Bet(const std::string& p_id, double bet_amount, int round);
    Bet(const std::string& p_id, double bet_amount, int round,const std::string& p_name);
    
    // Getter'lar
    std::string get_player_id() const;
    std::string get_player_name() const;
    double get_amount() const;
    double get_cashout_multiplier() const;
    BetStatus get_status() const;
    int get_game_round() const;
    
    // Bahis işlemleri
    bool cashout(double current_multiplier);
    void mark_as_crashed();
    double calculate_winnings() const;
};