#include "server.h"
#include "json_utils.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

CrashGameServer::CrashGameServer(Address address) : running(false) {
    httpEndpoint = std::make_shared<Http::Endpoint>(address);
    
    // HTTP ayarları
    auto opts = Http::Endpoint::options()
        .threads(2)  // 2 thread kullan
        .flags(Tcp::Options::ReuseAddr);
    
    httpEndpoint->init(opts);
    setupRoutes();
}

CrashGameServer::~CrashGameServer() {
    stop();
}

void CrashGameServer::setupRoutes() {
    using namespace Rest;
    
    // CORS için OPTIONS handler
    Routes::Options(router, "*", [this](const Request&, Http::ResponseWriter res) {
        enableCors(res);
        res.send(Http::Code::Ok);
        return Route::Result::Ok;
    });
    
    // Game status endpoint
    Routes::Get(router, "/api/game/status", 
        Routes::bind(&CrashGameServer::getGameStatus, this));
    
    // Join game endpoint
    Routes::Post(router, "/api/game/join", 
        Routes::bind(&CrashGameServer::joinGame, this));
    Routes::Options(router, "/api/game/join", 
        Routes::bind(&CrashGameServer::handleOptions, this));
    
    // Place bet endpoint
    Routes::Post(router, "/api/game/bet", 
        Routes::bind(&CrashGameServer::placeBet, this));
    Routes::Options(router, "/api/game/bet", 
        Routes::bind(&CrashGameServer::handleOptions, this));
    
    // Cashout endpoint
    Routes::Post(router, "/api/game/cashout", 
        Routes::bind(&CrashGameServer::cashout, this));
    Routes::Options(router, "/api/game/cashout", 
        Routes::bind(&CrashGameServer::handleOptions, this));

    // bringBeko endpoint
    Routes::Post(router, "/api/game/bring-beko", 
        Routes::bind(&CrashGameServer::bringBeko, this));
    Routes::Options(router, "/api/game/bring-beko", 
        Routes::bind(&CrashGameServer::handleOptions, this));

    // Load balance endpoint
    Routes::Post(router, "/api/game/load-balance", 
        Routes::bind(&CrashGameServer::loadBalance, this));
    Routes::Options(router, "/api/game/load-balance", 
        Routes::bind(&CrashGameServer::handleOptions, this));

    // Get players info endpoint
    Routes::Put(router, "/api/game/players", 
        Routes::bind(&CrashGameServer::getPlayersInfo, this));
    Routes::Options(router, "/api/game/players", 
        Routes::bind(&CrashGameServer::handleOptions, this));

    // Get active bets endpoint
    Routes::Get(router, "/api/game/active-bets", 
        Routes::bind(&CrashGameServer::getActiveBets, this));

    httpEndpoint->setHandler(router.handler());
}

void CrashGameServer::enableCors(Http::ResponseWriter& response) {
    response.headers()
        .add<Http::Header::AccessControlAllowOrigin>("*")
        .add<Http::Header::AccessControlAllowMethods>("GET, POST, PUT, OPTIONS")
        .add<Http::Header::AccessControlAllowHeaders>("Content-Type");
}

void CrashGameServer::start() {
    running = true;
    game_thread = std::thread(&CrashGameServer::game_loop, this);
    
    std::cout << "🚀 Crash Game REST API Server başlatıldı!" << std::endl;
    std::cout << "📡 http://localhost:8080" << std::endl;
    
    httpEndpoint->serve();
}

void CrashGameServer::stop() {
    running = false;
    if (game_thread.joinable()) {
        game_thread.join();
    }
    if (httpEndpoint) {
        httpEndpoint->shutdown();
    }
}

void CrashGameServer::game_loop() {
    while (running) {
        game.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 50ms update rate
    }
}

void CrashGameServer::getGameStatus(const Rest::Request&, Http::ResponseWriter response) {
    enableCors(response);
    
    try {
        // 🎮 Modern JSON serialization ile oyun durumunu döndür
        json gameState = GameStateSerializer::serializeGameState(game);
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, gameState.dump());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Game status error: " << e.what() << std::endl;
        
        json errorResponse = JsonUtils::createErrorResponse(
            "Oyun durumu alınamadı", 
            e.what()
        );
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Internal_Server_Error, errorResponse.dump());
    }
}

void CrashGameServer::joinGame(const Rest::Request& request, Http::ResponseWriter response) {
    enableCors(response);
    
    try {
        // 🔍 Request'i parse et ve validate et
        json requestJson = JsonUtils::parseRequest(request.body());
        
        if (!JsonUtils::validateJoinRequest(requestJson)) {
            json errorResponse = JsonUtils::createErrorResponse(
                "Geçersiz request formatı",
                "player_id ve name alanları gerekli"
            );
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, errorResponse.dump());
            return;
        }
        
        // 🎮 Type-safe JSON parsing
        std::string playerId = JsonUtils::getString(requestJson, "player_id");
        std::string name = JsonUtils::getString(requestJson, "name");
        
        std::cout << "🎯 Join request: " << playerId << " (" << name << ")" << std::endl;
        
        bool success = game.add_player(playerId, name);
        
        json responseJson = success ? 
            JsonUtils::createSuccessResponse("Oyuna başarıyla katıldınız") :
            JsonUtils::createErrorResponse("Zaten oyunda varsınız");
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseJson.dump());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Join game error: " << e.what() << std::endl;
        
        json errorResponse = JsonUtils::createErrorResponse(
            "Oyuna katılma hatası", 
            e.what()
        );
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, errorResponse.dump());
    }
}

void CrashGameServer::placeBet(const Rest::Request& request, Http::ResponseWriter response) {
    enableCors(response);
    
    try {
        // 🔍 Request'i parse et ve validate et
        json requestJson = JsonUtils::parseRequest(request.body());
        
        if (!JsonUtils::validateBetRequest(requestJson)) {
            json errorResponse = JsonUtils::createErrorResponse(
                "Geçersiz bahis formatı",
                "player_id ve pozitif amount gerekli"
            );
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, errorResponse.dump());
            return;
        }
        
        // 🎮 Type-safe JSON parsing
        std::string playerId = JsonUtils::getString(requestJson, "player_id");
        double amount = JsonUtils::getDouble(requestJson, "amount");
        
        std::cout << "💰 Bet request: " << playerId << " -> " << amount << " TL" << std::endl;
        
        bool success = game.place_bet(playerId, amount);
        
        json responseJson = success ? 
            JsonUtils::createSuccessResponse("Bahis başarıyla yerleştirildi") :
            JsonUtils::createErrorResponse("Bahis yerleştirilemedi", "Geçersiz oyuncu veya miktar");
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseJson.dump());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Place bet error: " << e.what() << std::endl;
        
        json errorResponse = JsonUtils::createErrorResponse(
            "Bahis yerleştirme hatası", 
            e.what()
        );
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, errorResponse.dump());
    }
}

void CrashGameServer::cashout(const Rest::Request& request, Http::ResponseWriter response) {
    enableCors(response);
    
    try {
        // 🔍 Request'i parse et ve validate et
        json requestJson = JsonUtils::parseRequest(request.body());
        
        if (!JsonUtils::validateCashoutRequest(requestJson)) {
            json errorResponse = JsonUtils::createErrorResponse(
                "Geçersiz cashout formatı",
                "player_id gerekli"
            );
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, errorResponse.dump());
            return;
        }
        
        // 🎮 Type-safe JSON parsing
        std::string playerId = JsonUtils::getString(requestJson, "player_id");
        
        std::cout << "💸 Cashout request: " << playerId << std::endl;
        
        bool success = game.cashout(playerId);
        
        json responseJson = success ? 
            JsonUtils::createSuccessResponse("Başarıyla cashout yapıldı") :
            JsonUtils::createErrorResponse("Cashout yapılamadı", "Aktif bahis bulunamadı");
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseJson.dump());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Cashout error: " << e.what() << std::endl;
        
        json errorResponse = JsonUtils::createErrorResponse(
            "Cashout hatası", 
            e.what()
        );
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, errorResponse.dump());
    }
}

void CrashGameServer::handleOptions(const Rest::Request&, Http::ResponseWriter response) {
    enableCors(response);
    response.send(Http::Code::Ok, "");
}

void CrashGameServer::bringBeko(const Rest::Request& request, Http::ResponseWriter response) {
    enableCors(response);
    try {
        json requestJson = JsonUtils::parseRequest(request.body());
        std::string playerId = JsonUtils::getString(requestJson, "player_id");
        auto player = game.get_player(playerId);
        if (!player) {
            json errorResponse = JsonUtils::createErrorResponse("Oyuncu bulunamadı");
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, errorResponse.dump());
            return;
        }
        double balance = player->get_balance();
        if (balance <= 3000) {
            json errorResponse = JsonUtils::createErrorResponse("Bakiye 3000 TL'den fazla olmalı");
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Ok, errorResponse.dump());
            return;
        }
        player->deduct_balance(balance);
        std::vector<std::string> ulkeler = {"türkiye", "kuzey irak", "fildisi sahilleri"};
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, ulkeler.size() - 1);
        std::string secilen_ulke = ulkeler[dis(gen)];
        json responseJson = JsonUtils::createSuccessResponse("Ülke seçildi", { {"ulke", secilen_ulke} });
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseJson.dump());
    } catch (const std::exception& e) {
        json errorResponse = JsonUtils::createErrorResponse("bringBeko hatası", e.what());
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, errorResponse.dump());
    }
}

void CrashGameServer::loadBalance(const Rest::Request& request, Http::ResponseWriter response) {
    enableCors(response);
    
    try {
        json requestJson = JsonUtils::parseRequest(request.body());
        std::string playerName = JsonUtils::getString(requestJson, "player_name");
        double amount = JsonUtils::getDouble(requestJson, "amount");

        std::cout << "💳 Load balance request: " << playerName << " -> " << amount << " TL" << std::endl;
        
        std::shared_ptr<Player> _player =  nullptr;
        game.get_player_by_name(playerName, _player);
        if (_player == nullptr) {
            json errorResponse = JsonUtils::createErrorResponse("Oyuncu bulunamadı", "Geçersiz oyuncu adı");
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, errorResponse.dump());
            return;
        }

        bool success = game.load_balance(_player->get_id(), amount);
        
        json responseJson = success ? 
            JsonUtils::createSuccessResponse("Bakiye başarıyla yüklendi") :
            JsonUtils::createErrorResponse("Bakiye yüklenemedi", "Geçersiz oyuncu veya miktar");
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseJson.dump());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Load balance error: " << e.what() << std::endl;
        
        json errorResponse = JsonUtils::createErrorResponse(
            "Bakiye yükleme hatası", 
            e.what()
        );
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, errorResponse.dump());
    }
}

void CrashGameServer::getPlayersInfo(const Rest::Request& request, Http::ResponseWriter response) {
    enableCors(response);

    try {
        json requestJson = JsonUtils::parseRequest(request.body());
        std::string playerId = JsonUtils::getString(requestJson, "player_id");
        auto player = game.get_player(playerId);
        if (!player) {
            json errorResponse = JsonUtils::createErrorResponse("Oyuncu bulunamadı");
            response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
            response.send(Http::Code::Bad_Request, errorResponse.dump());
            return;
        }
        
        
        json playerJson;
        playerJson["player_id"] = player->get_id();
        playerJson["name"] = player->get_name();
        playerJson["balance"] = player->get_balance();
        
        json responseJson = JsonUtils::createSuccessResponse("Oyuncu bilgileri alındı", playerJson);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, responseJson.dump());
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Get players info error: " << e.what() << std::endl;
        
        json errorResponse = JsonUtils::createErrorResponse(
            "Oyuncu bilgileri alınamadı", 
            e.what()
        );
        
        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Bad_Request, errorResponse.dump());
    }
}

void CrashGameServer::getActiveBets(const Rest::Request&, Http::ResponseWriter response) {
    enableCors(response);
    
    try {
        // 🎮 Modern JSON serialization ile aktif bahisleri döndür

        json activeBets {};
        game.get_current_bets_json(activeBets);

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Ok, activeBets.dump());

    } catch (const std::exception& e) {
        std::cerr << "❌ Game status error: " << e.what() << std::endl;

        json errorResponse = JsonUtils::createErrorResponse(
            "Oyun durumu alınamadı",
            e.what()
        );

        response.headers().add<Http::Header::ContentType>(MIME(Application, Json));
        response.send(Http::Code::Internal_Server_Error, errorResponse.dump());
    }
}