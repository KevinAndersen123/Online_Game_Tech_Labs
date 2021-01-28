#include "Game.h"

Game::Game() :
	m_window{ sf::VideoMode{ 600, 600, 32 }, "Tag: Project 1" },
	m_state(GameState::Select)
{
	m_player[0].setColour(sf::Color::Yellow);
	m_player[0].setName("Yellow");
	m_player[0].setStartPosition(sf::Vector2f(200.0f, 100.0f));

	m_player[1].setColour(sf::Color::Green);
	m_player[1].setName("Green");
	m_player[1].setStartPosition(sf::Vector2f(50.0f, 300.0f));

	m_player[2].setColour(sf::Color::Blue);
	m_player[2].setName("Blue");
	m_player[2].setStartPosition(sf::Vector2f(400.0f, 550.0f));

	reset();
	loadFont();

	m_joinButton.setSize(sf::Vector2f(150.0f, 50.0f));
	m_joinButton.setOrigin(sf::Vector2f(75.0f, 25.0f));
	m_joinButton.setOutlineThickness(5.0f);
	m_joinButton.setOutlineColor(sf::Color::Blue);
	m_joinButton.setFillColor(sf::Color::Cyan);
	m_joinButton.setPosition(sf::Vector2f(300.0f, 400.0f));
	
	m_hostButton.setSize(sf::Vector2f(150.0f, 50.0f));
	m_hostButton.setOrigin(sf::Vector2f(75.0f, 25.0f));
	m_hostButton.setOutlineThickness(5.0f);
	m_hostButton.setOutlineColor(sf::Color::Blue);
	m_hostButton.setFillColor(sf::Color::Cyan);
	m_hostButton.setPosition(sf::Vector2f(300.0f, 300.0f));

	m_hostText.setString("Host");
	m_hostText.setFont(m_font);
	m_hostText.setCharacterSize(30);
	m_hostText.setOrigin(sf::Vector2f(m_hostText.getLocalBounds().width / 2, m_hostText.getLocalBounds().height / 1.3f));
	m_hostText.setFillColor(sf::Color::Black);
	m_hostText.setPosition(m_hostButton.getPosition());
	
	m_joinText.setString("Join");
	m_joinText.setFont(m_font);
	m_joinText.setCharacterSize(30);
	m_joinText.setOrigin(sf::Vector2f(m_joinText.getLocalBounds().width / 2, m_joinText.getLocalBounds().height / 1.3f));
	m_joinText.setFillColor(sf::Color::Black);
	m_joinText.setPosition(m_joinButton.getPosition());

	m_titleText.setString("Online Tag");
	m_titleText.setFont(m_font);
	m_titleText.setCharacterSize(50);
	m_titleText.setOrigin(sf::Vector2f(m_titleText.getLocalBounds().width / 2, m_titleText.getLocalBounds().height / 1.3f));
	m_titleText.setFillColor(sf::Color::White);
	m_titleText.setPosition(sf::Vector2f(300.0f, 100.0f));

	m_gameStartText.setString("Game Is About To Begin");
	m_gameStartText.setFont(m_font);
	m_gameStartText.setCharacterSize(30);
	m_gameStartText.setOrigin(sf::Vector2f(m_gameStartText.getLocalBounds().width / 2, m_gameStartText.getLocalBounds().height / 2));
	m_gameStartText.setPosition(sf::Vector2f(300.0f, 25.0f));

	m_gameOverText.setString("GameOver");
	m_gameOverText.setFont(m_font);
	m_gameOverText.setCharacterSize(30);
	m_gameOverText.setOrigin(sf::Vector2f(m_gameOverText.getLocalBounds().width / 2, m_gameOverText.getLocalBounds().height / 2));
	m_gameOverText.setPosition(sf::Vector2f(300.0f, 200.0f));

	m_startText.setFont(m_font);
	m_startText.setCharacterSize(25);
	m_endingText.setFont(m_font);
	m_endingText.setCharacterSize(25);m_waitingText.setString("Waiting For More\n Players To Join...");
	m_waitingText.setFont(m_font);
	m_waitingText.setCharacterSize(30);
	m_waitingText.setOrigin(sf::Vector2f(m_waitingText.getLocalBounds().width / 2, m_waitingText.getLocalBounds().height / 2));
	m_waitingText.setPosition(sf::Vector2f(300.0f, 280.0f));
	
	m_timeSurvivedText.setFont(m_font);
	m_timeSurvivedText.setCharacterSize(25);
}

void Game::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	sf::Time timePerFrame = sf::seconds(1.f / 60.f);	//60 fps

	while (m_window.isOpen())
	{
		processEvents();								//As many as possible
		timeSinceLastUpdate += clock.restart();
		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents();							//At least 60 fps
			update(timePerFrame);						//60 fps
		}
		render();										//As many as possible
	}
}

void Game::updatePlayer(PlayerData t_data)
{
	m_player[t_data.m_playerID].updatePosition(t_data.m_pos);
}

void Game::updateAllPositions(std::array<sf::Vector2f, 3> t_positions)
{
	for (int i = 0; i < 3; i++)
	{
		m_player[i].setPosition(t_positions[i]);
	}
}

void Game::setStartofGameData(StartGameData t_data)
{
	changeState(GameState::Start);

	m_targetID = t_data.m_targetID;
	m_playerID = t_data.m_playerID;

	std::string string;

	string = "You are " + m_player[m_playerID].getName() + ".";

	if (m_targetID == m_playerID)
	{
		string += " Don't get caught.";
	}
	else
	{
		string += " Catch " + m_player[m_targetID].getName() + ".";
	}

	m_startText.setString(string);

	m_centreText = true;
}

void Game::setEndGameData(EndGameData t_data)
{
	changeState(GameState::Gameover);

	std::stringstream timeStream;

	timeStream << std::fixed << std::setprecision(2) << t_data.m_timeSurvived.asSeconds();

	std::string timeString = timeStream.str();

	if (t_data.m_playerID == m_playerID)
	{
		m_endingText.setString("Well done. You caught " + m_player[t_data.m_targetID].getName());
		m_timeSurvivedText.setString(m_player[t_data.m_targetID].getName() + " " + timeString + "s");
	}
	else if(t_data.m_targetID == m_playerID)
	{
		m_endingText.setString(" Oh, Oh " + m_player[t_data.m_playerID].getName() + " has caught you");
		m_timeSurvivedText.setString("You Lasted " + timeString + "s");
	}
	else
	{ 
		m_endingText.setString(m_player[t_data.m_playerID].getName() + " has caught " + m_player[t_data.m_targetID].getName());
		m_timeSurvivedText.setString(m_player[t_data.m_targetID].getName() + " " + timeString + "s");
	}
	m_centreText = true;
}

void Game::changeState(GameState t_changeState)
{
	m_state = t_changeState;

	if (m_state == GameState::Wait)
	{
		reset();
	}
}

void Game::loadFont()
{
	if (!m_font.loadFromFile("Assets/Fonts/Iceberg.ttf"))
	{
		std::cout << "Failed loading font file" << std::endl;
	}
	else
	{
		std::cout << "Successfully loaded font file" << std::endl;
	}
}


void Game::processEvents()
{
	sf::Event event;

	while (m_window.pollEvent(event))
	{
		sf::Vector2f m_pos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));

		if (m_state == GameState::Select)
		{
			if (event.type == sf::Event::MouseButtonPressed)
			{
				//check for button input
				if (event.key.code == sf::Mouse::Left)
				{
					if (m_hostButton.getGlobalBounds().contains(m_pos))
					{
						startServer();
						m_state = GameState::Wait;
					}
					else if (m_joinButton.getGlobalBounds().contains(m_pos))
					{
						connect();
						m_state = GameState::Wait;
					}
				}
			}
		}

		if (event.type == sf::Event::KeyPressed)
		{
			switch (event.key.code)
			{
			case sf::Keyboard::Escape:
				m_window.close();
				break;
			default:
				break;
			}
		}

		if (event.type == sf::Event::Closed)
		{
			m_window.close();
		}
	}
}

void Game::update(sf::Time dt)
{
	if (m_state == GameState::Gameplay)
	{
		sf::Vector2f moveVector = m_player[m_playerID].move();

		if (m_host)
		{
			if (m_server->m_activeConnectionsCount < 2)
			{
				changeState(GameState::Wait);
				m_server->sendChangeState(GameState::Wait);
				return;
			}

			m_player[m_playerID].updatePosition(moveVector);
			m_timeSurvived += dt;

			if (!checkForCollisions())
			{
				std::array<sf::Vector2f, 3> positions;

				for (int i = 0; i < 3; i++)
				{
					positions[i] = m_player[i].getPosition();
				}

				m_server->sendGameUpdate(positions);
			}
		}
		else
		{
			PlayerData data;
			data.m_playerID = m_playerID;
			data.m_pos = moveVector;
			m_client->sendPlayerUpdate(data);
		}
	}

	else if (m_state == GameState::Wait)
	{
		if (m_host)
		{
			if (m_server->m_activeConnectionsCount == 2)
			{
				m_countDownTime = sf::seconds(3.0f);
				m_state = GameState::Start;

				int playerIndex = rand() % 3;
				int targetIndex = rand() % 3;

				m_playerID = playerIndex;
				m_targetID = targetIndex;

				std::string string;

				string = "You are " + m_player[m_playerID].getName() + ".";

				if (m_targetID == m_playerID)
				{
					string += " Don't get caught.";
				}
				else
				{
					string += " Catch " + m_player[m_targetID].getName() + ".";
				}

				m_startText.setString(string);

				m_centreText = true;

				StartGameData startData;
				startData.m_targetID = targetIndex;

				for (int i = 0; i < 2; i++)
				{
					playerIndex = (playerIndex + 1) % 3;
					startData.m_playerID = playerIndex;
					m_server->sendGameStart(startData, i);
				}
			}
		}
	}

	else if (m_state == GameState::Start)
	{
		if (m_host)
		{
			m_countDownTime -= dt;

			if (m_countDownTime <= sf::seconds(0.0f))
			{
				m_state = GameState::Gameplay;
				m_server->sendChangeState(GameState::Gameplay);
			}
		}
	}
}

void Game::render()
{
	m_window.clear(sf::Color(255,62,3));

	switch (m_state)
	{
	case GameState::Select:
		m_window.draw(m_hostButton);
		m_window.draw(m_joinButton);
		m_window.draw(m_titleText);
		m_window.draw(m_hostText);
		m_window.draw(m_joinText);
		break;
	case GameState::Wait:
		m_window.draw(m_waitingText);
		break;
	case GameState::Start:
		if (m_centreText)
		{
			centreText();
		}
		m_window.draw(m_gameStartText);
		m_window.draw(m_startText);
		break;
	case GameState::Gameplay:
		for (int i = 0; i < 3; i++)
		{
			m_player[i].render(m_window);
		}
		break;
	case GameState::Gameover:
		if (m_centreText)
		{
			centreText();
		}
		m_window.draw(m_gameOverText);
		m_window.draw(m_endingText);
		m_window.draw(m_timeSurvivedText);
		break;
	default:
		break;
	}

	m_window.display();
}

float Game::getDistance(sf::Vector2f t_pos1, sf::Vector2f t_pos2)
{
	return sqrt(powf(t_pos1.x - t_pos2.x, 2) + powf(t_pos1.y - t_pos2.y, 2));
}

void Game::startServer()
{
	m_host = true;
	m_server = new Server(80,true);
	m_server->m_game = this;

	std::thread serverThread(&Game::ListenForConnections, this);
	serverThread.detach();

	m_serverThread = &serverThread;
}

void Game::connect()
{ 
	m_client = new Client("127.0.0.1", 80);
	m_client->m_game = this;

	if (!m_client->connectSocket())
	{
		m_client->disconnect();
		m_window.close();
	}
}

void Game::ListenForConnections()
{
	while(m_server->m_activeConnectionsCount < 10)
	{
		m_server->listenForNewConnection();
	}
}

void Game::centreText()
{
	m_endingText.setOrigin(sf::Vector2f(m_endingText.getLocalBounds().width / 2, m_endingText.getLocalBounds().height / 2));
	m_endingText.setPosition(sf::Vector2f(300.0f, 325.0f));

	m_gameOverText.setOrigin(sf::Vector2f(m_gameOverText.getLocalBounds().width / 2, m_gameOverText.getLocalBounds().height / 2));
	m_gameOverText.setPosition(sf::Vector2f(300.0f, 275.0f));

	m_gameStartText.setOrigin(sf::Vector2f(m_gameStartText.getLocalBounds().width / 2, m_gameStartText.getLocalBounds().height / 2));
	m_gameStartText.setPosition(sf::Vector2f(300.0f, 275.0f));

	m_startText.setOrigin(sf::Vector2f(m_startText.getLocalBounds().width / 2, m_startText.getLocalBounds().height / 2));
	m_startText.setPosition(sf::Vector2f(300.0f, 75.0f));

	m_timeSurvivedText.setOrigin(sf::Vector2f(m_timeSurvivedText.getLocalBounds().width / 2, m_timeSurvivedText.getLocalBounds().height / 2));
	m_timeSurvivedText.setPosition(sf::Vector2f(300.0f, 375.0f));

	m_centreText = false;
}

void Game::reset()
{
	for (int i = 0; i < 3; i++)
	{
		m_player[i].setPosition(m_player[i].getStartPosition());
	}

	m_timeSurvived = sf::seconds(0.0f);
}


bool Game::checkForCollisions()
{
	for (int i = 0; i < 3; i++)
	{
		if (m_targetID != i)
		{
			if (getDistance(m_player[m_targetID].getPosition(), m_player[i].getPosition()) <= Player::s_radius * 2)
			{
				EndGameData endData;
				endData.m_playerID = i;
				endData.m_targetID = m_targetID;
				endData.m_timeSurvived = m_timeSurvived;

				m_server->sendGameEnd(endData);
				setEndGameData(endData);

				return true;
			}
		}
	}
	return false;
}
