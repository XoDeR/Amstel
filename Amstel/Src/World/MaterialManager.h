// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/FileSystem/FileSystemTypes.h"
#include "Core/Strings/StringId.h"
#include "Core/Base/Types.h"
#include "Resource/ResourceTypes.h"
#include "World/Material.h"

namespace Rio
{

class MaterialManager
{
public:
	MaterialManager(Allocator& a, ResourceManager& resourceManager);
	~MaterialManager();
	void* load(File& file, Allocator& a);
	void online(StringId64 id, ResourceManager& resourceManager);
	void offline(StringId64 id, ResourceManager& resourceManager);
	void unload(Allocator& a, void* resource);
	void createMaterial(StringId64 id);
	void destroyMaterial(StringId64 id);
	Material* get(StringId64 id);
private:
	Allocator* allocator;
	ResourceManager* resourceManager;
	SortMap<StringId64, Material*> materialsMap;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka