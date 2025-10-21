#include <gtest/gtest.h>
#include "game.h"
#include <thread>
#include <chrono>

class GameTest : public ::testing::Test {
protected:
    void SetUp() override {
        game = std::make_unique<CrashGame>(true); // Test modu aktif
    }

    void TearDown() override {
        game.reset();
    }

    std::unique_ptr<CrashGame> game;
};

TEST_F(GameTest, InitialState) {
    EXPECT_EQ(game->get_phase(), GamePhase::WAITING);
    EXPECT_EQ(game->get_current_round(), 1);
    EXPECT_EQ(game->get_current_multiplier(), 1.0);
    EXPECT_GT(game->get_remaining_time_ms(), 0);
}

TEST_F(GameTest, AddPlayer_Success) {
    EXPECT_TRUE(game->add_player("player1", "Ahmet"));
    
    auto player = game->get_player("player1");
    ASSERT_NE(player, nullptr);
    EXPECT_EQ(player->get_id(), "player1");
    EXPECT_EQ(player->get_name(), "Ahmet");
    EXPECT_EQ(player->get_balance(), 1000.0); // Default balance
}

TEST_F(GameTest, AddPlayer_DuplicateId) {
    EXPECT_TRUE(game->add_player("player1", "Ahmet"));
    EXPECT_FALSE(game->add_player("player1", "Mehmet")); // Aynı ID
    
    auto player = game->get_player("player1");
    EXPECT_EQ(player->get_name(), "Ahmet"); // İlk isim korunmalı
}

TEST_F(GameTest, GetPlayer_NotFound) {
    auto player = game->get_player("nonexistent");
    EXPECT_EQ(player, nullptr);
}

TEST_F(GameTest, PlaceBet_Success) {
    game->add_player("player1", "Ahmet");
    
    EXPECT_TRUE(game->place_bet("player1", 100.0));
    
    auto player = game->get_player("player1");
    EXPECT_EQ(player->get_balance(), 900.0); // 1000 - 100
}

TEST_F(GameTest, PlaceBet_InsufficientBalance) {
    game->add_player("player1", "Ahmet");
    
    EXPECT_FALSE(game->place_bet("player1", 1500.0)); // 1000'den fazla
    
    auto player = game->get_player("player1");
    EXPECT_EQ(player->get_balance(), 1000.0); // Bakiye değişmemeli
}

TEST_F(GameTest, PlaceBet_PlayerNotFound) {
    EXPECT_FALSE(game->place_bet("nonexistent", 100.0));
}

TEST_F(GameTest, CrashPointGeneration) {
    // FLYING phase'e geçmek için update'leri çağır
    while (game->get_phase() == GamePhase::WAITING) {
        game->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    double crash_point = game->get_crash_point();
    EXPECT_GE(crash_point, 1.01); // Minimum 1.01x
    EXPECT_LE(crash_point, 1000.0); // Maksimum 1000x
    
    // Birkaç kez daha test et
    for (int i = 0; i < 10; ++i) {
        auto test_game = std::make_unique<CrashGame>(true);
        while (test_game->get_phase() == GamePhase::WAITING) {
            test_game->update();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        double cp = test_game->get_crash_point();
        EXPECT_GE(cp, 1.01);
        EXPECT_LE(cp, 1000.0);
    }
}

TEST_F(GameTest, MultiplierProgression) {
    // FLYING phase'e geç
    while (game->get_phase() == GamePhase::WAITING) {
        game->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    double initial_multiplier = game->get_current_multiplier();
    EXPECT_EQ(initial_multiplier, 1.0);
    
    // Daha uzun bekle ve multiplier'ın arttığını kontrol et
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    game->update();
    
    double new_multiplier = game->get_current_multiplier();
    EXPECT_GT(new_multiplier, initial_multiplier);
}

TEST_F(GameTest, GameStateJSON) {
    std::string json = game->get_game_state_json();
    
    // JSON'da temel alanların varlığını kontrol et
    EXPECT_NE(json.find("\"round\""), std::string::npos);
    EXPECT_NE(json.find("\"phase\""), std::string::npos);
    EXPECT_NE(json.find("\"multiplier\""), std::string::npos);
    EXPECT_NE(json.find("\"remaining_time_ms\""), std::string::npos);
}

// Performance test - çok sayıda oyuncu ekle
TEST_F(GameTest, ManyPlayersPerformance) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // 1000 oyuncu ekle
    for (int i = 0; i < 1000; ++i) {
        std::string player_id = "player" + std::to_string(i);
        std::string name = "Player" + std::to_string(i);
        EXPECT_TRUE(game->add_player(player_id, name));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 1000 oyuncu ekleme 1 saniyeden az sürmeli
    EXPECT_LT(duration.count(), 1000);
}

// Integration test - tam oyun döngüsü
TEST_F(GameTest, FullGameCycle) {
    // Oyuncular ekle
    game->add_player("player1", "Ahmet");
    game->add_player("player2", "Mehmet");
    
    // Bahis yap
    EXPECT_TRUE(game->place_bet("player1", 100.0));
    EXPECT_TRUE(game->place_bet("player2", 200.0));
    
    // FLYING phase'e geçmesini bekle
    while (game->get_phase() == GamePhase::WAITING) {
        game->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    EXPECT_EQ(game->get_phase(), GamePhase::FLYING);
    
    // Multiplier artışını kontrol et
    double initial_multiplier = game->get_current_multiplier();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    game->update();
    EXPECT_GT(game->get_current_multiplier(), initial_multiplier);
}