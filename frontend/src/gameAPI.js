// 🔗 API Service - Backend ile iletişim kurar
class GameAPI {
  // 🇹🇷 POST: Bekoyu Türkiye'ye getir
  async bringBeko() {
    try {
      const response = await fetch(`${this.baseURL}/game/bring-beko`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          player_id: this.playerId
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Bring Beko error:', error);
      return { success: false, error: error.message };
    }
  }
  constructor() {
    this.baseURL = '/api';
    this.playerId = this.generatePlayerId();
  }

  // 🎲 Unique player ID oluştur
  generatePlayerId() {
    return 'player_' + Math.random().toString(36).substr(2, 9);
  }

  // 📡 GET: Oyun durumunu al
  async getGameStatus() {
    try {
      const response = await fetch(`${this.baseURL}/game/status`);
      return await response.json();
    } catch (error) {
      console.error('Game status fetch error:', error);
      return null;
    }
  }

  // 👤 POST: Oyuna katıl
  async joinGame(playerName) {
    try {
      const response = await fetch(`${this.baseURL}/game/join`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          player_id: this.playerId,
          name: playerName
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Join game error:', error);
      return { success: false, error: error.message };
    }
  }

  // 💰 POST: Bahis yap
  async placeBet(amount) {
    try {
      const response = await fetch(`${this.baseURL}/game/bet`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          player_id: this.playerId,
          amount: amount
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Place bet error:', error);
      return { success: false, error: error.message };
    }
  }

  // 🚪 GET: Oyuncu bilgilerini al
  async getPlayersInfo() {
    try {
      const response = await fetch(`${this.baseURL}/game/players`, {
        method: 'PUT',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          player_id: this.playerId
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Get players info error:', error);
      return { success: false, error: error.message };
    }
  }

  // 💸 POST: Cashout yap
  async cashout() {
    try {
      const response = await fetch(`${this.baseURL}/game/cashout`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          player_id: this.playerId
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Cashout error:', error);
      return { success: false, error: error.message };
    }
  }

  // 🆔 Player ID'yi al
  getPlayerId() {
    return this.playerId;
  }

  // 🎯 GET: Aktif bahisleri al
  async getActiveBets() {
    try {
      const response = await fetch(`${this.baseURL}/game/active-bets`);
      return await response.json();
    } catch (error) {
      console.error('Get active bets error:', error);
      return [];
    }
  }

  // 💳 POST: Bakiye yükle (Admin)
  async loadBalance(playerName, amount) {
    try {
      const response = await fetch(`${this.baseURL}/game/load-balance`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          player_name: playerName,
          amount: amount
        })
      });
      return await response.json();
    } catch (error) {
      console.error('Load balance error:', error);
      return { success: false, error: error.message };
    }
  }

  // 📈 GET: Eski crash pointlerini al
  async getOldCrashPoints() {
    try {
      const response = await fetch(`${this.baseURL}/game/old-crash-points`);
      return await response.json();
    } catch (error) {
      console.error('Get old crash points error:', error);
      return [];
    }
  }
}

// 🌟 Singleton pattern - Tek API instance
export const gameAPI = new GameAPI();