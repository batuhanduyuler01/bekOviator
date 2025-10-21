#pragma once

#include <string>

class Player {
private:
    std::string player_id;
    std::string name;
    double balance;
    
public:
    Player(const std::string& id, const std::string& player_name, double initial_balance = 1000.0);
    
    // Getter'lar
    std::string get_id() const;
    std::string get_name() const;
    double get_balance() const;
    
    // Balance işlemleri
    bool deduct_balance(double amount);
    void add_balance(double amount);
};