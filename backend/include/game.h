#pragma once

#include <random>
#include <chrono>
#include <vector>
#include <map>
#include <memory>
#include "player.h"
#include "bet.h"

enum class GamePhase {
    WAITING,     // Oyuncuların bahis yapması için bekleme
    FLYING,      // Helikopter uçuyor, multiplier artıyor
    CRASHED      // Oyun bitti, sonuçlar hesaplanıyor
};

class CrashGame {
private:
    std::mt19937 rng;
    double crash_point;
    double current_multiplier;
    GamePhase phase;
    std::chrono::steady_clock::time_point phase_start_time;
    int current_round;
    
    // Test modu için hızlandırma
    bool test_mode;
    
    // Oyuncu ve bahis yönetimi
    std::map<std::string, std::shared_ptr<Player>> players;
    std::vector<Bet> current_bets;     // Mevcut round'un bahisleri
    std::vector<Bet> next_round_bets;  // Bir sonraki round için bahisler
    
    // Timing ayarları
    static const int WAITING_TIME_MS = 10000;  // 10 saniye bahis zamanı
    static const int CRASHED_TIME_MS = 3000;   // 3 saniye sonuç gösterme
    static const int TEST_WAITING_TIME_MS = 100;  // Test için 100ms
    static const int TEST_CRASHED_TIME_MS = 50;   // Test için 50ms
    
public:
    CrashGame(bool test_mode = false);
    
    // Oyun yönetimi
    void update();
    void start_flying_phase();
    void end_game();
    
    // Oyuncu yönetimi
    bool add_player(const std::string& player_id, const std::string& name);
    std::shared_ptr<Player> get_player(const std::string& player_id);
    bool get_player_by_name(const std::string& name, std::shared_ptr<Player>& out_player);
    
    // Bahis yönetimi
    bool place_bet(const std::string& player_id, double amount);
    bool cashout(const std::string& player_id);
    bool load_balance(const std::string& player_id, double amount);
    
    // Getter'lar
    double get_current_multiplier() const;
    double get_crash_point() const;
    GamePhase get_phase() const;
    std::string get_phase_string() const;
    int get_current_round() const;
    int get_round() const;
    double get_multiplier() const;
    int get_remaining_time_ms() const;
    int get_active_bet_count() const;
    
    // Test modunda hızlı çalışma
    void enable_test_mode();
    bool is_test_mode() const;
    
    // Oyun durumu JSON
    std::string get_game_state_json() const;
    
private:
    // Crash noktası hesaplama
    double calculate_crash_point();
    void update_multiplier();
    void process_crashed_bets();
};