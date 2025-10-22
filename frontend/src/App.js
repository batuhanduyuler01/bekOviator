import React, { useState, useEffect } from 'react';
import './App.css';
import { gameAPI } from './gameAPI';
import ActiveBets from './ActiveBets';
import AdminPanel from './AdminPanel';

// 🎯 REACT + BACKEND ENTEGRASYONU
function App() {
    // 🇹🇷 BEKOYU GETİRME FONKSİYONU
  const handleBringBeko = async () => {
    const result = await gameAPI.bringBeko();
    if (result && result.data.ulke) {
      const ulke = result.data.ulke.toLowerCase();
      if (ulke === 'türkiye') {
        addNotification('Oldu! Beko yanımıza döndü.', 'success');
        setBekoOverlayType('istanbul');
      } else if (ulke === 'kuzey irak') {
        addNotification(`Hayır! Beko burada çalışmaya başladı: ${result.data.ulke}`, 'error');
        setBekoOverlayType('irak');
      } else if (ulke === 'fildisi sahilleri') {
        addNotification(`Hayır! Beko burada çalışmaya başladı: ${result.data.ulke}`, 'error');
        setBekoOverlayType('fildisi');
      } else {
        addNotification(`Hayır! Beko burada çalışmaya başladı: ${result.data.ulke}`, 'error');
      }
      setPlayerData(prev => ({
        ...prev,
        balance: 0
      }));
    } else {
      addNotification('Bekoyu getirme işlemi başarısız oldu.', 'error');
    }
  };
  // 📦 STATE (Oyun durumu) - Backend'den gelecek veriler
  const [gameState, setGameState] = useState({
    phase: 'waiting',          // 'waiting', 'flying', 'crashed'
    multiplier: 1.0,           // Mevcut çarpan
    round: 1,                  // Hangi round
    remaining_time_ms: 10000,  // Kalan süre
    active_bets: 0,           // Aktif bahis sayısı
    crash_point: 0            // Crash noktası
  });
  
  const [playerData, setPlayerData] = useState({
    balance: 1000,   // Oyuncu bakiyesi
    currentBet: 0,   // Mevcut bahis
    isInGame: false, // Oyunda mı
    playerName: '',  // Oyuncu adı
    isJoined: false, // Oyuna katıldı mı
    betAmount: 100   // Bahis miktarı input
  });

  const [notifications, setNotifications] = useState([]);
  const [beerBottles, setBeerBottles] = useState([]); // 🍺 Düşen bira şişeleri
  const [helicopterPosition, setHelicopterPosition] = useState({ left: 10, bottom: 20 }); // 🚁 Helikopter pozisyonu
  const [showAdminPanel, setShowAdminPanel] = useState(false); // 🔧 Admin panel
  const [bekoOverlayType, setBekoOverlayType] = useState(null); // 🇹🇷 Beko overlay tipi: 'istanbul', 'irak', 'fildisi sahilleri' veya null
  const [oldCrashPoints, setOldCrashPoints] = useState([]); // 📈 Eski crash pointleri

  // 🔄 BACKEND BAĞLANTISI - Her 100ms'de oyun durumunu güncelle
  useEffect(() => {
    console.log('🎮 Crash Game başlatıldı!');

    // 🔧 Admin panel kontrolü
    const checkAdminHash = () => {
      setShowAdminPanel(window.location.hash === '#admin');
    };
    checkAdminHash();
    window.addEventListener('hashchange', checkAdminHash);
    
    const fetchGameStatus = async () => {
      const status = await gameAPI.getGameStatus();
      if (status) {
        // ⚠️ OYUN DURUMU KONTROLÜ - Yeni round başladığında bahisleri resetle
        if (status.round !== gameState.round && gameState.round > 0) {
          console.log('🔄 Yeni round başladı, bahisler resetleniyor');
          setPlayerData(prev => ({
            ...prev,
            currentBet: 0,
            isInGame: false
          }));
        }
        
        // Set Player Balance
        const players_data = await gameAPI.getPlayersInfo();
        if (players_data.success == true) {
          setPlayerData(prev => ({
            ...prev,
            balance: players_data.data.balance
          }));
        }
   
        // 💥 CRASH DURUMU - Oyun crashedse ve bahisimiz varsa kaybettik
        if (status.phase === 'crashed' && playerData.isInGame && playerData.currentBet > 0) {
          console.log('💥 Oyun crashed! Bahis kaybedildi');
          setPlayerData(prev => ({
            ...prev,
            currentBet: 0,
            isInGame: false
          }));
          addNotification(`💥 Oyun ${status.crash_point?.toFixed(2)}x'te crashed! Bahis kaybedildi.`, 'error');
        }
        
        setGameState(status);

        // 📈 Eski crash pointlerini çek
        const oldPoints = await gameAPI.getOldCrashPoints();
        if (oldPoints) {
          setOldCrashPoints(oldPoints);
        }
      }
    };

    // İlk veri çekme
    fetchGameStatus();

    // Her 100ms'de güncelle (real-time)
    const interval = setInterval(fetchGameStatus, 100);
    
    // Temizlik fonksiyonu
    return () => {
      clearInterval(interval);
      console.log('🛑 Component kapanıyor');
    };
  }, [gameState.round, playerData.isInGame, playerData.currentBet]); // Dependencies eklendi

  // 🚁 HELİKOPTER ANİMASYON LOGIC
  useEffect(() => {
    if (gameState.phase === 'flying') {
      // Multiplier'a göre helikopter pozisyonu hesapla (çapraz hareket - sağa ve yukarı)
      const progress = Math.min((gameState.multiplier - 1) / 9, 1); // 1x-10x arası için normalize
      const newLeft = 10 + (progress * 70); // %10'dan %80'e kadar sağa doğru
      const newBottom = 20 + (progress * 210); // 20px'den 230px'e kadar yukarı doğru
      
      setHelicopterPosition({ left: newLeft, bottom: newBottom });
      
      // 🍺 Her 500ms'de bir bira şişesi düşür
      const bottleInterval = setInterval(() => {
        const newBottle = {
          id: Date.now() + Math.random(),
          left: newLeft + (Math.random() * 20 - 10), // Helikopter etrafında rastgele
          startTime: Date.now()
        };
        
        setBeerBottles(prev => [...prev, newBottle]);
        
        // 2 saniye sonra şişeyi temizle
        setTimeout(() => {
          setBeerBottles(prev => prev.filter(bottle => bottle.id !== newBottle.id));
        }, 2000);
      }, 500);
      
      return () => clearInterval(bottleInterval);
    } else if (gameState.phase === 'waiting') {
      // Waiting durumunda helikopteri başlangıç pozisyonuna getir
      setHelicopterPosition({ left: 10, bottom: 20 });
      setBeerBottles([]); // Tüm şişeleri temizle
    } else if (gameState.phase === 'crashed') {
      // Crash durumunda helikopteri aşağı düşür
      setHelicopterPosition({ left: helicopterPosition.left, bottom: 20 });
      setBeerBottles([]); // Şişeleri temizle
    }
  }, [gameState.phase, gameState.multiplier]);

  // 🎉 OYUNA KATILMA FONKSİYONU
  const handleJoinGame = async () => {
    if (!playerData.playerName.trim()) {
      addNotification('Lütfen isminizi girin!', 'error');
      return;
    }

    const result = await gameAPI.joinGame(playerData.playerName);
    if (result.success) {
      setPlayerData(prev => ({ ...prev, isJoined: true }));
      addNotification(`Oyuna katıldınız! ID: ${gameAPI.getPlayerId()}`, 'success');
    } else {
      addNotification(result.error || 'Oyuna katılınamadı', 'error');
    }
  };

  // 💰 BAHİS YAPMA FONKSİYONU
  const handlePlaceBet = async () => {
    if (!playerData.isJoined) {
      addNotification('Önce oyuna katılmalısınız!', 'error');
      return;
    }

    if (playerData.betAmount > playerData.balance) {
      addNotification('Yetersiz bakiye!', 'error');
      return;
    }

    const result = await gameAPI.placeBet(playerData.betAmount);
    if (result.success) {
      setPlayerData(prev => ({
        ...prev,
        balance: prev.balance - prev.betAmount,
        currentBet: prev.betAmount,
        isInGame: true
      }));
      addNotification(`${playerData.betAmount} TL bahis yapıldı!`, 'success');
    } else {
      addNotification(result.message || 'Bahis yapılamadı', 'error');
    }
  };

  // 💸 CASHOUT FONKSİYONU
  const handleCashout = async () => {
    const result = await gameAPI.cashout();
    if (result.success) {
      const winnings = playerData.currentBet * gameState.multiplier;
      setPlayerData(prev => ({
        ...prev,
        balance: prev.balance + winnings,
        currentBet: 0,
        isInGame: false
      }));
      addNotification(`${winnings.toFixed(2)} TL kazandınız! (${gameState.multiplier}x)`, 'success');
    } else {
      addNotification(result.message || 'Cashout yapılamadı', 'error');
    }
  };

  // 📢 BİLDİRİM SİSTEMİ
  const addNotification = (message, type = 'info') => {
    const notification = {
      id: Date.now(),
      message,
      type,
      timestamp: new Date().toLocaleTimeString()
    };
    setNotifications(prev => [notification, ...prev.slice(0, 4)]); // Son 5 bildirimi tut
  };

  // 🎨 JSX - HTML benzeri syntax (React'in özel dili)
  return (
    <div className="app">
      {/* 🇹🇷 BEKO OVERLAY */}
      {bekoOverlayType && (
        <div className="beko-overlay" onClick={() => setBekoOverlayType(null)}>
          <div className="beko-content">
            <img 
              src={
                bekoOverlayType === 'istanbul' ? '/beko_istanbul.png' : 
                bekoOverlayType === 'irak' ? '/beko_irak.png' : 
                '/beko_fildisi.png'
              } 
              alt={`Beko ${bekoOverlayType === 'istanbul' ? 'İstanbul' : bekoOverlayType === 'irak' ? 'Irak' : 'Fildişi Sahilleri'}`} 
              className="beko-image" 
            />
            <div className="beko-text">
              {bekoOverlayType === 'istanbul' 
                ? `TEŞEKKÜRLER ${playerData.playerName.toUpperCase()}!! AKŞAM BEŞİKTAŞTA KAHVE?`
                : bekoOverlayType === 'irak'
                ? `HAY A**!! ÇALIŞMAYA DEVAM ${playerData.playerName.toUpperCase()}..`
                : `SALMIYORLAR ${playerData.playerName.toUpperCase()}!! ECO BENİ ARASIN KANKAM`
              }
            </div>
          </div>
        </div>
      )}

      {/* 📈 ESKİ CRASH POINTLERİ */}
      <div className="old-crash-points">
        {oldCrashPoints.slice().reverse().map((point, index) => {
          let colorClass = '';
          if (point >= 0 && point < 3) colorClass = 'red';
          else if (point >= 3 && point < 6) colorClass = 'blue';
          else if (point >= 6 && point <= 10) colorClass = 'green';
          return (
            <span key={index} className={`crash-point ${colorClass}`}>
              {point.toFixed(2)}x
            </span>
          );
        })}
      </div>

      <header className="game-header">
        <h1>✈️ bekOviator</h1>
        <div className="round-info">Round {gameState.round}</div>
        <div className="connection-status">
          🔗 {gameState ? 'Bağlı' : 'Bağlantı kesik'}
        </div>
        <div style={{marginTop: '0.5rem', fontSize: '1.1rem', color: '#FF9800'}}>
          Uçak bileti parasını topla, bekoyu Türkiye'ye getir!
        </div>
      </header>

      <main className="game-container">
        {/* Sol taraf - Oyun sahnesi */}
        <section className="game-area">
          <div className={`helicopter-container ${gameState.phase}`}>
            {/* ✈️ UÇAK SPRITE */}
            <div 
              className="helicopter"
              style={{
                left: `${helicopterPosition.left}%`,
                bottom: `${helicopterPosition.bottom}px`,
                transition: gameState.phase === 'flying' ? 'all 0.3s ease' : 'all 1s ease'
              }}
            >
              <div className="plane-sprite">
                <img 
                  src="/game_plane.png" 
                  alt="Uçak" 
                  className="plane-image"
                />
              </div>
            </div>
            
            {/* 🍺 DÜŞEN BİRA ŞİŞELERİ */}
            {beerBottles.map(bottle => (
              <div
                key={bottle.id}
                className="beer-bottle"
                style={{
                  left: `${bottle.left}%`,
                  top: `${helicopterPosition.bottom + 50}px`
                }}
              >
                🍺
              </div>
            ))}
          </div>
          
          <div className="multiplier-display">
            <span className={`multiplier-value ${gameState.phase}`}>
              {gameState.multiplier.toFixed(2)}x
            </span>
            <div className="game-phase">{getPhaseText(gameState.phase)}</div>
          </div>
          
          <div className="timer">
            {gameState.phase === 'waiting' || gameState.phase === 'crashed' ? 
              `Kalan: ${(gameState.remaining_time_ms / 1000).toFixed(1)}s` :
              `Aktif BekOviator: ${gameState.active_bets}`
            }
          </div>

            {gameState.phase === 'crashed' && (
            <div className="crash-info">
              💥 BEKO DÜŞTÜ! {gameState.crash_point?.toFixed(2)}x'te yere indi!
              <div style={{fontSize: '0.8rem', marginTop: '0.5rem'}}>
                Bekoyu Türkiye'ye getiremedik! ✈️💥🍺
              </div>
            </div>
          )}
        </section>

        {/* Sağ taraf - Kontroller */}
        <aside className="control-panel">
          {/* Oyuna katılma */}
          {!playerData.isJoined && (
            <div className="join-section">
              <h3>👋 Oyuna Katılın</h3>
              <input 
                type="text" 
                placeholder="İsminizi girin" 
                value={playerData.playerName}
                onChange={(e) => setPlayerData(prev => ({
                  ...prev, playerName: e.target.value
                }))}
              />
              <button className="join-button" onClick={handleJoinGame}>
                OYUNA KATIL
              </button>
            </div>
          )}

          {/* Bahis paneli */}
          {playerData.isJoined && (
            <div className="betting-section">
              <h3>💰 Göreve Başla</h3>
              <h4> Uçak Bileti ✈️ : 2000 TL 💰  </h4>
              <div className="player-info">
                🎮 {playerData.playerName} | ID: {gameAPI.getPlayerId().slice(-4)}
              </div>
              <div className="bet-input-group">
                <input 
                  type="number" 
                  placeholder="Bahis miktarı" 
                  min="1" 
                  max={playerData.balance}
                  value={playerData.betAmount}
                  onChange={(e) => setPlayerData(prev => ({
                    ...prev, betAmount: parseFloat(e.target.value) || 0
                  }))}
                  disabled={gameState.phase !== 'waiting' || playerData.currentBet > 0}
                />
                <button 
                  className="bet-button" 
                  onClick={handlePlaceBet}
                  disabled={gameState.phase !== 'waiting' || playerData.currentBet > 0 || playerData.betAmount <= 0}
                >
                  {playerData.currentBet > 0 ? 'BAHİS YAPILDI' : 'BAHİS YAP'}
                </button>
              </div>
              
              <button 
                className="cashout-button" 
                disabled={playerData.currentBet <= 0 || gameState.phase !== 'flying'}
                onClick={handleCashout}
              >
                💸 ÇIKIŞ YAP ({playerData.currentBet > 0 ? (playerData.currentBet * gameState.multiplier).toFixed(2) + ' TL' : '0 TL'})
              </button>
              
              <div className="balance">
                Bakiye: {playerData.balance.toFixed(2)} TL
                {playerData.currentBet > 0 && (
                  <div className="current-bet">
                    Aktif Bahis: {playerData.currentBet} TL
                    {gameState.phase === 'flying' && (
                      <div className="potential-win">
                        Potansiyel Kazanç: {(playerData.currentBet * gameState.multiplier).toFixed(2)} TL
                      </div>
                    )}
                  </div>
                )}
                {/* 🇹🇷 BEKOYU GETİR BUTONU */}
                {playerData.balance > 2000 && (
                  <button 
                    className="bring-beko-button"
                    style={{marginTop: '1rem', background: '#2196F3', color: 'white', fontWeight: 'bold', fontSize: '1.1rem', padding: '0.7rem 1.2rem', borderRadius: '8px'}}
                    onClick={handleBringBeko}
                  >
                    🇹🇷 Bekoyu Türkiye'ye getir
                  </button>
                )}
              </div>
            </div>
          )}

          {/* Admin Panel - Gizli */}
          {showAdminPanel && <AdminPanel />}

          {/* Aktif Bahisler - Her zaman göster */}
          <ActiveBets round={gameState.round} />

          {/* Bildirimler */}
          <div className="notifications">
            <h4>📢 Bildirimler</h4>
            {notifications.map(notif => (
              <div key={notif.id} className={`notification ${notif.type}`}>
                <span className="time">{notif.timestamp}</span>
                <span className="message">{notif.message}</span>
              </div>
            ))}
          </div>
        </aside>
      </main>
    </div>
  );
}

// 🔧 YARDIMCI FONKSİYON
function getPhaseText(phase) {
  switch(phase) {
    case 'waiting': return 'BEKO KALKIŞA HAZIR!';
    case 'flying': return 'BEKO HAVADA!';
    case 'crashed': return 'BEKO DÜŞTÜ!';
    default: return 'HAZIRLIK';
  }
}

export default App;