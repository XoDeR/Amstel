// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Base/Types.h"
#include "Core/Strings/StringId.h"
#include "Resource/ResourceTypes.h"

namespace Rio
{

// Collection of resources to load in a batch
struct ResourcePackage
{
private:
	uint32_t marker = 0;
public:
	ResourcePackage(StringId64 id, ResourceManager& resman);
	~ResourcePackage();
	// Loads all the resources in the package
	// The resources are not immediately available after the call is made,
	// instead, you have to poll for completion with hasLoaded()
	void load();
	// Unloads all the resources in the package
	void unload();
	// Waits until the package has been loaded
	void flush();
	// Returns whether the package has been loaded
	bool hasLoaded() const;
private:
	ResourceManager* resourceManager;
	StringId64 id;
	const PackageResource* packageResource = nullptr;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka