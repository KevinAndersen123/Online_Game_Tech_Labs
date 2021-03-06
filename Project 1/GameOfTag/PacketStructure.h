#pragma once
#include "Packet.h"
#include <string> 
#include <memory> 
#include <array>

namespace PS
{
	class ChatMessage
	{
	public:
		ChatMessage(const std::string& t_string);
		std::shared_ptr<Packet> toPacket();

	private:

		std::string m_message;
	};

	class ChangeState
	{
	public:
		ChangeState(const GameState& t_state);
		std::shared_ptr<Packet> toPacket();
	private:

		GameState m_state;
	};

	class GameStart
	{
	public:
		GameStart(const StartGameData& t_startData);
		std::shared_ptr<Packet> toPacket();

	private:

		StartGameData m_data;
	};

	class PlayerUpdate
	{
	public:
		PlayerUpdate(const PlayerData& t_updateData);
		std::shared_ptr<Packet> toPacket();
	private:

		PlayerData m_data;
	};

	class GameUpdate
	{
	public:
		GameUpdate(const std::array<sf::Vector2f, 3>& t_positions);
		std::shared_ptr<Packet> toPacket();
	private:

		std::array<sf::Vector2f, 3> m_positions;
	};

	class GameEnd
	{
	public:
		GameEnd(const EndGameData& t_endData);
		std::shared_ptr<Packet> toPacket();

	private:

		EndGameData m_data;
	};
}