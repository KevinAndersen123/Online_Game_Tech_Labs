#include "PacketStructure.h"

PS::ChatMessage::ChatMessage(const std::string& t_string) :
	m_message(t_string)
{
}

std::shared_ptr<Packet> PS::ChatMessage::toPacket()
{
	std::shared_ptr<Packet> packet = std::make_shared<Packet>();
	packet->append(PacketType::Chat_message);
	packet->append(m_message.size());
	packet->append(m_message);
	return packet;
}

PS::PlayerUpdate::PlayerUpdate(const PlayerData& t_updateData) :
	m_data(t_updateData)
{
}

std::shared_ptr<Packet> PS::PlayerUpdate::toPacket()
{
	std::shared_ptr<Packet> packet = std::make_shared<Packet>();
	packet->append(PacketType::Player_update);
	packet->append(sizeof(m_data));
	packet->append(m_data);
	return packet;
}

PS::ChangeState::ChangeState(const GameState& t_state):
	m_state(t_state)
{
}

std::shared_ptr<Packet> PS::ChangeState::toPacket()
{
	std::shared_ptr<Packet> packet = std::make_shared<Packet>();
	packet->append(PacketType::State_change);
	packet->append(sizeof(m_state));
	packet->append(m_state);
	return packet;
}

PS::GameStart::GameStart(const StartGameData& t_startData):
	m_data(t_startData)
{
}

std::shared_ptr<Packet> PS::GameStart::toPacket()
{
	std::shared_ptr<Packet> packet = std::make_shared<Packet>();
	packet->append(PacketType::Player_set);
	packet->append(sizeof(m_data));
	packet->append(m_data);
	return packet;
}

PS::GameUpdate::GameUpdate(const std::array<sf::Vector2f, 3>& t_positions):
	m_positions(t_positions)
{
}

std::shared_ptr<Packet> PS::GameUpdate::toPacket()
{
	std::shared_ptr<Packet> packet = std::make_shared<Packet>();
	packet->append(PacketType::Game_update);
	packet->append(sizeof(m_positions));
	packet->append(m_positions);
	return packet;
}

PS::GameEnd::GameEnd(const EndGameData& t_endData):
	m_data(t_endData)
{

}

std::shared_ptr<Packet> PS::GameEnd::toPacket()
{
	std::shared_ptr<Packet> packet = std::make_shared<Packet>();
	packet->append(PacketType::Game_end);
	packet->append(sizeof(m_data));
	packet->append(m_data);
	return packet;
}
