#pragma once

enum class PacketType
{
	Chat_message,
	Player_set,
	State_change,
	Player_update,
	Game_update,
	Game_end
};

enum class GameState
{
	Select,
	Wait,
	Start,
	Gameplay,
	Gameover
};