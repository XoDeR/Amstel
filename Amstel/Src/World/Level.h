// Copyright (c) 2016 Volodymyr Syvochka
#pragma once

#include "Core/Containers/ContainerTypes.h"
#include "Core/Math/MathTypes.h"
#include "Core/Memory/MemoryTypes.h"
#include "Resource/ResourceTypes.h"
#include "World/WorldTypes.h"

namespace Rio
{

class Level
{
public:
	Level(Allocator& a, UnitManager& unitManager, World& world, const LevelResource& levelResource);
	~Level();
	void load(const Vector3& position, const Quaternion& rotation);

	static const uint32_t MARKER = 0x1f2b43fe;
private:
	uint32_t marker;

	Allocator* allocator;
	UnitManager* unitManager;
	World* world;
	const LevelResource* levelResource;
	Array<UnitId> unitLookupList;
};

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka