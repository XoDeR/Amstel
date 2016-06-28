// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Memory/ProxyAllocator.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"
#include "Resource/ResourceTypes.h"

namespace Rio
{

class ResourceManager
{
private:
	using LoadFunction  = void* (*)(File& file, Allocator& a);
	using OnlineFunction = void (*)(StringId64 name, ResourceManager& resourceManager);
	using OfflineFunction = void (*)(StringId64 name, ResourceManager& resourceManager);
	using UnloadFunction = void (*)(Allocator& allocator, void* resource);

	struct ResourcePair
	{
		StringId64 type;
		StringId64 name;

		bool operator<(const ResourcePair& a) const
		{
			return type < a.type || (type == a.type && name < a.name);
		}
	};

	struct ResourceEntry
	{
		bool operator==(const ResourceEntry& e)
		{
			return references == e.references && data == e.data;
		}

		static const ResourceEntry NOT_FOUND;

		uint32_t references;
		void* data;
	};

	struct ResourceTypeData
	{
		LoadFunction load;
		OnlineFunction online;
		OfflineFunction offline;
		UnloadFunction unload;
	};

	using ResourceTypeDataMap = SortMap<StringId64, ResourceTypeData>;
	using ResourceMap = SortMap<ResourcePair, ResourceEntry>;
public:
	// Uses <rl> to load resources
	ResourceManager(ResourceLoader& rl);
	~ResourceManager();
	// Loads the resource (<type>, <name>)
	// You can check whether the resource is available with canGet()
	void load(StringId64 type, StringId64 name);
	// Unloads the resource (<type>, <name>)
	void unload(StringId64 type, StringId64 name);
	// Reloads the resource (<type>, <name>)
	// The user has to manually update all the references to the old resource
	void reload(StringId64 type, StringId64 name);
	// Returns whether the manager has the resource (<type>, <name>)
	bool canGet(StringId64 type, StringId64 name);
	// Returns the data of the resource (<type>, <name>)
	const void* get(StringId64 type, StringId64 name);
	// Sets whether resources should be automatically loaded when accessed
	void enableAutoload(bool enable);
	// Blocks until all load() requests have been completed
	void flush();
	// Completes all load() requests which have been loaded by ResourceLoader
	void completeRequests();
	// Registers a new resource <type> into the resource manager
	void registerType(StringId64 type, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline);
private:
	void onOnline(StringId64 type, StringId64 name);
	void onOffline(StringId64 type, StringId64 name);
	void onUnload(StringId64 type, void* data);
	void completeRequest(StringId64 type, StringId64 name, void* data);

	ProxyAllocator resourceHeap;
	ResourceLoader* resourceLoader;
	ResourceTypeDataMap resourceTypeDataMap;
	ResourceMap resourceMap;
	bool autoloadEnabled = false;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka