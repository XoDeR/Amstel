// Copyright (c) 2016 Volodymyr Syvochka
#include "Resource/ResourcePackage.h"
#include "Core/Containers/Array.h"
#include "Resource/ResourceManager.h"
#include "Resource/PackageResource.h"
#include "World/WorldTypes.h"

namespace Rio
{

ResourcePackage::ResourcePackage(StringId64 id, ResourceManager& resourceManager)
	: marker(RESOURCE_PACKAGE_MARKER)
	, resourceManager(&resourceManager)
	, id(id)
{
	resourceManager.load(RESOURCE_TYPE_PACKAGE, id);
	resourceManager.flush();
	packageResource = (const PackageResource*)resourceManager.get(RESOURCE_TYPE_PACKAGE, id);
}

ResourcePackage::~ResourcePackage()
{
	resourceManager->unload(RESOURCE_TYPE_PACKAGE, id);
	marker = 0;
}

void ResourcePackage::load()
{
	for (uint32_t i = 0; i < ArrayFn::getCount(packageResource->resources); ++i)
	{
		resourceManager->load(packageResource->resources[i].type, packageResource->resources[i].name);
	}
}

void ResourcePackage::unload()
{
	for (uint32_t i = 0; i < ArrayFn::getCount(packageResource->resources); ++i)
	{
		resourceManager->unload(packageResource->resources[i].type, packageResource->resources[i].name);
	}
}

void ResourcePackage::flush()
{
	resourceManager->flush();
}

bool ResourcePackage::hasLoaded() const
{
	for (uint32_t i = 0; i < ArrayFn::getCount(packageResource->resources); ++i)
	{
		if (!resourceManager->canGet(packageResource->resources[i].type, packageResource->resources[i].name))
		{
			return false;
		}
	}

	return true;
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka