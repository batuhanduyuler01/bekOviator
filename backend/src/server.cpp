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
    
    httpEndpoint->setHandler(router.handler());
}

void CrashGameServer::enableCors(Http::ResponseWriter& response) {
    response.headers()
        .add<Http::Header::AccessControlAllowOrigin>("*")
        .add<Http::Header::AccessControlAllowMethods>("GET, POST, OPTIONS")
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