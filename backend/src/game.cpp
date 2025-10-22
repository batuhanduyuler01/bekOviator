#include "game.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

CrashGame::CrashGame(bool test_mode_param) : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    current_multiplier = 1.0;
    crash_point = 0.0;
    phase = GamePhase::WAITING;
    current_round = 1;
    test_mode = test_mode_param;
    phase_start_time = std::chrono::steady_clock::now();
    
    if (!test_mode) {
        std::cout << "Crash Game başlatıldı! İlk round için bahis alma süresi başladı." << std::endl;
    }
}

void CrashGame::update() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - phase_start_time);
    
    int waiting_time = test_mode ? TEST_WAITING_TIME_MS : WAITING_TIME_MS;
    int crashed_time = test_mode ? TEST_CRASHED_TIME_MS : CRASHED_TIME_MS;
    
    switch (phase) {
        case GamePhase::WAITING:
            if (elapsed.count() >= waiting_time) {
                start_flying_phase();
            }
            break;
            
        case GamePhase::FLYING:
            update_multiplier();
            if (current_multiplier >= crash_point) {
                end_game();
            }
            break;
            
        case GamePhase::CRASHED:
            if (elapsed.count() >= crashed_time) {
                // Yeni round başlat
                current_round++;
                current_bets = std::move(next_round_bets);
                next_round_bets.clear();
                phase = GamePhase::WAITING;
                phase_start_time = now;
                if (!test_mode) {
                    std::cout << "\n=== Round " << current_round << " başladı! Bahis zamanı ===" << std::endl;
                }
            }
            break;
    }
}

void CrashGame::start_flying_phase() {
    crash_point = calculate_crash_point();
    current_multiplier = 1.0;
    phase = GamePhase::FLYING;
    phase_start_time = std::chrono::steady_clock::now();
    
    if (!test_mode) {
        std::cout << "\n🚁 Helikopter havalandı! Crash noktası: " << crash_point << "x" << std::endl;
        std::cout << "Aktif bahis sayısı: " << current_bets.size() << std::endl;
    }
}

void CrashGame::update_multiplier() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - phase_start_time);
    
    // Exponential growth formula - daha gerçekçi
    double time_seconds = duration.count() / 1000.0;
    current_multiplier = 1.0 + (std::exp(time_seconds * 0.1) - 1.0) * 2.0;
    
    // 2 ondalık basamağa yuvarla
    current_multiplier = std::round(current_multiplier * 100.0) / 100.0;
}

void CrashGame::end_game() {
    current_multiplier = crash_point;
    phase = GamePhase::CRASHED;
    phase_start_time = std::chrono::steady_clock::now();
    
    if (!test_mode) {
        std::cout << "\n💥 CRASH! " << crash_point << "x'te düştü!" << std::endl;
    }
    
    process_crashed_bets();
}

void CrashGame::process_crashed_bets() {
    for (auto& bet : current_bets) {
        if (bet.get_status() == BetStatus::ACTIVE) {
            bet.mark_as_crashed();
            if (!test_mode) {
                std::cout << "Oyuncu " << bet.get_player_id() << " bahsini kaybetti: " 
                          << bet.get_amount() << " TL" << std::endl;
            }
        } else if (bet.get_status() == BetStatus::CASHED_OUT) {
            auto player = get_player(bet.get_player_id());
            if (player) {
                double winnings = bet.calculate_winnings();
                player->add_balance(winnings);
                if (!test_mode) {
                    std::cout << "Oyuncu " << bet.get_player_id() << " kazandı: " 
                              << winnings << " TL (Çarpan: " << bet.get_cashout_multiplier() << "x)" << std::endl;
                }
            }
        }
    }
}

bool CrashGame::add_player(const std::string& player_id, const std::string& name) {
    if (players.find(player_id) == players.end()) {
        players[player_id] = std::make_shared<Player>(player_id, name);
        if (!test_mode) {
            std::cout << "Yeni oyuncu katıldı: " << name << " (ID: " << player_id << ")" << std::endl;
        }
        return true;
    }
    return false;
}

bool CrashGame::get_player_by_name(const std::string& name, std::shared_ptr<Player>& out_player) {
    for (const auto& pair : players) {
        if (pair.second->get_name() == name) {
            out_player = pair.second;
            return true;
        }
    }
    return false;
}

std::shared_ptr<Player> CrashGame::get_player(const std::string& player_id) {
    auto it = players.find(player_id);
    return (it != players.end()) ? it->second : nullptr;
}

bool CrashGame::place_bet(const std::string& player_id, double amount) {
    auto player = get_player(player_id);
    if (!player) return false;
    
    if (!player->deduct_balance(amount)) {
        if (!test_mode) {
            std::cout << "Oyuncu " << player_id << " yetersiz bakiye!" << std::endl;
        }
        return false;
    }
    
    if (phase == GamePhase::WAITING) {
        // Mevcut round için bahis
        current_bets.emplace_back(player_id, amount, current_round, player->get_name());
        if (!test_mode) {
            std::cout << "Oyuncu " << player_id << " mevcut round için bahis yaptı: " << amount << " TL" << std::endl;
        }
    } else {
        // Bir sonraki round için bahis
        next_round_bets.emplace_back(player_id, amount, current_round + 1, player->get_name());
        if (!test_mode) {
            std::cout << "Oyuncu " << player_id << " bir sonraki round için bahis yaptı: " << amount << " TL" << std::endl;
        }
    }
    
    return true;
}

bool CrashGame::cashout(const std::string& player_id) {
    if (phase != GamePhase::FLYING) return false;
    
    for (auto& bet : current_bets) {
        if (bet.get_player_id() == player_id && bet.get_status() == BetStatus::ACTIVE) {
            bet.cashout(current_multiplier);
            if (!test_mode) {
                std::cout << "Oyuncu " << player_id << " cashout yaptı: " 
                          << current_multiplier << "x (" << bet.calculate_winnings() << " TL)" << std::endl;
            }
            return true;
        }
    }
    return false;
}

bool CrashGame::load_balance(const std::string& player_id, double amount) {
    auto player = get_player(player_id);
    if (!player) return false;
    player->add_balance(amount);
    if (!test_mode) {
        std::cout << "Oyuncu " << player_id << " bakiyesini yükledi: " << amount << " TL" << std::endl;
    }
    return true;
}

double CrashGame::get_current_multiplier() const {
    return current_multiplier;
}

double CrashGame::get_crash_point() const {
    return crash_point;
}

GamePhase CrashGame::get_phase() const {
    return phase;
}

int CrashGame::get_current_round() const {
    return current_round;
}

int CrashGame::get_remaining_time_ms() const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - phase_start_time);
    
    int waiting_time = test_mode ? TEST_WAITING_TIME_MS : WAITING_TIME_MS;
    int crashed_time = test_mode ? TEST_CRASHED_TIME_MS : CRASHED_TIME_MS;
    
    switch (phase) {
        case GamePhase::WAITING:
            return std::max(0, static_cast<int>(waiting_time - elapsed.count()));
        case GamePhase::CRASHED:
            return std::max(0, static_cast<int>(crashed_time - elapsed.count()));
        default:
            return 0;
    }
}

std::string CrashGame::get_game_state_json() const {
    std::stringstream json;
    json << std::fixed << std::setprecision(2);
    
    json << "{";
    json << "\"round\": " << current_round << ",";
    json << "\"phase\": \"";
    
    switch (phase) {
        case GamePhase::WAITING: json << "waiting"; break;
        case GamePhase::FLYING: json << "flying"; break;
        case GamePhase::CRASHED: json << "crashed"; break;
    }
    
    json << "\",";
    json << "\"multiplier\": " << current_multiplier << ",";
    json << "\"crash_point\": " << crash_point << ",";
    json << "\"remaining_time_ms\": " << get_remaining_time_ms() << ",";
    json << "\"active_bets\": " << current_bets.size() << ",";
    json << "\"next_round_bets\": " << next_round_bets.size();
    json << "}";
    
    return json.str();
}

double CrashGame::calculate_crash_point() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double random_value = dist(rng);
    
    // House edge %1 ile crash point hesaplama
    double crash_point = 0.99 / random_value;
    
    // Minimum 1.01x, maksimum 10.0x sınırları
    if (crash_point < 1.01) crash_point = 1.01;
    if (crash_point > 10.0) crash_point = 10.0;
    
    // 2 ondalık basamağa yuvarla
    return std::round(crash_point * 100.0) / 100.0;
}

void CrashGame::enable_test_mode() {
    test_mode = true;
}

bool CrashGame::is_test_mode() const {
    return test_mode;
}

// 🔄 ADDITIONAL GETTER METHODS FOR JSON SERIALIZATION

std::string CrashGame::get_phase_string() const {
    switch (phase) {
        case GamePhase::WAITING: return "waiting";
        case GamePhase::FLYING: return "flying";
        case GamePhase::CRASHED: return "crashed";
        default: return "unknown";
    }
}

int CrashGame::get_round() const {
    return current_round;
}

double CrashGame::get_multiplier() const {
    return current_multiplier;
}

int CrashGame::get_active_bet_count() const {
    return static_cast<int>(current_bets.size());
}

void CrashGame::get_current_bets_json(json &resp) const {
    json active_bet_array = json::array();
    for (const auto& bet : this->current_bets) {
        if (bet.get_status() != BetStatus::CRASHED) {
            json bet_json {};
            bet_json["player_name"] = bet.get_player_name();
            bet_json["amount"] = bet.get_amount();

            active_bet_array.push_back(bet_json);
        }
    }
    resp = active_bet_array;
}
