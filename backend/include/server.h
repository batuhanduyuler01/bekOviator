#pragma once

#include "game.h"
#include <string>
#include <thread>
#include <memory>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>

using namespace Pistache;

class CrashGameServer {
private:
    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
    CrashGame game;
    bool running;
    std::thread game_thread;
    
    void setupRoutes();
    void game_loop();
    
    // REST endpoint handlers
    void getGameStatus(const Rest::Request& request, Http::ResponseWriter response);
    void joinGame(const Rest::Request& request, Http::ResponseWriter response);
    void placeBet(const Rest::Request& request, Http::ResponseWriter response);
    void cashout(const Rest::Request& request, Http::ResponseWriter response);
    void loadBalance(const Rest::Request& request, Http::ResponseWriter response);
    void getPlayersInfo(const Rest::Request& request, Http::ResponseWriter response);
    void bringBeko(const Rest::Request& request, Http::ResponseWriter response);
    void handleOptions(const Rest::Request& request, Http::ResponseWriter response);
    void enableCors(Http::ResponseWriter& response);
    void getActiveBets(const Rest::Request& request, Http::ResponseWriter response);
    void getOldCrashPoints(const Rest::Request& request, Http::ResponseWriter response);
    
public:
    CrashGameServer(Address address);
    ~CrashGameServer();
    
    void start();
    void stop();
};