// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/ResourceLoader.h"
#include "Config.h"
#include "Core/Strings/DynamicString.h"
#include "Core/FileSystem/FileSystem.h"
#include "Core/Memory/Memory.h"
#include "Core/FileSystem/Path.h"
#include "Core/Containers/Queue.h"
#include "Core/Memory/TempAllocator.h"

namespace Rio
{

ResourceLoader::ResourceLoader(FileSystem& fileSystem)
	: fileSystem(fileSystem)
	, resourceRequestList(getDefaultAllocator())
	, loadedResourceRequestList(getDefaultAllocator())
{
	resourceLoaderThread.start(ResourceLoader::threadProcedure, this);
}

ResourceLoader::~ResourceLoader()
{
	exitRequested = true;
	resourceLoaderThread.stop();
}

bool ResourceLoader::canLoad(StringId64 type, StringId64 name)
{
	TempAllocator128 ta;
	DynamicString resourceTypeStr(ta);
	DynamicString resourceNameStr(ta);
	type.toString(resourceTypeStr);
	name.toString(resourceNameStr);

	DynamicString resoursePath(ta);
	resoursePath += resourceTypeStr;
	resoursePath += '-';
	resoursePath += resourceNameStr;

	DynamicString path(ta);
	PathFn::join(RIO_DATA_DIRECTORY, resoursePath.getCStr(), path);

	return fileSystem.getDoesExist(path.getCStr());
}

void ResourceLoader::addRequest(const ResourceRequest& rr)
{
	ScopedMutex scopedMutex(mutex);
	QueueFn::pushBack(resourceRequestList, rr);
}

void ResourceLoader::flush()
{
	while (getRequestsCount() != 0) 
	{
	}
}

uint32_t ResourceLoader::getRequestsCount()
{
	ScopedMutex scopedMutex(mutex);
	return QueueFn::getCount(resourceRequestList);
}

void ResourceLoader::addLoaded(ResourceRequest rr)
{
	ScopedMutex scopedMutex(loadedMutex);
	QueueFn::pushBack(loadedResourceRequestList, rr);
}

void ResourceLoader::getLoaded(Array<ResourceRequest>& loaded)
{
	ScopedMutex scopedMutex(loadedMutex);

	const uint32_t resourcesCount = QueueFn::getCount(loadedResourceRequestList);
	ArrayFn::reserve(loaded, resourcesCount);

	for (uint32_t i = 0; i < resourcesCount; ++i)
	{
		ArrayFn::pushBack(loaded, QueueFn::front(loadedResourceRequestList));
		QueueFn::popFront(loadedResourceRequestList);
	}
}

int32_t ResourceLoader::run()
{
	while (exitRequested == false)
	{
		mutex.lock();
		if (QueueFn::getIsEmpty(resourceRequestList))
		{
			mutex.unlock();
			continue;
		}
		ResourceRequest resourceRequest = QueueFn::front(resourceRequestList);
		mutex.unlock();

		TempAllocator128 ta;
		DynamicString resourceTypeStr(ta);
		DynamicString resourceNameStr(ta);
		resourceRequest.type.toString(resourceTypeStr);
		resourceRequest.name.toString(resourceNameStr);

		DynamicString resoursePath(ta);
		resoursePath += resourceTypeStr;
		resoursePath += '-';
		resoursePath += resourceNameStr;

		DynamicString path(ta);
		PathFn::join(RIO_DATA_DIRECTORY, resoursePath.getCStr(), path);

		File* file = fileSystem.open(path.getCStr(), FileOpenMode::READ);
		resourceRequest.data = resourceRequest.loadFunction(*file, *resourceRequest.allocator);
		fileSystem.close(*file);

		addLoaded(resourceRequest);
		mutex.lock();
		QueueFn::popFront(resourceRequestList);
		mutex.unlock();
	}

	return 0;
}

int32_t ResourceLoader::threadProcedure(void* thiz)
{
	return ((ResourceLoader*)thiz)->run();
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka