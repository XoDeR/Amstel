// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/ResourceManager.h"
#include "Core/Containers/Array.h"
#include "Core/Strings/DynamicString.h"
#include "Core/Containers/SortMap.h"
#include "Core/Memory/TempAllocator.h"
#include "Resource/ResourceLoader.h"

namespace Rio
{

const ResourceManager::ResourceEntry ResourceManager::ResourceEntry::NOT_FOUND = { 0xffffffffu, NULL };

ResourceManager::ResourceManager(ResourceLoader& rl)
	: resourceHeap(getDefaultAllocator(), "resource")
	, resourceLoader(&rl)
	, resourceTypeDataMap(getDefaultAllocator())
	, resourceMap(getDefaultAllocator())
{
}

ResourceManager::~ResourceManager()
{
	const ResourceMap::Entry* begin = SortMapFn::begin(resourceMap);
	const ResourceMap::Entry* end = SortMapFn::end(resourceMap);

	for (; begin != end; begin++)
	{
		const StringId64 type = begin->pair.first.type;
		const StringId64 name = begin->pair.first.name;
		onOffline(type, name);
		onUnload(type, begin->pair.second.data);
	}
}

void ResourceManager::load(StringId64 type, StringId64 name)
{
	ResourcePair id = { type, name };
	ResourceEntry& entry = SortMapFn::get(resourceMap, id, ResourceEntry::NOT_FOUND);

	if (entry == ResourceEntry::NOT_FOUND)
	{
		TempAllocator64 ta;
		DynamicString resourceTypeStr(ta);
		DynamicString resourceNameStr(ta);
		type.toString(resourceTypeStr);
		name.toString(resourceNameStr);

		RIO_ASSERT(resourceLoader->canLoad(type, name)
			, "Can't load resource #ID(%s-%s)"
			, resourceTypeStr.getCStr()
			, resourceNameStr.getCStr()
			);
		RIO_UNUSED(resourceTypeStr);
		RIO_UNUSED(resourceNameStr);

		ResourceRequest resourceRequest;
		resourceRequest.type = type;
		resourceRequest.name = name;
		resourceRequest.load_function = SortMapFn::get(resourceTypeDataMap, type, ResourceTypeData()).load;
		resourceRequest.allocator = &resourceHeap;
		resourceRequest.data = nullptr;

		resourceLoader->addRequest(resourceRequest);
		return;
	}

	entry.references++;
}

void ResourceManager::unload(StringId64 type, StringId64 name)
{
	flush();

	ResourcePair id = { type, name };
	ResourceEntry& entry = SortMapFn::get(resourceMap, id, ResourceEntry::NOT_FOUND);

	if (--entry.references == 0)
	{
		onOffline(type, name);
		onUnload(type, entry.data);

		SortMapFn::remove(resourceMap, id);
		SortMapFn::sort(resourceMap);
	}
}

void ResourceManager::reload(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	const ResourceEntry& entry = SortMapFn::get(resourceMap, id, ResourceEntry::NOT_FOUND);
	const uint32_t oldReferences = entry.references;

	unload(type, name);
	load(type, name);
	flush();

	ResourceEntry& newResourceEntry = SortMapFn::get(resourceMap, id, ResourceEntry::NOT_FOUND);
	newResourceEntry.references = oldReferences;
}

bool ResourceManager::canGet(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	return autoloadEnabled ? true : SortMapFn::has(resourceMap, id);
}

const void* ResourceManager::get(StringId64 type, StringId64 name)
{
	const ResourcePair id = { type, name };
	TempAllocator128 ta;
	DynamicString resourceTypeStr(ta);
	DynamicString resourceNameStr(ta);
	type.toString(resourceTypeStr);
	name.toString(resourceNameStr);

	RIO_ASSERT(canGet(type, name)
		, "Resource not loaded #ID(%s-%s)"
		, resourceTypeStr.getCStr()
		, resourceNameStr.getCStr()
		);
	RIO_UNUSED(resourceTypeStr);
	RIO_UNUSED(resourceNameStr);

	if (autoloadEnabled && !SortMapFn::has(resourceMap, id))
	{
		load(type, name);
		flush();
	}

	const ResourceEntry& entry = SortMapFn::get(resourceMap, id, ResourceEntry::NOT_FOUND);
	return entry.data;
}

void ResourceManager::enableAutoload(bool autoloadEnabled)
{
	this->autoloadEnabled = autoloadEnabled;
}

void ResourceManager::flush()
{
	resourceLoader->flush();
	completeRequests();
}

void ResourceManager::completeRequests()
{
	TempAllocator1024 ta;
	Array<ResourceRequest> loaded(ta);
	resourceLoader->getLoaded(loaded);

	for (uint32_t i = 0; i < ArrayFn::getCount(loaded); ++i)
	{
		completeRequest(loaded[i].type, loaded[i].name, loaded[i].data);
	}
}

void ResourceManager::completeRequest(StringId64 type, StringId64 name, void* data)
{
	ResourceEntry entry;
	entry.references = 1;
	entry.data = data;

	ResourcePair id = { type, name };

	SortMapFn::set(resourceMap, id, entry);
	SortMapFn::sort(resourceMap);

	onOnline(type, name);
}

void ResourceManager::registerType(StringId64 type, LoadFunction load, UnloadFunction unload, OnlineFunction online, OfflineFunction offline)
{
	RIO_ASSERT_NOT_NULL(load);
	RIO_ASSERT_NOT_NULL(unload);

	ResourceTypeData resourceTypeData;
	resourceTypeData.load = load;
	resourceTypeData.online = online;
	resourceTypeData.offline = offline;
	resourceTypeData.unload = unload;

	SortMapFn::set(resourceTypeDataMap, type, resourceTypeData);
	SortMapFn::sort(resourceTypeDataMap);
}

void ResourceManager::onOnline(StringId64 type, StringId64 name)
{
	OnlineFunction onlineFunction = SortMapFn::get(resourceTypeDataMap, type, ResourceTypeData()).online;
	if (onlineFunction)
	{
		onlineFunction(name, *this);
	}
}

void ResourceManager::onOffline(StringId64 type, StringId64 name)
{
	OfflineFunction offlineFunction = SortMapFn::get(resourceTypeDataMap, type, ResourceTypeData()).offline;
	if (offlineFunction)
	{
		offlineFunction(name, *this);
	}
}

void ResourceManager::onUnload(StringId64 type, void* data)
{
	SortMapFn::get(resourceTypeDataMap, type, ResourceTypeData()).unload(resourceHeap, data);
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka