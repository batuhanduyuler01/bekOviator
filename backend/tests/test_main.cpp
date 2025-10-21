#include <gtest/gtest.h>

// Test main - Google Test otomatik olarak sağlar
// Bu dosya sadece custom setup'lar için gerekirse kullanılır

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}