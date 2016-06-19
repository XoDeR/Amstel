// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Network/Socket.h"
#include "Core/Strings/StringTypes.h"

namespace Rio
{

// Provides the service to communicate with engine via TCP/IP
class ConsoleServer
{
private:
	using CommandFunction = void (*)(void* data, ConsoleServer& cs, TcpSocket client, const char* json);

	struct CommandData
	{
		CommandFunction commandFunction = nullptr;
		void* data = nullptr;
	};
public:
	ConsoleServer(Allocator& a);
	// If <wait> is true, this function blocks until a client is connected
	void listen(uint16_t port, bool wait);
	void shutdown();
	// Collects requests from clients and processes them all
	void update();
	// Sends JSON-encoded string to all clients
	void send(const char* json);
	// Sends JSON-encoded string to <client>
	void send(TcpSocket client, const char* json);
	// Sends the error message to <client>
	void error(TcpSocket client, const char* msg);
	// Sends the success message to <client>
	void success(TcpSocket client, const char* msg);
	// Registers the command <type>
	void registerCommand(StringId32 type, CommandFunction commandFunction, void* data);
private:
	void addClient(TcpSocket socket);
	ReadResult updateClient(TcpSocket client);
	void process(TcpSocket client, const char* json);

	TcpSocket server;
	Vector<TcpSocket> clientList;
	SortMap<StringId32, CommandData> commandDataList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka