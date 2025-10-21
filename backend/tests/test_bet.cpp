#include <gtest/gtest.h>
#include "bet.h"

class BetTest : public ::testing::Test {
protected:
    void SetUp() override {
        bet = std::make_unique<Bet>("player1", 100.0, 1);
    }

    void TearDown() override {
        bet.reset();
    }

    std::unique_ptr<Bet> bet;
};

TEST_F(BetTest, BetCreation) {
    EXPECT_EQ(bet->get_player_id(), "player1");
    EXPECT_EQ(bet->get_amount(), 100.0);
    EXPECT_EQ(bet->get_cashout_multiplier(), 0.0);
    EXPECT_EQ(bet->get_status(), BetStatus::ACTIVE);
    EXPECT_EQ(bet->get_game_round(), 1);
}

TEST_F(BetTest, Cashout_Success) {
    EXPECT_TRUE(bet->cashout(2.5));
    EXPECT_EQ(bet->get_status(), BetStatus::CASHED_OUT);
    EXPECT_EQ(bet->get_cashout_multiplier(), 2.5);
    EXPECT_EQ(bet->calculate_winnings(), 250.0); // 100 * 2.5
}

TEST_F(BetTest, Cashout_AlreadyCashedOut) {
    // İlk cashout
    EXPECT_TRUE(bet->cashout(2.0));
    
    // İkinci cashout denemesi - başarısız olmalı
    EXPECT_FALSE(bet->cashout(3.0));
    EXPECT_EQ(bet->get_cashout_multiplier(), 2.0); // İlk değer korunmalı
}

TEST_F(BetTest, MarkAsCrashed) {
    bet->mark_as_crashed();
    EXPECT_EQ(bet->get_status(), BetStatus::CRASHED);
    EXPECT_EQ(bet->calculate_winnings(), 0.0); // Crash olduğunda kazanç yok
}

TEST_F(BetTest, MarkAsCrashed_AfterCashout) {
    // Önce cashout yap
    bet->cashout(1.5);
    
    // Sonra crash işareti koy - etki etmemeli
    bet->mark_as_crashed();
    EXPECT_EQ(bet->get_status(), BetStatus::CASHED_OUT); // Hala CASHED_OUT olmalı
    EXPECT_EQ(bet->calculate_winnings(), 150.0); // Kazanç korunmalı
}

TEST_F(BetTest, CalculateWinnings_NoCashout) {
    // Cashout yapılmadan kazanç hesaplama
    EXPECT_EQ(bet->calculate_winnings(), 0.0);
    
    // Crash olduktan sonra da kazanç yok
    bet->mark_as_crashed();
    EXPECT_EQ(bet->calculate_winnings(), 0.0);
}

TEST_F(BetTest, HighMultiplierCashout) {
    // Yüksek çarpan test
    EXPECT_TRUE(bet->cashout(50.75));
    EXPECT_EQ(bet->calculate_winnings(), 5075.0); // 100 * 50.75
}

TEST_F(BetTest, LowMultiplierCashout) {
    // Düşük çarpan test (1.01x gibi)
    EXPECT_TRUE(bet->cashout(1.01));
    EXPECT_EQ(bet->calculate_winnings(), 101.0); // 100 * 1.01
}