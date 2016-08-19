// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Thread/Thread.h"
#include "Core/Containers/ContainerTypes.h"
#include "Core/Thread/Mutex.h"
#include "Core/Strings/StringId.h"

namespace Rio
{

struct ResourceRequest
{
	using LoadFunction = void* (*)(File& file, Allocator& a);

	StringId64 type;
	StringId64 name;
	LoadFunction loadFunction;
	Allocator* allocator;
	void* data;
};

// Loads resources in a background thread
class ResourceLoader
{
public:
	ResourceLoader(FileSystem& fs);
	~ResourceLoader();
	// Returns whether the resource (type, name) can be loaded
	bool canLoad(StringId64 type, StringId64 name);
	// Adds a request for loading the resource described by <resourceRequest>
	void addRequest(const ResourceRequest& resourceRequest);
	// Blocks until all pending requests have been processed
	void flush();
	// Returns all the resources that have been loaded
	void getLoaded(Array<ResourceRequest>& loaded);
private:
	uint32_t getRequestsCount();
	void addLoaded(ResourceRequest resourceRequest);
	int32_t run();
	static int32_t threadProcedure(void* thiz);

	FileSystem& fileSystem;

	Queue<ResourceRequest> resourceRequestList;
	Queue<ResourceRequest> loadedResourceRequestList;

	Thread resourceLoaderThread;
	Mutex mutex;
	Mutex loadedMutex;
	bool exitRequested = false;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka