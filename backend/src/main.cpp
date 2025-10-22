#include "server.h"
#include <iostream>
#include <signal.h>
#include <memory>
#include <pistache/endpoint.h>

using namespace Pistache;

std::unique_ptr<CrashGameServer> server_instance = nullptr;

void signal_handler(int) {
    std::cout << "\nSunucu kapatılıyor..." << std::endl;
    if (server_instance) {
        server_instance->stop();
    }
    exit(0);
}

int main() {
    std::cout << "=== 🚁 Crash Game REST API Server ===" << std::endl;
    
    // Signal handler kurulumu
    signal(SIGINT, signal_handler);
    
    try {
        // Server'ı localhost:5050'de başlat
        Address address(Ipv4::any(), Port(5050));
        server_instance = std::make_unique<CrashGameServer>(address);
        
        std::cout << "✅ Server hazır!" << std::endl;
        std::cout << "🌐 Frontend: http://localhost:3000" << std::endl;
        std::cout << "🔗 API: http://localhost:5050" << std::endl;
        std::cout << "\n📋 Endpoints:" << std::endl;
        std::cout << "  GET  /api/game/status      - Oyun durumu" << std::endl;
        std::cout << "  POST /api/game/join        - Oyuna katıl" << std::endl;
        std::cout << "  POST /api/game/bet         - Bahis yap" << std::endl;
        std::cout << "  POST /api/game/cashout     - Para çek" << std::endl;
        std::cout << "\n🛑 Durdurmak için Ctrl+C'ye basın\n" << std::endl;
        
        server_instance->start();
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Server hatası: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}