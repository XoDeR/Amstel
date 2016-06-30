// Copyright (c) 2016 Volodymyr Syvochka
#include "World/Level.h"
#include "Resource/LevelResource.h"
#include "Resource/UnitResource.h"
#include "World/World.h"
#include "World/UnitManager.h"

namespace Rio
{

Level::Level(Allocator& a, UnitManager& unitManager, World& world, const LevelResource& levelResource)
	: marker(MARKER)
	, allocator(&a)
	, unitManager(&unitManager)
	, world(&world)
	, levelResource(&levelResource)
	, unitLookupList(a)
{
}

Level::~Level()
{
	marker = 0;
}

void Level::load(const Vector3& position, const Quaternion& rotation)
{
	// Spawn units
	const UnitResource* unitResource = LevelResourceFn::getUnitResource(levelResource);
	const uint32_t unitListCount = unitResource->unitListCount;

	ArrayFn::resize(unitLookupList, unitListCount);

	for (uint32_t i = 0; i < unitListCount; ++i)
	{
		unitLookupList[i] = unitManager->create();
	}

	spawnUnits(*world, *unitResource, position, rotation, ArrayFn::begin(unitLookupList));

	// Post events
	for (uint32_t i = 0; i < unitListCount; ++i)
	{
		world->postUnitSpawnedEvent(unitLookupList[i]);
	}

	// Play sounds
	const uint32_t soundCount = LevelResourceFn::getSoundCount(levelResource);
	for (uint32_t i = 0; i < soundCount; ++i)
	{
		const LevelSound* levelSound = LevelResourceFn::getSound(levelResource, i);
		world->playSound(levelSound->name
			, levelSound->loop
			, levelSound->volume
			, levelSound->position
			, levelSound->range
			);
	}
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka