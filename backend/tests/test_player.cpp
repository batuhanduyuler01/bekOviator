#include <gtest/gtest.h>
#include "player.h"

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Her test öncesi çalışır
        player = std::make_unique<Player>("test_id", "TestPlayer", 1000.0);
    }

    void TearDown() override {
        // Her test sonrası çalışır
        player.reset();
    }

    std::unique_ptr<Player> player;
};

TEST_F(PlayerTest, PlayerCreation) {
    EXPECT_EQ(player->get_id(), "test_id");
    EXPECT_EQ(player->get_name(), "TestPlayer");
    EXPECT_EQ(player->get_balance(), 1000.0);
}

TEST_F(PlayerTest, DeductBalance_Success) {
    EXPECT_TRUE(player->deduct_balance(500.0));
    EXPECT_EQ(player->get_balance(), 500.0);
}

TEST_F(PlayerTest, DeductBalance_InsufficientFunds) {
    EXPECT_FALSE(player->deduct_balance(1500.0));
    EXPECT_EQ(player->get_balance(), 1000.0); // Bakiye değişmemeli
}

TEST_F(PlayerTest, DeductBalance_ExactAmount) {
    EXPECT_TRUE(player->deduct_balance(1000.0));
    EXPECT_EQ(player->get_balance(), 0.0);
}

TEST_F(PlayerTest, AddBalance) {
    player->add_balance(250.0);
    EXPECT_EQ(player->get_balance(), 1250.0);
}

TEST_F(PlayerTest, MultipleOperations) {
    // Karma işlemler test et
    EXPECT_TRUE(player->deduct_balance(300.0));  // 1000 -> 700
    player->add_balance(150.0);                   // 700 -> 850
    EXPECT_TRUE(player->deduct_balance(850.0));  // 850 -> 0
    EXPECT_FALSE(player->deduct_balance(1.0));   // 0'dan çıkarılamaz
    
    EXPECT_EQ(player->get_balance(), 0.0);
}

TEST_F(PlayerTest, NegativeAmounts) {
    // Negatif miktarlar için edge case'ler
    double initial_balance = player->get_balance();
    
    // Negatif düşme - bakiye değişmemeli
    EXPECT_FALSE(player->deduct_balance(-50.0));
    EXPECT_EQ(player->get_balance(), initial_balance);
    
    // Negatif ekleme - yine de çalışmalı (borç verebiliriz)
    player->add_balance(-100.0);
    EXPECT_EQ(player->get_balance(), initial_balance - 100.0);
}