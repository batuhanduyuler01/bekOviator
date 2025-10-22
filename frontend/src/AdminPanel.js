import React, { useState } from 'react';
import { gameAPI } from './gameAPI';

function AdminPanel() {
  const [playerName, setPlayerName] = useState('');
  const [amount, setAmount] = useState('');
  const [message, setMessage] = useState('');

  const handleLoadBalance = async () => {
    if (!playerName || !amount) {
      setMessage('Lütfen tüm alanları doldurun');
      return;
    }

    const result = await gameAPI.loadBalance(playerName, parseFloat(amount));
    if (result.success) {
      setMessage('✅ Bakiye başarıyla yüklendi!');
      setPlayerName('');
      setAmount('');
    } else {
      setMessage('❌ Hata: ' + (result.error || 'Bilinmeyen hata'));
    }
  };

  return (
    <div className="admin-panel">
      <h3>🔧 Admin Panel - Bakiye Yükleme</h3>
      <div className="admin-form">
        <input
          type="text"
          placeholder="Oyuncu Adı"
          value={playerName}
          onChange={(e) => setPlayerName(e.target.value)}
        />
        <input
          type="number"
          placeholder="Miktar (TL)"
          value={amount}
          onChange={(e) => setAmount(e.target.value)}
        />
        <button onClick={handleLoadBalance}>💳 Bakiye Yükle</button>
      </div>
      {message && (
        <p className={`admin-message ${message.startsWith('✅') ? 'success' : 'error'}`}>
          {message}
        </p>
      )}
    </div>
  );
}

export default AdminPanel;