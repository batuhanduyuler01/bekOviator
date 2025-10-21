#include "json_utils.h"
#include "game.h"
#include <stdexcept>
#include <iostream>

// 🔧 JSON UTILITY METHODS

json JsonUtils::createSuccessResponse(const std::string& message, const json& data) {
    json response;
    response["success"] = true;
    response["message"] = message;
    
    if (!data.empty() && !data.is_null()) {
        response["data"] = data;
    }
    
    return response;
}

json JsonUtils::createErrorResponse(const std::string& error, const std::string& details) {
    json response;
    response["success"] = false;
    response["error"] = error;
    
    if (!details.empty()) {
        response["details"] = details;
    }
    
    return response;
}

json JsonUtils::parseRequest(const std::string& body) {
    try {
        if (body.empty()) {
            throw std::invalid_argument("Request body is empty");
        }
        
        auto parsed = json::parse(body);
        
        if (parsed.is_null()) {
            throw std::invalid_argument("Invalid JSON: null");
        }
        
        return parsed;
    } catch (const json::parse_error& e) {
        throw std::invalid_argument("JSON parse error: " + std::string(e.what()));
    }
}

// 🔍 VALIDATION METHODS

bool JsonUtils::validateJoinRequest(const json& request) {
    return request.contains("player_id") && 
           request.contains("name") &&
           request["player_id"].is_string() &&
           request["name"].is_string() &&
           !request["player_id"].get<std::string>().empty() &&
           !request["name"].get<std::string>().empty();
}

bool JsonUtils::validateBetRequest(const json& request) {
    return request.contains("player_id") && 
           request.contains("amount") &&
           request["player_id"].is_string() &&
           request["amount"].is_number() &&
           !request["player_id"].get<std::string>().empty() &&
           request["amount"].get<double>() > 0;
}

bool JsonUtils::validateCashoutRequest(const json& request) {
    return request.contains("player_id") &&
           request["player_id"].is_string() &&
           !request["player_id"].get<std::string>().empty();
}

// 🛡️ TYPE-SAFE GETTERS

std::string JsonUtils::getString(const json& obj, const std::string& key, const std::string& defaultValue) {
    if (obj.contains(key) && obj[key].is_string()) {
        return obj[key].get<std::string>();
    }
    return defaultValue;
}

double JsonUtils::getDouble(const json& obj, const std::string& key, double defaultValue) {
    if (obj.contains(key) && obj[key].is_number()) {
        return obj[key].get<double>();
    }
    return defaultValue;
}

int JsonUtils::getInt(const json& obj, const std::string& key, int defaultValue) {
    if (obj.contains(key) && obj[key].is_number_integer()) {
        return obj[key].get<int>();
    }
    return defaultValue;
}

bool JsonUtils::getBool(const json& obj, const std::string& key, bool defaultValue) {
    if (obj.contains(key) && obj[key].is_boolean()) {
        return obj[key].get<bool>();
    }
    return defaultValue;
}

// 🎮 GAME STATE SERIALIZATION

json GameStateSerializer::serializeGameState(const CrashGame& game) {
    json gameState;
    
    // Ana oyun bilgileri
    gameState["round"] = game.get_round();
    gameState["phase"] = game.get_phase_string();
    gameState["multiplier"] = game.get_multiplier();
    gameState["remaining_time_ms"] = game.get_remaining_time_ms();
    gameState["active_bets"] = game.get_active_bet_count();
    
    // Crash bilgisi (sadece crashed phase'de)
    if (game.get_phase_string() == "crashed") {
        gameState["crash_point"] = game.get_crash_point();
    }
    
    // Timestamp
    gameState["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    return gameState;
}

json GameStateSerializer::serializePlayer(const Player& player) {
    json playerJson;
    playerJson["id"] = player.get_id();
    playerJson["name"] = player.get_name();
    playerJson["balance"] = player.get_balance();
    
    return playerJson;
}

json GameStateSerializer::serializeBet(const Bet& bet) {
    json betJson;
    betJson["player_id"] = bet.get_player_id();
    betJson["amount"] = bet.get_amount();
    betJson["cashout_multiplier"] = bet.get_cashout_multiplier();
    betJson["status"] = static_cast<int>(bet.get_status());
    betJson["game_round"] = bet.get_game_round();
    
    return betJson;
}