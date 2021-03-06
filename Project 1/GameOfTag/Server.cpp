#include "Server.h"
#include <iostream>
#include "PacketStructure.h"
#pragma comment(lib,"ws2_32.lib")
#include "Game.h"

Server::Server(int t_port, bool t_broadcastPublically)
{
	//Winsock Startup
	WSAData wsaData;
	WORD dllVersion = MAKEWORD(2, 1);

	if (WSAStartup(dllVersion, &wsaData) != 0)
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	if (t_broadcastPublically)
	{
		m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		m_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //Broadcast locally
	}

	m_addr.sin_port = htons(t_port);					//Port
	m_addr.sin_family = AF_INET;						//IPv4 Socket

	m_listener = socket(AF_INET, SOCK_STREAM, NULL);	//Create socket to listen for new connections
	if (bind(m_listener, (SOCKADDR*)&m_addr, sizeof(m_addr)) == SOCKET_ERROR)
	{
		std::string ErrorMsg = "Failed to bind the address to our listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	if (listen(m_listener, SOMAXCONN) == SOCKET_ERROR)
	{
		std::string ErrorMsg = "Failed to listen on listening socket. Winsock Error:" + std::to_string(WSAGetLastError());
		MessageBoxA(NULL, ErrorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	m_idCounter = 0;

	std::thread PST(packetSenderThread, std::ref(*this));
	PST.detach();
	m_threads.push_back(&PST);
}

Server::~Server()
{
	m_terminateThreads = true;

	for (std::thread* t : m_threads) //wait for all created threads to end
	{
		t->join();
	}
}

bool Server::listenForNewConnection()
{
	if (m_activeConnectionsCount == 2)
	{
		return true;
	}

	int addrlen = sizeof(m_addr);
	SOCKET newConnectionSocket = accept(m_listener, (SOCKADDR*)&m_addr, &addrlen); //Accept a new connection

	if (newConnectionSocket == 0) //if accepting the clients connection failed
	{
		std::cout << "Failed to accept the client's connection." << std::endl;
		return false;
	}
	else //if client connection properly accepted
	{
		std::lock_guard<std::shared_mutex> lock(m_mutexConnectionManager); //lock connection manager mutex
		std::shared_ptr<Connection> newConnection(std::make_shared<Connection>(newConnectionSocket));
		m_connections.push_back(newConnection); //push 

		newConnection->m_id = m_idCounter;
		
		m_idCounter += 1;
		m_activeConnectionsCount += 1;

		std::cout << "Client Connected! ID:" << newConnection->m_id << std::endl;
		std::thread clientHandlerThread(clientHandlerThread, std::ref(*this), newConnection);
		clientHandlerThread.detach();
		m_threads.push_back(&clientHandlerThread);
		return true;
	}
}

bool Server::processPacket(std::shared_ptr<Connection> t_connection, PacketType t_packetType)
{
	switch (t_packetType)
	{
	case PacketType::Chat_message:
	{
		std::string message;
		if (!getString(t_connection, message))
		{
			return false;
		}

		PS::ChatMessage chatMessage(message);
		std::shared_ptr<Packet> msgPacket = std::make_shared<Packet>(chatMessage.toPacket()); //use shared_ptr instead of sending with SendString so we don't have to reallocate packet for each connection
		{
			std::shared_lock<std::shared_mutex> lock(m_mutexConnectionManager);

			for (auto connection : m_connections) //For each connection...
			{
				if (connection == t_connection)
				{
					continue;
				}

				connection->m_packetManager.append(msgPacket);
			}
		}

		std::cout << "Processed chat message packet from user ID: " << t_connection->m_id << std::endl;
		break;
	}
	case PacketType::Player_set:
	{
		StartGameData data;
		if (!getGameStart(t_connection, data))
		{
			return false;
		}
		break;
	}
	case PacketType::Player_update:
	{
		PlayerData data;
		if (!getPlayerUpdate(t_connection, data))
		{
			return false;
		}
		m_game->updatePlayer(data);
		break;
	}
	default:
	{
		std::cout << "Unrecognized packet: " << (std::int32_t)t_packetType << std::endl;
		return false;
	}
	}
}

void Server::clientHandlerThread(Server& t_server, std::shared_ptr<Connection> t_connection)
{
	PacketType packetType;
	while (true)
	{
		if (t_server.m_terminateThreads == true)
		{
			break;
		}

		if (!t_server.getPacketType(t_connection, packetType))
		{
			break;
		}
		if (!t_server.processPacket(t_connection, packetType))
		{
			break;
		}
	}

	std::cout << "Lost connection to client ID: " << t_connection->m_id << std::endl;
	t_server.disconnectClient(t_connection);
	return;
}

void Server::packetSenderThread(Server& t_server)
{
	while (true)
	{
		if (t_server.m_terminateThreads == true)
		{
			break;
		}

		std::shared_lock<std::shared_mutex> lock(t_server.m_mutexConnectionManager);

		for (auto connection : t_server.m_connections)
		{
			if (connection->m_packetManager.hasPendingPackets())
			{
				std::shared_ptr<Packet> p = connection->m_packetManager.retrieve();
				if (!t_server.sendAll(connection, (const char*)(&p->m_buffer[0]), p->m_buffer.size())) //send packet to connection
				{
					std::cout << "Failed to send packet to ID: " << connection->m_id << std::endl; //Print out if failed to send packet
				}
			}
		}
		Sleep(5);
	}

	std::cout << "Ending Packet Sender Thread..." << std::endl;
}

void Server::disconnectClient(std::shared_ptr<Connection> t_connection)
{
	std::lock_guard<std::shared_mutex> lock(m_mutexConnectionManager);
	t_connection->m_packetManager.clear();
	closesocket(t_connection->m_socket);

	m_connections.erase(std::remove(m_connections.begin(), m_connections.end(), t_connection));

	std::cout << "Cleaned up client: " << t_connection->m_id << "." << std::endl;
	std::cout << "Total connections: " << m_connections.size() << std::endl;

	m_activeConnectionsCount -= 1;
}

bool Server::sendAll(std::shared_ptr<Connection> t_connection, const char* t_data, int t_totalBytes)
{
	int bytesSent = 0;

	while (bytesSent < t_totalBytes)
	{
		int returnCheck = send(t_connection->m_socket, t_data + bytesSent, t_totalBytes - bytesSent, NULL);

		if (returnCheck == SOCKET_ERROR)
		{
			return false;
		}

		bytesSent += returnCheck;
	}

	return true;
}

bool Server::recieveAll(std::shared_ptr<Connection> t_connection, char* t_data, int t_totalBytes)
{
	int bytesReceived = 0;

	while (bytesReceived < t_totalBytes)
	{
		int returnCheck = recv(t_connection->m_socket, t_data + bytesReceived, t_totalBytes - bytesReceived, NULL);

		if (returnCheck == SOCKET_ERROR)
		{
			return false;
		}

		bytesReceived += returnCheck;
	}

	return true;
}

bool Server::getint32t(std::shared_ptr<Connection> t_connection, std::int32_t& t_int32t)
{
	if (!recieveAll(t_connection, (char*)&t_int32t, sizeof(int32_t)))
	{
		return false;
	}

	t_int32t = ntohl(t_int32t);
	return true;
}

bool Server::getPacketType(std::shared_ptr<Connection> t_connection, PacketType& t_packetType)
{
	std::int32_t packetTypeInt;

	if (!getint32t(t_connection, packetTypeInt))
	{
		return false;
	}

	t_packetType = (PacketType)packetTypeInt;
	return true;
}

void Server::sendString(std::shared_ptr<Connection> t_connection, const std::string& t_string)
{
	PS::ChatMessage message(t_string);
	t_connection->m_packetManager.append(message.toPacket());
}

bool Server::getString(std::shared_ptr<Connection> t_connection, std::string& t_string)
{
	std::int32_t bufferlength;

	if (!getint32t(t_connection, bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	t_string.resize(bufferlength);
	return recieveAll(t_connection, &t_string[0], bufferlength);
}

void Server::sendPlayerUpdate(PlayerData& t_updateData)
{
	PS::PlayerUpdate gameUpdate(t_updateData);
	std::shared_ptr<Packet> updatePacket = std::make_shared<Packet>(gameUpdate.toPacket());
	{
		std::shared_lock<std::shared_mutex> lock(m_mutexConnectionManager);

		for (auto connection : m_connections) //For each connection...
		{
			connection->m_packetManager.append(updatePacket);
		}
	}
}

bool Server::getPlayerUpdate(std::shared_ptr<Connection> t_connection, PlayerData& t_updateData)
{
	std::int32_t bufferlength;

	if (!getint32t(t_connection, bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return recieveAll(t_connection, (char*)&t_updateData, bufferlength);;
}

void Server::sendGameStart(StartGameData& t_startData, int t_index)
{
	PS::GameStart gameStart(t_startData);
	std::shared_ptr<Packet> startPacket = std::make_shared<Packet>(gameStart.toPacket());
	std::shared_lock<std::shared_mutex> lock(m_mutexConnectionManager);

	m_connections[t_index]->m_packetManager.append(startPacket);
}

bool Server::getGameStart(std::shared_ptr<Connection> t_connection, StartGameData& t_startData)
{
	std::int32_t bufferlength;

	if (!getint32t(t_connection, bufferlength))
	{
		return false;
	}

	if (bufferlength == 0)
	{
		return true;
	}

	return recieveAll(t_connection, (char*)&t_startData, bufferlength);;
}

void Server::sendGameEnd(EndGameData& t_endData)
{
	PS::GameEnd gameEnd(t_endData);

	std::shared_ptr<Packet> startPacket = std::make_shared<Packet>(gameEnd.toPacket());
	std::shared_lock<std::shared_mutex> lock(m_mutexConnectionManager);

	for (auto connection : m_connections)
	{
		connection->m_packetManager.append(startPacket);
	}
}

void Server::sendGameUpdate(std::array<sf::Vector2f, 3>& t_positions)
{
	PS::GameUpdate gameUpdate(t_positions);
	std::shared_ptr<Packet> updatePacket = std::make_shared<Packet>(gameUpdate.toPacket());
	std::shared_lock<std::shared_mutex> lock(m_mutexConnectionManager);

	for (auto connection : m_connections)
	{
		connection->m_packetManager.append(updatePacket);
	}
}

void Server::sendChangeState(const GameState& t_gameState)
{
	PS::ChangeState changeState(t_gameState);

	std::shared_ptr<Packet> startPacket = std::make_shared<Packet>(changeState.toPacket());
	std::shared_lock<std::shared_mutex> lock(m_mutexConnectionManager);

	for (auto connection : m_connections)
	{
		connection->m_packetManager.append(startPacket);
	}
}
