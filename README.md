# 🚁 BekOviator - Crash Game

[![Docker](https://img.shields.io/badge/Docker-Ready-blue.svg)](https://www.docker.com/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-orange.svg)](https://isocpp.org/)
[![React](https://img.shields.io/badge/React-18.2.0-blue.svg)](https://reactjs.org/)
[![Pistache](https://img.shields.io/badge/Pistache-REST%20API-green.svg)](https://pistache.io/)

**BekOviator**, modern bir crash oyunudur. Oyuncular helikopterin ne zaman düşeceğini tahmin ederek bahis yaparlar. Oyun, gerçek zamanlı multiplier artışı ile heyecan verici bir deneyim sunar.

## 🎮 Özellikler

- **Gerçek Zamanlı Oyun**: 50ms'lik güncelleme döngüsü ile akıcı deneyim
- **RESTful API**: Pistache tabanlı yüksek performanslı backend
- **Modern Frontend**: React 18 ile geliştirilmiş responsive arayüz
- **Çoklu Oyuncu**: Aynı anda birden fazla oyuncu desteği
- **Bahis Sistemi**: Dinamik bahis yerleştirme ve cashout
- **Admin Paneli**: Oyuncu bakiye yönetimi
- **Docker Desteği**: Kolay deployment
- **Cross-Platform**: macOS, Linux, Docker desteği

## 🚀 Kurulum ve Çalıştırma

### Gereksinimler

- **Backend**: C++17, CMake, Pistache, nlohmann/json
- **Frontend**: Node.js 18+, npm
- **Docker**: Opsiyonel (kolay deployment için)

### Hızlı Başlatma (Docker)

```bash
# Projeyi klonlayın
git clone <repository-url>
cd crash-game

# Docker ile build ve çalıştır
docker build -t bekoviator .
docker run -p 80:80 bekoviator

# Tarayıcıda açın: http://localhost
```

### Manuel Kurulum

#### Backend Kurulumu

```bash
# macOS (Homebrew)
brew install pistache nlohmann-json

# Ubuntu/Debian
sudo apt update
sudo apt install libpistache-dev nlohmann-json3-dev cmake build-essential

# Build
cd backend
chmod +x build_*.sh
./build_pistache.sh
./build_json.sh
./build_backend.sh

# Çalıştır
./build/crash_server
```

#### Frontend Kurulumu

```bash
cd frontend
npm install
npm start  # Development: http://localhost:3000
npm run build  # Production build
```

## 🏗️ Proje Yapısı

```
crash-game/
├── backend/                    # C++ REST API Server
│   ├── CMakeLists.txt         # CMake build configuration
│   ├── include/               # Header files
│   │   ├── server.h          # HTTP server class
│   │   ├── game.h            # Main game logic
│   │   ├── player.h          # Player management
│   │   ├── bet.h             # Bet management
│   │   ├── json_utils.h      # JSON utilities
│   │   └── fixed_queue.h     # Circular buffer for crash history
│   └── src/                  # Source files
│       ├── main.cpp          # Server entry point
│       ├── server.cpp        # HTTP request handlers
│       ├── game.cpp          # Game state management
│       ├── player.cpp        # Player operations
│       ├── bet.cpp           # Bet operations
│       └── json_utils.cpp    # JSON serialization
├── frontend/                  # React Web Application
│   ├── package.json          # Node.js dependencies
│   ├── webpack.config.js     # Webpack configuration
│   ├── public/               # Static assets
│   │   └── index.html        # HTML template
│   └── src/                  # React components
│       ├── index.js          # App entry point
│       ├── App.js            # Main application component
│       ├── gameAPI.js        # API client
│       ├── ActiveBets.js     # Active bets display
│       ├── AdminPanel.js     # Admin controls
│       └── App.css           # Styles
├── Dockerfile                 # Docker build configuration
├── docker-compose.yml         # Docker Compose (optional)
├── nginx.conf                 # Nginx reverse proxy config
├── entrypoint.sh              # Docker entrypoint script
└── fly.toml                   # Fly.io deployment config
```

## 📋 API Dokümantasyonu

### Temel Endpoints

| Method | Endpoint | Açıklama |
|--------|----------|----------|
| GET | `/api/game/status` | Oyun durumu (multiplier, phase, vb.) |
| POST | `/api/game/join` | Oyuna katıl |
| POST | `/api/game/bet` | Bahis yap |
| POST | `/api/game/cashout` | Bahsi nakde çevir |
| GET | `/api/game/active-bets` | Aktif bahisleri listele |
| GET | `/api/game/old-crash-points` | Geçmiş crash noktaları |
| POST | `/api/game/bring-beko` | Beko'yu Türkiye'ye getir (özel özellik) |
| POST | `/api/game/load-balance` | Admin: Bakiye yükle |

### Örnek API Kullanımı

```bash
# Oyun durumunu kontrol et
curl http://localhost:5050/api/game/status

# Oyuna katıl
curl -X POST http://localhost:5050/api/game/join \
  -H "Content-Type: application/json" \
  -d '{"player_id": "player123", "name": "Ahmet"}'

# Bahis yap
curl -X POST http://localhost:5050/api/game/bet \
  -H "Content-Type: application/json" \
  -d '{"player_id": "player123", "amount": 100}'

# Cashout
curl -X POST http://localhost:5050/api/game/cashout \
  -H "Content-Type: application/json" \
  -d '{"player_id": "player123"}'
```

## 🏛️ Backend Mimarisi

### Ana Sınıflar

#### `CrashGame`
Ana oyun motoru. Oyun durumunu yönetir:
- **GamePhase**: WAITING, FLYING, CRASHED
- **Multiplier**: Üstel artış ile gerçekçi fizik
- **Crash Point**: Rastgele hesaplanan düşüş noktası
- **Round Management**: Otomatik round geçişi

#### `CrashGameServer`
HTTP sunucusu (Pistache tabanlı):
- **REST API**: JSON tabanlı request/response
- **CORS Support**: Cross-origin requests
- **Threading**: 2 thread ile yüksek performans

#### `Player`
Oyuncu yönetimi:
- **Balance**: Bakiye takibi
- **ID/Name**: Benzersiz kimlik

#### `Bet`
Bahis sistemi:
- **Status**: ACTIVE, CASHED_OUT, CRASHED
- **Multiplier**: Cashout noktası
- **Winnings**: Kazanç hesaplaması

#### `FixedQueue<double>`
Crash geçmişi için circular buffer (maksimum 15 öğe).

### Oyun Akışı

1. **WAITING Phase** (10 saniye): Oyuncular bahis yapabilir
2. **FLYING Phase**: Multiplier artar, oyuncular cashout yapabilir
3. **CRASHED Phase** (3 saniye): Sonuçlar hesaplanır, yeni round başlar

## 🎨 Frontend Mimarisi

### Ana Bileşenler

#### `App.js`
Ana uygulama bileşeni:
- **State Management**: useState hooks
- **Real-time Updates**: 100ms interval
- **Game Logic**: Bahis, cashout işlemleri

#### `gameAPI.js`
API istemcisi:
- **Singleton Pattern**: Tek instance
- **Error Handling**: Network hataları
- **Player ID**: Otomatik ID üretimi

#### `ActiveBets.js`
Aktif bahisleri gösterir:
- **Auto-refresh**: 2 saniyede bir güncelleme
- **Real-time**: Canlı bahis takibi

### UI Özellikleri

- **Responsive Design**: Mobil uyumlu
- **Animations**: CSS transitions ve keyframe animasyonları
- **Notifications**: Toast mesajları
- **Admin Panel**: Gizli admin özellikleri (`#admin` hash ile)

## 🛠️ Teknoloji Stack

### Backend
- **C++17**: Modern C++ özellikleri
- **Pistache**: HTTP REST framework
- **nlohmann/json**: JSON serialization
- **CMake**: Build sistemi
- **Threading**: std::thread ile concurrency

### Frontend
- **React 18**: Modern React hooks
- **Webpack 5**: Module bundling
- **Babel**: ES6+ transpilation
- **CSS3**: Modern styling

### DevOps
- **Docker**: Containerization
- **Nginx**: Reverse proxy
- **Fly.io**: Cloud deployment

## 🔧 Geliştirme Notları

### Build Scripts

```bash
# Backend build scripts
./build_pistache.sh    # Pistache kütüphanesi
./build_json.sh        # JSON kütüphanesi
./build_backend.sh     # Ana proje build
```

### Test Modu

Backend test modunda hızlı çalışır:
- WAITING: 100ms
- CRASHED: 50ms

### Admin Özellikleri

- URL'ye `#admin` ekleyerek admin paneli açılır
- Oyuncu bakiye yükleme
- Özel API endpoint'leri

### Özel Özellikler

- **BekOviator**: Türk futbol temalı özel mod
- **Crash History**: Son 15 crash noktasını saklar
- **Real-time Updates**: Frontend 100ms'de bir güncellenir

## 📊 Oyun Mekaniği

### Crash Hesaplama
```cpp
double CrashGame::calculate_crash_point() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    double random_value = dist(rng);
    double crash_point = 0.99 / random_value;
    // 1.01x - 10.0x arası
    return std::round(crash_point * 100.0) / 100.0;
}
```

### Multiplier Artışı
```cpp
void CrashGame::update_multiplier() {
    double time_seconds = duration.count() / 1000.0;
    current_multiplier = 1.0 + (std::exp(time_seconds * 0.1) - 1.0) * 2.0;
}
```

## 🚀 Deployment

### Fly.io

```bash
fly launch
fly deploy
```

### Docker

```bash
docker build -t bekoviator .
docker run -p 80:80 bekoviator
```

## 🤝 Katkıda Bulunma

1. Fork edin
2. Feature branch oluşturun (`git checkout -b feature/amazing-feature`)
3. Commit edin (`git commit -m 'Add amazing feature'`)
4. Push edin (`git push origin feature/amazing-feature`)
5. Pull Request açın

## 📄 Lisans

Bu proje MIT lisansı altında lisanslanmıştır.

## 👥 Yazar

**BekOviator** - Crash Game projesi

---

🎮 **İyi eğlenceler!** 🚁💥🍺</content>
<parameter name="filePath">/Users/batuhanduyuler/Desktop/Bireysel_Calismalar/bos_denemeler/crash-game/README.md