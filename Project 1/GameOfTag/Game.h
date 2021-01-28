#pragma once
#include "Client.h"
#include "Server.h"
#include <iostream>
#include "Player.h"
#include <iomanip>
#include <sstream>

class Game
{
public:

	Game();
	void run();
	void updatePlayer(PlayerData t_data);
	void updateAllPositions(std::array<sf::Vector2f, 3> t_positions);
	void setStartofGameData(StartGameData t_data);
	void setEndGameData(EndGameData t_data);
	void changeState(GameState t_changeState);

	void loadFont();

private:

	void update(sf::Time dt);
	void processEvents();
	void render();
	void startServer();
	void connect();
	void centreText();
	void ListenForConnections();
	bool checkForCollisions();
	void reset();
	float getDistance(sf::Vector2f t_pos1, sf::Vector2f t_pos2);

	sf::RenderWindow m_window;

	Player m_player[3];
	GameState m_state;

	std::thread* m_serverThread;
	sf::Time m_timeSurvived;
	sf::Time m_countDownTime;

	bool m_host{ false };
	bool m_centreText{ false };

	int m_playerID{ -1 };
	int m_targetID{ -1 };

	Client* m_client{ nullptr };
	Server* m_server{ nullptr };

	sf::RectangleShape m_hostButton;
	sf::RectangleShape m_joinButton;

	/// <summary>
	/// Text for the host button
	/// </summary>
	sf::Text m_hostText;

	/// <summary>
	/// Text for the join button
	/// </summary>
	sf::Text m_joinText;

	/// <summary>
	/// Text for the title
	/// </summary>
	sf::Text m_titleText;

	/// <summary>
	/// Game is about to begin text
	/// </summary>
	sf::Text m_startText;

	/// <summary>
	/// Text for when not enough players to start
	/// </summary>
	sf::Text m_waitingText;

	/// <summary>
	/// Text for when the game as ended
	/// </summary>
	sf::Text m_endingText;

	sf::Text m_gameStartText;
	sf::Text m_gameOverText;
	sf::Text m_timeSurvivedText;

	sf::Font m_font;
};