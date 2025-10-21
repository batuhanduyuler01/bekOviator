#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// 🔧 JSON UTILITY SINIFI - Temiz JSON işlemleri için
class JsonUtils {
public:
    // ✅ Response JSON'ları oluşturma
    static json createSuccessResponse(const std::string& message, const json& data = json::object());
    static json createErrorResponse(const std::string& error, const std::string& details = "");
    
    // ✅ Request JSON'ları parse etme
    static json parseRequest(const std::string& body);
    
    // ✅ JSON validation
    static bool validateJoinRequest(const json& request);
    static bool validateBetRequest(const json& request);
    static bool validateCashoutRequest(const json& request);
    
    // ✅ Type-safe JSON getters
    static std::string getString(const json& obj, const std::string& key, const std::string& defaultValue = "");
    static double getDouble(const json& obj, const std::string& key, double defaultValue = 0.0);
    static int getInt(const json& obj, const std::string& key, int defaultValue = 0);
    static bool getBool(const json& obj, const std::string& key, bool defaultValue = false);
};

// 🎮 GAME STATE SERIALIZATION - Oyun durumunu JSON'a çevirme
class GameStateSerializer {
public:
    static json serializeGameState(const class CrashGame& game);
    static json serializePlayer(const class Player& player);
    static json serializeBet(const class Bet& bet);
};