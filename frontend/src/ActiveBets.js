import React, { useState, useEffect } from 'react';
import { gameAPI } from './gameAPI';

function ActiveBets({ round }) {
  const [activeBets, setActiveBets] = useState([]);

  useEffect(() => {
    const fetchActiveBets = async () => {
      const bets = await gameAPI.getActiveBets();
      setActiveBets(bets);
    };

    fetchActiveBets();

    // Her 2 saniyede bir güncelle
    const interval = setInterval(fetchActiveBets, 2000);

    return () => clearInterval(interval);
  }, [round]); // round değiştiğinde yeniden fetch et

  return (
    <div className="active-bets">
      <h4>🎯 Aktif Bahisler</h4>
      {activeBets.length === 0 ? (
        <p>Henüz bahis yok</p>
      ) : (
        <ul>
          {activeBets.map((bet, index) => (
            <li key={index}>
              <span className="player-name">{bet.player_name}</span>
              <span className="bet-amount">{bet.amount} TL</span>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}

export default ActiveBets;