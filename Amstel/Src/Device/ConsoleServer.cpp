// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/ConsoleServer.h"
#include "Core/Json/JsonObject.h"
#include "Core/Json/JsonR.h"
#include "Core/Containers/SortMap.h"
#include "Core/Strings/StringId.h"
#include "Core/Strings/StringStream.h"
#include "Core/Memory/TempAllocator.h"

namespace Rio
{

ConsoleServer::ConsoleServer(Allocator& a)
	: clientList(a)
	, commandFunctionMap(a)
{
}

void ConsoleServer::listen(uint16_t port, bool wait)
{
	server.bind(port);
	server.listen(5);

	if (wait)
	{
		AcceptResult acceptResult;
		TcpSocket client;
		do
		{
			acceptResult = server.accept(client);
		}
		while (acceptResult.error != AcceptResult::NO_ERROR);

		addClient(client);
	}
}

void ConsoleServer::shutdown()
{
	for (uint32_t i = 0; i < VectorFn::getCount(clientList); ++i)
	{
		clientList[i].close();
	}

	server.close();
}

void ConsoleServer::send(TcpSocket client, const char* json)
{
	uint32_t length = getStringLength32(json);
	client.write(&length, 4);
	client.write(json, length);
}

void ConsoleServer::error(TcpSocket client, const char* msg)
{
	TempAllocator4096 ta;
	StringStream stringStream(ta);
	stringStream << "{\"type\":\"error\",\"message\":\"" << msg << "\"}";
	send(client, StringStreamFn::getCStr(stringStream));
}

void ConsoleServer::success(TcpSocket client, const char* msg)
{
	TempAllocator4096 ta;
	StringStream stringStream(ta);
	stringStream << "{\"type\":\"success\",\"message\":\"" << msg << "\"}";
	send(client, StringStreamFn::getCStr(stringStream));
}

void ConsoleServer::send(const char* json)
{
	for (uint32_t i = 0; i < VectorFn::getCount(clientList); ++i)
	{
		send(clientList[i], json);
	}
}

void ConsoleServer::update()
{
	TcpSocket client;
	AcceptResult acceptResult = server.acceptNonblock(client);
	if (acceptResult.error == AcceptResult::NO_ERROR)
	{
		addClient(client);
	}

	TempAllocator256 alloc;
	Array<uint32_t> toRemove(alloc);

	// Update all clients
	for (uint32_t i = 0; i < VectorFn::getCount(clientList); ++i)
	{
		ReadResult readResult = updateClient(clientList[i]);
		if (readResult.error != ReadResult::NO_ERROR)
		{
			ArrayFn::pushBack(toRemove, i);
		}
	}

	// Remove clients
	for (uint32_t i = 0; i < ArrayFn::getCount(toRemove); ++i)
	{
		const uint32_t last = VectorFn::getCount(clientList) - 1;
		const uint32_t clientIndex = toRemove[i];

		clientList[clientIndex].close();
		// swap with last idiom
		clientList[clientIndex] = clientList[last];
		VectorFn::popBack(clientList);
	}
}

void ConsoleServer::addClient(TcpSocket socket)
{
	VectorFn::pushBack(clientList, socket);
}

ReadResult ConsoleServer::updateClient(TcpSocket client)
{
	uint32_t messageLength = 0;
	ReadResult readResult = client.readNonblock(&messageLength, 4);

	// If no data received, return
	if (readResult.error == ReadResult::NO_ERROR && readResult.bytesRead == 0)
	{
		return readResult;
	}
	if (readResult.error == ReadResult::REMOTE_CLOSED)
	{
		return readResult;
	}
	if (readResult.error != ReadResult::NO_ERROR)
	{
		return readResult;
	}

	// Else read the message
	TempAllocator4096 ta;
	Array<char> messageBuffer(ta);
	ArrayFn::resize(messageBuffer, messageLength);
	ReadResult messageResult = client.read(ArrayFn::begin(messageBuffer), messageLength);
	ArrayFn::pushBack(messageBuffer, '\0');

	if (messageResult.error == ReadResult::REMOTE_CLOSED)
	{
		return messageResult;
	}
	if (messageResult.error != ReadResult::NO_ERROR)
	{
		return messageResult;
	}

	process(client, ArrayFn::begin(messageBuffer));
	return messageResult;
}

void ConsoleServer::process(TcpSocket client, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	JsonRFn::parse(json, jsonObject);

	CommandFunction commandFunction = SortMapFn::get(commandFunctionMap, JsonRFn::parseStringId(jsonObject["type"]), (CommandFunction)nullptr);
	if (commandFunction != nullptr)
	{
		commandFunction(*this, client, json);
	}
	else
	{
		error(client, "Unknown command");
	}
}

void ConsoleServer::registerCommand(const char* type, CommandFunction commandFunction)
{
	RIO_ASSERT_NOT_NULL(type);
	RIO_ASSERT_NOT_NULL(commandFunction);

	SortMapFn::set(commandFunctionMap, StringId32(type), commandFunction);
	SortMapFn::sort(commandFunctionMap);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka