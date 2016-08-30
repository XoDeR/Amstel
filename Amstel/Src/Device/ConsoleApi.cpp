// Copyright (c) 2016 Volodymyr Syvochka
#include "Device/ConsoleServer.h"
#include "Resource/DataCompiler.h"
#include "Device/Device.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Json/JsonObject.h"
#include "Script/ScriptEnvironment.h"
#include "Core/Json/JsonR.h"
#include "Core/Strings/StringStream.h"
#include "Core/Memory/TempAllocator.h"

namespace Rio
{

static void consoleCommandExecuteScript(ConsoleServer& /*consoleServer*/, TcpSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	DynamicString script(ta);
	JsonRFn::parse(json, jsonObject);
	JsonRFn::parseString(jsonObject["script"], script);
	getDevice()->getScriptEnvironment()->executeScriptString(script.getCStr());
}

static void consoleCommandReloadResource(ConsoleServer& /*consoleServer*/, TcpSocket /*client*/, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	DynamicString type(ta);
	DynamicString name(ta);
	JsonRFn::parse(json, jsonObject);
	JsonRFn::parseString(jsonObject["resourceType"], type);
	JsonRFn::parseString(jsonObject["resourceName"], name);

	RIO_LOGI("Reloading resource '%s.%s'", name.getCStr(), type.getCStr());
	getDevice()->reload(ResourceId(type.getCStr()), ResourceId(name.getCStr()));
	RIO_LOGI("Reloaded resource '%s.%s'", name.getCStr(), type.getCStr());
}

static void consoleCommandPause(ConsoleServer& /*consoleServer*/, TcpSocket /*client*/, const char* /*json*/)
{
	getDevice()->pause();
}

static void consoleCommandUnpause(ConsoleServer& /*consoleServer*/, TcpSocket /*client*/, const char* /*json*/)
{
	getDevice()->unpause();
}

static void consoleCommandCompileResource(ConsoleServer& consoleServer, TcpSocket client, const char* json)
{
	TempAllocator4096 ta;
	JsonObject jsonObject(ta);
	
	DynamicString id(ta);
	DynamicString dataDirectory(ta);
	DynamicString platform(ta);

	JsonRFn::parse(json, jsonObject);
	JsonRFn::parseString(jsonObject["id"], id);
	JsonRFn::parseString(jsonObject["dataDirectory"], dataDirectory);
	JsonRFn::parseString(jsonObject["platform"], platform);

	{
		TempAllocator512 ta;
		StringStream stringStream(ta);
		stringStream << "{\"type\":\"compile\",\"id\":\"" << id.getCStr() << "\",\"start\":true}";
		consoleServer.send(client, StringStreamFn::getCStr(stringStream));
	}

	RIO_LOGI("Compiling '%s'", id.getCStr());
	bool compileSuccess = getDevice()->getDataCompiler()->compile(dataDirectory.getCStr(), platform.getCStr());
	if (compileSuccess)
	{
		RIO_LOGI("Compiled '%s'", id.getCStr());
	}
	else
	{
		RIO_LOGE("Error while compiling '%s'", id.getCStr());
	}
	{
		TempAllocator512 ta;
		StringStream stringStream(ta);
		stringStream << "{\"type\":\"compile\",\"id\":\"" << id.getCStr() << "\",\"success\":" << (compileSuccess ? "true" : "false") << "}";
		consoleServer.send(client, StringStreamFn::getCStr(stringStream));
	}
}

void loadConsoleApi(ConsoleServer& consoleServer)
{
	consoleServer.registerCommand("script", consoleCommandExecuteScript);
	consoleServer.registerCommand("reload", consoleCommandReloadResource);
	consoleServer.registerCommand("pause", consoleCommandPause);
	consoleServer.registerCommand("unpause", consoleCommandUnpause);
	consoleServer.registerCommand("compile", consoleCommandCompileResource);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka