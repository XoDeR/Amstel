// Copyright (c) 2016 Volodymyr Syvochka
#include "World/UnitManager.h"
#include "Core/Containers/Array.h"
#include "Core/Containers/Queue.h"
#include "World/World.h"

#define MINIMUM_FREE_INDICES 1024

namespace Rio
{

UnitManager::UnitManager(Allocator& a)
	: generation(a)
	, freeIndicesQueue(a)
	, destroyCallbackList(a)
{
}

UnitId UnitManager::makeUnit(uint32_t index, uint8_t generationIndex)
{
	UnitId id = { 0 | index | uint32_t(generationIndex) << UNIT_INDEX_BITS };
	return id;
}

UnitId UnitManager::create()
{
	uint32_t index;

	if (QueueFn::getCount(freeIndicesQueue) > MINIMUM_FREE_INDICES)
	{
		index = QueueFn::front(freeIndicesQueue);
		QueueFn::popFront(freeIndicesQueue);
	}
	else
	{
		ArrayFn::pushBack(generation, uint8_t(0));
		index = ArrayFn::getCount(generation) - 1;
		RIO_ASSERT(index < (1 << UNIT_INDEX_BITS), "Index out of bounds");
	}

	return makeUnit(index, generation[index]);
}

UnitId UnitManager::create(World& world)
{
	return world.spawnEmptyUnit();
}

bool UnitManager::getIsAlive(UnitId id) const
{
	return generation[id.getIndex()] == id.getId();
}

void UnitManager::destroy(UnitId id)
{
	const uint32_t index = id.getIndex();
	++generation[index];
	QueueFn::pushBack(freeIndicesQueue, index);

	triggerDestroyCallbacks(id);
}

void UnitManager::registerDestroyFunction(DestroyFunction destroyFunction, void* userPtr)
{
	DestroyData destroyData;
	destroyData.destroyFunction = destroyFunction;
	destroyData.userPtr = userPtr;
	ArrayFn::pushBack(destroyCallbackList, destroyData);
}

void UnitManager::unregisterDestroyFunction(void* userPtr)
{
	for (uint32_t i = 0, n = ArrayFn::getCount(destroyCallbackList); i < n; ++i)
	{
		if (destroyCallbackList[i].userPtr == userPtr)
		{
			destroyCallbackList[i] = destroyCallbackList[n - 1];
			ArrayFn::popBack(destroyCallbackList);
			return;
		}
	}

	RIO_ASSERT(false, "Bad destroy function");
}

void UnitManager::triggerDestroyCallbacks(UnitId id)
{
	for (uint32_t i = 0; i < ArrayFn::getCount(destroyCallbackList); ++i)
	{
		destroyCallbackList[i].destroyFunction(id, destroyCallbackList[i].userPtr);
	}
}

} // namespace Rio
// Copyright (c) 2016 Volodymyr Syvochka