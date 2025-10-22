// 🔗 API Service - Backend ile iletişim kurar
class GameAPI {
  // 🇹🇷 POST: Bekoyu Türkiye'ye getir
  async bringBeko() {
    try {
      const response = await fetch(`${this.baseURL}/api/game/bring-beko`, {
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
    this.baseURL = 'http://localhost:5050';
    this.playerId = this.generatePlayerId();
  }

  // 🎲 Unique player ID oluştur
  generatePlayerId() {
    return 'player_' + Math.random().toString(36).substr(2, 9);
  }

  // 📡 GET: Oyun durumunu al
  async getGameStatus() {
    try {
      const response = await fetch(`${this.baseURL}/api/game/status`);
      return await response.json();
    } catch (error) {
      console.error('Game status fetch error:', error);
      return null;
    }
  }

  // 👤 POST: Oyuna katıl
  async joinGame(playerName) {
    try {
      const response = await fetch(`${this.baseURL}/api/game/join`, {
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
      const response = await fetch(`${this.baseURL}/api/game/bet`, {
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
      const response = await fetch(`${this.baseURL}/api/game/players`, {
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
      const response = await fetch(`${this.baseURL}/api/game/cashout`, {
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
}

// 🌟 Singleton pattern - Tek API instance
export const gameAPI = new GameAPI();